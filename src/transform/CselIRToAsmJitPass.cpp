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
#include "../stdhl/cpp/Log.h"
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
        &value, value.name(), value.label(), value.type().name(),              \
        value.type().bitsize(), cxt, ##ARGS )
#else
#define TRACE( FMT, ARGS... )
#endif

#define VERBOSE( FMT, ARGS... )                                                \
    libstdhl::Log::info( "[%s %s] %s = " FMT, value.name(),                    \
        value.type().name(), value.label(), ##ARGS )

#define FIXME()                                                                \
    {                                                                          \
        libstdhl::Log::error(                                                  \
            "%s:%i: FIXME: unimplemented", __FUNCTION__, __LINE__ );           \
        assert( 0 );                                                           \
    }

static u32 calc_byte_size( libcsel_ir::Type& type )
{
    switch( type.id() )
    {
        case Type::BIT:
        {
            if( type.bitsize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type.bitsize() <= 8 )
            {
                return 1;
            }
            else if( type.bitsize() <= 16 )
            {
                return 2;
            }
            else if( type.bitsize() <= 32 )
            {
                return 4;
            }
            else if( type.bitsize() <= 64 )
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
            for( auto t : type.results() )
            {
                byte_size += calc_byte_size( *t );
            }
            return byte_size;
        }
        default:
        {
            libstdhl::Log::error(
                "unsupported type '%s' to calculate byte_size!",
                type.description() );
            assert( 0 );
            return 0;
        }
    }
}

void CselIRToAsmJitPass::alloc_reg_for_value( Value& value, Context& c )
{
    Type& type = value.type();

    if( c.val2reg().find( &value ) != c.val2reg().end() )
    {
        // already allocated!
        return;
    }

    if( isa< Reference >( value ) )
    {
        Context::Callable& func = c.callable();

        c.val2reg()[&value ] = c.compiler().newUIntPtr( value.label() );
        VERBOSE( "newUIntPtr" );

        c.compiler().setArg( func.argsize( 1 ) - 1, c.val2reg()[&value ] );
        VERBOSE( "setArg( %u, %s )", func.argsize() - 1, value.label() );

        return;
    }

    if( isa< AllocInstruction >( value ) )
    {
        u32 byte_size = calc_byte_size( type );

        c.val2reg()[&value ] = c.compiler().newUIntPtr( value.label() );
        VERBOSE( "newUIntPtr" );

        c.compiler().lea(
            c.val2reg()[&value ], c.compiler().newStack( byte_size, 4 ) );
        VERBOSE(
            "lea %s, newStack( %u, 4 ) ;; alloc", value.label(), byte_size );

        X86Gp tmp = c.compiler().newU8( "tmp" );
        c.compiler().mov( tmp, asmjit::imm( 0 ) );

        u32 offset = 0;
        for( auto element : value.type().results() )
        {
            assert( element->isBit() );

            u32 top = calc_byte_size( *element );
            u32 idx = 0;
            for( u32 i = 0; i < top; i++ )
            {
                idx = i + offset;
                c.compiler().mov( x86::ptr( c.val2reg()[&value ], idx ), tmp );
                VERBOSE( "mov ptr( %s, %lu ), imm( 0 )", value.label(), idx );
            }

            offset += top;
        }
        return;
    }

    switch( type.id() )
    {
        case Type::BIT:
        {
            if( type.bitsize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type.bitsize() <= 8 )
            {
                c.val2reg()[&value ] = c.compiler().newU8( value.label() );
                VERBOSE( "newU8" );
            }
            else if( type.bitsize() <= 16 )
            {
                c.val2reg()[&value ] = c.compiler().newU16( value.label() );
                VERBOSE( "newU16" );
            }
            else if( type.bitsize() <= 32 )
            {
                c.val2reg()[&value ] = c.compiler().newU32( value.label() );
                VERBOSE( "newU32" );
            }
            else if( type.bitsize() <= 64 )
            {
                c.val2reg()[&value ] = c.compiler().newU64( value.label() );
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
            c.val2reg()[&value ] = c.compiler().newUIntPtr( value.label() );
            VERBOSE( "newUIntPtr" );
            break;
        }
        default:
        {
            libstdhl::Log::error(
                "unsupported type '%s' to allocate a register!",
                type.description() );
            assert( 0 );
            break;
        }
    }

    // libstdhl::Log::info( "alloc '%p' for '%s' of type '%s'",
    //     &c.val2reg()[&value ],
    //     value.label(),
    //     value.type().name() );

    if( isa< Constant >( value ) )
    {
        switch( value.id() )
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
                    type.description() );
                assert( 0 );
                break;
            }
        }
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

    Context::Callable& func = c.callable( &value );
    func.argsize( -1 );

    FuncSignatureX& fsig = func.funcsig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
}
void CselIRToAsmJitPass::visit_interlog(
    Intrinsic& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );
    Context::Callable& func = c.callable();

    c.compiler().addFunc( func.funcsig() );
    VERBOSE( "addFunc( %s )", value.name() );

    for( auto param : value.inParameters() )
    {
        assert( isa< Reference >( param ) );
        alloc_reg_for_value( *param, c );
    }

    for( auto param : value.outParameters() )
    {
        assert( isa< Reference >( param ) );
        alloc_reg_for_value( *param, c );
    }

    for( auto param : value.linkage() )
    {
        assert( param and 0 );
    }

    assert( func.funcsig().getArgCount() == func.argsize() );
}
void CselIRToAsmJitPass::visit_epilog(
    Intrinsic& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    c.compiler().endFunc();
    c.compiler().finalize();

    void** func_ptr;
    Error err = c.runtime().add( &func_ptr, &c.codeholder() );

    fprintf( stderr,
        "asmjit: %s @ %p\n"
        "~~~{.asm}\n"
        "%s"
        "~~~\n",
        value.name(), func_ptr, c.logger().getString() );

    if( err )
    {
        libstdhl::Log::error( "asmjit: %s", DebugUtils::errorAsString( err ) );
        assert( 0 );
    }

    Context::Callable& func = c.callable();
    func.funcptr( func_ptr );
}

