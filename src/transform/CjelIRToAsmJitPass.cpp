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

#include "CjelIRToAsmJitPass.h"

#include <libcjel-ir/Constant>
#include <libcjel-ir/Function>
#include <libcjel-ir/Instruction>
#include <libcjel-ir/Intrinsic>
#include <libcjel-ir/Type>
#include <libcjel-ir/Value>
#include <libcjel-ir/analyze/CjelIRDumpPass>

#include <libpass/PassRegistry>

#include <libstdhl/Log>

using namespace libcjel_ir;
using namespace libcjel_rt;
using namespace asmjit;

char CjelIRToAsmJitPass::id = 0;

static libpass::PassRegistration< CjelIRToAsmJitPass > PASS( "CJEL IR to AsmJit", "TBD", 0, 0 );

bool CjelIRToAsmJitPass::run( libpass::PassResult& pr )
{
    assert( not" PPA: TODO!!! " );
    return false;
}

#if 0
#define TRACE( FMT, ARGS... )                                    \
    fprintf(                                                     \
        stderr,                                                  \
        "asmjit:%i: %p: %s | %s | '%s' | size=%lu | @ %p\n" FMT, \
        __LINE__,                                                \
        &value,                                                  \
        value.name().c_str(),                                    \
        value.label().c_str(),                                   \
        value.type().name().c_str(),                             \
        value.type().bitsize(),                                  \
        cxt,                                                     \
        ##ARGS )
#else
#define TRACE( FMT, ARGS... )
#endif

#define VERBOSE( FMT, ARGS... )      \
    fprintf(                         \
        stderr,                      \
        "[%s %s] %s = " FMT "\n",    \
        value.name().c_str(),        \
        value.type().name().c_str(), \
        value.label().c_str(),       \
        ##ARGS )

#define FIXME()                                                                     \
    {                                                                               \
        fprintf( stderr, "%s:%i: FIXME: unimplemented\n", __FUNCTION__, __LINE__ ); \
        assert( 0 );                                                                \
    }

static u32 calc_byte_size( const libcjel_ir::Type& type )
{
    switch( type.id() )
    {
        case libcjel_ir::Type::BIT:
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
                assert( not" a bit type of bit-size greater than 64-bit is unsupported for now! " );
            }
            break;
        }
        case libcjel_ir::Type::VECTOR:  // fall-through
        case libcjel_ir::Type::STRUCTURE:
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
            fprintf(
                stderr,
                "unsupported type '%s' to calculate byte_size!\n",
                type.description().c_str() );
            assert( 0 );
            return 0;
        }
    }
}

