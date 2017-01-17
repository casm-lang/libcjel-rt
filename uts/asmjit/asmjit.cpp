//
//  Copyright (c) 2017 CASM Organization
//  All rights reserved.
//
//  Developed by: Philipp Paulweber
//                https://github.com/casm-lang/libcsel-rt
//
//  This file is part of libcsel-rt.
//
//  libcsel-rt is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  libcsel-rt is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with libcsel-rt. If not, see <http://www.gnu.org/licenses/>.
//

#include "gtest/gtest.h"

#include "libcsel-rt.h"

#include "asmjit/asmjit.h"

using namespace asmjit;

TEST( libcsel_rt__asmjit, assembler )
{
    typedef void ( *SumIntsFunc )( int* dst, const int* a, const int* b );

    JitRuntime rt;

    CodeHolder code;
    code.init( rt.getCodeInfo() );
    X86Assembler a( &code );

    X86Gp dst = a.zax();
    X86Gp src_a = a.zcx();
    X86Gp src_b = a.zdx();

    X86Xmm vec0 = x86::xmm0;
    X86Xmm vec1 = x86::xmm1;

    FuncDetail func;
    func.init( FuncSignature3< void, int*, const int*, const int* >(
        CallConv::kIdHost ) );

    FuncFrameInfo ffi;
    ffi.setDirtyRegs( X86Reg::kKindVec, Utils::mask( 0, 1 ) );

    FuncArgsMapper args( &func );
    args.assignAll( dst, src_a, src_b );
    args.updateFrameInfo( ffi );

    FuncFrameLayout layout;
    layout.init( func, ffi );

    FuncUtils::emitProlog( &a, layout );
    FuncUtils::allocArgs( &a, layout, args );
    a.movdqu( vec0, x86::ptr( src_a ) );
    a.movdqu( vec1, x86::ptr( src_b ) );
    a.paddd( vec0, vec1 );
    a.movdqu( x86::ptr( dst ), vec0 );
    FuncUtils::emitEpilog( &a, layout );

    SumIntsFunc fn;
    Error err = rt.add( &fn, &code );

    ASSERT_EQ( err, 0 );

    int inA[ 4 ] = { 4, 3, 2, 1 };
    int inB[ 4 ] = { 1, 5, 2, 8 };
    int out[ 4 ];
    fn( out, inA, inB );

    ASSERT_EQ( out[ 0 ], 5 );
    ASSERT_EQ( out[ 1 ], 8 );
    ASSERT_EQ( out[ 2 ], 4 );
    ASSERT_EQ( out[ 3 ], 9 );

    rt.release( fn );
}

TEST( libcsel_rt__asmjit, compiler )
{
    typedef uint8_t ( *Func )( void );

    JitRuntime rt;
    CodeHolder code;
    code.init( rt.getCodeInfo() );

    X86Compiler cc( &code );

    // c.addFunc(kFuncConvHost, FuncBuilder0<double>());
    cc.addFunc( FuncSignature0< u8 >( CallConv::kIdHost ) );

    X86Gp r0 = cc.newU8( "r0" );

    cc.mov( r0, 1 );
    cc.add( r0, r0 );
    cc.mul( r0, r0, r0 );

    cc.ret( r0 );
    cc.endFunc();

    cc.finalize();

    Func fn;
    Error err = rt.add( &fn, &code );

    ASSERT_EQ( err, 0 );

    u8 result = fn();

    ASSERT_EQ( (int)result, 4 );

    rt.release( fn );
}

//
//  Local variables:
//  mode: c++
//  indent-tabs-mode: nil
//  c-basic-offset: 4
//  tab-width: 4
//  End:
//  vim:noexpandtab:sw=4:ts=4:
//
