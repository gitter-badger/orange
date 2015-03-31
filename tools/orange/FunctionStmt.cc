/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level 
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file 
** may not be copied, modified, or distributed except according to those terms.
*/ 

#include <orange/FunctionStmt.h>
#include <orange/generator.h>

FunctionStmt::FunctionStmt(std::string name, SymTable* symtab) : Block(symtab) {
	m_name = name;
}

Value* FunctionStmt::Codegen() {
	// Push ourselves onto the stack first.
	GE::runner()->pushBlock(this);

	// Create a list of types that serve as our function's arguments
	std::vector<Type*> Args;

	// Create the function itself, and set it as our AST value.
	FunctionType* funcType = FunctionType::get(getType(), Args, m_isVarArg);
	Function* generatedFunc = Function::Create(funcType, m_linkageType, m_name, GE::module());
	m_value = generatedFunc;

	// TODO: Set argument names, if any (generatedFunc->arg_begin, setName on iterator)

	// Before generating, we need to get the current insert block so we don't lose our place.
	auto oldInsertBlock = GE::builder()->GetInsertBlock();

	// Create basic block and set our insertion point to it.
	BasicBlock *funcBody = BasicBlock::Create(getGlobalContext(), "entry", generatedFunc);
	GE::builder()->SetInsertPoint(funcBody);

	// Generate the body, which will add code to our new insert block
	generateStatements();

	// If our body has a return statement, then we don't have to do anything else here.
	// If we don't have a return statement:
	//		- Are we a main function? If we are, force return 0.
	//		- Are we an auto return? If not, throw an error about a missing return of type for function.

	// Optimize our function.
	GE::sharedEngine()->functionOptimizer()->run(*generatedFunc);

	// Finally, restore our insert block, pop our block and return our function 
	GE::builder()->SetInsertPoint(oldInsertBlock);
	GE::runner()->popBlock();
	return m_value;
}

ASTNode* FunctionStmt::clone() {
	FunctionStmt* clonedFunc = new FunctionStmt(m_name, symtab()->clone());
	for (auto stmt : m_statements) {
		clonedFunc->addStatement(stmt->clone());
	}
	return clonedFunc;
}

std::string FunctionStmt::string() {
	std::stringstream ss;

	GE::runner()->pushBlock(this);

	ss << m_name << ":" << std::endl;

	for (ASTNode *s : m_statements) {
		if (s == nullptr) continue;
		ss << "\t" << s->string() << std::endl;
	}

	GE::runner()->popBlock();
	return ss.str();
}

void FunctionStmt::resolve() {
	if (m_resolved) return; 
	
	Block::resolve();

	// If we don't exist in the parent symtab, add us as a reference.
	// If the parent doesn't exist, we're in the global block, so 
	// nothing could call is anyway.
	if (symtab()->parent() != nullptr) {
		bool added = symtab()->parent()->create(m_name, this);
		if (added == false) {
			throw std::runtime_error("Can't create function " + m_name + ". Something with that name already exists!");
		}
	}

	// Add us to our own symtab so we can do recursion.
	bool added = symtab()->create(m_name, this);
	if (added == false) {
		throw std::runtime_error("Can't create function " + m_name + ". Something with that name already exists!");
	}	
}
