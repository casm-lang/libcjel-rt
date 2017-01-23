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

TEST( libcsel_rt__asmjit, compiler2 )
{
    JitRuntime rt;
    CodeHolder code;
    code.init( rt.getCodeInfo() );

    X86Compiler cc( &code );

    FuncSignatureX fsx;
    fsx.setCallConv( CallConv::kIdHost );
    fsx.addArg( TypeId::kUIntPtr );
    fsx.addArg( TypeId::kUIntPtr );
    fsx.addArg( TypeId::kUIntPtr );
    fsx.addArg( TypeId::kUIntPtr );
    cc.addFunc( fsx );

    // in
    X86Gp ivp = cc.newUIntPtr( "ivp" );
    cc.setArg( 0, ivp );
    X86Gp idp = cc.newUIntPtr( "idp" );
    cc.setArg( 1, idp );

    // out
    X86Gp bvp = cc.newUIntPtr( "bvp" );
    cc.setArg( 2, bvp );
    X86Gp bdp = cc.newUIntPtr( "bdp" );
    cc.setArg( 3, bdp );

    X86Gp iv = cc.newU64( "iv" );
    X86Gp id = cc.newU8( "id" );

    cc.mov( iv, x86::ptr64( ivp ) );
    cc.mov( id, x86::ptr8( idp ) );

    cc.and_( iv, imm( 0x1 ) );

    cc.mov( x86::ptr8( bvp ), iv.r8() );
    cc.mov( x86::ptr8( bdp ), id );

    cc.endFunc();
    cc.finalize();

    void** fn;
    Error err = rt.add( &fn, &code );

    ASSERT_EQ( err, 0 );

    u64 ivc;
    u8 idc;
    u8 bvc;
    u8 bdc;

    typedef void ( *FP )( u64*, u8*, u8*, u8* );

    ivc = 25;
    idc = 1;
    bvc = -1;
    bdc = -1;

    ( (FP)fn )( &ivc, &idc, &bvc, &bdc );

    EXPECT_EQ( (int)ivc, 25 );
    EXPECT_EQ( (int)idc, 1 );
    EXPECT_EQ( (int)bvc, 1 );
    EXPECT_EQ( (int)bdc, 1 );

    ivc = 123;
    idc = 0;
    bvc = -1;
    bdc = -1;

    ( (FP)fn )( &ivc, &idc, &bvc, &bdc );

    EXPECT_EQ( (int)ivc, 123 );
    EXPECT_EQ( (int)idc, 0 );
    EXPECT_EQ( (int)bvc, 1 );
    EXPECT_EQ( (int)bdc, 0 );

    rt.release( fn );
}

TEST( libcsel_rt__asmjit, compiler3 )
{
    JitRuntime rt;
    CodeHolder code;
    code.init( rt.getCodeInfo() );

    X86Compiler cc( &code );

    FuncSignatureX fsx;
    fsx.setCallConv( CallConv::kIdHost );
    fsx.addArg( TypeId::kUIntPtr );
    fsx.addArg( TypeId::kUIntPtr );
    cc.addFunc( fsx );

    // in
    X86Gp ip = cc.newUIntPtr( "ip" );
    cc.setArg( 0, ip );

    // out
    X86Gp bp = cc.newUIntPtr( "bp" );
    cc.setArg( 1, bp );

    X86Gp iv = cc.newU64( "iv" );
    X86Gp id = cc.newU8( "id" );

    cc.mov( iv, x86::ptr64( ip, 0 ) );
    cc.mov( id, x86::ptr8( ip, 8 ) );

    cc.add( iv, 1 );
    cc.sub( id, 1 );

    cc.mov( x86::ptr8( bp, 0 ), iv.r8() );
    cc.mov( x86::ptr8( bp, 1 ), id );

    cc.endFunc();
    cc.finalize();

    void** fn;
    Error err = rt.add( &fn, &code );

    ASSERT_EQ( err, 0 );

    typedef void ( *FP )( void*, void* );

    struct I
    {
        u64 v;
        u8 d;
    };

    struct B
    {
        u8 v;
        u8 d;
    };

    {
        I i{ 44, 22 };
        B b{ 1, 0 };

        ( (FP)fn )( &i, &b );

        EXPECT_EQ( (int)i.v, 44 );
        EXPECT_EQ( (int)i.d, 22 );
        EXPECT_EQ( (int)b.v, 45 );
        EXPECT_EQ( (int)b.d, 21 );
    }

    {
        I i{ 25, 1 };
        B b{ 255, 255 };

        ( (FP)fn )( &i, &b );

        EXPECT_EQ( (int)i.v, 25 );
        EXPECT_EQ( (int)i.d, 1 );
        EXPECT_EQ( (int)b.v, 26 );
        EXPECT_EQ( (int)b.d, 0 );
    }

    rt.release( fn );
}