//
// Reference
//

void CselIRToAsmJitPass::visit_prolog(
    Reference& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );
    Context::Callable& func = c.callable();
    FuncSignatureX& fsig = func.funcsig();
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

    assert( c.hasCallable( value.callee() ) );
    Context::Callable& callee = c.callable( &value.callee() );

    alloc_reg_for_value( value, c );

    for( auto v : value.values() )
    {
        if( v == &value.callee() )
        {
            continue;
        }

        alloc_reg_for_value( *v, c );
    }

    X86Gp fp = c.compiler().newIntPtr( value.callee().label() );
    c.compiler().mov( fp, imm_ptr( callee.funcptr() ) );
    CCFuncCall* call = c.compiler().call( fp, callee.funcsig() );

    VERBOSE(
        "call( %s ) --> %lu", value.callee().label(), (u64)callee.funcptr() );

    u32 i = 0;
    for( i = 1; i < value.values().size(); i++ )
    {
        call->setArg( ( i - 1 ), c.val2reg()[ value.values()[ i ] ] );
        VERBOSE(
            "call->setArg( %u, %s )", i - 1, value.values()[ i ]->label() );
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

    Value* base = value.value( 0 );
    Value* offset = value.value( 1 );

    if( isa< Reference >( base ) and base->type().isStructure() )
    {
        assert( isa< BitConstant >( offset ) );
        BitConstant& index = static_cast< BitConstant& >( *offset );

        assert( index.value() < base->type().results().size() );

        u32 byte_offset = 0;
        u32 byte_size = 0;
        u32 bit_size = 0;
        for( u32 i = 0; i < index.value(); i++ )
        {
            bit_size = base->type().results()[ i ]->bitsize();
            byte_size = bit_size / 8 + ( ( bit_size % 8 ) % 2 );
            byte_offset += byte_size;
        }
        // libstdhl::Log::info( "byte size = %lu, %lu", byte_size, byte_offset
        // );

        c.val2mem()[&value ] = x86::ptr( c.val2reg()[ base ], byte_offset );
        VERBOSE( "ptr( %s, %lu ) [ '%s' @ %lu --> bs = %lu ]", base->label(),
            byte_offset, base->type().name(), index.value(),
            base->type().results()[ index.value() ]->bitsize() );
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

    Value* src = value.value( 0 );

    if( isa< ExtractInstruction >( src ) )
    {
        c.compiler().mov( c.val2reg()[&value ], c.val2mem()[ src ] );
        VERBOSE( "mov %s, %s", value.label(), src->label() );
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

    Value* src = value.value( 0 );
    Value* dst = value.value( 1 );

    alloc_reg_for_value( *src, c );

    if( isa< ExtractInstruction >( dst ) )
    {
        c.compiler().mov( c.val2mem()[ dst ], c.val2reg()[ src ] );
        VERBOSE( "mov %s, %s (%s)", dst->label(), src->label(), src->name() );
    }
    else if( isa< Reference >( dst ) and dst->type().isBit() )
    {
        c.compiler().mov(
            x86::ptr( c.val2reg()[ dst ], 0 ), c.val2reg()[ src ] );
        VERBOSE(
            "mov ptr( %s ), %s (%s)", dst->label(), src->label(), src->name() );
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
    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.value( 0 );

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label(), lhs->label() );

    c.compiler().not_( c.val2reg()[ res ] );
    VERBOSE( "not_ %s", res->label() );
}
void CselIRToAsmJitPass::visit_epilog(
    NotInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// LnotInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    LnotInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.value( 0 );

    Label lbl_true = c.compiler().newLabel();
    Label lbl_exit = c.compiler().newLabel();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );

    c.compiler().cmp( c.val2reg()[ lhs ], asmjit::imm( 0 ) );
    VERBOSE( "cmp %s, imm( 0 )", lhs->label() );

    // jump if equal to true path, else cont with false path
    c.compiler().je( lbl_true );
    VERBOSE( "je 'lbl_true'" );

    // false path
    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 0 ) );
    VERBOSE( "mov %s, imm( 0 )", value.label() );

    c.compiler().jmp( lbl_exit );
    VERBOSE( "jmp 'lbl_exit'" );

    // true path
    c.compiler().bind( lbl_true );
    VERBOSE( "bind 'lbl_true'" );

    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 1 ) );
    VERBOSE( "mov %s, imm( 1 )", value.label() );

    // end if compare
    c.compiler().bind( lbl_exit );
    VERBOSE( "bind 'lbl_exit'" );
}
void CselIRToAsmJitPass::visit_epilog(
    LnotInstruction& value, libcsel_ir::Context& cxt )
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
    Value* lhs = value.value( 0 );
    Value* rhs = value.value( 1 );

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label(), lhs->label() );

    c.compiler().and_( c.val2reg()[ res ], c.val2reg()[ rhs ] );
    VERBOSE( "and_ %s, %s", res->label(), rhs->label() );
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
    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.value( 0 );
    Value* rhs = value.value( 1 );

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label(), lhs->label() );

    c.compiler().or_( c.val2reg()[ res ], c.val2reg()[ rhs ] );
    VERBOSE( "or_ %s, %s", res->label(), rhs->label() );
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
// AddUnsignedInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    AddUnsignedInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.value( 0 );
    Value* rhs = value.value( 1 );

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label(), lhs->label() );

    c.compiler().add( c.val2reg()[ res ], c.val2reg()[ rhs ] );
    VERBOSE( "add %s, %s", res->label(), rhs->label() );
}
void CselIRToAsmJitPass::visit_epilog(
    AddUnsignedInstruction& value, libcsel_ir::Context& cxt )
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
// EquInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    EquInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.value( 0 );
    Value* rhs = value.value( 1 );

    Label lbl_true = c.compiler().newLabel();
    Label lbl_exit = c.compiler().newLabel();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().cmp( c.val2reg()[ lhs ], c.val2reg()[ rhs ] );
    VERBOSE( "cmp %s, %s", lhs->label(), rhs->label() );

    // jump if equal to true path, else cont with false path
    c.compiler().je( lbl_true );
    VERBOSE( "je 'lbl_true'" );

    // false path
    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 0 ) );
    VERBOSE( "mov %s, imm( 0 )", value.label() );

    c.compiler().jmp( lbl_exit );
    VERBOSE( "jmp 'lbl_exit'" );

    // true path
    c.compiler().bind( lbl_true );
    VERBOSE( "bind 'lbl_true'" );

    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 1 ) );
    VERBOSE( "mov %s, imm( 1 )", value.label() );

    // end if compare
    c.compiler().bind( lbl_exit );
    VERBOSE( "bind 'lbl_exit'" );
}
void CselIRToAsmJitPass::visit_epilog(
    EquInstruction& value, libcsel_ir::Context& cxt )
{
}

