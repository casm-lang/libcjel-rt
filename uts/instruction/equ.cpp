//
//  Copyright (C) 2017-2018 CASM Organization <https://casm-lang.org>
//  All rights reserved.
//
//  Developed by: Philipp Paulweber
//                <https://github.com/casm-lang/libcjel-rt>
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
//  Additional permission under GNU GPL version 3 section 7
//
//  libcjel-rt is distributed under the terms of the GNU General Public License
//  with the following clarification and special exception: Linking libcjel-rt
//  statically or dynamically with other modules is making a combined work
//  based on libcjel-rt. Thus, the terms and conditions of the GNU General
//  Public License cover the whole combination. As a special exception,
//  the copyright holders of libcjel-rt give you permission to link libcjel-rt
//  with independent modules to produce an executable, regardless of the
//  license terms of these independent modules, and to copy and distribute
//  the resulting executable under terms of your choice, provided that you
//  also meet, for each linked independent module, the terms and conditions
//  of the license of that module. An independent module is a module which
//  is not derived from or based on libcjel-rt. If you modify libcjel-rt, you
//  may extend this exception to your version of the library, but you are
//  not obliged to do so. If you do not wish to do so, delete this exception
//  statement from your version.
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
