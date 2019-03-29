/*
 * MultiWindowEx.h
 *
 *  Created on: Mar 29, 2019
 *      Author: blake
 */

#ifndef SRC_EXAMPLE_MULTIWINDOWEX_H_
#define SRC_EXAMPLE_MULTIWINDOWEX_H_
#include "ui/AlloyApplication.h"
namespace aly {
class MultiWindowEx: public aly::Application {
public:
	MultiWindowEx();
	virtual ~MultiWindowEx(){}
	bool init(aly::Composite& rootNode);
	void draw(aly::AlloyContext* context);
};

} /* namespace aly */

#endif /* SRC_EXAMPLE_MULTIWINDOWEX_H_ */
