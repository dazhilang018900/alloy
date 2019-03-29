/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#include "ui/AlloyApplication.h"
#include "system/AlloyFileUtil.h"
#include "ui/AlloyUI.h"
#include "ui/AlloyDrawUtil.h"
#include <thread>
#include <chrono>

namespace aly {

std::shared_ptr<AlloyContext>& Application::context =
		AlloyContext::getDefaultContext();
void Application::initInternal() {
	context->addAssetDirectory("assets/");
	context->addAssetDirectory("data/assets/");
	context->addAssetDirectory("../assets/");
	context->addAssetDirectory("../../assets/");
	context->addAssetDirectory("../../../assets/");
	context->addAssetDirectory("../../../../assets/");
	imageShader = std::shared_ptr<ImageShader>(
			new ImageShader(ImageShader::Filter::NONE, context));
	try {
		ReadImageFromFile(getContext()->getFullPath("images/alloy_logo128.png"),
				iconImage1);
		iconImages[0].pixels = iconImage1.ptr();
		iconImages[0].width = iconImage1.width;
		iconImages[0].height = iconImage1.height;
		ReadImageFromFile(getContext()->getFullPath("images/alloy_logo64.png"),
				iconImage2);
		iconImages[1].pixels = iconImage2.ptr();
		iconImages[1].width = iconImage2.width;
		iconImages[1].height = iconImage2.height;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
std::shared_ptr<GLTextureRGBA> Application::loadTextureRGBA(
		const std::string& partialFile) {
	ImageRGBA image;
	ReadImageFromFile(AlloyDefaultContext()->getFullPath(partialFile), image);
	return std::shared_ptr<GLTextureRGBA>(new GLTextureRGBA(image, context));
}
std::shared_ptr<GLTextureRGB> Application::loadTextureRGB(
		const std::string& partialFile) {
	ImageRGB image;
	ReadImageFromFile(AlloyDefaultContext()->getFullPath(partialFile), image);
	return std::shared_ptr<GLTextureRGB>(new GLTextureRGB(image, context));
}
std::shared_ptr<Font> Application::loadFont(const std::string& name,
		NVGcontext* nvg, const std::string& file) {
	return std::shared_ptr<Font>(
			new Font(name, nvg, AlloyDefaultContext()->getFullPath(file),
					context.get()));
}
Application::Application(int w, int h, const std::string& title,
		bool showDebugIcon) :
		frameRate(0.0f), showDebugIcon(showDebugIcon), onResize(nullptr) {
	if (context.get() == nullptr) {
		context.reset(new AlloyContext(title, w, h));
	} else {
		throw std::runtime_error(
				"Cannot instantiate more than one application.");
	}
	initInternal();
}
void Application::draw(const WindowPtr& win) {
	std::lock_guard<std::mutex> lockMe(context->getLock());
	glfwSetInputMode(win->handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glClearColor(0.0, 0.0, 0.0, 10);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int2 fbSize = win->getFrameBufferSize();
	glViewport(0, 0, fbSize.x, fbSize.y);
	nvgFontFaceId(win->nvg,context->getFontHandle(FontType::Normal,win.get()));
	draw(context.get());
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, fbSize.x, fbSize.y);
	drawUI(win);
	if (context->isDebugEnabled()) {
		if(context->focusedWindow==win.get()||context->focusedWindow==nullptr){
			drawDebugUI(win);
		}
	}
	const Cursor* cursor = context->getCursor();
	if (!cursor) {
		cursor = &Cursor::Normal;
	}
	int2 screenSize = win->getScreenSize();
	nvgBeginFrame(win->nvg, screenSize.x, screenSize.y, 1.0f);
	cursor->draw(context.get());
	nvgEndFrame(win->nvg);
}
void Application::drawUI(const WindowPtr& win) {
	context->setCursor(nullptr);
	int2 screenSize = win->getScreenSize();
	glViewport(0, 0, screenSize.x, screenSize.y);
	NVGcontext* nvg = win->nvg;
	nvgBeginFrame(nvg, screenSize.x, screenSize.y, 1.0f); //(float) context->pixelRatio
	nvgScissor(nvg, 0.0f, 0.0f, (float) screenSize.x, (float) screenSize.y);
	win->ui->draw(context.get());
	nvgScissor(nvg, 0.0f, 0.0f, (float) screenSize.x, (float) screenSize.y);
	Region* onTop = context->getOnTopRegion();
	if (onTop != nullptr) {
		if (onTop->isVisible())
			onTop->draw(context.get());
	}
	const Cursor* cursor = context->getCursor();
	if (!cursor) {
		cursor = &Cursor::Normal;
	}
	nvgEndFrame(nvg);
	context->dirtyUI = false;
}
void Application::drawDebugUI(const WindowPtr& win) {
	NVGcontext* nvg = win->nvg;
	int2 screenSize = win->getScreenSize();
	box2px viewport = win->getViewport();
	nvgBeginFrame(nvg, (float) screenSize.x, (float) screenSize.y, 1.0f);
	nvgResetScissor(nvg);
	win->ui->drawDebug(context.get());
	Region* onTop = context->getOnTopRegion();
	if (onTop != nullptr) {
		onTop->drawDebug(context.get());
	}
	float cr = context->theme.CORNER_RADIUS;
	if (viewport.contains(context->cursorPosition)) {
		nvgFontSize(nvg, 15);
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold,win.get()));
		/*
		 int alignment = 0;
		 if (context->cursorPosition.x < context->width() * 0.5f) {
		 alignment = NVG_ALIGN_LEFT;
		 } else {
		 alignment = NVG_ALIGN_RIGHT;
		 }
		 if (context->cursorPosition.y < context->height() * 0.5f) {
		 alignment |= NVG_ALIGN_TOP;
		 } else {
		 alignment |= NVG_ALIGN_BOTTOM;
		 }
		 std::string txt = MakeString() << std::setprecision(4) << " "
		 << context->cursorPosition;
		 nvgTextAlign(nvg, alignment);
		 nvgFillColor(nvg, Color(0, 0, 0, 128));
		 if (context->hasFocus) {
		 drawText(nvg, context->cursorPosition, txt, FontStyle::Outline,
		 Color(255), Color(64, 64, 64));
		 }
		 */
		nvgTextAlign(nvg, NVG_ALIGN_TOP);
		float yoffset = 5;
		std::string txt =
				(context->focusedWindow != nullptr) ?
						context->focusedWindow->getName() : "Window Lost Focus";
		drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline, Color(255),
				Color(64, 64, 64));
		yoffset += 16;

		txt = MakeString() << "Cursor " << context->cursorPosition;
		drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline, Color(255),
				Color(64, 64, 64));
		yoffset += 16;
		if (context->mouseOverRegion != nullptr) {
			txt = MakeString() << "Mouse Over ["
					<< context->mouseOverRegion->name << "] "
					<< int2(context->mouseOverRegion->getBounds().dimensions);

			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}

