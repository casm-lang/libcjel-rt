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

TEST( libcsel_rt__instruction, AndInstruction )
{
    auto a = libstdhl::make< BitConstant >( 8, 0x18 );
    auto b = libstdhl::make< BitConstant >( 8, 0xff );

    auto i = AndInstruction( a, b );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( r == BitConstant( 8, 0x18 ) );
}

TEST( libcsel_rt__instruction, AddUnsignedInstruction )
{
    auto t = libstdhl::make< BitType >( 8 );

    auto a = libstdhl::make< BitConstant >( t, 0x11 );
    auto b = libstdhl::make< BitConstant >( t, 0x22 );

    auto i = AddUnsignedInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( r == BitConstant( t, 0x33 ) );
}

TEST( libcsel_rt__instruction_example, simple_move_test )
{
    auto b_t = libstdhl::make< BitType >( 8 );

    std::vector< StructureElement > structure_args
        = { { b_t, "v" }, { b_t, "w" } };

    Structure::Ptr structure
        = libstdhl::make< Structure >( "structure", structure_args );

    auto s_t = libstdhl::make< StructureType >( structure );

    const std::vector< Constant > a_args
        = { BitConstant( b_t, 0x12 ), BitConstant( b_t, 0x34 ) };

    auto a = libstdhl::make< StructureConstant >( s_t, a_args );

    auto x0 = libstdhl::make< BitConstant >( b_t, 0 );
    auto x1 = libstdhl::make< BitConstant >( b_t, 1 );

    const std::vector< Type::Ptr > f_t_i = { s_t };
    auto f_t = libstdhl::make< RelationType >( f_t_i, f_t_i );

    auto f = libstdhl::make< Intrinsic >(
        "sym", f_t ); // operation res.v := arg.v; res.w = arg.w
    auto f_i = f->in( "arg", s_t );
    auto f_o = f->out( "res", s_t );

    auto scope = libstdhl::make< ParallelScope >();
    f->setContext( scope );

    auto stmt = libstdhl::make< TrivialStatement >();
    stmt->setParent( scope );
    scope->add( stmt );

    auto v_ptr = stmt->add( libstdhl::make< ExtractInstruction >( f_i, x0 ) );
    auto v_ld = stmt->add( libstdhl::make< LoadInstruction >( v_ptr ) );
    auto w_ptr = stmt->add( libstdhl::make< ExtractInstruction >( f_i, x1 ) );
    auto w_ld = stmt->add( libstdhl::make< LoadInstruction >( w_ptr ) );

    auto res_v_ptr
        = stmt->add( libstdhl::make< ExtractInstruction >( f_o, x0 ) );
    auto res_w_ptr
        = stmt->add( libstdhl::make< ExtractInstruction >( f_o, x1 ) );

    auto res_v_st
        = stmt->add( libstdhl::make< StoreInstruction >( v_ld, res_v_ptr ) );
    auto res_w_st
        = stmt->add( libstdhl::make< StoreInstruction >( w_ld, res_w_ptr ) );

    auto m = libstdhl::make< AllocInstruction >( s_t );
    auto i = CallInstruction( f, { a, m } );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( r == *a );
}

TEST( libcsel_rt__instruction_example, TODO_NAME )
{
    auto b_t = libstdhl::make< BitType >( 8 );

    const std::vector< StructureElement > structure_args
        = { { b_t, "v" }, { b_t, "w" } };

    auto structure = libstdhl::make< Structure >( "structure", structure_args );

    auto s_t = libstdhl::make< StructureType >( structure );

    const std::vector< Constant > a_args
        = { BitConstant( b_t, 0x04 ), BitConstant( b_t, 0x08 ) };
    auto a = libstdhl::make< StructureConstant >( s_t, a_args );

    auto x0 = libstdhl::make< BitConstant >( b_t, 0 );
    auto x1 = libstdhl::make< BitConstant >( b_t, 1 );

    const std::vector< Type::Ptr > f_t_i = { s_t };
    const std::vector< Type::Ptr > f_t_o = { b_t };
    auto f_t = libstdhl::make< RelationType >( f_t_o, f_t_i );

    auto f = libstdhl::make< Intrinsic >(
        "sym", f_t ); // operation res := arg.v + arg.w + 0xa0
    auto f_i = f->in( "arg", s_t );
    auto f_o = f->out( "res", b_t );

    auto scope = libstdhl::make< ParallelScope >();
    f->setContext( scope );

    auto stmt = libstdhl::make< TrivialStatement >();
    stmt->setParent( scope );
    scope->add( stmt );

    auto v_ptr = stmt->add( libstdhl::make< ExtractInstruction >( f_i, x0 ) );
    auto v_ld = stmt->add( libstdhl::make< LoadInstruction >( v_ptr ) );

    auto w_ptr = stmt->add( libstdhl::make< ExtractInstruction >( f_i, x1 ) );
    auto w_ld = stmt->add( libstdhl::make< LoadInstruction >( w_ptr ) );

    auto r0
        = stmt->add( libstdhl::make< AddUnsignedInstruction >( v_ld, w_ld ) );

    auto c0 = libstdhl::make< BitConstant >( b_t, 0xa0 );
    auto r1 = stmt->add( libstdhl::make< AddUnsignedInstruction >( r0, c0 ) );
    stmt->add( libstdhl::make< StoreInstruction >( r1, f_o ) );

    auto m = libstdhl::make< AllocInstruction >( b_t );

    auto i = CallInstruction( f, { a, m } );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( r == BitConstant( b_t, 0x04 + 0x08 + 0xa0 ) );
}

TEST( libcsel_rt__instruction_example, lala )
{
    auto b_t = libstdhl::make< BitType >( 8 );

    const std::vector< StructureElement > structure_args
        = { { b_t, "v" }, { b_t, "w" } };

    auto structure = libstdhl::make< Structure >( "structure", structure_args );

    auto s_t = libstdhl::make< StructureType >( structure );

    const std::vector< Constant > a_args
        = { BitConstant( b_t, 0x04 ), BitConstant( b_t, 0x08 ) };
    auto a = libstdhl::make< StructureConstant >( s_t, a_args );

    const std::vector< Type::Ptr > f_t_i = { s_t };
    const std::vector< Type::Ptr > f_t_o = { b_t };
    auto f_t = libstdhl::make< RelationType >( f_t_o, f_t_i );

    auto f = libstdhl::make< Intrinsic >( "sym", f_t );
    f->in( "arg", s_t );
    f->out( "res", b_t );

    auto scope = libstdhl::make< ParallelScope >();
    f->setContext( scope );

    auto stmt = libstdhl::make< TrivialStatement >();
    stmt->setParent( scope );
    scope->add( stmt );

    stmt->add( libstdhl::make< NopInstruction >() );

    auto m = libstdhl::make< AllocInstruction >( b_t );

    auto i = CallInstruction( f, { a, m } );
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