//
// NeqInstruction
//

void CselIRToAsmJitPass::visit_prolog(
    NeqInstruction& value, libcsel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Value* res = &value;
    Value* lhs = value.value( 0 );
    Value* rhs = value.value( 1 );

    Label lbl_true = c.compiler().newLabel();
    Label lbl_exit = c.compiler().newLabel();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().cmp( c.val2reg()[ lhs ], c.val2reg()[ rhs ] );
    VERBOSE( "cmp %s, %s", lhs->label(), rhs->label() );

    // jump if not equal to true path, else cont with false path
    c.compiler().jne( lbl_true );
    VERBOSE( "jne 'lbl_true'" );

    // false path
    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 0 ) );
    VERBOSE( "mov %s, imm( 0 )", value.label() );

    c.compiler().jmp( lbl_exit );
    VERBOSE( "jmp 'lbl_exit'" );

    // true path
    c.compiler().bind( lbl_true );
    VERBOSE( "bind 'lbl_true'" );

    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 1 ) );
    VERBOSE( "mov %s, imm(1)", value.label() );

    // end if compare
    c.compiler().bind( lbl_exit );
    VERBOSE( "bind 'lbl_exit'" );
}
void CselIRToAsmJitPass::visit_epilog(
    NeqInstruction& value, libcsel_ir::Context& cxt )
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

    Type& type = value.type();
    Value* res = &value;
    Value* arg = value.value( 0 );

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *arg, c );

    switch( type.id() )
    {
        case Type::BIT:
        {
            if( type.bitsize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type.bitsize() <= 8 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r8();
                VERBOSE( "%s.r8()", arg->label() );
            }
            else if( type.bitsize() <= 16 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r16();
                VERBOSE( "%s.r16()", arg->label() );
            }
            else if( type.bitsize() <= 32 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r32();
                VERBOSE( "%s.r32()", arg->label() );
            }
            else if( type.bitsize() <= 64 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r64();
                VERBOSE( "%s.r64()", arg->label() );
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
                type.description() );
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

    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( value.value() ) );
    VERBOSE( "mov %s, imm( %lu ) [0x%x]", value.label(), value.value(),
        value.value() );
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

    u32 bit_size = value.type().bitsize();
    u32 byte_size = bit_size / 8 + ( ( bit_size % 8 ) % 2 );
    byte_size = ( byte_size == 0 ? 1 : byte_size );

    c.compiler().lea(
        c.val2reg()[&value ], c.compiler().newStack( byte_size, 4 ) );
    VERBOSE( "lea %s, newStack( %u, 4 )", value.label(), byte_size );

    u32 byte_offset = 0;
    for( auto v : value.value() )
    {
        alloc_reg_for_value( *v, c );

        c.compiler().mov(
            x86::ptr( c.val2reg()[&value ], byte_offset ), c.val2reg()[ v ] );
        VERBOSE(
            "mov ptr( %s, %u), %s", value.label(), byte_offset, v->label() );

        bit_size = v->type().bitsize();
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

//
// JiT
//

libcsel_ir::Value CselIRToAsmJitPass::execute(
    libcsel_ir::OperatorInstruction& value, Context& c )
{
    libcsel_ir::CselIRDumpPass dump;

    c.reset();

    Context::Callable& func = c.callable( &value );
    func.argsize( -1 );

    FuncSignatureX& fsig = func.funcsig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
    fsig.addArg( TypeId::kUIntPtr );

    c.compiler().addFunc( func.funcsig() );
    VERBOSE( "addFunc( %s )", value.name() );

    X86Gp out = c.compiler().newUIntPtr( "out" );
    c.compiler().setArg( 0, out );
    VERBOSE( "setArg( %u, %s )", 0, "out" );

    assert( value.values().size() <= 2 );
    assert( libcsel_ir::isa< libcsel_ir::Constant >( value.values()[ 0 ] ) );
    if( value.values().size() > 1 )
    {
        assert(
            libcsel_ir::isa< libcsel_ir::Constant >( value.values()[ 1 ] ) );
    }

    value.iterate( libcsel_ir::Traversal::PREORDER, this, &c );
    value.iterate( libcsel_ir::Traversal::PREORDER, &dump );

    u32 byte_size = calc_byte_size( value.type() );

    X86Gp tmp = c.compiler().newU8( "tmp" );
    for( u32 i = 0; i < byte_size; i++ )
    {
        c.compiler().mov( tmp, c.val2reg()[&value ] );
        c.compiler().mov( x86::ptr( out, i ), tmp );
    }

    c.compiler().endFunc();
    c.compiler().finalize();

    void** func_ptr;
    Error err = c.runtime().add( &func_ptr, &c.codeholder() );
    if( err )
    {
        libstdhl::Log::error( "asmjit: %s", DebugUtils::errorAsString( err ) );
        assert( 0 );
    }

    func.funcptr( func_ptr );

    fprintf( stderr,
        "asmjit: %s @ %p\n"
        "~~~{.asm}\n"
        "%s"
        "~~~\n",
        value.name(), func_ptr, c.logger().getString() );

    u8 b[ 10 ];
    for( u32 i = 0; i < 10; i++ )
    {
        b[ i ] = 0xff;
    }

    printf( "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", b[ 0 ],
        b[ 1 ], b[ 2 ], b[ 3 ], b[ 4 ], b[ 5 ], b[ 6 ], b[ 7 ], b[ 8 ],
        b[ 9 ] );

    printf( "calling: %p\n", c.callable( &value ).funcptr() );
    typedef void ( *CallableType )( void* );
    ( (CallableType)c.callable( &value ).funcptr() )( &b );

    printf( "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", b[ 0 ],
        b[ 1 ], b[ 2 ], b[ 3 ], b[ 4 ], b[ 5 ], b[ 6 ], b[ 7 ], b[ 8 ],
        b[ 9 ] );

    assert( value.type().isBit() );
    assert( value.type().bitsize() <= 8 );

    return libcsel_ir::BitConstant( &value.type(), b[ 0 ] );
}

libcsel_ir::Value CselIRToAsmJitPass::execute(
    libcsel_ir::CallInstruction& value, Context& c )
{
    libcsel_ir::CselIRDumpPass dump;

    // create Builtin/Rule asm jit
    c.reset();

    value.callee().iterate( libcsel_ir::Traversal::PREORDER, &dump );
    value.callee().iterate( libcsel_ir::Traversal::PREORDER, this, &c );

    // create CallInstruction asm jit
    c.reset();

    value.iterate( libcsel_ir::Traversal::PREORDER, &dump );

    Context::Callable& func = c.callable( &value );
    func.argsize( -1 );

    FuncSignatureX& fsig = func.funcsig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
    fsig.addArg( TypeId::kUIntPtr );

    c.compiler().addFunc( func.funcsig() );
    VERBOSE( "addFunc( %s )", value.name() );

    // PPA: check if output type matches !!!, maybe we need more
    // registers/pointers here!!! as args I mean
    X86Gp out = c.compiler().newUIntPtr( "out" );
    c.compiler().setArg( 0, out );
    VERBOSE( "setArg( %u, %s )", 0, "out" );

    value.iterate( libcsel_ir::Traversal::PREORDER, this, &c );

    X86Gp tmp = c.compiler().newU8( "tmp" );
    for( auto v : value.values() )
    {
        if( auto res = cast< libcsel_ir::AllocInstruction >( v ) )
        {
            if( res->type().isBit() )
            {
                u32 top = calc_byte_size( res->type() );
                for( u32 idx = 0; idx < top; idx++ )
                {
                    c.compiler().mov(
                        tmp, x86::ptr( c.val2reg()[ res ], idx ) );
                    c.compiler().mov( x86::ptr( out, idx ), tmp );

                    VERBOSE( "mov( ptr( out, %lu ), ptr( %s, %lu ) )", idx,
                        res->label(), idx );
                }
            }
            else if( res->type().isStructure() )
            {
                u32 offset = 0;

                for( auto element : res->type().results() )
                {
                    assert( element->isBit() );

                    u32 top = calc_byte_size( *element );
                    u32 idx = 0;
                    for( u32 i = 0; i < top; i++ )
                    {
                        idx = i + offset;

                        c.compiler().mov(
                            tmp, x86::ptr( c.val2reg()[ res ], idx ) );
                        c.compiler().mov( x86::ptr( out, idx ), tmp );

                        VERBOSE( "mov( ptr( out, %lu ), ptr( %s, %lu ) )", idx,
                            res->label(), idx );
                    }

                    offset += top;
                }
            }
            else
            {
                assert( !"UNIMPLEMENTED!" );
            }
        }
    }

    c.compiler().endFunc();
    c.compiler().finalize();

    void** func_ptr;
    Error err = c.runtime().add( &func_ptr, &c.codeholder() );

    fprintf( stderr,
        "asmjit: %s @ %p, calling %p\n"
        "~~~{.asm}\n"
        "%s"
        "~~~\n",
        value.name(), func_ptr, c.callable( &value.callee() ).funcptr(),
        c.logger().getString() );

    if( err )
    {
        libstdhl::Log::error( "asmjit: %s", DebugUtils::errorAsString( err ) );
        assert( 0 );
    }

    func.funcptr( func_ptr );

    u8 b[ 10 ];
    for( u32 i = 0; i < 10; i++ )
    {
        b[ i ] = 0xff;
    }

    libstdhl::Log::info( "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
        b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ], b[ 4 ], b[ 5 ], b[ 6 ], b[ 7 ], b[ 8 ],
        b[ 9 ] );

    libstdhl::Log::info( "calling: %p", c.callable( &value ).funcptr() );
    typedef void ( *CallableType )( void* );
    ( (CallableType)c.callable( &value ).funcptr() )( b );

    libstdhl::Log::info( "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
        b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ], b[ 4 ], b[ 5 ], b[ 6 ], b[ 7 ], b[ 8 ],
        b[ 9 ] );

    Type& type = value.type();

    switch( type.id() )
    {
        case Type::BIT:
        {
            if( type.bitsize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type.bitsize() <= 8 )
            {
                return libcsel_ir::BitConstant( &type, b[ 0 ] );
            }
            else if( type.bitsize() <= 16 )
            {
                return libcsel_ir::BitConstant( &type, (u16)b[ 0 ] );
            }
            else if( type.bitsize() <= 32 )
            {
                return libcsel_ir::BitConstant( &type, (u32)b[ 0 ] );
            }
            else if( type.bitsize() <= 64 )
            {
                return libcsel_ir::BitConstant( &type, (u64)b[ 0 ] );
            }
            else
            {
                assert( not " a bit type of bit-size greater than 64-bit is unsupported for now! " );
            }
            break;
        }
        case Type::STRUCTURE:
        {
            assert( value.type().results().size() == 2
                    and value.type().results()[ 0 ]->isBit()
                    and value.type().results()[ 0 ]->bitsize() <= 8
                    and *value.type().results()[ 0 ]
                            == *value.type().results()[ 1 ] );

            return libcsel_ir::StructureConstant( value.type(),
                { new libcsel_ir::BitConstant(
                      value.type().results()[ 0 ], b[ 0 ] ),
                    new libcsel_ir::BitConstant(
                        value.type().results()[ 1 ], b[ 1 ] ) } );
        }
        default:
        {
            libstdhl::Log::error(
                "unsupported value '%s' to return", value.c_str() );

            assert( 0 );
            return libcsel_ir::VoidConstant();
        }
    }
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
