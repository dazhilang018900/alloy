/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef INCLUDE_EXAMPLE_SPRINGLS3DEX_H_
#define INCLUDE_EXAMPLE_SPRINGLS3DEX_H_


#include "segmentation/SpringLevelSet3D.h"
#include "AlloyApplication.h"
#include "AlloyWidget.h"
#include "AlloyTimeline.h"
#include "CommonShaders.h"
class Springls3DEx: public aly::Application {
protected:
	int lastTime;
	aly::Mesh isosurface;
	aly::Mesh springls;
	aly::Mesh particles;
	aly::Mesh trails;
	aly::RegionPtr renderRegion;
	aly::GLFrameBuffer springlsBuffer;
	aly::GLFrameBuffer particleBuffer;
	aly::GLFrameBuffer isosurfBuffer;
	aly::DepthAndNormalShader depthAndNormalShader;
	aly::LineShader lineShader;
	aly::MatcapShader matcapShaderIso;
	aly::MatcapShader matcapShaderParticles;
	aly::MatcapShader matcapShaderSpringls;
	aly::SpringlShader springlShader;
	aly::ImageShader imageShader;
	aly::ParticleDepthShader particleShader;
	aly::Camera camera;
	aly::SpringLevelSet3D simulation;
	aly::IconButtonPtr playButton,stopButton;
    std::shared_ptr<aly::TimelineSlider> timelineSlider;
    bool running;
    bool showParticles;
    bool showSpringls;
    bool showIsoSurface;
    bool showTracking;
    int example;
public:
	Springls3DEx(int exampleIndex);
	bool init(aly::Composite& rootNode);
	void draw(aly::AlloyContext* context);
};

class SpringlsSegmentation3DEx: public Springls3DEx {
public:
	SpringlsSegmentation3DEx():Springls3DEx(0){
	}
};


class Enright3DEx: public Springls3DEx {
public:
	Enright3DEx():Springls3DEx(1){
	}
};

#endif /* INCLUDE_EXAMPLE_SPRINGLS3DEX_H_ */
