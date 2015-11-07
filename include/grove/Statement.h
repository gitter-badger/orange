/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file
** may not be copied, modified, or distributed except according to those terms.
*/

#pragma once

#include "ASTNode.h"

class Library;

/**
 * Statement is the base class for elements that do not have values.
 */
class Statement : public ASTNode {
public:
	/// Register this statement in the library. Override to add behavior.
	virtual void registerInLibrary(Library* library);
};
