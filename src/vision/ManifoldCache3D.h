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

#ifndef INCLUDE_MANIFOLDCACHE3D_H_
#define INCLUDE_MANIFOLDCACHE3D_H_
#include "Manifold3D.h"
#include <mutex>
#include <map>
#include <set>
namespace aly {
	class CacheElement3D {
	protected:
		bool loaded;
		bool deleteOnExit;
		bool writeOnce;
		std::string contourFile;
		std::shared_ptr<Manifold3D> contour;

		std::mutex accessLock;
	public:
		bool isLoaded() {
			std::lock_guard<std::mutex> lockMe(accessLock);
			return loaded;
		}
		CacheElement3D():loaded(false),deleteOnExit(true), writeOnce(true){
		}
		~CacheElement3D();
		std::string getFile() const {
			return contourFile;
		}
		void load();
		void unload();
		void set(const std::string& file);
		void set(const Manifold3D& springl,const std::string& file);
		std::shared_ptr<Manifold3D> getManifold();
	};
	struct CacheCompare3D {
		inline bool operator() (const std::pair<uint64_t, int>& lhs, const std::pair<uint64_t, int>& rhs) const {
			return lhs.first < rhs.first;
		}
	};
	class ManifoldCache3D {
	protected:
		std::map<int, std::shared_ptr<CacheElement3D>> cache;
		std::set<std::pair<uint64_t, int>, CacheCompare3D> loadedList;
		std::mutex accessLock;
		int maxElements;
		uint64_t counter;
	public:
		ManifoldCache3D(int elem=32):maxElements(elem),counter(0){

		}
		void setMaxElements(int e){
			maxElements=e;
		}
		std::shared_ptr<CacheElement3D> set(int frame, const std::string& file);
		std::shared_ptr<CacheElement3D> set(int frame, const Manifold3D& springl);
		std::shared_ptr<CacheElement3D> get(int frame);
		int unload();
		void clear();
	};


	class CacheCollectionElement3D {
		protected:
			bool loaded;
			bool deleteOnExit;
			bool writeOnce;
			std::string contourFile;
			std::shared_ptr<std::vector<Manifold3D>> contour;
			std::mutex accessLock;
		public:
			bool isLoaded() {
				std::lock_guard<std::mutex> lockMe(accessLock);
				return loaded;
			}
			CacheCollectionElement3D():loaded(false),deleteOnExit(true), writeOnce(true){
			}
			~CacheCollectionElement3D();
			std::string getFile() const {
				return contourFile;
			}
			void load();
			void unload();
			void set(const std::string& file);
			void set(const std::vector<Manifold3D>& springl,const std::string& file);
			std::shared_ptr<std::vector<Manifold3D>> getManifold();
		};
		struct CacheCollectionCompare3D {
			inline bool operator() (const std::pair<uint64_t, int>& lhs, const std::pair<uint64_t, int>& rhs) const {
				return lhs.first < rhs.first;
			}
		};
		class ManifoldCollectionCache3D {
		protected:
			std::map<int, std::shared_ptr<CacheCollectionElement3D>> cache;
			std::set<std::pair<uint64_t, int>, CacheCollectionCompare3D> loadedList;
			std::mutex accessLock;
			int maxElements;
			uint64_t counter;
		public:
			ManifoldCollectionCache3D(int elem=32):maxElements(elem),counter(0){
			}
			void setMaxElements(int e){
				maxElements=e;
			}
			std::shared_ptr<CacheCollectionElement3D> set(int frame, const std::vector<Manifold3D>& springl);
			std::shared_ptr<CacheCollectionElement3D> set(int frame, const std::string& file);
			std::shared_ptr<CacheCollectionElement3D> get(int frame);
			int unload();
			void clear();
		};
}

#endif /* INCLUDE_SpringlCache_H_ */
