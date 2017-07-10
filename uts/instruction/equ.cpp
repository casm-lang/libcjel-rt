//
//  Copyright (c) 2017 CASM Organization
//  All rights reserved.
//
//  Developed by: Philipp Paulweber
//                https://github.com/casm-lang/libcjel-rt
//
//  This file is part of libcjel-rt.
//
//  libcjel-rt is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  libcjel-rt is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with libcjel-rt. If not, see <http://www.gnu.org/licenses/>.
//

#include "uts/main.h"

using namespace libcjel_ir;

TEST( libcjel_rt__instruction_equ, EquInstruction_Bit7 )
{
    auto a = libstdhl::make< BitConstant >( 7, 17 );
    auto b = libstdhl::make< BitConstant >( 7, 71 );

    auto i = EquInstruction( a, b );
    auto r = libcjel_rt::Instruction::execute( i );

    EXPECT_TRUE( r == BitConstant( 1, false ) );

    EXPECT_STREQ( r.name().c_str(), "0" );
    EXPECT_STREQ( r.type().name().c_str(), "u1" );
}

TEST( libcjel_rt__instruction_equ, EquInstruction_true )
{
    auto a = libstdhl::make< BitConstant >( 8, 123 );
    auto b = libstdhl::make< BitConstant >( 8, 123 );

    auto i = EquInstruction( a, b );
    auto r = libcjel_rt::Instruction::execute( i );

    EXPECT_TRUE( r == BitConstant( 1, true ) );
}

TEST( libcjel_rt__instruction_equ, EquInstruction_true_zero )
{
    auto a = libstdhl::make< BitConstant >( 8, 0 );
    auto b = libstdhl::make< BitConstant >( 8, 0 );

    auto i = EquInstruction( a, b );
    auto r = libcjel_rt::Instruction::execute( i );

    EXPECT_TRUE( r == BitConstant( 1, true ) );
}

TEST( libcjel_rt__instruction_equ, EquInstruction_false )
{
    auto a = libstdhl::make< BitConstant >( 8, 0x0f );
    auto b = libstdhl::make< BitConstant >( 8, 0xf0 );

    auto i = EquInstruction( a, b );
    auto r = libcjel_rt::Instruction::execute( i );

    EXPECT_TRUE( r == BitConstant( 1, false ) );
}

TEST( libcjel_rt__instruction_equ, EquInstruction_false_zero )
{
    auto a = libstdhl::make< BitConstant >( 8, 0 );
    auto b = libstdhl::make< BitConstant >( 8, 123 );

    auto i = EquInstruction( a, b );
    auto r = libcjel_rt::Instruction::execute( i );

    EXPECT_TRUE( r == BitConstant( 1, false ) );
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
