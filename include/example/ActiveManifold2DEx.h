/*
 * ActiveContour2DEx.h
 *
 *  Created on: May 1, 2018
 *      Author: blake
 */

#ifndef INCLUDE_EXAMPLE_ACTIVEMANIFOLD2DEX_H_
#define INCLUDE_EXAMPLE_ACTIVEMANIFOLD2DEX_H_


#include <segmentation/ActiveManifold2D.h>
#include <segmentation/ManifoldCache2D.h>
#include "AlloyApplication.h"
#include "AlloyVector.h"
#include "AlloyWorker.h"
#include "AlloyTimeline.h"
#include "segmentation/Simulation.h"
class ActiveManifold2DEx: public aly::Application {
protected:
	float currentIso;
	int example;
	aly::Image1f gray;
	aly::ImageRGBA img;
	bool parametersDirty;
	bool frameBuffersDirty;
	bool running = false;
	aly::Image2f vecField;
	aly::Number lineWidth;
	aly::Number particleSize;
	aly::Color lineColor;
	aly::Color pointColor;
	aly::Color particleColor;
	aly::Color normalColor;
	aly::Color springlColor;
	aly::Color matchColor;
	aly::Color vecfieldColor;
	aly::AdjustableCompositePtr resizeableRegion;
	std::shared_ptr<aly::ActiveManifold2D> simulation;
	aly::IconButtonPtr playButton,stopButton;
    std::shared_ptr<aly::TimelineSlider> timelineSlider;
	aly::Image1f createCircleLevelSet(int w,int h,aly::float2 center,float r);
	void createRotationField(aly::Image2f& vecField, int w, int h);
	void createTextLevelSet(aly::Image1f& levelSet,aly::Image1f& gray,int w,int h,const std::string& text,float textSize,float maxDistance);
public:
	std::shared_ptr<aly::ManifoldCache2D> cache;
	ActiveManifold2DEx(int example=0);
	virtual void draw(aly::AlloyContext* context) override;
	bool init(aly::Composite& rootNode);

};
class ActiveContour2DEx : public ActiveManifold2DEx{
public:
	ActiveContour2DEx():ActiveManifold2DEx(0){
	}
};
class Springls2DEx : public ActiveManifold2DEx{
public:
	Springls2DEx():ActiveManifold2DEx(1){
	}
};
class SpringlsSecondOrder2DEx : public ActiveManifold2DEx{
public:
	SpringlsSecondOrder2DEx():ActiveManifold2DEx(2){
	}
};



#endif /* INCLUDE_EXAMPLE_ACTIVEMANIFOLD2DEX_H_ */
