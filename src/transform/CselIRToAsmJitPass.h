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
    class CselIRToAsmJitPass final : public libpass::Pass,
                                     public libcsel_ir::Visitor
    {
      public:
        static char id;

        bool run( libpass::PassResult& pr ) override;

        LIB_CSELIR_VISITOR_INTERFACE;

        class Context : public libcsel_ir::Context
        {
          public:
            class Callable
            {
              private:
                asmjit::FuncSignatureX func_sig;
                void** func_ptr;
                u32 arg_size;

              public:
                Callable()
                : func_sig()
                , func_ptr( nullptr )
                , arg_size( 0 ){};

                asmjit::FuncSignatureX& getSig( void )
                {
                    return func_sig;
                }

                void** getPtr( void** set = nullptr )
                {
                    if( set )
                    {
                        func_ptr = set;
                    }
                    return func_ptr;
                }

                u32 getArgSize( i8 increment = 0 )
                {
                    if( increment > 0 )
                    {
                        arg_size++;
                    }
                    else if( increment < 0 )
                    {
                        arg_size = 0;
                    }
                    return arg_size;
                }
            };

          private:
            asmjit::JitRuntime runtime;
            asmjit::CodeHolder codeholder;
            asmjit::X86Compiler compiler;

            Callable* callable_last_accessed;

            std::unordered_map< libcsel_ir::Value*, Callable > callables;

            std::unordered_map< libcsel_ir::Value*, asmjit::X86Gp > val2reg;
            std::unordered_map< libcsel_ir::Value*, asmjit::X86Mem > val2mem;

          public:
            Context( void )
            : runtime()
            , codeholder()
            , compiler()
            , callable_last_accessed( 0 )
            {
                codeholder.init( runtime.getCodeInfo() );
                codeholder.attach( &compiler );
            };

            u1 hasCallable( libcsel_ir::Value& value )
            {
                return callables.find( &value ) != callables.end();
            }

            Callable& getCallable( libcsel_ir::Value* value = nullptr )
            {
                if( value )
                {
                    callable_last_accessed
                        = &callables.emplace( value, Callable() ).first->second;
                }

                return *callable_last_accessed;
            }

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

            std::unordered_map< libcsel_ir::Value*, asmjit::X86Gp >& getVal2Reg(
                void )
            {
                return val2reg;
            }

            std::unordered_map< libcsel_ir::Value*, asmjit::X86Mem >&
            getVal2Mem( void )
            {
                return val2mem;
            }
        };

      private:
        void alloc_reg_for_value( libcsel_ir::Value& value, Context& c );

      public:
        libcsel_ir::Value* execute( libcsel_ir::CallInstruction& value );
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
