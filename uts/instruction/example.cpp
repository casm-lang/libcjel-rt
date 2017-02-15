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

#include "uts/main.h"

using namespace libcsel_ir;

TEST( libcsel_rt__instruction, and)
{
    auto a = BitConstant( 8, 0x18 );
    auto b = BitConstant( 8, 0xff );

    auto i = AndInstruction( &a, &b );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( *r == BitConstant( 8, 0x18 ) );
}

TEST( libcsel_rt__instruction, AddUnsignedInstruction )
{
    auto t = Type::Bit( 8 );

    auto a = BitConstant( t, 0x11 );
    auto b = BitConstant( t, 0x22 );

    auto i = AddUnsignedInstruction( &a, &b );
    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( *r == BitConstant( t, 0x33 ) );
}

TEST( libcsel_rt__instruction_example, simple_move_test )
{
    auto b_t = Type::Bit( 8 );
    auto s_t = Type::Structure( { { b_t, "v" }, { b_t, "w" } } );

    auto a = StructureConstant(
        *s_t, { libstdhl::make< BitConstant >( b_t, 0x12 ),
                  libstdhl::make< BitConstant >( b_t, 0x34 ) } );

    auto x0 = BitConstant( b_t, 0 );
    auto x1 = BitConstant( b_t, 1 );

    auto f_t = Type::Relation( { s_t }, { s_t } );

    auto f = Intrinsic( "sym", f_t ); // operation res.v := arg.v; res.w = arg.w
    auto f_i = f.in( "arg", s_t );
    auto f_o = f.out( "res", s_t );

    auto scope = ParallelScope( &f );
    auto stmt = TrivialStatement( &scope );

    auto v_ptr = ExtractInstruction( f_i, &x0 );
    auto v_ld = LoadInstruction( &v_ptr );
    stmt.add( &v_ld );

    auto w_ptr = ExtractInstruction( f_i, &x1 );
    auto w_ld = LoadInstruction( &w_ptr );
    stmt.add( &w_ld );

    auto res_v_ptr = ExtractInstruction( f_o, &x0 );
    auto res_w_ptr = ExtractInstruction( f_o, &x1 );

    auto res_v_st = StoreInstruction( &v_ld, &res_v_ptr );
    auto res_w_st = StoreInstruction( &w_ld, &res_w_ptr );

    stmt.add( &res_v_st );
    stmt.add( &res_w_st );

    auto m = AllocInstruction( s_t );
    auto i = CallInstruction( &f, { &a, &m } );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == a );
}

TEST( libcsel_rt__instruction_example, TODO_NAME )
{
    auto b_t = Type::Bit( 8 );
    auto s_t = Type::Structure( { { b_t, "v" }, { b_t, "w" } } );

    auto a = StructureConstant(
        *s_t, { libstdhl::make< BitConstant >( b_t, 0x04 ),
                  libstdhl::make< BitConstant >( b_t, 0x08 ) } );

    auto x0 = BitConstant( b_t, 0 );
    auto x1 = BitConstant( b_t, 1 );

    auto f_t = Type::Relation( { b_t }, { s_t } );

    auto f = Intrinsic( "sym", f_t ); // operation res := arg.v + arg.w + 0xa0
    auto f_i = f.in( "arg", s_t );
    auto f_o = f.out( "res", b_t );
    auto scope = ParallelScope( &f );
    auto stmt = TrivialStatement( &scope );

    auto v_ptr = ExtractInstruction( f_i, &x0 );
    auto v_ld = LoadInstruction( &v_ptr );
    auto v = stmt.add( &v_ld );

    auto w_ptr = ExtractInstruction( f_i, &x1 );
    auto w_ld = LoadInstruction( &w_ptr );
    auto w = stmt.add( &w_ld );

    auto r0 = stmt.add( new AddUnsignedInstruction( v, w ) );

    auto c0 = BitConstant( b_t, 0xa0 );
    auto r1 = stmt.add( new AddUnsignedInstruction( r0, &c0 ) );

    stmt.add( new StoreInstruction( r1, f_o ) );

    auto m = AllocInstruction( b_t );
    auto i = CallInstruction( &f, { &a, &m } );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == BitConstant( b_t, 0x04 + 0x08 + 0xa0 ) );
}

TEST( libcsel_rt__instruction_example, lala )
{
    auto b_t = Type::Bit( 8 );
    auto s_t = Type::Structure( { { b_t, "v" }, { b_t, "w" } } );

    auto a = StructureConstant(
        *s_t, { libstdhl::make< BitConstant >( b_t, 0x04 ),
                  libstdhl::make< BitConstant >( b_t, 0x08 ) } );

    auto f_t = Type::Relation( { b_t }, { s_t } );

    auto f = Intrinsic( "sym", f_t );
    f.in( "arg", s_t );
    f.out( "res", b_t );

    auto scope = ParallelScope( &f );
    auto stmt = TrivialStatement( &scope );
    stmt.add( new NopInstruction() );

    auto m = AllocInstruction( b_t );
    auto i = CallInstruction( &f, { &a, &m } );
    auto r = libcsel_rt::Instruction::execute( i );
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
