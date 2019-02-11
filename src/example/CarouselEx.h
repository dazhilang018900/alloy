/*
 * CascadeEx.h
 *
 *  Created on: Feb 9, 2019
 *      Author: blake
 */

#ifndef SRC_EXAMPLE_CAROUSELEX_H_
#define SRC_EXAMPLE_CAROUSELEX_H_

#include "../ui/AlloyScrollPane.h"
#include "ui/AlloyApplication.h"
namespace aly {
class CarouselEx : public aly::Application{
protected:
	ScrollPanelPtr hscroller;
	ScrollPanelPtr vscroller;
public:
	CarouselEx();
	bool init(aly::Composite& rootNode);
	virtual ~CarouselEx(){}
};

} /* namespace aly */

#endif /* SRC_EXAMPLE_CAROUSELEX_H_ */
