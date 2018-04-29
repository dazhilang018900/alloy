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

#ifndef INCLUDE_EXAMPLE_ACTIVECONTOUR3DEX_H_
#define INCLUDE_EXAMPLE_ACTIVECONTOUR3DEX_H_

#include "AlloyApplication.h"
#include "AlloyWidget.h"
#include "AlloyTimeline.h"
#include "CommonShaders.h"
#include "segmentation/ActiveContour3D.h"

class ActiveContour3DEx: public aly::Application {
protected:
	aly::GLFrameBuffer depthFrameBuffer;
	aly::DepthAndNormalShader depthAndNormalShader;
	aly::MatcapShader matcapShader;
	aly::ImageShader imageShader;
	aly::Camera camera;
	aly::ActiveManifold3D simulation;
	aly::IconButtonPtr playButton,stopButton;
    std::shared_ptr<aly::TimelineSlider> timelineSlider;
    bool running;
public:
	ActiveContour3DEx();
	bool init(aly::Composite& rootNode);
	void draw(aly::AlloyContext* context);
};



#endif /* INCLUDE_EXAMPLE_ACTIVECONTOUR3DEX_H_ */
