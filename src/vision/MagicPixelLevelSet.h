/*
 * Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef INCLUDE_MultiResolutionLevelSet_H
#define INCLUDE_MultiResolutionLevelSet_H
#include "vision/MagicPixels.h"
#include "vision/MultiActiveContour2D.h"
namespace aly {
class MagicPixelLevelSet: public MultiActiveContour2D {
protected:
	aly::ImageRGBA referenceImage;
	Number pruneInterval;
	Number splitInterval;
	Number colorThreshold;
	Number backgroundPressureParam;
	Number tileSizeParam;
	Number pruneSizeParam;
	MagicPixels magicPixels;
	void magicPixelMotion(int i, int j, size_t index);
	virtual float evolve(float maxStep) override;
	virtual bool stepInternal() override;
	void reinit();
public:
	void setPruneInterval(int v) {
		pruneInterval.setValue(v);
	}
	void setSplitInterval(int v) {
		splitInterval.setValue(v);
	}
	void setColorThreshold(float v) {
		colorThreshold.setValue(v);
	}
	void setMaxTileSize(int v) {
		tileSizeParam.setValue(v);
	}
	void setMinTileSize(int v) {
		pruneSizeParam.setValue(v);
	}
	void setBackgroundPressure(float v) {
		backgroundPressureParam.setValue(v);
	}
	MagicPixelLevelSet(const std::shared_ptr<ManifoldCache2D>& cache = nullptr);
	MagicPixelLevelSet(const std::string& name,
			const std::shared_ptr<ManifoldCache2D>& cache = nullptr);
	virtual void setup(const aly::ParameterPanePtr& pane) override;
	virtual bool init() override;
	void setReference(const ImageRGBA& img, int smoothIterations,
			int diffuseIterations);

	virtual bool updateOverlay() override;
	void setPruneIterval(int v) {
		pruneInterval.setValue(v);
	}
	void setSplitIterval(int v) {
		splitInterval.setValue(v);
	}

};
}
#endif
