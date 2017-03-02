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

/**
   @brief    TODO

   TODO
*/

#ifndef _LIB_CSELRT_INSTRUCTION_H_
#define _LIB_CSELRT_INSTRUCTION_H_

#include "CselRT.h"

namespace libcsel_ir
{
    class Constant;
    class Instruction;
}

namespace libcsel_rt
{
    class Instruction : public CselRT
    {
      public:
        static libcsel_ir::Constant execute( libcsel_ir::Instruction& value );
    };
}

#endif // _LIB_CSELRT_INSTRUCTION_H_

//
//  Local variables:
//  mode: c++
//  indent-tabs-mode: nil
//  c-basic-offset: 4
//  tab-width: 4
//  End:
//  vim:noexpandtab:sw=4:ts=4:
//
