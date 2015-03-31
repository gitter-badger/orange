/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level 
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file 
** may not be copied, modified, or distributed except according to those terms.
*/ 

#include <orange/BinOpExpr.h>
#include <orange/VarExpr.h>
#include <orange/generator.h>

bool BinOpExpr::LHSShouldNotBeNull() {
	// Right now, = can produce a LHS with a null value if the LHS is a variable 
	// that has yet to be created. 
	if (m_LHS->getClass() == "VarExpr" && m_op == "=") return false;
	return true; 
}

bool BinOpExpr::RHSShouldNotBeNull() {
	// For the moment, RHS should never be null.
	return true; 
}

bool BinOpExpr::Validate(Value* LHS, Value* RHS) {
	// First, validate that LHS and RHS exist as needed.
	if (LHSShouldNotBeNull() && LHS == nullptr) {
		throw std::runtime_error("The LHS of this expression did not generate a value!");
	}

	if (RHSShouldNotBeNull() && RHS == nullptr) {
		throw std::runtime_error("The RHS of this expression did not generate a value!");
	}

	// If we're assigning, this would mean that LHS *has* to return a pointer for us to load in.
	// If it didn't, then something is wrong. 
	if (IsAssignOp(m_op) && m_LHS->returnsPtr() == false) {
		throw std::runtime_error("LHS of this expression is not assignable.");
	}

	// The types of LHS and RHS must be compatible.
	if (LHS && RHS && CastingEngine::AreTypesCompatible(m_LHS->getType(), m_RHS->getType()) == false) {
		throw std::runtime_error("Cannot do operation.");
	}

	return true; 
}

bool BinOpExpr::IsAssignOp(std::string op) {
	return op == "=" || op == "+=" || op == "-=" || op == "/=" ||
		op == "*=";
}

bool BinOpExpr::IsCompareOp(std::string op) {
	return op == "<" || op == "<=" || op == ">" || op == ">=" ||
		op == "==" || op == "!=";
}

Instruction::BinaryOps BinOpExpr::GetBinOpFunction(Value* value1, StrElement op, Value* value2) {
	if ((value1->getType()->isFloatingPointTy() && value2->getType()->isFloatingPointTy() == false) || 
		(value2->getType()->isFloatingPointTy() && value1->getType()->isFloatingPointTy() == false)) {
		throw CompilerMessage(op, "Can't do operation with a float and non-float value.");
	} 

	bool isFPOp = value1->getType()->isFloatingPointTy() && value2->getType()->isFloatingPointTy();

	if (op == "+") {
		return isFPOp ? Instruction::FAdd : Instruction::Add;
	}

	throw CompilerMessage(op, "Unhandled operation " + op);
}


Value* BinOpExpr::Codegen() {
	// Generate the LHS side of the expression, expecting it to return a value if it needs to.
	Value* LHS = m_LHS->Codegen();
	Value* RHS = m_RHS->Codegen(); 

	Value* OrigLHS = LHS; 
	Value* OrigRHS = RHS; 

	// Validate should never return false, since it throws an exception any time it encounters 
	// something that is invalid, but let's make sure anyway.
	if (Validate(LHS, RHS) == false) {
		throw std::runtime_error("The expression is not valid!");
	}	

	// If we're assigning a variable that doesn't exist, let's create it. 
	if (m_op == "=" && LHS == nullptr) {
		VarExpr* vExpr = (VarExpr *)m_LHS; 
		vExpr->create();
		vExpr->setValue(RHS);
		return GE::builder()->CreateLoad(vExpr->getValue());
	}

	// If we're assigning, we want to cast RHS to LHS (forced).
	// Otherwise, cast them to fit.
	if (IsAssignOp(m_op)) {
		CastingEngine::CastValueToType(&RHS, m_LHS->getType(), m_LHS->isSigned(), true);
	} else {
		CastingEngine::CastValuesToFit(&LHS, &RHS, m_LHS->isSigned(), m_RHS->isSigned());
	}

	// Load the LHS if it's a pointer and isn't used as an assign. Load the RHS if it's a pointer.
	if (IsAssignOp(m_op) == false && m_LHS->returnsPtr()) LHS = GE::builder()->CreateLoad(LHS);
	if (m_RHS->returnsPtr()) RHS = GE::builder()->CreateLoad(RHS);

	if (m_op == "=") {
		GE::builder()->CreateStore(RHS, LHS);
		return GE::builder()->CreateLoad(LHS);
	} // other assign ops 

	return GE::builder()->CreateBinOp(GetBinOpFunction(LHS, m_op, RHS), LHS, RHS);
}

std::string BinOpExpr::string() {
	std::stringstream ss; 
	ss << "(" << m_LHS->string() << " " << m_op << " " << m_RHS->string() << ")";
	return ss.str();
}

Type* BinOpExpr::getType() {
	if (IsAssignOp(m_op)) {
		return m_LHS->getType(); 
	} else {
		return CastingEngine::GetFittingType(m_LHS->getType(), m_RHS->getType());
	}
}

void BinOpExpr::resolve() {
	if (m_resolved) return;
	m_resolved = true; 

	m_LHS->resolve();
	m_RHS->resolve();
}

bool BinOpExpr::isSigned() {
	return m_LHS->isSigned() || m_RHS->isSigned();
}

bool BinOpExpr::isConstant() {
	return m_LHS->isConstant() && m_RHS->isConstant();
}

BinOpExpr::BinOpExpr(Expression* LHS, StrElement op, Expression* RHS) : m_LHS(LHS), m_RHS(RHS), m_op(op) {

}



