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

#include "CallableUnit.h"

#include "transform/CselIRToAsmJitPass.h"

#include "../stdhl/cpp/Default.h"
#include "../stdhl/cpp/Log.h"

#include "../csel-ir/src/CallableUnit.h"
#include "../csel-ir/src/Value.h"
#include "../csel-ir/src/Visitor.h"

using namespace libcsel_rt;

void CallableUnit::compile( libcsel_ir::CallableUnit& value )
{
    libcsel_rt::CselIRToAsmJitPass x;
    libcsel_rt::CselIRToAsmJitPass::Context c;

    value.iterate( libcsel_ir::Traversal::PREORDER, &x, &c );

    typedef void ( *CallableType )( void*, void* );

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

    I i{ 44, 22 };
    B b{ 200, 100 };

    libstdhl::Log::info( "%s: %p:  pre: %lu, %u --> %u, %u", __FUNCTION__,
        c.getCallable( &value ).getPtr(), i.v, i.d, b.v, b.d );

    ( (CallableType)c.getCallable( &value ).getPtr() )( &i, &b );

    libstdhl::Log::info( "%s: %p: post: %lu, %u --> %u, %u", __FUNCTION__,
        c.getCallable( &value ).getPtr(), i.v, i.d, b.v, b.d );
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
