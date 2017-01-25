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

#include "Instruction.h"

#include "transform/CselIRToAsmJitPass.h"

#include "../csel-ir/src/Instruction.h"
#include "../csel-ir/src/Value.h"

using namespace libcsel_rt;

libcsel_ir::Value* Instruction::execute( libcsel_ir::Instruction& value )
{
    libstdhl::Log::info( "%s", __FUNCTION__ );

    if( not libcsel_ir::isa< libcsel_ir::CallInstruction >( value ) )
    {
        libstdhl::Log::error( "%s:%i: unimplemented instruction to be executed",
            __FILE__, __LINE__ );
        assert( 0 );
    }

    libcsel_ir::CallInstruction& call
        = static_cast< libcsel_ir::CallInstruction& >( value );

    libcsel_rt::CselIRToAsmJitPass x;

    return x.execute( call );
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
