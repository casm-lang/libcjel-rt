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

#include "CselIRToAsmJitPass.h"

#include "../csel-ir/src/analyze/CselIRDumpPass.h"

#include "../stdhl/cpp/Default.h"
#include "../stdhl/cpp/Math.h"

using namespace libcsel_ir;
using namespace libcsel_rt;
using namespace asmjit;

char CselIRToAsmJitPass::id = 0;

static libpass::PassRegistration< CselIRToAsmJitPass > PASS(
    "CSEL IR to AsmJit", "TBD", 0, 0 );

bool CselIRToAsmJitPass::run( libpass::PassResult& pr )
{
    assert( not" PPA: TODO!!! " );
    return false;
}

#define TRACE( FMT, ARGS... )                                                  \
    libstdhl::Log::info( "%s:%i: %p: %s | %s | '%s' | size=%lu | @ %p" FMT,    \
        __FUNCTION__,                                                          \
        __LINE__,                                                              \
        &value,                                                                \
        value.getName(),                                                       \
        value.getLabel(),                                                      \
        value.getType()->getName(),                                            \
        value.getType()->getSize(),                                            \
        cxt,                                                                   \
        ##ARGS )

static asmjit::X86Gp& alloc_reg_for_value(
    Value& value, CselIRToAsmJitPass::Context& c )
{
    assert( value.getType() );
    Type& type = *value.getType();

    switch( type.getID() )
    {
        case Type::BIT:
        {
            if( type.getSize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type.getSize() <= 8 )
            {
                c.getVal2Reg()[&value ]
                    = c.getCompiler().newU8( value.getLabel() );
            }
            else if( type.getSize() <= 16 )
            {
                c.getVal2Reg()[&value ]
                    = c.getCompiler().newU16( value.getLabel() );
            }
            else if( type.getSize() <= 32 )
            {
                c.getVal2Reg()[&value ]
                    = c.getCompiler().newU32( value.getLabel() );
            }
            else if( type.getSize() <= 64 )
            {
                c.getVal2Reg()[&value ]
                    = c.getCompiler().newU64( value.getLabel() );
            }
            else
            {
                assert( not " a bit type of bit-size greater than 64-bit is unsupported for now! " );
            }
            break;
        }
        case Type::VECTOR: // fall-through
        case Type::STRUCTURE:
        {
            c.getVal2Reg()[&value ]
                = c.getCompiler().newUIntPtr( value.getLabel() );
            break;
        }
        case Type::STRING:
        case Type::INTERCONNECT:
        case Type::VOID:
        case Type::LABEL:
        case Type::RELATION:
        case Type::_BOTTOM_:
        case Type::_TOP_:
        {
            libstdhl::Log::error(
                "unsupported type '%s' to allocate a register!",
                type.getDescription() );
            assert( 0 );
            break;
        }
    }

    if( libcsel_ir::isa< libcsel_ir::Reference >( value ) )
    {
        CselIRToAsmJitPass::Context::Callable& func = c.getCallable();

        c.getCompiler().setArg(
            func.getArgSize( 1 ) - 1, c.getVal2Reg()[&value ] );
    }

    libstdhl::Log::info( "alloc '%p' for '%s' of type '%s'",
        &c.getVal2Reg()[&value ],
        value.getLabel(),
        value.getType()->getName() );

    return c.getVal2Reg()[&value ];
}

static asmjit::X86Mem* alloc_ptr_for_value(
    Value& value, CselIRToAsmJitPass::Context& c )
{
    assert( value.getType() );
    Type& type = *value.getType();
    return 0;
}

//
// Module
//

void CselIRToAsmJitPass::visit_prolog( Module& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( Module& value, void* cxt )
{
    TRACE( "" );
}

//
// Function
//

void CselIRToAsmJitPass::visit_prolog( Function& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_interlog( Function& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( Function& value, void* cxt )
{
    TRACE( "" );
}

//
// Intrinsic
//

void CselIRToAsmJitPass::visit_prolog( Intrinsic& value, void* cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( *( (Context*)cxt ) );

    CselIRToAsmJitPass::Context::Callable& func = c.getCallable( &value );
    func.getArgSize( -1 );

    FuncSignatureX& fsig = c.getFuncSignature();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
}
void CselIRToAsmJitPass::visit_interlog( Intrinsic& value, void* cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( *( (Context*)cxt ) );
    Context::Callable& func = c.getCallable();

    c.getCompiler().addFunc( c.getFuncSignature() );

    for( auto param : value.getInParameters() )
    {
        assert( isa< Reference >( param ) );
        alloc_reg_for_value( *param, c );
    }

    for( auto param : value.getOutParameters() )
    {
        assert( isa< Reference >( param ) );
        alloc_reg_for_value( *param, c );
    }

    for( auto param : value.getLinkage() )
    {
        assert( param and 0 );
    }

    assert( c.getFuncSignature().getArgCount() == func.getArgSize() );
}
void CselIRToAsmJitPass::visit_epilog( Intrinsic& value, void* cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( *( (Context*)cxt ) );

    c.getCompiler().endFunc();
    c.getCompiler().finalize();

    void** fn;
    Error err = c.getRunTime().add( &fn, &c.getCodeHolder() );
    assert( not err );

    Context::Callable& func = c.getCallable();
    func.getPtr( fn );
}

//
// Reference
//

void CselIRToAsmJitPass::visit_prolog( Reference& value, void* cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( *( (Context*)cxt ) );
    FuncSignatureX& fsig = c.getFuncSignature();
    fsig.addArg( TypeId::kUIntPtr );
}
void CselIRToAsmJitPass::visit_epilog( Reference& value, void* cxt )
{
}

//
// Structure
//

void CselIRToAsmJitPass::visit_prolog( Structure& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( Structure& value, void* cxt )
{
    TRACE( "" );
}

//
// Variable
//

void CselIRToAsmJitPass::visit_prolog( Variable& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( Variable& value, void* cxt )
{
    TRACE( "" );
}

//
// Memory
//

void CselIRToAsmJitPass::visit_prolog( Memory& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( Memory& value, void* cxt )
{
    TRACE( "" );
}

//
// ParallelScope
//

void CselIRToAsmJitPass::visit_prolog( ParallelScope& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( ParallelScope& value, void* cxt )
{
}

//
// SequentialScope
//

void CselIRToAsmJitPass::visit_prolog( SequentialScope& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( SequentialScope& value, void* cxt )
{
}

//
// TrivialStatement
//

void CselIRToAsmJitPass::visit_prolog( TrivialStatement& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( TrivialStatement& value, void* cxt )
{
}

//
// BranchStatement
//

void CselIRToAsmJitPass::visit_prolog( BranchStatement& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_interlog( BranchStatement& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( BranchStatement& value, void* cxt )
{
    TRACE( "" );
}

//
// LoopStatement
//

void CselIRToAsmJitPass::visit_prolog( LoopStatement& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_interlog( LoopStatement& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( LoopStatement& value, void* cxt )
{
    TRACE( "" );
}

//
// CallInstruction
//

void CselIRToAsmJitPass::visit_prolog( CallInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( CallInstruction& value, void* cxt )
{
}

//
// IdCallInstruction
//

void CselIRToAsmJitPass::visit_prolog( IdCallInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( IdCallInstruction& value, void* cxt )
{
}

//
// StreamInstruction
//

void CselIRToAsmJitPass::visit_prolog( StreamInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( StreamInstruction& value, void* cxt )
{
}

//
// NopInstruction
//

void CselIRToAsmJitPass::visit_prolog( NopInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( NopInstruction& value, void* cxt )
{
}

//
// AllocInstruction
//

void CselIRToAsmJitPass::visit_prolog( AllocInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( AllocInstruction& value, void* cxt )
{
}

//
// IdInstruction
//

void CselIRToAsmJitPass::visit_prolog( IdInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( IdInstruction& value, void* cxt )
{
}

//
// CastInstruction
//

void CselIRToAsmJitPass::visit_prolog( CastInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( CastInstruction& value, void* cxt )
{
}

//
// ExtractInstruction
//

void CselIRToAsmJitPass::visit_prolog( ExtractInstruction& value, void* cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( *( (Context*)cxt ) );

    Value* base = value.getValue( 0 );
    Value* offset = value.getValue( 1 );

    if( isa< Reference >( base ) and base->getType()->isStructure() )
    {
        assert( isa< BitConstant >( offset ) );
        BitConstant& index = static_cast< BitConstant& >( *offset );

        assert( index.getValue() < base->getType()->getResults().size() );

        u32 byte_offset = 0;
        u32 byte_size = 0;
        for( u32 i = 0; i < index.getValue(); i++ )
        {
            byte_size = base->getType()->getResults()[ i ]->getSize() / 8;
            byte_size = ( byte_size % 2 == 0 ) ? byte_size : byte_size + 1;
            byte_offset += byte_size;
        }
        libstdhl::Log::info( "byte size = %lu, %lu", byte_size, byte_offset );

        c.getVal2Mem()[&value ]
            = x86::ptr( c.getVal2Reg()[ base ], byte_offset );
    }
    else
    {
        assert( 0 );
    }

    alloc_ptr_for_value( value, c );
}
void CselIRToAsmJitPass::visit_epilog( ExtractInstruction& value, void* cxt )
{
}

//
// LoadInstruction
//

void CselIRToAsmJitPass::visit_prolog( LoadInstruction& value, void* cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( *( (Context*)cxt ) );

    alloc_reg_for_value( value, c );

    Value* src = value.getValue( 0 );

    if( isa< ExtractInstruction >( src ) )
    {
        c.getCompiler().mov( c.getVal2Reg()[&value ], c.getVal2Mem()[ src ] );
    }
    else
    {
        assert( 0 );
    }
}
void CselIRToAsmJitPass::visit_epilog( LoadInstruction& value, void* cxt )
{
}

//
// StoreInstruction
//

void CselIRToAsmJitPass::visit_prolog( StoreInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( StoreInstruction& value, void* cxt )
{
}

//
// NotInstruction
//

void CselIRToAsmJitPass::visit_prolog( NotInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( NotInstruction& value, void* cxt )
{
}

//
// AndInstruction
//

void CselIRToAsmJitPass::visit_prolog( AndInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( AndInstruction& value, void* cxt )
{
}

//
// OrInstruction
//

void CselIRToAsmJitPass::visit_prolog( OrInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( OrInstruction& value, void* cxt )
{
}

//
// XorInstruction
//

void CselIRToAsmJitPass::visit_prolog( XorInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( XorInstruction& value, void* cxt )
{
}

//
// AddSignedInstruction
//

void CselIRToAsmJitPass::visit_prolog( AddSignedInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( AddSignedInstruction& value, void* cxt )
{
}

//
// DivSignedInstruction
//

void CselIRToAsmJitPass::visit_prolog( DivSignedInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( DivSignedInstruction& value, void* cxt )
{
}

//
// ModUnsignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    ModUnsignedInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    ModUnsignedInstruction& value, void* cxt )
{
}

//
// EquUnsignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    EquUnsignedInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    EquUnsignedInstruction& value, void* cxt )
{
}

//
// NeqUnsignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    NeqUnsignedInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    NeqUnsignedInstruction& value, void* cxt )
{
}

//
// ZeroExtendInstruction
//

void CselIRToAsmJitPass::visit_prolog( ZeroExtendInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( ZeroExtendInstruction& value, void* cxt )
{
}

//
// TruncationInstruction
//

void CselIRToAsmJitPass::visit_prolog( TruncationInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( TruncationInstruction& value, void* cxt )
{
}

//
// BitConstant
//

void CselIRToAsmJitPass::visit_prolog( BitConstant& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( BitConstant& value, void* cxt )
{
}

//
// StructureConstant
//

void CselIRToAsmJitPass::visit_prolog( StructureConstant& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( StructureConstant& value, void* cxt )
{
    TRACE( "" );
}

//
// StringConstant
//

void CselIRToAsmJitPass::visit_prolog( StringConstant& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( StringConstant& value, void* cxt )
{
    TRACE( "" );
}

//
// Interconnect
//

void CselIRToAsmJitPass::visit_prolog( Interconnect& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( Interconnect& value, void* cxt )
{
    TRACE( "" );
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