void CjelIRToAsmJitPass::alloc_reg_for_value( Value& value, Context& c )
{
    const auto& type = value.type();

    if( c.val2reg().find( &value ) != c.val2reg().end() )
    {
        // already allocated!
        return;
    }

    if( isa< Reference >( value ) )
    {
        Context::Callable& func = c.callable();

        c.val2reg()[&value ] = c.compiler().newUIntPtr( value.label().c_str() );
        VERBOSE( "newUIntPtr" );

        c.compiler().setArg( func.argsize( 1 ) - 1, c.val2reg()[&value ] );
        VERBOSE( "setArg( %u, %s )", func.argsize() - 1, value.label().c_str() );

        return;
    }

    if( isa< AllocInstruction >( value ) )
    {
        u32 byte_size = calc_byte_size( type );

        c.val2reg()[&value ] = c.compiler().newUIntPtr( value.label().c_str() );
        VERBOSE( "newUIntPtr" );

        c.compiler().lea( c.val2reg()[&value ], c.compiler().newStack( byte_size, 4 ) );
        VERBOSE( "lea %s, newStack( %u, 4 ) ;; alloc", value.label().c_str(), byte_size );

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
                VERBOSE( "mov ptr( %s, %u ), imm( 0 )", value.label().c_str(), idx );
            }

            offset += top;
        }
        return;
    }

    switch( type.id() )
    {
        case libcjel_ir::Type::BIT:
        {
            if( type.bitsize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type.bitsize() <= 8 )
            {
                c.val2reg()[&value ] = c.compiler().newU8( value.label().c_str() );
                VERBOSE( "newU8" );
            }
            else if( type.bitsize() <= 16 )
            {
                c.val2reg()[&value ] = c.compiler().newU16( value.label().c_str() );
                VERBOSE( "newU16" );
            }
            else if( type.bitsize() <= 32 )
            {
                c.val2reg()[&value ] = c.compiler().newU32( value.label().c_str() );
                VERBOSE( "newU32" );
            }
            else if( type.bitsize() <= 64 )
            {
                c.val2reg()[&value ] = c.compiler().newU64( value.label().c_str() );
                VERBOSE( "newU64" );
            }
            else
            {
                assert( not" a bit type of bit-size greater than 64-bit is unsupported for now! " );
            }
            break;
        }
        case libcjel_ir::Type::VECTOR:  // fall-through
        case libcjel_ir::Type::STRUCTURE:
        {
            c.val2reg()[&value ] = c.compiler().newUIntPtr( value.label().c_str() );
            VERBOSE( "newUIntPtr" );
            break;
        }
        default:
        {
            fprintf(
                stderr,
                "unsupported type '%s' to allocate a register!\n",
                type.description().c_str() );
            assert( 0 );
            break;
        }
    }

    // fprintf( stderr,  "alloc '%p' for '%s' of type '%s'",
    //     &c.val2reg()[&value ],
    //     value.label(),
    //     value.type().name() );

    if( isa< Constant >( value ) )
    {
        switch( value.id() )
        {
            case Value::BIT_CONSTANT:
            {
                visit_prolog(
                    static_cast< BitConstant& >( value ),
                    static_cast< libcjel_ir::Context& >( c ) );
                break;
            }
            case Value::STRUCTURE_CONSTANT:
            {
                visit_prolog(
                    static_cast< StructureConstant& >( value ),
                    static_cast< libcjel_ir::Context& >( c ) );
                break;
            }
            default:
            {
                fprintf(
                    stderr,
                    "unsupported constant value of type '%s' to allocate a "
                    "register!\n",
                    type.description().c_str() );
                assert( 0 );
                break;
            }
        }
    }
}

