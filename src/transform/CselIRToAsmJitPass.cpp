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
    libstdhl::Log::info( "%s:%i: %p: %s '%s' @ %p" FMT, __FUNCTION__,          \
        __LINE__, &value, value.getName(), value.getType()->getName(), cxt,    \
        ##ARGS )

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
    Context* c = (Context*)cxt;
    FuncSignatureX& fsx = c->getFuncSignature();
    fsx.reset();
    fsx.setCallConv( CallConv::kIdHost );
    fsx.setRet( TypeId::kVoid );
}
void CselIRToAsmJitPass::visit_interlog( Intrinsic& value, void* cxt )
{
    TRACE( "" );
    Context* c = (Context*)cxt;
    c->getCompiler().addFunc( c->getFuncSignature() );
}
void CselIRToAsmJitPass::visit_epilog( Intrinsic& value, void* cxt )
{
    TRACE( "" );
    Context* c = (Context*)cxt;
    c->getCompiler().endFunc();
    c->getCompiler().finalize();

    void** fn;
    Error err = c->getRunTime().add( &fn, &c->getCodeHolder() );

    assert( not err );
}

//
// Reference
//

void CselIRToAsmJitPass::visit_prolog( Reference& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( Reference& value, void* cxt )
{
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
}

//
// ExtractInstruction
//

void CselIRToAsmJitPass::visit_prolog( ExtractInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( ExtractInstruction& value, void* cxt )
{
    TRACE( "" );
}

//
// LoadInstruction
//

void CselIRToAsmJitPass::visit_prolog( LoadInstruction& value, void* cxt )
{
    TRACE( "" );
}
void CselIRToAsmJitPass::visit_epilog( LoadInstruction& value, void* cxt )
{
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
    TRACE( "" );
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
