/*
* Copyright(C) 2017, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
*
*  Created on: Feb 6, 2017
*      Author: Blake Lucas
*/

#ifndef INCLUDE_CORE_ALLOYEVENT_H_
#define INCLUDE_CORE_ALLOYEVENT_H_
#include "AlloyEnum.h"
#include "AlloyUnits.h"
#ifndef GLFW_RELEASE
	#define GLFW_RELEASE                0
#endif
#ifndef GLFW_PRESS
	#define GLFW_PRESS                  1
#endif
#ifndef GLFW_MOD_SHIFT
	#define GLFW_MOD_SHIFT           0x0001
#endif
#ifndef GLFW_MOD_CONTROL
	#define GLFW_MOD_CONTROL         0x0002
#endif
#ifndef GLFW_MOD_ALT
	#define GLFW_MOD_ALT             0x0004
#endif
#ifndef GLFW_MOD_SUPER
	#define GLFW_MOD_SUPER           0x0008
#endif
namespace aly{
class AlloyContext;

struct InputEvent {
		InputType type = InputType::Unspecified;
		pixel2 cursor = pixel2(-1, -1);
		pixel2 scroll = pixel2(0, 0);
		uint32_t codepoint = 0;
		int action = 0;
		int mods = 0;
		int scancode = 0;
		int key = 0;
		int clicks = 0;
		int button = -1;
		inline bool isDown() const {
			return (action != GLFW_RELEASE);
		}
		inline bool isUp() const {
			return (action == GLFW_RELEASE);
		}
		inline bool isShiftDown() const {
			return ((mods & GLFW_MOD_SHIFT) != 0);
		}
		inline bool isControlDown() const {
			return ((mods & GLFW_MOD_CONTROL) != 0);
		}
		inline bool isAltDown() const {
			return ((mods & GLFW_MOD_ALT) != 0);
		}
		inline bool isSuperDown() const {
			return ((mods & GLFW_MOD_SUPER) != 0);
		}
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const InputEvent& e) {
		return ss << "event type=" << e.type << " button=" << e.button << " key=" << e.key << " clicks=" << e.clicks;
	}
	struct EventHandler {
		virtual bool onEventHandler(AlloyContext* context, const InputEvent& event) = 0;
		virtual std::string getName() const = 0;
		virtual ~EventHandler();
	};
}


#endif /* INCLUDE_CORE_ALLOYEVENT_H_ */
