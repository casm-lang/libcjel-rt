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

#if 0
#define TRACE( FMT, ARGS... )                                                  \
    libstdhl::Log::info(                                                       \
        "Asmjit:%i: %p: %s | %s | '%s' | size=%lu | @ %p" FMT, __LINE__,       \
        &value, value.getName(), value.getLabel(), value.getType()->getName(), \
        value.getType()->getSize(), cxt, ##ARGS )
#else
#define TRACE( FMT, ARGS... )
#endif

#define VERBOSE( FMT, ARGS... )                                                \
    libstdhl::Log::info( "[%s %s] %s = " FMT, value.getName(),                 \
        value.getType()->getName(), value.getLabel(), ##ARGS )

#define FIXME()                                                                \
    {                                                                          \
        libstdhl::Log::error(                                                  \
            "%s:%i: FIXME: unimplemented", __FUNCTION__, __LINE__ );           \
        assert( 0 );                                                           \
    }

static u32 calc_byte_size( libcsel_ir::Type& type )
{
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
                return 1;
            }
            else if( type.getSize() <= 16 )
            {
                return 2;
            }
            else if( type.getSize() <= 32 )
            {
                return 4;
            }
            else if( type.getSize() <= 64 )
            {
                return 8;
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
            u32 byte_size = 0;
            for( auto t : type.getResults() )
            {
                byte_size += calc_byte_size( *t );
            }
            return byte_size;
        }
        default:
        {
            libstdhl::Log::error(
                "unsupported type '%s' to calculate byte_size!",
                type.getDescription() );
            assert( 0 );
            return 0;
        }
    }
}

void CselIRToAsmJitPass::alloc_reg_for_value( Value& value, Context& c )
{
    assert( value.getType() );
    Type& type = *value.getType();

    if( c.getVal2Reg().find( &value ) != c.getVal2Reg().end() )
    {
        // already allocated!
        return;
    }

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
                VERBOSE( "newU8" );
            }
            else if( type.getSize() <= 16 )
            {
                c.getVal2Reg()[&value ]
                    = c.getCompiler().newU16( value.getLabel() );
                VERBOSE( "newU16" );
            }
            else if( type.getSize() <= 32 )
            {
                c.getVal2Reg()[&value ]
                    = c.getCompiler().newU32( value.getLabel() );
                VERBOSE( "newU32" );
            }
            else if( type.getSize() <= 64 )
            {
                c.getVal2Reg()[&value ]
                    = c.getCompiler().newU64( value.getLabel() );
                VERBOSE( "newU64" );
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
            VERBOSE( "newUIntPtr" );
            break;
        }
        default:
        {
            libstdhl::Log::error(
                "unsupported type '%s' to allocate a register!",
                type.getDescription() );
            assert( 0 );
            break;
        }
    }

    if( isa< Reference >( value ) )
    {
        Context::Callable& func = c.getCallable();

        c.getCompiler().setArg(
            func.getArgSize( 1 ) - 1, c.getVal2Reg()[&value ] );
        VERBOSE( "setArg( %u, %s )", func.getArgSize() - 1, value.getLabel() );
    }

    // libstdhl::Log::info( "alloc '%p' for '%s' of type '%s'",
    //     &c.getVal2Reg()[&value ],
    //     value.getLabel(),
    //     value.getType()->getName() );

    if( isa< Constant >( value ) )
    {
        switch( value.getValueID() )
        {
            case Value::BIT_CONSTANT:
            {
                visit_prolog( static_cast< BitConstant& >( value ),
                    static_cast< libcsel_ir::Context& >( c ) );
                break;
            }
            case Value::STRUCTURE_CONSTANT:
            {
                visit_prolog( static_cast< StructureConstant& >( value ),
                    static_cast< libcsel_ir::Context& >( c ) );
                break;
            }
            default:
            {
                libstdhl::Log::error(
                    "unsupported constant value of type '%s' to allocate a "
                    "register!",
                    type.getDescription() );
                assert( 0 );
                break;
            }
        }
    }

    if( isa< AllocInstruction >( value ) )
    {
        u32 byte_size = calc_byte_size( type );

        c.getCompiler().lea(
            c.getVal2Reg()[&value ], c.getCompiler().newStack( byte_size, 4 ) );
        VERBOSE( "lea %s, newStack( %u, 4 )", value.getLabel(), byte_size );
    }
}

