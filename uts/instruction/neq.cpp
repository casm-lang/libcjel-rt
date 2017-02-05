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

TEST( libcsel_rt__instruction_neq, NeqInstruction_true )
{
    auto a = Constant::Bit( Type::Bit( 8 ), 0xf0 );
    auto b = Constant::Bit( Type::Bit( 8 ), 0x0f );
    auto i = NeqInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == *Constant::TRUE() );
}

TEST( libcsel_rt__instruction_neq, NeqInstruction_true_zero )
{
    auto a = Constant::Bit( Type::Bit( 8 ), 0 );
    auto b = Constant::Bit( Type::Bit( 8 ), 123 );
    auto i = NeqInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == *Constant::TRUE() );
}

TEST( libcsel_rt__instruction_neq, NeqInstruction_false )
{
    auto a = Constant::Bit( Type::Bit( 8 ), 123 );
    auto b = Constant::Bit( Type::Bit( 8 ), 123 );
    auto i = NeqInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == *Constant::FALSE() );
}

TEST( libcsel_rt__instruction_neq, NeqInstruction_false_zero )
{
    auto a = Constant::Bit( Type::Bit( 8 ), 0 );
    auto b = Constant::Bit( Type::Bit( 8 ), 0 );
    auto i = NeqInstruction( a, b );
    auto r = libcsel_rt::Instruction::execute( i );

    EXPECT_TRUE( *r == *Constant::FALSE() );
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
