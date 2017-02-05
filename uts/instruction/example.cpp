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
    auto a = Constant::Bit( Type::Bit( 8 ), 0x18 );
    auto b = Constant::Bit( Type::Bit( 8 ), 0xff );

    auto i = AndInstruction( a, b );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( *r == *Constant::Bit( Type::Bit( 8 ), 0x18 ) );
}

TEST( libcsel_rt__instruction, AddUnsignedInstruction )
{
    auto t = Type::Bit( 8 );
    auto a = Constant::Bit( t, 0x11 );
    auto b = Constant::Bit( t, 0x22 );
    auto i = AddUnsignedInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( *r == *Constant::Bit( t, 0x33 ) );
}

TEST( libcsel_rt__instruction_example, TODO_NAME )
{
    auto b_t = Type::Bit( 8 );
    auto s_t = Type::Structure( { { b_t, "v" }, { b_t, "w" } } );

    auto a = Constant::Structure(
        s_t, { Constant::Bit( b_t, 0x04 ), Constant::Bit( b_t, 0x08 ) } );

    auto x0 = Constant::Bit( b_t, 0 );
    auto x1 = Constant::Bit( b_t, 1 );

    auto f_t = Type::Relation( { b_t }, { s_t } );

    auto f
        = new Intrinsic( "sym", f_t ); // operation res := arg.v + arg.w + 0xa0
    auto f_i = f->in( "arg", s_t );
    auto f_o = f->out( "res", b_t );
    auto scope = new ParallelScope( f );
    auto stmt = new TrivialStatement( scope );

    auto v
        = stmt->add( new LoadInstruction( new ExtractInstruction( f_i, x0 ) ) );
    auto w
        = stmt->add( new LoadInstruction( new ExtractInstruction( f_i, x1 ) ) );

    auto r0 = stmt->add( new AddUnsignedInstruction( v, w ) );
    auto r1 = stmt->add(
        new AddUnsignedInstruction( r0, Constant::Bit( b_t, 0xa0 ) ) );

    stmt->add( new StoreInstruction( r1, f_o ) );

    auto i = CallInstruction( f, { a, new AllocInstruction( b_t ) } );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == *Constant::Bit( b_t, 0x04 + 0x08 + 0xa0 ) );
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
