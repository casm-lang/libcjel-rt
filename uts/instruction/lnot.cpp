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

TEST( libcsel_rt__instruction_lnot, LnotInstruction_false_64 )
{
    auto a_t = Type::Bit( 64 );
    auto r_t = Type::Bit( 1 );

    auto a = BitConstant( a_t, 0xdeadbeef );
    auto i = LnotInstruction( &a );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( r == BitConstant( r_t, 0 ) );
}

TEST( libcsel_rt__instruction_lnot, LnotInstruction_true_64 )
{
    auto a_t = Type::Bit( 64 );
    auto r_t = Type::Bit( 1 );

    auto a = BitConstant( a_t, 0 );
    auto i = LnotInstruction( &a );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( r == BitConstant( r_t, 1 ) );
}

TEST( libcsel_rt__instruction_lnot, LnotInstruction_8_true )
{
    auto a_t = Type::Bit( 8 );
    auto r_t = Type::Bit( 1 );

    auto a = BitConstant( a_t, 0 );
    auto i = LnotInstruction( &a );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( r == BitConstant( r_t, 1 ) );
}

TEST( libcsel_rt__instruction_lnot, LnotInstruction_8_false )
{
    auto a_t = Type::Bit( 64 );
    auto r_t = Type::Bit( 1 );

    auto a = BitConstant( a_t, 0xdeadbeef );
    auto i = LnotInstruction( &a );

    auto r = libcsel_rt::Instruction::execute( i );

    ASSERT_TRUE( r == BitConstant( r_t, 0 ) );
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
