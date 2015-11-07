/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file
** may not be copied, modified, or distributed except according to those terms.
*/

#pragma once

class Type;

namespace llvm { class Type; }

/**
 * Typed is an interface for elements that do have a type.
 */
class Typed {
protected:
	Type* m_type = nullptr;
public:
	/// Returns the current type.
	Type* getType() const;
	
	/// Returns the LLVM type. Equivalent to getType()->getLLVMType().
	llvm::Type* getLLVMType() const;
	
	/// Overrides the type.
	void setType(Type* type);
};
