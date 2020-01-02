//
//  Copyright (C) 2017-2020 CASM Organization <https://casm-lang.org>
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

#ifndef _LIBCJEL_RT_CJELIR_TO_ASMJIT_PASS_H_
#define _LIBCJEL_RT_CJELIR_TO_ASMJIT_PASS_H_

#include <libpass/Pass>
#include <libpass/PassData>
#include <libpass/PassResult>

#include <libcjel-ir/Visitor>

#include <asmjit/asmjit.h>

namespace libcjel_ir
{
    class Value;
    class Constant;
    class CallInstruction;
    class OperatorInstruction;
}

namespace libcjel_rt
{
    class CjelIRToAsmJitPass final
    : public libpass::Pass
    , public libcjel_ir::Visitor
    {
      public:
        static char id;

        bool run( libpass::PassResult& pr ) override;

        LIBCJEL_IR_VISITOR_INTERFACE;

        class Context : public libcjel_ir::Context
        {
          public:
            class Callable
            {
              private:
                asmjit::FuncSignatureX m_func_sig;
                void** m_func_ptr;
                u32 m_arg_size;

              public:
                Callable()
                : m_func_sig()
                , m_func_ptr( nullptr )
                , m_arg_size( 0 ){};

                asmjit::FuncSignatureX& funcsig( void )
                {
                    return m_func_sig;
                }

                void** funcptr( void** set = nullptr )
                {
                    if( set )
                    {
                        m_func_ptr = set;
                    }
                    return m_func_ptr;
                }

                u32 argsize( i8 increment = 0 )
                {
                    if( increment > 0 )
                    {
                        m_arg_size++;
                    }
                    else if( increment < 0 )
                    {
                        m_arg_size = 0;
                    }
                    return m_arg_size;
                }
            };

          private:
            asmjit::JitRuntime m_runtime;
            asmjit::CodeHolder m_codeholder;
            asmjit::StringLogger m_logger;

            asmjit::X86Compiler m_compiler;

            Callable* m_callable_last_accessed;

            std::unordered_map< libcjel_ir::Value*, Callable > m_callables;

            std::unordered_map< libcjel_ir::Value*, asmjit::X86Gp > m_val2reg;
            std::unordered_map< libcjel_ir::Value*, asmjit::X86Mem > m_val2mem;

          public:
            Context( void )
            : m_runtime()
            , m_codeholder()
            , m_compiler()
            , m_callable_last_accessed( 0 )
            {
                reset();

                m_logger.addOptions( asmjit::Logger::kOptionBinaryForm );
            }

            void reset( void )
            {
                m_compiler.onDetach( &m_codeholder );

                m_codeholder.reset();
                m_codeholder.init( m_runtime.getCodeInfo() );
                m_codeholder.attach( &m_compiler );
                m_codeholder.setLogger( &m_logger );

                m_logger.clearString();

                m_val2reg.clear();
                m_val2mem.clear();
            }

            u1 hasCallable( libcjel_ir::Value* value )
            {
                return m_callables.find( value ) != m_callables.end();
            }

            Callable& callable( libcjel_ir::Value* value = nullptr )
            {
                if( value )
                {
                    m_callable_last_accessed =
                        &m_callables.emplace( value, Callable() ).first->second;
                }

                return *m_callable_last_accessed;
            }

            asmjit::JitRuntime& runtime( void )
            {
                return m_runtime;
            }

            asmjit::CodeHolder& codeholder( void )
            {
                return m_codeholder;
            }

            asmjit::StringLogger& logger( void )
            {
                return m_logger;
            }

            asmjit::X86Compiler& compiler( void )
            {
                return m_compiler;
            }

            std::unordered_map< libcjel_ir::Value*, asmjit::X86Gp >& val2reg( void )
            {
                return m_val2reg;
            }

            std::unordered_map< libcjel_ir::Value*, asmjit::X86Mem >& val2mem( void )
            {
                return m_val2mem;
            }
        };

      private:
        void alloc_reg_for_value( libcjel_ir::Value& value, Context& c );

      public:
        libcjel_ir::Constant execute( libcjel_ir::OperatorInstruction& value, Context& c );

        libcjel_ir::Constant execute( libcjel_ir::CallInstruction& value, Context& c );
    };
}

#endif  // _LIBCJEL_RT_CJELIR_TO_ASMJIT_PASS_H_

//
//  Local variables:
//  mode: c++
//  indent-tabs-mode: nil
//  c-basic-offset: 4
//  tab-width: 4
//  End:
//  vim:noexpandtab:sw=4:ts=4:
//