void CjelIRToAsmJitPass::visit_prolog( Module& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( Module& value, libcjel_ir::Context& cxt )
{
}

//
// Function
//

void CjelIRToAsmJitPass::visit_prolog( Function& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_interlog( Function& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}
void CjelIRToAsmJitPass::visit_epilog( Function& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Intrinsic
//

void CjelIRToAsmJitPass::visit_prolog( Intrinsic& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    Context::Callable& func = c.callable( &value );
    func.argsize( -1 );

    FuncSignatureX& fsig = func.funcsig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
}
void CjelIRToAsmJitPass::visit_interlog( Intrinsic& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );
    Context::Callable& func = c.callable();

    c.compiler().addFunc( func.funcsig() );
    VERBOSE( "addFunc( %s )", value.name().c_str() );

    for( auto param : value.inputs() )
    {
        assert( isa< Reference >( param ) );
        alloc_reg_for_value( *param, c );
    }

    for( auto param : value.outputs() )
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
void CjelIRToAsmJitPass::visit_epilog( Intrinsic& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    c.compiler().endFunc();
    c.compiler().finalize();

    void** func_ptr;
    Error err = c.runtime().add( &func_ptr, &c.codeholder() );

    fprintf(
        stderr,
        "asmjit: %s @ %p\n"
        "~~~{.asm}\n"
        "%s"
        "~~~\n",
        value.name().c_str(),
        func_ptr,
        c.logger().getString() );

    if( err )
    {
        fprintf( stderr, "asmjit: %s", DebugUtils::errorAsString( err ) );
        assert( 0 );
    }

    Context::Callable& func = c.callable();
    func.funcptr( func_ptr );
}

//
// Reference
//

void CjelIRToAsmJitPass::visit_prolog( Reference& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );
    Context::Callable& func = c.callable();
    FuncSignatureX& fsig = func.funcsig();
    fsig.addArg( TypeId::kUIntPtr );
}
void CjelIRToAsmJitPass::visit_epilog( Reference& value, libcjel_ir::Context& cxt )
{
}

//
// Structure
//

void CjelIRToAsmJitPass::visit_prolog( Structure& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( Structure& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Variable
//

void CjelIRToAsmJitPass::visit_prolog( Variable& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( Variable& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Memory
//

void CjelIRToAsmJitPass::visit_prolog( libcjel_ir::Memory& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( libcjel_ir::Memory& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// ParallelScope
//

void CjelIRToAsmJitPass::visit_prolog( ParallelScope& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}
void CjelIRToAsmJitPass::visit_epilog( ParallelScope& value, libcjel_ir::Context& cxt )
{
}

//
// SequentialScope
//

void CjelIRToAsmJitPass::visit_prolog( SequentialScope& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}
void CjelIRToAsmJitPass::visit_epilog( SequentialScope& value, libcjel_ir::Context& cxt )
{
}

//
// TrivialStatement
//

void CjelIRToAsmJitPass::visit_prolog( TrivialStatement& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}
void CjelIRToAsmJitPass::visit_epilog( TrivialStatement& value, libcjel_ir::Context& cxt )
{
}

//
// BranchStatement
//

void CjelIRToAsmJitPass::visit_prolog( BranchStatement& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_interlog( BranchStatement& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}
void CjelIRToAsmJitPass::visit_epilog( BranchStatement& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// LoopStatement
//

void CjelIRToAsmJitPass::visit_prolog( LoopStatement& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_interlog( LoopStatement& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}
void CjelIRToAsmJitPass::visit_epilog( LoopStatement& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// CallInstruction
//

void CjelIRToAsmJitPass::visit_prolog( CallInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    assert( c.hasCallable( value.callee().get() ) );
    Context::Callable& callee = c.callable( value.callee().get() );

    alloc_reg_for_value( value, c );

    for( auto v : value.operands() )
    {
        if( *v == *value.callee() )
        {
            continue;
        }

        alloc_reg_for_value( *v, c );
    }

    X86Gp fp = c.compiler().newIntPtr( value.callee()->label().c_str() );
    c.compiler().mov( fp, imm_ptr( callee.funcptr() ) );
    CCFuncCall* call = c.compiler().call( fp, callee.funcsig() );

    VERBOSE( "call( %s ) --> %lu", value.callee()->label().c_str(), (u64)callee.funcptr() );

    u32 i = 0;
    for( i = 1; i < value.operands().size(); i++ )
    {
        call->setArg( ( i - 1 ), c.val2reg()[ value.operands()[ i ].get() ] );
        VERBOSE( "call->setArg( %u, %s )", i - 1, value.operands()[ i ]->label().c_str() );
    }
}
void CjelIRToAsmJitPass::visit_epilog( CallInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// IdCallInstruction
//

void CjelIRToAsmJitPass::visit_prolog( IdCallInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( IdCallInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// StreamInstruction
//

void CjelIRToAsmJitPass::visit_prolog( StreamInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( StreamInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// NopInstruction
//

void CjelIRToAsmJitPass::visit_prolog( NopInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}
void CjelIRToAsmJitPass::visit_epilog( NopInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// AllocInstruction
//

void CjelIRToAsmJitPass::visit_prolog( AllocInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( AllocInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// IdInstruction
//

void CjelIRToAsmJitPass::visit_prolog( IdInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( IdInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// CastInstruction
//

void CjelIRToAsmJitPass::visit_prolog( CastInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( CastInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// ExtractInstruction
//

void CjelIRToAsmJitPass::visit_prolog( ExtractInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    auto base = value.operand( 0 );
    auto offset = value.operand( 1 );

    if( isa< Reference >( base ) and base->type().isStructure() )
    {
        assert( isa< BitConstant >( offset ) );
        BitConstant& index = static_cast< BitConstant& >( *offset );

        assert(
            index.value().value() < base->type().results().size() );  // PPA: FIXME: use real
                                                                      // operator< for: Type < u64

        u32 byte_offset = 0;
        u32 byte_size = 0;
        u32 bit_size = 0;
        for( u32 i = 0; i < index.value().value();
             i++ )  // PPA: FIXME: check if value is not greater one word!
        {
            bit_size = base->type().results()[ i ]->bitsize();
            byte_size = bit_size / 8 + ( ( bit_size % 8 ) % 2 );
            byte_offset += byte_size;
        }
        // fprintf( stderr,  "byte size = %lu, %lu", byte_size, byte_offset
        // );

        c.val2mem()[&value ] = x86::ptr( c.val2reg()[ base.get() ], byte_offset );
        VERBOSE(
            "ptr( %s, %u ) [ '%s' @ %s --> bs = %lu ]",
            base->label().c_str(),
            byte_offset,
            base->type().name().c_str(),
            index.name().c_str(),
            base->type()
                .results()[ index.value().value() ]
                ->bitsize() );  // PPA: FIXME: index access!!!
    }
    else
    {
        assert( 0 );
    }
}
void CjelIRToAsmJitPass::visit_epilog( ExtractInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// LoadInstruction
//

void CjelIRToAsmJitPass::visit_prolog( LoadInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    alloc_reg_for_value( value, c );

    auto src = value.operand( 0 ).get();

    if( isa< ExtractInstruction >( src ) )
    {
        c.compiler().mov( c.val2reg()[&value ], c.val2mem()[ src ] );
        VERBOSE( "mov %s, %s", value.label().c_str(), src->label().c_str() );
    }
    else
    {
        assert( 0 );
    }
}
void CjelIRToAsmJitPass::visit_epilog( LoadInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// StoreInstruction
//

void CjelIRToAsmJitPass::visit_prolog( StoreInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    auto src = value.operand( 0 ).get();
    auto dst = value.operand( 1 ).get();

    alloc_reg_for_value( *src, c );

    if( isa< ExtractInstruction >( dst ) )
    {
        c.compiler().mov( c.val2mem()[ dst ], c.val2reg()[ src ] );
        VERBOSE(
            "mov %s, %s (%s)", dst->label().c_str(), src->label().c_str(), src->name().c_str() );
    }
    else if( isa< Reference >( dst ) and dst->type().isBit() )
    {
        c.compiler().mov( x86::ptr( c.val2reg()[ dst ], 0 ), c.val2reg()[ src ] );
        VERBOSE(
            "mov ptr( %s ), %s (%s)",
            dst->label().c_str(),
            src->label().c_str(),
            src->name().c_str() );
    }
    else
    {
        assert( not" unsupported 'store' instruction usage! " );
    }
}
void CjelIRToAsmJitPass::visit_epilog( StoreInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// NotInstruction
//

void CjelIRToAsmJitPass::visit_prolog( NotInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    auto res = &value;
    auto lhs = value.operand( 0 ).get();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label().c_str(), lhs->label().c_str() );

    c.compiler().not_( c.val2reg()[ res ] );
    VERBOSE( "not_ %s", res->label().c_str() );
}
void CjelIRToAsmJitPass::visit_epilog( NotInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// LnotInstruction
//

void CjelIRToAsmJitPass::visit_prolog( LnotInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    auto res = &value;
    auto lhs = value.operand( 0 ).get();

    Label lbl_true = c.compiler().newLabel();
    Label lbl_exit = c.compiler().newLabel();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );

    c.compiler().cmp( c.val2reg()[ lhs ], asmjit::imm( 0 ) );
    VERBOSE( "cmp %s, imm( 0 )", lhs->label().c_str() );

    // jump if equal to true path, else cont with false path
    c.compiler().je( lbl_true );
    VERBOSE( "je 'lbl_true'" );

    // false path
    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 0 ) );
    VERBOSE( "mov %s, imm( 0 )", value.label().c_str() );

    c.compiler().jmp( lbl_exit );
    VERBOSE( "jmp 'lbl_exit'" );

    // true path
    c.compiler().bind( lbl_true );
    VERBOSE( "bind 'lbl_true'" );

    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 1 ) );
    VERBOSE( "mov %s, imm( 1 )", value.label().c_str() );

    // end if compare
    c.compiler().bind( lbl_exit );
    VERBOSE( "bind 'lbl_exit'" );
}
void CjelIRToAsmJitPass::visit_epilog( LnotInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// AndInstruction
//

void CjelIRToAsmJitPass::visit_prolog( AndInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    auto res = &value;
    auto lhs = value.operand( 0 ).get();
    auto rhs = value.operand( 1 ).get();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label().c_str(), lhs->label().c_str() );

    c.compiler().and_( c.val2reg()[ res ], c.val2reg()[ rhs ] );
    VERBOSE( "and_ %s, %s", res->label().c_str(), rhs->label().c_str() );
}
void CjelIRToAsmJitPass::visit_epilog( AndInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// OrInstruction
//

void CjelIRToAsmJitPass::visit_prolog( OrInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    const auto res = &value;
    const auto lhs = value.operand( 0 ).get();
    const auto rhs = value.operand( 1 ).get();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label().c_str(), lhs->label().c_str() );

    c.compiler().or_( c.val2reg()[ res ], c.val2reg()[ rhs ] );
    VERBOSE( "or_ %s, %s", res->label().c_str(), rhs->label().c_str() );
}
void CjelIRToAsmJitPass::visit_epilog( OrInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// XorInstruction
//

void CjelIRToAsmJitPass::visit_prolog( XorInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( XorInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// AddUnsignedInstruction
//

void CjelIRToAsmJitPass::visit_prolog( AddUnsignedInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    const auto res = &value;
    const auto lhs = value.operand( 0 ).get();
    const auto rhs = value.operand( 1 ).get();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().mov( c.val2reg()[ res ], c.val2reg()[ lhs ] );
    VERBOSE( "mov %s, %s", res->label().c_str(), lhs->label().c_str() );

    c.compiler().add( c.val2reg()[ res ], c.val2reg()[ rhs ] );
    VERBOSE( "add %s, %s", res->label().c_str(), rhs->label().c_str() );
}
void CjelIRToAsmJitPass::visit_epilog( AddUnsignedInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// AddSignedInstruction
//

void CjelIRToAsmJitPass::visit_prolog( AddSignedInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( AddSignedInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// DivSignedInstruction
//

void CjelIRToAsmJitPass::visit_prolog( DivSignedInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( DivSignedInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// ModUnsignedInstruction
//

void CjelIRToAsmJitPass::visit_prolog( ModUnsignedInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( ModUnsignedInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// EquInstruction
//

void CjelIRToAsmJitPass::visit_prolog( EquInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    const auto res = &value;
    const auto lhs = value.operand( 0 ).get();
    const auto rhs = value.operand( 1 ).get();

    Label lbl_true = c.compiler().newLabel();
    Label lbl_exit = c.compiler().newLabel();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().cmp( c.val2reg()[ lhs ], c.val2reg()[ rhs ] );
    VERBOSE( "cmp %s, %s", lhs->label().c_str(), rhs->label().c_str() );

    // jump if equal to true path, else cont with false path
    c.compiler().je( lbl_true );
    VERBOSE( "je 'lbl_true'" );

    // false path
    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 0 ) );
    VERBOSE( "mov %s, imm( 0 )", value.label().c_str() );

    c.compiler().jmp( lbl_exit );
    VERBOSE( "jmp 'lbl_exit'" );

    // true path
    c.compiler().bind( lbl_true );
    VERBOSE( "bind 'lbl_true'" );

    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 1 ) );
    VERBOSE( "mov %s, imm( 1 )", value.label().c_str() );

    // end if compare
    c.compiler().bind( lbl_exit );
    VERBOSE( "bind 'lbl_exit'" );
}
void CjelIRToAsmJitPass::visit_epilog( EquInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// NeqInstruction
//

void CjelIRToAsmJitPass::visit_prolog( NeqInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    const auto res = &value;
    const auto lhs = value.operand( 0 ).get();
    const auto rhs = value.operand( 1 ).get();

    Label lbl_true = c.compiler().newLabel();
    Label lbl_exit = c.compiler().newLabel();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *lhs, c );
    alloc_reg_for_value( *rhs, c );

    c.compiler().cmp( c.val2reg()[ lhs ], c.val2reg()[ rhs ] );
    VERBOSE( "cmp %s, %s", lhs->label().c_str(), rhs->label().c_str() );

    // jump if not equal to true path, else cont with false path
    c.compiler().jne( lbl_true );
    VERBOSE( "jne 'lbl_true'" );

    // false path
    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 0 ) );
    VERBOSE( "mov %s, imm( 0 )", value.label().c_str() );

    c.compiler().jmp( lbl_exit );
    VERBOSE( "jmp 'lbl_exit'" );

    // true path
    c.compiler().bind( lbl_true );
    VERBOSE( "bind 'lbl_true'" );

    c.compiler().mov( c.val2reg()[&value ], asmjit::imm( 1 ) );
    VERBOSE( "mov %s, imm(1)", value.label().c_str() );

    // end if compare
    c.compiler().bind( lbl_exit );
    VERBOSE( "bind 'lbl_exit'" );
}
void CjelIRToAsmJitPass::visit_epilog( NeqInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// ZeroExtendInstruction
//

void CjelIRToAsmJitPass::visit_prolog( ZeroExtendInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( ZeroExtendInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// TruncationInstruction
//

void CjelIRToAsmJitPass::visit_prolog( TruncationInstruction& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    const auto& type = value.type();
    const auto res = &value;
    const auto arg = value.operand( 0 ).get();

    alloc_reg_for_value( *res, c );
    alloc_reg_for_value( *arg, c );

    switch( type.id() )
    {
        case libcjel_ir::Type::BIT:
        {
            if( type.bitsize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type.bitsize() <= 8 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r8();
                VERBOSE( "%s.r8()", arg->label().c_str() );
            }
            else if( type.bitsize() <= 16 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r16();
                VERBOSE( "%s.r16()", arg->label().c_str() );
            }
            else if( type.bitsize() <= 32 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r32();
                VERBOSE( "%s.r32()", arg->label().c_str() );
            }
            else if( type.bitsize() <= 64 )
            {
                c.val2reg()[ res ] = c.val2reg()[ arg ].r64();
                VERBOSE( "%s.r64()", arg->label().c_str() );
            }
            else
            {
                assert( not" a bit type of bit-size greater than 64-bit is unsupported for now! " );
            }
            break;
        }
        default:
        {
            fprintf(
                stderr,
                "unsupported type '%s' for 'trunc' instruction!\n",
                type.description().c_str() );
            assert( 0 );
            break;
        }
    }
}
void CjelIRToAsmJitPass::visit_epilog( TruncationInstruction& value, libcjel_ir::Context& cxt )
{
}

//
// BitConstant
//

void CjelIRToAsmJitPass::visit_prolog( BitConstant& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    alloc_reg_for_value( value, c );

    c.compiler().mov(
        c.val2reg()[&value ],
        asmjit::imm( value.value().value() ) );  // FIXME: PPA: value access limited
                                                 // to 1 word!!!
    VERBOSE( "mov %s, imm( %s )", value.label().c_str(), value.name().c_str() );
}
void CjelIRToAsmJitPass::visit_epilog( BitConstant& value, libcjel_ir::Context& cxt )
{
}

//
// StructureConstant
//

void CjelIRToAsmJitPass::visit_prolog( StructureConstant& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    Context& c = static_cast< Context& >( cxt );

    alloc_reg_for_value( value, c );

    u64 byte_size = value.type().wordsize();
    u64 byte_offset = 0;

    c.compiler().lea( c.val2reg()[&value ], c.compiler().newStack( byte_size, 4 ) );
    VERBOSE( "lea %s, newStack( %lu, 4 )", value.label().c_str(), byte_size );

    for( std::size_t i = 0; i < value.value().size(); i++ )
    {
        alloc_reg_for_value( value.value()[ i ], c );

        c.compiler().mov(
            x86::ptr( c.val2reg()[&value ], byte_offset ), c.val2reg()[&value.value()[ i ] ] );
        VERBOSE(
            "mov ptr( %s, %lu ), %s",
            value.label().c_str(),
            byte_offset,
            value.value()[ i ].label().c_str() );

        byte_offset += value.value()[ i ].type().wordsize();
    }
}
void CjelIRToAsmJitPass::visit_epilog( StructureConstant& value, libcjel_ir::Context& cxt )
{
}

//
// StringConstant
//

void CjelIRToAsmJitPass::visit_prolog( StringConstant& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( StringConstant& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// Interconnect
//

void CjelIRToAsmJitPass::visit_prolog( Interconnect& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
    FIXME();
}
void CjelIRToAsmJitPass::visit_epilog( Interconnect& value, libcjel_ir::Context& cxt )
{
    TRACE( "" );
}

//
// JiT
//

libcjel_ir::Constant CjelIRToAsmJitPass::execute(
    libcjel_ir::OperatorInstruction& value, Context& c )
{
    libcjel_ir::CjelIRDumpPass dump;

    c.reset();

    Context::Callable& func = c.callable( &value );
    func.argsize( -1 );

    FuncSignatureX& fsig = func.funcsig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
    fsig.addArg( TypeId::kUIntPtr );

    c.compiler().addFunc( func.funcsig() );
    VERBOSE( "addFunc( %s )", value.name().c_str() );

    X86Gp out = c.compiler().newUIntPtr( "out" );
    c.compiler().setArg( 0, out );
    VERBOSE( "setArg( %u, %s )", 0, "out" );

    assert( value.operands().size() <= 2 );
    assert( libcjel_ir::isa< libcjel_ir::Constant >( value.operand( 0 ) ) );
    if( value.operands().size() > 1 )
    {
        assert( libcjel_ir::isa< libcjel_ir::Constant >( value.operand( 1 ) ) );
    }

    value.iterate( libcjel_ir::Traversal::PREORDER, this, &c );
    value.iterate( libcjel_ir::Traversal::PREORDER, &dump );

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
        fprintf( stderr, "asmjit: %s", DebugUtils::errorAsString( err ) );
        assert( 0 );
    }

    func.funcptr( func_ptr );

    fprintf(
        stderr,
        "asmjit: %s @ %p\n"
        "~~~{.asm}\n"
        "%s"
        "~~~\n",
        value.name().c_str(),
        func_ptr,
        c.logger().getString() );

    u8 b[ 10 ];
    for( u32 i = 0; i < 10; i++ )
    {
        b[ i ] = 0xff;
    }

    printf(
        "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        b[ 0 ],
        b[ 1 ],
        b[ 2 ],
        b[ 3 ],
        b[ 4 ],
        b[ 5 ],
        b[ 6 ],
        b[ 7 ],
        b[ 8 ],
        b[ 9 ] );

    printf( "calling: %p\n", c.callable( &value ).funcptr() );
    typedef void ( *CallableType )( void* );
    ( (CallableType)c.callable( &value ).funcptr() )( &b );

    printf(
        "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        b[ 0 ],
        b[ 1 ],
        b[ 2 ],
        b[ 3 ],
        b[ 4 ],
        b[ 5 ],
        b[ 6 ],
        b[ 7 ],
        b[ 8 ],
        b[ 9 ] );

    assert( value.type().isBit() );
    assert( value.type().bitsize() <= 8 );

    return libcjel_ir::BitConstant(
        std::static_pointer_cast< libcjel_ir::BitType >( value.ptr_type() ), b[ 0 ] );
}

libcjel_ir::Constant CjelIRToAsmJitPass::execute( libcjel_ir::CallInstruction& value, Context& c )
{
    libcjel_ir::CjelIRDumpPass dump;

    // create Builtin/Rule asm jit
    c.reset();

    value.callee()->iterate( libcjel_ir::Traversal::PREORDER, &dump );
    value.callee()->iterate( libcjel_ir::Traversal::PREORDER, this, &c );

    // create CallInstruction asm jit
    c.reset();

    value.iterate( libcjel_ir::Traversal::PREORDER, &dump );

    Context::Callable& func = c.callable( &value );
    func.argsize( -1 );

    FuncSignatureX& fsig = func.funcsig();
    fsig.init( CallConv::kIdHost, TypeId::kVoid, fsig._builderArgList, 0 );
    fsig.addArg( TypeId::kUIntPtr );

    c.compiler().addFunc( func.funcsig() );
    VERBOSE( "addFunc( %s )", value.name().c_str() );

    // PPA: check if output type matches !!!, maybe we need more
    // registers/pointers here!!! as args I mean
    X86Gp out = c.compiler().newUIntPtr( "out" );
    c.compiler().setArg( 0, out );
    VERBOSE( "setArg( %u, %s )", 0, "out" );

    value.iterate( libcjel_ir::Traversal::PREORDER, this, &c );

    X86Gp tmp = c.compiler().newU8( "tmp" );
    for( auto v : value.operands() )
    {
        if( auto res = cast< libcjel_ir::AllocInstruction >( v ) )
        {
            if( res->type().isBit() )
            {
                u32 top = calc_byte_size( res->type() );
                for( u32 idx = 0; idx < top; idx++ )
                {
                    c.compiler().mov(
                        tmp, x86::ptr( c.val2reg()[ (libcjel_ir::Value*)res ], idx ) );
                    c.compiler().mov( x86::ptr( out, idx ), tmp );

                    VERBOSE(
                        "mov( ptr( out, %u ), ptr( %s, %u ) )", idx, res->label().c_str(), idx );
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
                            tmp, x86::ptr( c.val2reg()[ (libcjel_ir::Value*)res ], idx ) );
                        c.compiler().mov( x86::ptr( out, idx ), tmp );

                        VERBOSE(
                            "mov( ptr( out, %u ), ptr( %s, %u ) )",
                            idx,
                            res->label().c_str(),
                            idx );
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

    fprintf(
        stderr,
        "asmjit: %s @ %p, calling %p\n"
        "~~~{.asm}\n"
        "%s"
        "~~~\n",
        value.name().c_str(),
        func_ptr,
        c.callable( value.callee().get() ).funcptr(),
        c.logger().getString() );

    if( err )
    {
        fprintf( stderr, "asmjit: %s\n", DebugUtils::errorAsString( err ) );
        assert( 0 );
    }

    func.funcptr( func_ptr );

    u8 b[ 10 ];
    for( u32 i = 0; i < 10; i++ )
    {
        b[ i ] = 0xff;
    }

    fprintf(
        stderr,
        "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        b[ 0 ],
        b[ 1 ],
        b[ 2 ],
        b[ 3 ],
        b[ 4 ],
        b[ 5 ],
        b[ 6 ],
        b[ 7 ],
        b[ 8 ],
        b[ 9 ] );

    fprintf( stderr, "calling: %p\n", c.callable( &value ).funcptr() );
    typedef void ( *CallableType )( void* );
    ( (CallableType)c.callable( &value ).funcptr() )( b );

    fprintf(
        stderr,
        "b: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        b[ 0 ],
        b[ 1 ],
        b[ 2 ],
        b[ 3 ],
        b[ 4 ],
        b[ 5 ],
        b[ 6 ],
        b[ 7 ],
        b[ 8 ],
        b[ 9 ] );

    const auto type = value.ptr_type();

    switch( type->id() )
    {
        case libcjel_ir::Type::BIT:
        {
            const auto ctype = std::static_pointer_cast< libcjel_ir::BitType >( type );

            if( type->bitsize() < 1 )
            {
                assert( not" bit type has invalid bit-size of '0' " );
            }
            else if( type->bitsize() <= 8 )
            {
                return libcjel_ir::BitConstant( ctype, b[ 0 ] );
            }
            else if( type->bitsize() <= 16 )
            {
                return libcjel_ir::BitConstant( ctype, ( u16 )( *&b[ 0 ] ) );
            }
            else if( type->bitsize() <= 32 )
            {
                return libcjel_ir::BitConstant( ctype, ( u32 )( *&b[ 0 ] ) );
            }
            else if( type->bitsize() <= 64 )
            {
                return libcjel_ir::BitConstant( ctype, ( u64 )( *&b[ 0 ] ) );
            }
            else
            {
                assert( not" a bit type of bit-size greater than 64-bit is unsupported for now! " );
            }
            break;
        }
        case libcjel_ir::Type::STRUCTURE:
        {
            assert(
                value.type().results().size() == 2 and value.type().results()[ 0 ]->isBit() and
                value.type().results()[ 0 ]->bitsize() <= 8 and
                *value.type().results()[ 0 ] == *value.type().results()[ 1 ] );

            const auto ctv =
                std::static_pointer_cast< libcjel_ir::StructureType >( value.ptr_type() );

            const auto ct0 =
                std::static_pointer_cast< libcjel_ir::BitType >( value.type().ptr_results()[ 0 ] );

            const auto ct1 =
                std::static_pointer_cast< libcjel_ir::BitType >( value.type().ptr_results()[ 1 ] );

            return libcjel_ir::StructureConstant(
                ctv, { BitConstant( ct0, b[ 0 ] ), BitConstant( ct1, b[ 1 ] ) } );
        }
        default:
        {
            fprintf( stderr, "unsupported value '%s' to return\n", value.description().c_str() );

            assert( 0 );
            return libcjel_ir::VoidConstant();
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
