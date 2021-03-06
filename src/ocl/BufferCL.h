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

#ifndef INCLUDE_BUFFERCL
#define INCLUDE_BUFFERCL
#include "ocl/MemoryCL.h"
#include "math/AlloyVector.h"
namespace aly {
	class BufferCL: public MemoryCL {
	public:
		BufferCL();
		void create(const cl_mem_flags& f, size_t bufferSize,void* data = nullptr);

		void* mapRead(bool block = true, int wait_events_num = 0, cl_event* wait_events = nullptr);
		void* mapWrite(bool block = true, int wait_events_num = 0, cl_event* wait_events = nullptr) const;
		void unmap(void* data) const;
		template<class T, int M> BufferCL(const Vector<T, M>& vec, const cl_mem_flags& f = CL_MEM_READ_WRITE) :
				BufferCL() {
			create(f, vec);
		}
		template<class T, int M> void read(Vector<T, M>& vec, bool block = true) const {
			MemoryCL::read(vec.data, block);
		}
		template<class T, int M> void write(const Vector<T, M>& vec, bool block = true) {
			MemoryCL::write(vec.data, block);
		}
		template<class T> void write(const std::vector<T>& vec, bool block = true) {
			MemoryCL::write(vec, block);
		}
		template<class T> void write(const T* vec, size_t sz, bool block = true) {
			MemoryCL::write(vec, sz*sizeof(T), block);
		}

		template<class T> void read(std::vector<T>& vec, bool block = true) {
			MemoryCL::read(vec, block);
		}
		template<class T> T readFront() {
			T value=(T)0;
			MemoryCL::read(&value,sizeof(T), true);
			return value;
		}
		template<class T> void writeFront(T value) {
			MemoryCL::write(&value,sizeof(T), true);
		}
		inline float readFloat() {
			return readFront<float>();
		}
		inline int readInteger() {
			return readFront<int>();
		}
		inline char readChar() {
			return readFront<char>();
		}
		inline double readDouble() {
			return readFront<double>();
		}
		void create(size_t bufferSize,void* data = nullptr) {
			create(CL_MEM_READ_WRITE, bufferSize, data);
		}
		template<class T, int M> void create(const cl_mem_flags& f, Vector<T, M>& vec) {
			create(f, vec.size() * vec.typeSize(), nullptr);
			write(vec,true);
		}
		template<class T> void create(const cl_mem_flags& f,  std::vector<T>& vec) {
			create(f, vec.size() * sizeof(T),nullptr);
			write(vec,true);
		}
		template<class T, int M> void create(const cl_mem_flags& f, const Vector<T, M>& vec) {
			Vector<T, M> tmp=vec;
			create(f,tmp);
		}
		template<class T> void create(const cl_mem_flags& f,  const std::vector<T>& vec) {
			std::vector<T> tmp=vec;
			create(f,tmp);
		}
	};
}
#endif