TEST( libcsel_rt__asmjit, compiler4 )
{
    JitRuntime rt;
    CodeHolder code;
    code.init( rt.getCodeInfo() );

    X86Compiler cc( &code );

    FuncSignatureX fsx;
    fsx.setCallConv( CallConv::kIdHost );
    fsx.addArg( TypeId::kUIntPtr );
    fsx.addArg( TypeId::kUIntPtr );
    cc.addFunc( fsx );

    // in
    X86Gp ip = cc.newUIntPtr( "ip" );
    cc.setArg( 0, ip );

    // out
    X86Gp bp = cc.newUIntPtr( "bp" );
    cc.setArg( 1, bp );

    X86Gp iv = cc.newU64( "iv" );
    X86Gp id = cc.newU8( "id" );

    cc.mov( id, x86::ptr( ip, 0 ) );
    cc.mov( iv, x86::ptr( ip, 8 ) );

    cc.add( id, 1 );
    cc.sub( iv, 1 );

    cc.mov( x86::ptr( bp, 0 ), id );
    cc.mov( x86::ptr( bp, 1 ), iv.r8() );

    cc.endFunc();
    cc.finalize();

    void** fn;
    Error err = rt.add( &fn, &code );

    ASSERT_EQ( err, 0 );

    typedef void ( *FP )( void*, void* );

    struct I
    {
        u64 v;
        u8 d;
    };

    struct B
    {
        u8 v;
        u8 d;
    };

    {
        I i{ 44, 22 };
        B b{ 1, 0 };

        ( (FP)fn )( &i, &b );

        EXPECT_EQ( (int)i.v, 44 );
        EXPECT_EQ( (int)i.d, 22 );
        EXPECT_EQ( (int)b.v, 45 );
        EXPECT_EQ( (int)b.d, 21 );
    }

    {
        I i{ 25, 1 };
        B b{ 255, 255 };

        ( (FP)fn )( &i, &b );

        EXPECT_EQ( (int)i.v, 25 );
        EXPECT_EQ( (int)i.d, 1 );
        EXPECT_EQ( (int)b.v, 26 );
        EXPECT_EQ( (int)b.d, 0 );
    }

    rt.release( fn );
}

TEST( libcsel_rt__asmjit, compiler5 )
{
    JitRuntime rt;

    FuncSignatureX callee_fs;
    void** callee = nullptr;

    FuncSignatureX caller_fs;
    void** caller = nullptr;
    {
        // callee context!
        CodeHolder code;
        code.init( rt.getCodeInfo() );
        X86Compiler cc( &code );

        callee_fs.setCallConv( CallConv::kIdHost );
        callee_fs.addArg( TypeId::kUIntPtr );
        callee_fs.addArg( TypeId::kUIntPtr );
        cc.addFunc( callee_fs );

        // in
        X86Gp ip = cc.newUIntPtr( "ip" );
        cc.setArg( 0, ip );

        // out
        X86Gp bp = cc.newUIntPtr( "bp" );
        cc.setArg( 1, bp );

        X86Gp iv = cc.newU64( "iv" );
        X86Gp id = cc.newU8( "id" );

        cc.mov( id, x86::ptr( ip, 0 ) );
        cc.mov( iv, x86::ptr( ip, 8 ) );

        cc.add( id, 1 );
        cc.sub( iv, 1 );

        cc.mov( x86::ptr( bp, 0 ), id );
        cc.mov( x86::ptr( bp, 1 ), iv.r8() );

        cc.endFunc();
        cc.finalize();

        // typedef void ( *callee_fp )( void*, void* );

        Error err = rt.add( &callee, &code );
        ASSERT_EQ( err, 0 );
    }

    {
        // caller context!
        CodeHolder code;
        code.init( rt.getCodeInfo() );
        X86Compiler cc( &code );

        caller_fs.setCallConv( CallConv::kIdHost );
        caller_fs.addArg( TypeId::kUIntPtr );
        caller_fs.addArg( TypeId::kUIntPtr );
        cc.addFunc( caller_fs );

        // out
        X86Gp ip = cc.newUIntPtr( "ip" );
        cc.setArg( 0, ip );
        X86Gp bp = cc.newUIntPtr( "bp" );
        cc.setArg( 1, bp );

        // internal mem
        X86Mem a = cc.newStack( 9, 4 ); // { u64, u8 }
        X86Gp ap = cc.newUIntPtr( "a" );
        cc.lea( ap, a );

        X86Gp tmp = cc.newU8( "tmp" );
        cc.mov( tmp, x86::ptr( ip, 0 ) );
        cc.mov( x86::ptr( ap, 0 ), tmp );
        cc.mov( tmp, x86::ptr( ip, 8 ) );
        cc.mov( x86::ptr( ap, 8 ), tmp );

        X86Mem r = cc.newStack( 2, 4 ); // { u8, u8 }
        X86Gp rp = cc.newUIntPtr( "r" );
        cc.lea( rp, r );

        // perform call!
        X86Gp fp = cc.newIntPtr( "fp" );
        cc.mov( fp, imm_ptr( callee ) );
        CCFuncCall* call = cc.call( fp, callee_fs );
        call->setArg( 0, ap );
        call->setArg( 1, rp );

        // write to outputs for debug and testing!!!
        cc.mov( tmp, x86::ptr( ap, 0 ) );
        cc.mov( x86::ptr( ip, 0 ), tmp );
        cc.mov( tmp, x86::ptr( ap, 8 ) );
        cc.mov( x86::ptr( ip, 8 ), tmp );

        cc.mov( tmp, x86::ptr( rp, 0 ) );
        cc.mov( x86::ptr( bp, 0 ), tmp );
        cc.mov( tmp, x86::ptr( rp, 1 ) );
        cc.mov( x86::ptr( bp, 1 ), tmp );

        cc.endFunc();
        cc.finalize();

        // typedef void ( *caller_fp )( void );
        typedef void ( *caller_fp )( void*, void* );

        void** caller;
        Error err = rt.add( &caller, &code );
        ASSERT_EQ( err, 0 );

        struct I
        {
            u64 v;
            u8 d;
        };

        struct B
        {
            u8 v;
            u8 d;
        };

        I sa = { 11, 22 };
        B sr = { 255, 255 };

        ( (caller_fp)caller )( &sa, &sr );

        EXPECT_EQ( (int)sa.v, 11 );
        EXPECT_EQ( (int)sa.d, 22 );
        EXPECT_EQ( (int)sr.v, 12 );
        EXPECT_EQ( (int)sr.d, 21 );
    }

    rt.release( callee );
    rt.release( caller );
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