void CselIRToAsmJitPass::visit_prolog( Module& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog( Module& value, libcsel_ir::Context& cxt )
{
}

//
// Function
//

void CselIRToAsmJitPass::visit_prolog(
    Function& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_interlog(
    Function& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    Function& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Intrinsic
//

void CselIRToAsmJitPass::visit_prolog(
    Intrinsic& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Context::Callable& func = c.getCallable( &value );
    func.getArgSize( -1 );

    FuncSignatureX& fsig = func.getSig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
}
void CselIRToAsmJitPass::visit_interlog(
    Intrinsic& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );
    Context::Callable& func = c.getCallable();

    c.getCompiler().addFunc( func.getSig() );
    VERBOSE( "addFunc( %s )", value.getLabel() );

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

    assert( func.getSig().getArgCount() == func.getArgSize() );
}
void CselIRToAsmJitPass::visit_epilog(
    Intrinsic& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

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

void CselIRToAsmJitPass::visit_prolog(
    Reference& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );
    Context::Callable& func = c.getCallable();
    FuncSignatureX& fsig = func.getSig();
    fsig.addArg( TypeId::kUIntPtr );
}
void CselIRToAsmJitPass::visit_epilog(
    Reference& value, libcsel_ir::Context& cxt )
{
}

//
// Structure
//

void CselIRToAsmJitPass::visit_prolog(
    Structure& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    Structure& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Variable
//

void CselIRToAsmJitPass::visit_prolog(
    Variable& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    Variable& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Memory
//

void CselIRToAsmJitPass::visit_prolog( Memory& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog( Memory& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// ParallelScope
//

void CselIRToAsmJitPass::visit_prolog(
    ParallelScope& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    ParallelScope& value, libcsel_ir::Context& cxt )
{
}

//
// SequentialScope
//

void CselIRToAsmJitPass::visit_prolog(
    SequentialScope& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    SequentialScope& value, libcsel_ir::Context& cxt )
{
}

//
// TrivialStatement
//

void CselIRToAsmJitPass::visit_prolog(
    TrivialStatement& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    TrivialStatement& value, libcsel_ir::Context& cxt )
{
}

//
// BranchStatement
//

void CselIRToAsmJitPass::visit_prolog(
    BranchStatement& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_interlog(
    BranchStatement& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    BranchStatement& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// LoopStatement
//

void CselIRToAsmJitPass::visit_prolog(
    LoopStatement& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_interlog(
    LoopStatement& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    LoopStatement& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// CallInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    CallInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    assert( c.hasCallable( value.getCallee() ) );
    Context::Callable& callee = c.getCallable( &value.getCallee() );

    for( auto v : value.getValues() )
    {
        if( v == &value.getCallee() )
        {
            continue;
        }

        alloc_reg_for_value( *v, c );
    }

    CCFuncCall* call
        = c.getCompiler().call( imm_ptr( callee.getPtr() ), callee.getSig() );
    VERBOSE( "call( %s )", value.getCallee().getLabel() );

    u32 i = 0;
    for( i = 1; i < value.getValues().size(); i++ )
    {
        call->setArg( ( i - 1 ), c.getVal2Reg()[ value.getValues()[ i ] ] );
        VERBOSE(
            "setArg( %u, %s )", i - 1, value.getValues()[ i ]->getLabel() );
    }
}
void CselIRToAsmJitPass::visit_epilog(
    CallInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// IdCallInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    IdCallInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    IdCallInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// StreamInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    StreamInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    StreamInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// NopInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    NopInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog(
    NopInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// AllocInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    AllocInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    AllocInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// IdInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    IdInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    IdInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// CastInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    CastInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    CastInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// ExtractInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    ExtractInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Value* base = value.getValue( 0 );
    Value* offset = value.getValue( 1 );

    if( isa< Reference >( base ) and base->getType()->isStructure() )
    {
        assert( isa< BitConstant >( offset ) );
        BitConstant& index = static_cast< BitConstant& >( *offset );

        assert( index.getValue() < base->getType()->getResults().size() );

        u32 byte_offset = 0;
        u32 byte_size = 0;
        u32 bit_size = 0;
        for( u32 i = 0; i < index.getValue(); i++ )
        {
            bit_size = base->getType()->getResults()[ i ]->getSize();
            byte_size = bit_size / 8 + ( ( bit_size % 8 ) % 2 );
            byte_offset += byte_size;
        }
        // libstdhl::Log::info( "byte size = %lu, %lu", byte_size, byte_offset
        // );

        c.getVal2Mem()[&value ]
            = x86::ptr( c.getVal2Reg()[ base ], byte_offset );
        VERBOSE( "ptr( %s, %lu ) [ '%s' @ %lu --> bs = %lu ]", base->getLabel(),
            byte_offset, base->getType()->getName(), index.getValue(),
            base->getType()->getResults()[ index.getValue() ]->getSize() );
    }
    else
    {
        assert( 0 );
    }
}
void CselIRToAsmJitPass::visit_epilog(
    ExtractInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// LoadInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    LoadInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    alloc_reg_for_value( value, c );

    Value* src = value.getValue( 0 );

    if( isa< ExtractInstruction >( src ) )
    {
        c.getCompiler().mov( c.getVal2Reg()[&value ], c.getVal2Mem()[ src ] );
        VERBOSE( "mov %s, %s", value.getLabel(), src->getLabel() );
    }
    else
    {
        assert( 0 );
    }
}
void CselIRToAsmJitPass::visit_epilog(
    LoadInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// StoreInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    StoreInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Value* src = value.getValue( 0 );
    Value* dst = value.getValue( 1 );

    if( isa< ExtractInstruction >( dst ) )
    {
        c.getCompiler().mov( c.getVal2Mem()[ dst ], c.getVal2Reg()[ src ] );
        VERBOSE( "mov %s, %s", dst->getLabel(), src->getLabel() );
    }
    else
    {
        assert( not" unsupported 'store' instruction usage! " );
    }
}
void CselIRToAsmJitPass::visit_epilog(
    StoreInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// NotInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    NotInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    NotInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// AndInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    AndInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.getValue( 0 );
    Value* rhs = value.getValue( 1 );

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.getCompiler().andn(
        c.getVal2Reg()[ res ], c.getVal2Reg()[ lhs ], c.getVal2Reg()[ rhs ] );
    VERBOSE( "andn %s, %s", lhs->getLabel(), rhs->getLabel() );
}
void CselIRToAsmJitPass::visit_epilog(
    AndInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// OrInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    OrInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    OrInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// XorInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    XorInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    XorInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// AddSignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    AddSignedInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    AddSignedInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// DivSignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    DivSignedInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    DivSignedInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// ModUnsignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    ModUnsignedInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    ModUnsignedInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// EquUnsignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    EquUnsignedInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    EquUnsignedInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// NeqUnsignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    NeqUnsignedInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );

    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.getValue( 0 );
    Value* rhs = value.getValue( 1 );

    Label lbl_true = c.getCompiler().newLabel();
    Label lbl_exit = c.getCompiler().newLabel();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.getCompiler().cmp( c.getVal2Reg()[ lhs ], c.getVal2Reg()[ rhs ] );
    VERBOSE( "cmp %s, %s", lhs->getLabel(), rhs->getLabel() );

    // jump if not equal to true path, else cont with false path
    c.getCompiler().jne( lbl_true );
    VERBOSE( "jne 'lbl_true'" );

    // false path
    c.getCompiler().mov( c.getVal2Reg()[&value ], asmjit::imm( 0 ) );
    VERBOSE( "mov %s, imm( 0 )", value.getLabel() );

    c.getCompiler().jmp( lbl_exit );
    VERBOSE( "jmp 'lbl_exit'" );

    // true path
    c.getCompiler().bind( lbl_true );
    VERBOSE( "bind 'lbl_true'" );

    c.getCompiler().mov( c.getVal2Reg()[&value ], asmjit::imm( 1 ) );
    VERBOSE( "mov %s, imm(1)", value.getLabel() );

    // end if compare
    c.getCompiler().bind( lbl_exit );
    VERBOSE( "bind 'lbl_exit'" );
}
void CselIRToAsmJitPass::visit_epilog(
    NeqUnsignedInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// ZeroExtendInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    ZeroExtendInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    ZeroExtendInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// TruncationInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    TruncationInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Type& type = *value.getType();
    Value* res = &value;
    Value* arg = value.getValue( 0 );

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *arg, c );

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
                c.getVal2Reg()[ res ] = c.getVal2Reg()[ arg ].r8();
                VERBOSE( "%s.r8()", arg->getLabel() );
            }
            else if( type.getSize() <= 16 )
            {
                c.getVal2Reg()[ res ] = c.getVal2Reg()[ arg ].r16();
                VERBOSE( "%s.r16()", arg->getLabel() );
            }
            else if( type.getSize() <= 32 )
            {
                c.getVal2Reg()[ res ] = c.getVal2Reg()[ arg ].r32();
                VERBOSE( "%s.r32()", arg->getLabel() );
            }
            else if( type.getSize() <= 64 )
            {
                c.getVal2Reg()[ res ] = c.getVal2Reg()[ arg ].r64();
                VERBOSE( "%s.r64()", arg->getLabel() );
            }
            else
            {
                assert( not " a bit type of bit-size greater than 64-bit is unsupported for now! " );
            }
            break;
        }
        default:
        {
            libstdhl::Log::error(
                "unsupported type '%s' for 'trunc' instruction!",
                type.getDescription() );
            assert( 0 );
            break;
        }
    }
}
void CselIRToAsmJitPass::visit_epilog(
    TruncationInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// BitConstant
//

void CselIRToAsmJitPass::visit_prolog(
    BitConstant& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    alloc_reg_for_value( value, c );

    c.getCompiler().mov(
        c.getVal2Reg()[&value ], asmjit::imm( value.getValue() ) );
    VERBOSE( "mov %s, %lu", value.getLabel(), value.getValue() );
}
void CselIRToAsmJitPass::visit_epilog(
    BitConstant& value, libcsel_ir::Context& cxt )
{
}

//
// StructureConstant
//

void CselIRToAsmJitPass::visit_prolog(
    StructureConstant& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    alloc_reg_for_value( value, c );

    u32 bit_size = value.getType()->getSize();
    u32 byte_size = bit_size / 8 + ( ( bit_size % 8 ) % 2 );
    byte_size = ( byte_size == 0 ? 1 : byte_size );

    c.getCompiler().lea(
        c.getVal2Reg()[&value ], c.getCompiler().newStack( byte_size, 4 ) );
    VERBOSE( "lea %s, newStack( %u, 4 )", value.getLabel(), byte_size );

    u32 byte_offset = 0;
    for( auto v : value.getValue() )
    {
        alloc_reg_for_value( *v, c );

        c.getCompiler().mov( x86::ptr( c.getVal2Reg()[&value ], byte_offset ),
            c.getVal2Reg()[ v ] );
        VERBOSE( "mov ptr( %s, %u), %s", value.getLabel(), byte_offset,
            v->getLabel() );

        bit_size = v->getType()->getSize();
        byte_size = bit_size / 8 + ( ( bit_size % 8 ) % 2 );
        byte_offset += byte_size;
    }
}
void CselIRToAsmJitPass::visit_epilog(
    StructureConstant& value, libcsel_ir::Context& cxt )
{
}

//
// StringConstant
//

void CselIRToAsmJitPass::visit_prolog(
    StringConstant& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    StringConstant& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Interconnect
//

void CselIRToAsmJitPass::visit_prolog(
    Interconnect& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CselIRToAsmJitPass::visit_epilog(
    Interconnect& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
}

libcsel_ir::Value* CselIRToAsmJitPass::execute(
    libcsel_ir::CallInstruction& value )
{
    libcsel_rt::CselIRToAsmJitPass::Context c;

    // create Builtin/Rule asm jit
    value.getCallee().iterate( libcsel_ir::Traversal::PREORDER, this, &c );

    Context::Callable& func = c.getCallable( &value );
    func.getArgSize( -1 );

    FuncSignatureX& fsig = func.getSig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );

    c.getCompiler().addFunc( func.getSig() );
    VERBOSE( "addFunc( %s )", value.getLabel() );

    // create CallInstruction asm jit
    value.iterate( libcsel_ir::Traversal::PREORDER, this, &c );

    c.getCompiler().endFunc();
    c.getCompiler().finalize();

    void** func_ptr;
    Error err = c.getRunTime().add( &func_ptr, &c.getCodeHolder() );
    assert( not err );
    func.getPtr( func_ptr );

    typedef void ( *CallableType )( void );
    ( (CallableType)c.getCallable( &value ).getPtr() )();

    return libcsel_ir::Constant::getStructureZero( *value.getType() );
    // Bit( libcsel_ir::Type::getBit( 37 ), 37 );
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
