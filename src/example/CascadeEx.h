/*
 * CascadeEx.h
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#ifndef SRC_EXAMPLE_CASCADEEX_H_
#define SRC_EXAMPLE_CASCADEEX_H_

#include "../ui/AlloyScrollPane.h"
#include "ui/AlloyApplication.h"
namespace aly {
class CascadeEx : public aly::Application{
protected:
	ScrollPanelPtr hscroller;
	ScrollPanelPtr vscroller;
public:
	CascadeEx();
	bool init(aly::Composite& rootNode);
	virtual ~CascadeEx(){}
};

} /* namespace aly */

#endif /* SRC_EXAMPLE_CASCADEEX_H_ */