		if (context->mouseDownRegion != nullptr) {
			txt = MakeString() << "Mouse Down ["
					<< context->mouseDownRegion->name << "] "
					<< context->cursorDownPosition;
			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}
		if (context->cursorFocusRegion != nullptr) {
			txt = MakeString() << "Cursor Focus ["
					<< context->cursorFocusRegion->name << "]";
			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}
		if (context->objectFocusRegion != nullptr) {
			txt = MakeString() << "Region Focus ["
					<< context->objectFocusRegion->name << "]";
			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}
		if (context->onTopRegion != nullptr) {
			txt =
					MakeString() << "On Top [" << context->onTopRegion->name
							<< ": "
							<< (context->onTopRegion->isVisible() ?
									"Visible" : "Hidden") << "]";
			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}
		if (context->leftMouseButton) {
			txt = "Left Mouse Button Down";
			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}
		if (context->rightMouseButton) {
			txt = "Right Mouse Button Down";
			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}
		if (context->middleMouseButton) {
			txt = "Middle Mouse Button Down";
			drawText(nvg, 5, yoffset, txt.c_str(), FontStyle::Outline,
					Color(255), Color(64, 64, 64));
			yoffset += 16;
		}
		if (context->focusedWindow == win.get()) {
			nvgBeginPath(nvg);
			nvgLineCap(nvg, NVG_ROUND);
			nvgStrokeWidth(nvg, 2.0f);
			nvgStrokeColor(nvg, Color(255, 255, 255, 255));
			nvgMoveTo(nvg, context->cursorPosition.x - cr,
					context->cursorPosition.y);
			nvgLineTo(nvg, context->cursorPosition.x + cr,
					context->cursorPosition.y);
			nvgMoveTo(nvg, context->cursorPosition.x,
					context->cursorPosition.y - cr);
			nvgLineTo(nvg, context->cursorPosition.x,
					context->cursorPosition.y + cr);

			nvgStroke(nvg);
			nvgBeginPath(nvg);
			nvgFillColor(nvg, Color(255, 255, 255, 255));
			nvgCircle(nvg, context->cursorPosition.x, context->cursorPosition.y,
					3.0f);
			nvgFill(nvg);

			nvgBeginPath(nvg);
			nvgFillColor(nvg, Color(255, 64, 32, 255));
			nvgCircle(nvg, context->cursorPosition.x, context->cursorPosition.y,
					1.5f);
			nvgFill(nvg);
		}
	}
	nvgEndFrame(nvg);
}
void Application::fireEvent(const InputEvent& event) {
	if (event.type == InputType::Cursor
			|| event.type == InputType::MouseButton) {
		event.window->dirtyLocator = true;
	}
	bool consumed = false;
	if (event.type == InputType::Scroll && context->mouseOverRegion != nullptr
			&& context->mouseOverRegion->onScrollWheel) {
		consumed = context->mouseOverRegion->onScrollWheel(context.get(),
				event);
	} else if (event.type == InputType::MouseButton) {
		if (event.isDown()) {
			if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
				context->leftMouseButton = true;
			}
			if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
				context->rightMouseButton = true;
			}
			if (event.button == GLFW_MOUSE_BUTTON_MIDDLE) {
				context->middleMouseButton = true;
			}
			context->mouseOverRegion = context->cursorFocusRegion =
					context->mouseDownRegion = event.window->locate(
							context->cursorPosition);

			if (context->mouseDownRegion != nullptr) {
				context->cursorDownPosition = event.cursor
						- context->mouseDownRegion->getBoundsPosition();
			}
		} else if (event.isUp()) {

			if (context->mouseDownRegion != nullptr
					&& context->getOnTopRegion() == context->mouseDownRegion
					&& context->mouseDownRegion->isDragEnabled()) {
				context->removeOnTopRegion(context->mouseDownRegion);
			}

			context->leftMouseButton = false;
			context->rightMouseButton = false;
			context->middleMouseButton = false;
			context->mouseDownRegion = nullptr;
			context->cursorDownPosition = pixel2(0, 0);
		}
	}

	//Fire events
	Region* mdr = context->mouseDownRegion;
	if (mdr != nullptr && event.type != InputType::MouseButton
			&& mdr->isDragEnabled()
			&& mdr->acceptDragEvent(context->cursorPosition)) {
		if (mdr->onMouseDrag) {
			consumed |= mdr->onMouseDrag(context.get(), event);
		} else {
			if ((context->leftMouseButton
					&& mdr->getDragButton() == GLFW_MOUSE_BUTTON_LEFT)
					|| (context->rightMouseButton
							&& mdr->getDragButton() == GLFW_MOUSE_BUTTON_RIGHT)
					|| (context->middleMouseButton
							&& mdr->getDragButton() == GLFW_MOUSE_BUTTON_MIDDLE)) {
				mdr->setDragOffset(context->cursorPosition,
						context->cursorDownPosition);
			}
		}
		context->requestPack();
	} else if (context->mouseOverRegion != nullptr) {
		if (event.type == InputType::MouseButton) {
			if (event.isDown()) {
				if (context->mouseOverRegion->onMouseDown) {
					consumed |= context->mouseOverRegion->onMouseDown(
							context.get(), event);
				} else if (context->mouseDownRegion->isDragEnabled()) {
					//context->setOnTopRegion(context->mouseDownRegion);
					context->mouseDownRegion->setDragOffset(
							context->cursorPosition,
							context->cursorDownPosition);
				}
			}
			if (context->mouseOverRegion != nullptr
					&& context->mouseOverRegion->onMouseUp && event.isUp())
				consumed |= context->mouseOverRegion->onMouseUp(context.get(),
						event);
			context->requestPack();
		}
		if (event.type == InputType::Cursor) {
			if (context->mouseOverRegion != nullptr
					&& context->mouseOverRegion->onMouseOver) {
				consumed |= context->mouseOverRegion->onMouseOver(context.get(),
						event);
			}
		}
	}
	if (!consumed) {
		consumed = context->fireListeners(event);
	}
	if (consumed)
		context->dirtyUI = true;
}
void Application::onWindowSize(Window* win, int width, int height) {
	context->dirtyUI = true;
	win->dirtyUI = true;
	if (onResize) {
		onResize(win, int2(width, height));
	}

}
void Application::onFrameBufferSize(Window* win, int width, int height) {
	glViewport(0, 0, width, height);
	context->dirtyUI = true;
	win->dirtyUI = true;
}
void Application::onCursorPos(Window* win, double xpos, double ypos) {
	context->focusedWindow = win;
	context->cursorPosition = pixel2((pixel) (xpos), (pixel) (ypos));
	InputEvent& e = inputEvent;
	e = InputEvent();
	e.window = win;
	e.type = InputType::Cursor;
	if (context->leftMouseButton) {
		e.button = GLFW_MOUSE_BUTTON_LEFT;
	} else if (context->rightMouseButton) {
		e.button = GLFW_MOUSE_BUTTON_RIGHT;
	} else if (context->middleMouseButton) {
		e.button = GLFW_MOUSE_BUTTON_MIDDLE;
	} else {
		e.button = -1;
	}
	e.mods = 0;
	if (glfwGetKey(win->handle, GLFW_KEY_LEFT_SHIFT)
			| glfwGetKey(win->handle, GLFW_KEY_RIGHT_SHIFT))
		e.mods |= GLFW_MOD_SHIFT;
	if (glfwGetKey(win->handle, GLFW_KEY_LEFT_CONTROL)
			| glfwGetKey(win->handle, GLFW_KEY_RIGHT_CONTROL))
		e.mods |= GLFW_MOD_CONTROL;
	if (glfwGetKey(win->handle, GLFW_KEY_LEFT_ALT)
			| glfwGetKey(win->handle, GLFW_KEY_RIGHT_ALT))
		e.mods |= GLFW_MOD_ALT;
	if (glfwGetKey(win->handle, GLFW_KEY_LEFT_SUPER)
			| glfwGetKey(win->handle, GLFW_KEY_RIGHT_SUPER))
		e.mods |= GLFW_MOD_SUPER;
	e.cursor = pixel2((pixel) (xpos), (pixel) (ypos));
	fireEvent(e);
}

