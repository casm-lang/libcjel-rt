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

#ifndef _LIB_CSELRT_CSELIR_TO_ASMJIT_PASS_H_
#define _LIB_CSELRT_CSELIR_TO_ASMJIT_PASS_H_

#include "libpass.h"

#include "../csel-ir/src/Visitor.h"

#include "../asmjit/src/asmjit/asmjit.h"

namespace libcsel_rt
{
    class CselIRToAsmJitPass : public libpass::Pass, public libcsel_ir::Visitor
    {
      public:
        static char id;

        bool run( libpass::PassResult& pr ) override final;

        LIB_CSELIR_VISITOR_INTERFACE;

        class Context
        {
          private:
            asmjit::JitRuntime runtime;
            asmjit::CodeHolder codeholder;
            asmjit::X86Compiler compiler;
            asmjit::FuncSignatureX fsx;

            std::unordered_map< libcsel_ir::Value*, void** > cache;

          public:
            Context( void )
            : runtime()
            , codeholder()
            , compiler()
            , fsx()
            {
                codeholder.init( runtime.getCodeInfo() );
                codeholder.attach( &compiler );
            };

            asmjit::JitRuntime& getRunTime( void )
            {
                return runtime;
            }

            asmjit::CodeHolder& getCodeHolder( void )
            {
                return codeholder;
            }

            asmjit::X86Compiler& getCompiler( void )
            {
                return compiler;
            }

            asmjit::FuncSignatureX& getFuncSignature( void )
            {
                return fsx;
            }
        };
    };
}

#endif // _LIB_CSELRT_CSELIR_TO_ASMJIT_PASS_H_

//
//  Local variables:
//  mode: c++
//  indent-tabs-mode: nil
//  c-basic-offset: 4
//  tab-width: 4
//  End:
//  vim:noexpandtab:sw=4:ts=4:
//
