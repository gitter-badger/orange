#include <gen/VarDeclExpr.h>
#include <gen/generator.h>
#include <gen/CastingEngine.h>

Value* VarDeclExpr::Codegen() {
	Value *v = CG::Builder.CreateAlloca(getType());

	if (value != nullptr) {
		Value *store = value->Codegen();

		if (value->getClass() == "VarExpr" || value->getClass() == "IfStatement") {
			// If it's a variable load it in. 
			store = CG::Builder.CreateLoad(store);
		}

		CastValueToType(&store, getType(), value->isSigned(), true);
		CG::Builder.CreateStore(store, v);
	}

	CG::Symtabs.top()->objs[name]->setValue(v);
	CG::Symtabs.top()->objs[name]->isLocked = true;

	return v; 
}

void VarDeclExpr::resolve() {
	if (resolved)
		return;

	if (value)
		value->resolve();

	resolved = true; 
}

VarDeclExpr::VarDeclExpr(AnyType *type, std::string *name, Expression *value) {
	if (type == nullptr) {
		std::cerr << "fatal: explicitly created variables require a type.\n";
		exit(1);
	}

	if (name == nullptr) {
		std::cerr << "fatal: trying to create a variable without a name.\n";
		exit(1); 
	}

	this->type = type; 
	this->name = *name; 
	this->value = value;

	CG::Symtabs.top()->create(this->name);
	CG::Symtabs.top()->objs[this->name]->setType(getType()->getPointerTo());
	CG::Symtabs.top()->objs[this->name]->isSigned = isSigned();
}

VarDeclExpr::~VarDeclExpr() {
	delete type;
	delete value;
}