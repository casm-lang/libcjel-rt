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

#include "libcsel-ir.h"
#include "libcsel-rt.h"

using namespace libcsel_ir;

TEST( libcsel_rt__instruction, neq )
{
    auto a = Constant::Bit( Type::Bit( 7 ), 17 );
    auto b = Constant::Bit( Type::Bit( 7 ), 71 );
    auto i = NeqInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( *r == *Constant::TRUE() );
}

TEST( libcsel_rt__instruction, equ )
{
    auto a = Constant::Bit( Type::Bit( 7 ), 17 );
    auto b = Constant::Bit( Type::Bit( 7 ), 71 );
    auto i = EquInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == *Constant::FALSE() );

    EXPECT_STREQ( r->name(), "0" );
    EXPECT_STREQ( r->type().name(), "u1" );
}

TEST( libcsel_rt__instruction, and)
{
    auto a = Constant::Bit( Type::Bit( 8 ), 0x18 );
    auto b = Constant::Bit( Type::Bit( 8 ), 0xff );

    auto i = AndInstruction( a, b );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( *r == *Constant::Bit( Type::Bit( 8 ), 0x18 ) );
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