void Application::onWindowFocus(Window* win, int focused) {
	if (focused) {
		context->focusedWindow = win;
		InputEvent& e = inputEvent;
		e = InputEvent();
		e.window = win;
		e.type = InputType::Cursor;
		e.cursor = context->cursorPosition;
		fireEvent(e);
	} else {
		context->mouseOverRegion = nullptr;
		context->mouseDownRegion = nullptr;
		context->cursorPosition = pixel2(-1, -1);
		context->cursorDownPosition = pixel2(-1, -1);
		context->focusedWindow = nullptr;
	}
}
void Application::onWindowClose(Window* win) {
	if (win->isHideOnClose()) {
		win->setVisible(false);
	}
}
WindowPtr Application::addWindow(const std::string& name, int width, int height,
		bool hideOnClose) {
	return context->addWindow(name, width, height, hideOnClose);
}
bool Application::remove(Window* win) {
	return context->remove(win);
}

std::shared_ptr<Window> Application::getMainWindow() const {
	return context->windows.front();
}
void Application::getScreenShot(ImageRGBA& img) {
	int w = 0, h = 0;
	glfwGetFramebufferSize(context->getCurrentWindow()->handle, &w, &h);
	img.resize(w, h);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.ptr());
	FlipVertical(img);
}
void Application::getScreenShot(ImageRGB& img) {
	int w = 0, h = 0;
	glfwGetFramebufferSize(context->getCurrentWindow()->handle, &w, &h);
	img.resize(w, h);
	glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, img.ptr());
	FlipVertical(img);
}
ImageRGBA Application::getScreenShot() {
	int w = 0, h = 0;
	glfwGetFramebufferSize(context->getCurrentWindow()->handle, &w, &h);
	ImageRGBA img(w, h);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.ptr());
	FlipVertical(img);
	return img;
}
void Application::onCursorEnter(Window* win, int enter) {
	if (!enter) {
		context->focusedWindow = nullptr;
		context->mouseOverRegion = nullptr;
		InputEvent& e = inputEvent;
		e = InputEvent();
		e.type = InputType::Cursor;
		e.cursor = context->cursorPosition;
		e.mods = 0;
		e.window = win;
		GLFWwindow* window = win->handle;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)
				| glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
			e.mods |= GLFW_MOD_SHIFT;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)
				| glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
			e.mods |= GLFW_MOD_CONTROL;
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT)
				| glfwGetKey(window, GLFW_KEY_RIGHT_ALT))
			e.mods |= GLFW_MOD_ALT;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER)
				| glfwGetKey(window, GLFW_KEY_RIGHT_SUPER))
			e.mods |= GLFW_MOD_SUPER;
		fireEvent(e);
	} else {
		context->focusedWindow = win;
	}
}
void Application::onScroll(Window* win, double xoffset, double yoffset) {
	InputEvent& e = inputEvent;
	e = InputEvent();
	e.cursor = context->cursorPosition;
	e.type = InputType::Scroll;
	e.scroll = pixel2((pixel) xoffset, (pixel) yoffset);
	e.window = win;
	GLFWwindow* window = win->handle;
	e.mods = 0;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)
			| glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
		e.mods |= GLFW_MOD_SHIFT;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)
			| glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
		e.mods |= GLFW_MOD_CONTROL;
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT)
			| glfwGetKey(window, GLFW_KEY_RIGHT_ALT))
		e.mods |= GLFW_MOD_ALT;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER)
			| glfwGetKey(window, GLFW_KEY_RIGHT_SUPER))
		e.mods |= GLFW_MOD_SUPER;

	fireEvent(e);
}
void Application::onMouseButton(Window* win, int button, int action, int mods) {
	InputEvent& e = inputEvent;
	e = InputEvent();
	e.type = InputType::MouseButton;
	e.cursor = context->cursorPosition;
	e.window = win;
	std::chrono::steady_clock::time_point currentTime =
			std::chrono::steady_clock::now();
	e.button = button;
	if (std::chrono::duration_cast<std::chrono::milliseconds>(
			currentTime - lastClickTime).count() <= context->doubleClickTime) {
		e.clicks = 2;
	} else {
		e.clicks = 1;
	}
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		lastClickTime = currentTime;
	}
	e.action = action;
	fireEvent(e);
}
void Application::onKey(Window* win, int key, int scancode, int action,
		int mods) {
	/*
	 if (isprint(key)) {
	 //Seems to be problem in GLFW for Ubuntu 15, it fires onKey() instead of onChar() when letters are pressed.
	 InputEvent& e = inputEvent;
	 e.type = InputType::Character;
	 e.action = action;
	 e.key=key;
	 e.codepoint=key;
	 GLFWwindow* window = context->window;
	 e.mods = 0;
	 if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)
	 | glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
	 e.mods |= GLFW_MOD_SHIFT;
	 if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)
	 | glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
	 e.mods |= GLFW_MOD_CONTROL;
	 if (glfwGetKey(window, GLFW_KEY_LEFT_ALT)
	 | glfwGetKey(window, GLFW_KEY_RIGHT_ALT))
	 e.mods |= GLFW_MOD_ALT;
	 if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER)
	 | glfwGetKey(window, GLFW_KEY_RIGHT_SUPER))
	 e.mods |= GLFW_MOD_SUPER;
	 e.cursor = context->cursorPosition;
	 fireEvent(e);
	 } else {
	 */
	InputEvent& e = inputEvent;
	e = InputEvent();
	e.type = InputType::Key;
	e.action = action;
	e.key = key;
	e.scancode = scancode;
	e.mods = mods;
	e.window = win;
	e.cursor = context->cursorPosition;
	if (key == GLFW_KEY_F11 && e.action == GLFW_PRESS) {
		for (int i = 0; i < 1000; i++) {
			std::string screenShot = MakeString() << GetDesktopDirectory()
					<< ALY_PATH_SEPARATOR<<"screenshot"<<std::setw(4)<<std::setfill('0')<<i<<".png";
			if(!FileExists(screenShot)) {
				std::cout<<"Saving "<<screenShot<<std::endl;
				win->setCurrent();
				ImageRGBA img;
				getScreenShot(img);
				WriteImageToFile(screenShot,img);
				break;
			}
		}
	}

	fireEvent(e);
	//}
}
void Application::onChar(Window* win, unsigned int codepoint) {
	InputEvent& e = inputEvent;
	e = InputEvent();
	e.type = InputType::Character;
	e.codepoint = codepoint;
	e.clicks = 0;
	e.cursor = context->cursorPosition;
	e.window = win;
	GLFWwindow* window = win->handle;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)
			| glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
		e.mods |= GLFW_MOD_SHIFT;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)
			| glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
		e.mods |= GLFW_MOD_CONTROL;
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT)
			| glfwGetKey(window, GLFW_KEY_RIGHT_ALT))
		e.mods |= GLFW_MOD_ALT;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER)
			| glfwGetKey(window, GLFW_KEY_RIGHT_SUPER))
		e.mods |= GLFW_MOD_SUPER;
	fireEvent(e);
}
void Application::runOnce(const std::string& fileName) {
	close();
	run(0);
	context->getCurrentWindow()->swapBuffers();
	ImageRGBA img;
	getScreenShot(img);
	WriteImageToFile(fileName, img);
}
void Application::loadFonts() {
	for (WindowPtr win : context->windows) {
		win->setCurrent();
		context->loadFont(FontType::Normal, "sans",
				"fonts/Roboto-Regular.ttf");
		context->loadFont(FontType::Bold, "sans-bold", "fonts/Roboto-Bold.ttf");
		context->loadFont(FontType::Italic, "sans-italic",
				"fonts/Roboto-Italic.ttf");
		context->loadFont(FontType::Code, "sans", "fonts/Hack-Regular.ttf");
		context->loadFont(FontType::CodeBold, "sans-bold",
				"fonts/Hack-Bold.ttf");
		context->loadFont(FontType::CodeItalic, "sans-bold-italic",
				"fonts/Hack-Italic.ttf");
		context->loadFont(FontType::CodeBoldItalic, "sans-bold-italic",
				"fonts/Hack-BoldItalic.ttf");
		context->loadFont(FontType::Entypo, "entypo", "fonts/entypo.ttf");
		context->loadFont(FontType::Icon, "basic_icons",
				"fonts/fontawesome.ttf");
		context->loadFont(FontType::AwesomeRegular, "regular_icons",
				"fonts/fa-regular.ttf");
		context->loadFont(FontType::AwesomeSolid, "solid_icons",
				"fonts/fa-solid.ttf");
		context->loadFont(FontType::AwesomeBrands, "brands",
				"fonts/fa-brands.ttf");
	}
}
void Application::run(int swapInterval) {
	const double POLL_INTERVAL_SEC = 0.5f;
	loadFonts();
	auto windows = context->getWindows();
	for (WindowPtr win : windows) {
		if (win->app == nullptr) {
			win->app = this;
			win->setCurrent();
			win->registerCallbacks(this);
			glfwSetWindowIcon(win->handle, 2, iconImages);
			if (!init(win)) {
				throw std::runtime_error(
						MakeString()
								<< "Error occurred in application init() for window "
								<< win->getName());
			}
			win->glass->setVisible(true);
			win->ui->add(win->glass);
			if (showDebugIcon&&win==windows.front()) {
				GlyphRegionPtr debug = MakeGlyphRegion(
						createAwesomeGlyph(0xf188, FontStyle::Outline, 20),
						CoordPercent(1.0f, 1.0f), CoordPX(20, 20),
						RGBA(0, 0, 0, 0), RGBA(64, 64, 64, 128),
						RGBA(192, 192, 192, 128), UnitPX(0));
				debug->setOrigin(Origin::BottomRight);
				debug->onMouseDown =
						[this,debug](AlloyContext* context,const InputEvent& e) {
							if(e.button==GLFW_MOUSE_BUTTON_LEFT) {
								context->toggleDebug();
								debug->foregroundColor=context->isDebugEnabled()?MakeColor(255,64,64,255):MakeColor(64,64,64,128);
								context->setCursorFocus(nullptr);
								return true;
							}
							return false;
						};
				win->ui->add(debug);
			}
			//First pack triggers computation of aspect ratios  for components.
			win->ui->pack(context.get());
			win->glass->setVisible(false);
			win->dirtyUI = true;
			win->setSwapInterval(swapInterval);
		}
	}
	glfwSetTime(0);
	uint64_t frameCounter = 0;
	std::chrono::steady_clock::time_point endTime;
	std::chrono::steady_clock::time_point lastFpsTime =
			std::chrono::steady_clock::now();
	if (forceClose) {
		context->executeDeferredTasks();
		context->dirtyUI = true;
		for (WindowPtr win : windows) {
			win->dirtyUI = true;
			draw(win);
		}
		context->dirtyUI = true;
		for (WindowPtr win : windows) {
			win->dirtyUI = true;
			context->update(win.get());
		}
	} else {
		for (WindowPtr win : windows) {
			win->setVisible(true);
		}
	}
	do {
		//Events could have modified layout! Pack before draw to make sure things are correctly positioned.
		for (WindowPtr win : windows) {
			win->setCurrent();
			if (win->app == nullptr) {
				std::cout << "Window Added " << win->getName() << std::endl;
				win->app = this;
				win->registerCallbacks(this);
				glfwSetWindowIcon(win->handle, 2, iconImages);
				win->setVisible(true);
				if (!init(win)) {
					throw std::runtime_error(
							MakeString()
									<< "Error occurred in application init() for window "
									<< win->getName());
				}
				win->glass->setVisible(true);
				win->ui->add(win->glass);
				if (showDebugIcon) {
					GlyphRegionPtr debug = MakeGlyphRegion(
							createAwesomeGlyph(0xf188, FontStyle::Outline, 20),
							CoordPercent(1.0f, 1.0f), CoordPX(20, 20),
							RGBA(0, 0, 0, 0), RGBA(64, 64, 64, 128),
							RGBA(192, 192, 192, 128), UnitPX(0));
					debug->setOrigin(Origin::BottomRight);
					debug->onMouseDown =
							[this,debug](AlloyContext* context,const InputEvent& e) {
								if(e.button==GLFW_MOUSE_BUTTON_LEFT) {
									context->toggleDebug();
									debug->foregroundColor=context->isDebugEnabled()?MakeColor(255,64,64,255):MakeColor(64,64,64,128);
									context->setCursorFocus(nullptr);
									return true;
								}
								return false;
							};
					win->ui->add(debug);
				}
				win->ui->pack(context.get());
				win->glass->setVisible(false);
				win->dirtyUI = true;
				win->setSwapInterval(swapInterval);
			}
			if (win->dirtyUI) {
				win->dirtyUI = false;
				win->dirtyLocator = true;
				win->ui->pack(context.get());
			}
			draw(win);
			context->update(win.get());
		}
		double elapsed =
				std::chrono::duration<double>(endTime - lastFpsTime).count();
		frameCounter++;
		if (elapsed > POLL_INTERVAL_SEC) {
			frameRate = (float) (frameCounter / elapsed);
			lastFpsTime = endTime;
			frameCounter = 0;
		}
		for (WindowPtr win : windows) {
			win->swapBuffers();
		}
		glfwPollEvents();
		for (std::exception_ptr e : caughtExceptions) {
			std::rethrow_exception(e);
		}
	} while (!windows.front()->shouldClose() && !forceClose);
	if (onExit)
		onExit();
}
}

