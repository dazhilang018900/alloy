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
#include "vision/ManifoldCache3D.h"
#include "system/AlloyFileUtil.h"
namespace aly {
void CacheElement3D::load() {
	std::lock_guard<std::mutex> lockMe(accessLock);
	if (!loaded) {
		contour.reset(new Manifold3D());
		ReadContourFromFile(contourFile, *contour);
		//std::cout << "Load: " << contour->getFile() << std::endl;
		loaded = true;
	}
}
void CacheElement3D::unload() {
	std::lock_guard<std::mutex> lockMe(accessLock);
	if (loaded) {
		if (writeOnce) {
			WriteContourToFile(contour->getFile(), *contour);
			//std::cout<<"Unload: "<<contour->getFile()<<std::endl;
			writeOnce = false;
		}
		contour.reset();
		loaded = false;
	}
}
void CacheElement3D::set(const Manifold3D& springl) {
	contour.reset(new Manifold3D());
	*contour = springl;
	contourFile = springl.getFile();
	loaded = true;
}
std::shared_ptr<Manifold3D> CacheElement3D::getContour() {
	load();
	return contour;
}
std::shared_ptr<CacheElement3D> ManifoldCache3D::set(int frame,
		const Manifold3D& springl) {
	std::lock_guard<std::mutex> lockMe(accessLock);
	auto iter = cache.find(frame);
	std::shared_ptr<CacheElement3D> elem;
	if (iter != cache.end()) {
		elem = iter->second;
	} else {
		elem = std::shared_ptr<CacheElement3D>(new CacheElement3D());
		cache[frame] = elem;
	}
	elem->set(springl);
	if (elem->isLoaded()) {
		while ((int) loadedList.size() >= maxElements) {
			cache[loadedList.begin()->second]->unload();
			loadedList.erase(loadedList.begin());
		}
		loadedList.insert(std::pair<uint64_t, int>(counter++, frame));
	}
	return elem;
}
int ManifoldCache3D::unload() {
	int sz = (int)loadedList.size();
	for(auto pr:loadedList){
		cache[pr.second]->unload();
	}
	loadedList.clear();
	return sz;
}
std::shared_ptr<CacheElement3D> ManifoldCache3D::get(int frame) {
	std::lock_guard<std::mutex> lockMe(accessLock);
	auto iter = cache.find(frame);
	if (iter != cache.end()) {
		std::shared_ptr<CacheElement3D> elem = iter->second;
		if (!elem->isLoaded()) {
			while ((int) loadedList.size() >= maxElements) {
				cache[loadedList.begin()->second]->unload();
				loadedList.erase(loadedList.begin());
			}
			elem->load();
			loadedList.insert(std::pair<uint64_t, int>(counter++, frame));
		}
		return elem;
	} else {
		return std::shared_ptr<CacheElement3D>();
	}
}
CacheElement3D::~CacheElement3D() {
	if (FileExists(contourFile)) {
		RemoveFile(contourFile);
		std::string imageFile = GetFileWithoutExtension(contourFile) + ".png";
		if (FileExists(imageFile))
			RemoveFile(imageFile);
	}
}
void ManifoldCache3D::clear() {
	std::lock_guard<std::mutex> lockMe(accessLock);
	counter = 0;
	loadedList.clear();
	cache.clear();

}
}
