/*
 * FluidToy.h
 *
 *  Created on: Jun 5, 2017
 *      Author: blake
 */

#ifndef INCLUDE_FLUIDSIMULATIONTOY_H_
#define INCLUDE_FLUIDSIMULATIONTOY_H_


#include <segmentation/ManifoldCache2D.h>
#include "AlloyApplication.h"
#include "AlloyVector.h"
#include "AlloyWorker.h"
#include "AlloyTimeline.h"
#include "fluid/FluidSimulation.h"
#include "segmentation/Simulation.h"
class FluidSimulationEx: public aly::Application {
protected:
	int simWidth,simHeight;
	bool parametersDirty;
	bool frameBuffersDirty;
	bool running = false;
	bool showContour=true;
	bool showPressure=true;
	bool showParticles=true;
	bool showVelocity=true;
	aly::Number lineWidth;
	aly::Color lineColor;
	aly::Color particleColor;
	aly::Color vecfieldColor;
	aly::GlyphRegionPtr glyphRegion;
	aly::AdjustableCompositePtr resizeableRegion;
	std::shared_ptr<aly::FluidSimulation> simulation;
	aly::IconButtonPtr playButton,stopButton;
    std::shared_ptr<aly::TimelineSlider> timelineSlider;
    std::shared_ptr<aly::ImageGlyph> overlayGlyph;
    int lastSimTime=-1;
public:
	std::shared_ptr<aly::ManifoldCache2D> cache;
	FluidSimulationEx(int width=64,int height=64);
	virtual void draw(aly::AlloyContext* context) override;
	bool init(aly::Composite& rootNode);

};



#endif /* INCLUDE_FLUIDSIMULATIONTOY_H_ */
