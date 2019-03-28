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

#include "system/AlloyFileUtil.h"
#include "ui/AlloyContext.h"
#include "common/nanovg.h"
#include "ui/AlloyUI.h"
#include "ui/AlloyDrawUtil.h"
#include "ui/AlloyApplication.h"

#define NANOVG_GL3_IMPLEMENTATION
#include "common/nanovg_gl.h"

#include <iostream>
#include <chrono>

int printOglError(const char *file, int line) {

	GLenum glErr;
	int retCode = 0;

	glErr = glGetError();
	if (glErr != GL_NO_ERROR) {
		throw std::runtime_error(
				aly::MakeString() << "GL error occurred in \"" << file
						<< "\" on line " << line << ": "
						<< gluErrorString(glErr) << "\nOn Screen Render: "
						<< (aly::AlloyDefaultContext()->isOnScreenRender() ?
								"true" : "false") << "\nOff Screen Render: "
						<< (aly::AlloyDefaultContext()->isOffScreenRender() ?
								"true" : "false"));
		return 1;
	}
	return retCode;
}
namespace aly {

std::shared_ptr<AlloyContext> AlloyContext::defaultContext;
const Cursor Cursor::Normal(0xf245, 24.0f);
const Cursor Cursor::Hidden(0, -1.0f);
const Cursor Cursor::Hand(0xf25a, 24.0f, NVG_ALIGN_TOP | NVG_ALIGN_LEFT,
		FontType::Icon, 0.0f, pixel2(-8.0f, 0.0f));
const Cursor Cursor::Grab(0xf255, 24.0f, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER,
		FontType::Icon);
const Cursor Cursor::Horizontal(0xf07e, 24.0f,
		NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
const Cursor Cursor::Vertical(0xf07d, 24.0f,
		NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
const Cursor Cursor::Position(0xf047, 24.0f,
		NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
const Cursor Cursor::Rotate(0xf021, 24.0f, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
const Cursor Cursor::TextInsert(0xf246, 24.0f,
		NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
const Cursor Cursor::SlantDown(0xf07d, 24.0f,
		NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER, FontType::Icon, -ALY_PI_4);
const Cursor Cursor::SlantUp(0xf07d, 24.0f, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER,
		FontType::Icon, ALY_PI_4);
const Cursor Cursor::CrossHairs(0xf05b, 24.0f,
		NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER, FontType::Icon, 0.0f,
		pixel2(-0.25f, -0.25f));
Window::Window(const std::string& title, int width, int height) :
		handle(nullptr), nvg(nullptr), visible(false), app(nullptr),dirtyUI(true),dirtyLocator(true) {
	init(title, width, height);
}
Window::Window() :
		handle(nullptr), nvg(nullptr), visible(false), app(nullptr) ,dirtyUI(false),dirtyLocator(false) {
}
int2 Window::getScreenSize() const {
	int2 dims;
	glfwGetWindowSize(handle, &dims.x, &dims.y);
	return dims;
}
void Window::requestPack(){
	dirtyUI=true;
}
void Window::requestCursorUpdate(){
	dirtyLocator=true;
}
box2px Window::getViewport() const {
	return box2px(pixel2(0.0f, 0.0f),
			pixel2(getFrameBufferSize()));
}
float Window::getPixelRatio() const {
	return (float) getFrameBufferSize().x / (float) getScreenSize().x;
}
int2 Window::getFrameBufferSize() const {
	int2 dims;
	glfwGetFramebufferSize(handle, &dims.x, &dims.y);
	return dims;
}

void Window::init(const std::string& title, int width, int height) {
	static const float2 TextureCoords[6] = { float2(1.0f, 0.0f), float2(0.0f,
			0.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f),
			float2(1.0f, 0.0f) };
	static const float3 PositionCoords[6] = { float3(1.0f, 1.0f, 0.0f), float3(
			0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f,
			0.0f), float3(1.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 0.0f) };
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
	glfwWindowHint(GLFW_VISIBLE, 0);
	name = title;
	handle = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (!handle) {
		glfwTerminate();
		throw std::runtime_error(
				MakeString() << "Could not create window: " << title);
	}
	glfwMakeContextCurrent(handle);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error(
				MakeString() << "Could not initialize GLEW for window: "
						<< title);
	}
	glGetError();
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int fwidth, fheight;
	glfwGetFramebufferSize(handle, &fwidth, &fheight);
	glViewport(0, 0, fwidth, fheight);

	glGenVertexArrays(1, &vao.vao);
	glBindVertexArray(vao.vao);
	glGenBuffers(1, &vao.positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vao.positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, PositionCoords,
	GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vao.uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vao.uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 6, TextureCoords,
	GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

	ui = CompositePtr(
			new Composite(name, CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	glass = std::shared_ptr<Composite>(
			new Composite(name + "_glass", CoordPX(0, 0),
					CoordPercent(1.0f, 1.0f)));
	glass->backgroundColor = MakeColor(COLOR_NONE);
	glass->setVisible(false);
}
void Window::setCurrent() const {
	if (handle)
		glfwMakeContextCurrent(handle);
}
CursorLocator* Window::resetLocator() {
	locator.reset(getScreenSize());
	return &locator;
}
void Window::add(Region* region) {
	locator.add(region);
}
Region* Window::locate(const pixel2& cursor) const {
	Region* onTopRegion=AlloyApplicationContext()->getOnTopRegion();
	if (onTopRegion != nullptr) {
		if (onTopRegion->isVisible()) {
			Region* r = onTopRegion->locate(cursor);
			if (r != nullptr)
				return r;
		}
	}
	return locator.locate(cursor);
}
void Window::setVisible(bool vis) {
	if (handle) {
		if (vis) {
			glfwShowWindow(handle);

		} else {
			glfwHideWindow(handle);
		}
		visible = vis;
	}
}
void Window::registerCallbacks(Application* app) {
	glfwSetWindowUserPointer(handle, this);
	glfwSetWindowRefreshCallback(handle, [](GLFWwindow * window ) {
		Window* win = (Window *)(glfwGetWindowUserPointer(window));
		try {
			win->app->onWindowRefresh(win);
		} catch(...) {
			win->app->throwException(std::current_exception());
		}
	});
	glfwSetWindowFocusCallback(handle,
			[](GLFWwindow * window, int focused ) {
				Window* win = (Window *)(glfwGetWindowUserPointer(window));
				try {win->app->onWindowFocus(win,focused);} catch(...) {win->app->throwException(std::current_exception());
				}});
	glfwSetWindowSizeCallback(handle,
			[](GLFWwindow * window, int width, int height ) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onWindowSize(win,width, height);} catch(...) {win->app->throwException(std::current_exception());}});
	glfwSetFramebufferSizeCallback(handle,
			[](GLFWwindow * window, int width, int height) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onFrameBufferSize(win,width, height);} catch (...) {win->app->throwException(std::current_exception());}});
	glfwSetCharCallback(handle,
			[](GLFWwindow * window, unsigned int codepoint ) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onChar(win,codepoint);} catch(...) {win->app->throwException(std::current_exception());}});
	glfwSetKeyCallback(handle,
			[](GLFWwindow * window, int key, int scancode, int action, int mods) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onKey(win,key, scancode,action,mods);} catch(...) {win->app->throwException(std::current_exception());}});
	glfwSetMouseButtonCallback(handle,
			[](GLFWwindow * window, int button, int action,int mods) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onMouseButton(win,button, action,mods);} catch(...) {win->app->throwException(std::current_exception());}});
	glfwSetCursorPosCallback(handle,
			[](GLFWwindow * window, double xpos, double ypos ) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onCursorPos(win,xpos, ypos);} catch(...) {win->app->throwException(std::current_exception());}});
	glfwSetCursorEnterCallback(handle,
			[](GLFWwindow * window, int enter) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onCursorEnter(win,enter);} catch(...) {win->app->throwException(std::current_exception());}});
	glfwSetScrollCallback(handle,
			[](GLFWwindow * window, double xoffset, double yoffset ) {Window* win = (Window *)(glfwGetWindowUserPointer(window)); try {win->app->onScroll(win,xoffset, yoffset);} catch(...) {win->app->throwException(std::current_exception());}});

}
bool Window::isVisible() const {
	return visible;
}
void Window::setFocused() const {
	glfwFocusWindow(handle);
}
bool Window::shouldClose() const {
	return glfwWindowShouldClose(handle);
}
void Window::swapBuffers() const {
	glfwSwapBuffers(handle);
}
void Window::setSwapInterval(int interval) const {
	glfwSwapInterval(interval);
}

void Window::begin() const {
	setCurrent();
	int2 viewSize = getFrameBufferSize();
	glViewport(0, 0, viewSize.x, viewSize.y);
	glScissor(0, 0, viewSize.x, viewSize.y);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
}
Window::~Window() {
	if (handle) {
		glfwMakeContextCurrent(handle);
		if (vao.vao) {
			glDeleteVertexArrays(1, &vao.vao);
		}
		if (vao.uvBuffer) {
			glDeleteBuffers(1, &vao.uvBuffer);
		}
		if (vao.positionBuffer) {
			glDeleteBuffers(1, &vao.positionBuffer);
		}
		nvgDeleteGL3(nvg);
		glfwDestroyWindow(handle);
	}
	handle = nullptr;
}
void Cursor::draw(AlloyContext* context) const {
	pixel2 cursor = context->cursorPosition;
	if (fontSize > 0.0f && context->hasFocus && cursor.x >= 0 && cursor.y >= 0
			&& cursor.x < context->getScreenWidth()
			&& cursor.y < context->getScreenHeight()) {
		NVGcontext* nvg = context->getNVG();
		nvgTextAlign(nvg, align);
		nvgSave(nvg);
		nvgFontFaceId(nvg, context->getFontHandle(fontType));
		nvgFontSize(nvg, fontSize);
		nvgFillColor(nvg, Color(255, 255, 255));
		nvgTranslate(nvg, cursor.x + nudge.x, cursor.y + nudge.y);
		nvgRotate(nvg, angle);
		const float shift = 1.0f;
		const char* txt = codeString.c_str();
		nvgFillColor(nvg, Color(0, 0, 0));
		nvgText(nvg, +shift, 0, txt, nullptr);
		nvgText(nvg, -shift, 0, txt, nullptr);
		nvgText(nvg, 0, +shift, txt, nullptr);
		nvgText(nvg, 0, -shift, txt, nullptr);
		nvgFillColor(nvg, Color(255, 255, 255));
		nvgText(nvg, 0, 0, txt, nullptr);
		nvgRestore(nvg);
	}
}
Font::Font(const std::string& name, const std::string& file,
		AlloyContext* context) :
		nvg(context->getNVG()), handle(0), name(name), file(file) {
	handle = nvgCreateFont(nvg, name.c_str(), file.c_str());
}
int Font::getCursorPosition(const std::string & text, float fontSize,
		int xCoord) const {
	std::vector<NVGglyphPosition> positions(text.size());
	nvgFontSize(nvg, fontSize);
	nvgFontFaceId(nvg, handle);
	nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	positions.resize(
			nvgTextGlyphPositions(nvg, 0, 0, text.data(),
					text.data() + text.size(), positions.data(),
					(int) positions.size()));
	for (size_t i = 0; i < positions.size(); ++i) {
		if (xCoord < positions[i].maxx) {
			return static_cast<int>(i);
		}
	}
	return static_cast<int>(positions.size());
}
AwesomeGlyph::AwesomeGlyph(int codePoint, AlloyContext* context,
		const FontStyle& style, pixel height) :
		Glyph(CodePointToUTF8(codePoint), GlyphType::Awesome, 0, height), codePoint(
				codePoint), style(style) {
	NVGcontext* nvg = context->getNVG();
	nvgFontSize(nvg, height);
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
	width = nvgTextBounds(nvg, 0, 0, name.c_str(), nullptr, nullptr);

}
void AwesomeGlyph::draw(const box2px& bounds, const Color& fgColor,
		const Color& bgColor, AlloyContext* context) {
	NVGcontext* nvg = context->getNVG();
	nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
	nvgFontSize(nvg, height);
	nvgTextAlign(nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
	drawText(nvg, bounds.position + HALF_PIX(bounds.dimensions), name, style,
			fgColor, bgColor, nullptr);
}

ImageGlyph::ImageGlyph(const std::string& file, AlloyContext* context,
		bool mipmap) :
		Glyph(GetFileNameWithoutExtension(file), GlyphType::Image, 0, 0), file(
				file) {
	handle = nvgCreateImage(context->getNVG(), file.c_str(),
			(mipmap) ? NVG_IMAGE_GENERATE_MIPMAPS : 0);
	int w, h;
	nvgImageSize(context->getNVG(), handle, &w, &h);
	width = (pixel) w;
	height = (pixel) h;
}
void ImageGlyph::set(const ImageRGBA& rgba, AlloyContext* context) {
	nvgUpdateImage(context->getNVG(), handle, rgba.ptr());
}
void ImageGlyph::set(const ImageRGB& rgb, AlloyContext* context) {
	ImageRGBA rgba;
	ConvertImage(rgb, rgba); //Slow!!
	nvgUpdateImage(context->getNVG(), handle, rgba.ptr());
}
void ImageGlyph::set(const ImageRGBAf& rgba, AlloyContext* context) {
	ImageRGBA tmp;
	ConvertImage(rgba, tmp);
	nvgUpdateImage(context->getNVG(), handle, tmp.ptr());
}
ImageGlyph::~ImageGlyph() {
	AlloyContext* context = AlloyDefaultContext().get();
	if (context)
		nvgDeleteImage(context->getNVG(), handle);
}
ImageGlyph::ImageGlyph(const ImageRGBA& rgba, AlloyContext* context,
		bool mipmap) :
		Glyph("image_rgba", GlyphType::Image, 0, 0) {
	handle = nvgCreateImageRGBA(context->getNVG(), rgba.width, rgba.height,
			(mipmap) ? NVG_IMAGE_GENERATE_MIPMAPS : 0, rgba.ptr());
	width = (pixel) rgba.width;
	height = (pixel) rgba.height;
}
ImageGlyph::ImageGlyph(const ImageRGB& rgb, AlloyContext* context, bool mipmap) :
		Glyph("image_rgba", GlyphType::Image, 0, 0) {
	ImageRGBA rgba;
	ConvertImage(rgb, rgba);
	handle = nvgCreateImageRGBA(context->getNVG(), rgba.width, rgba.height,
			(mipmap) ? NVG_IMAGE_GENERATE_MIPMAPS : 0, rgba.ptr());
	width = (pixel) rgba.width;
	height = (pixel) rgba.height;
}
void ImageGlyph::draw(const box2px& bounds, const Color& fgColor,
		const Color& bgColor, AlloyContext* context) {
	NVGcontext* nvg = context->getNVG();
	NVGpaint imgPaint = nvgImagePattern(nvg, bounds.position.x,
			bounds.position.y, bounds.dimensions.x, bounds.dimensions.y, 0.f,
			handle, 1.0f);
	nvgBeginPath(nvg);
	nvgFillColor(nvg, Color(COLOR_WHITE));
	nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
			bounds.dimensions.y);
	nvgFillPaint(nvg, imgPaint);
	nvgFill(nvg);
	if (fgColor.a > 0) {
		nvgBeginPath(nvg);
		nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
				bounds.dimensions.y);
		nvgFillColor(nvg, Color(fgColor));
		nvgFill(nvg);
	}

}
CheckerboardGlyph::CheckerboardGlyph(int width, int height, int horizTiles,
		int vertTiles, AlloyContext* context, bool mipmap) :
		Glyph("image_rgba", GlyphType::Image, (pixel) width, (pixel) height) {
	ImageRGBA img(width, height);
	MakeCheckerBoard(img, horizTiles, vertTiles);
	handle = nvgCreateImageRGBA(context->getNVG(), width, height,
			(mipmap) ? NVG_IMAGE_GENERATE_MIPMAPS : 0, img.ptr());
}
void CheckerboardGlyph::draw(const box2px& bounds, const Color& fgColor,
		const Color& bgColor, AlloyContext* context) {
	NVGcontext* nvg = context->getNVG();
	NVGpaint imgPaint = nvgImagePattern(nvg, bounds.position.x,
			bounds.position.y, bounds.dimensions.x, bounds.dimensions.y, 0.f,
			handle, 1.0f);
	if (bgColor.a > 0) {
		nvgBeginPath(nvg);
		nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
				bounds.dimensions.y);
		nvgFillColor(nvg, Color(bgColor));
		nvgFill(nvg);
	}
	nvgBeginPath(nvg);
	nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
			bounds.dimensions.y);
	nvgFillPaint(nvg, imgPaint);
	nvgFill(nvg);
	if (fgColor.a > 0) {
		nvgBeginPath(nvg);
		nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
				bounds.dimensions.y);
		nvgFillColor(nvg, Color(fgColor));
		nvgFill(nvg);
	}
}
void AlloyContext::addAssetDirectory(const std::string& dir) {
	std::string dirCopy = dir;
	if (ALY_PATH_SEPARATOR[0] != '/') {
		for (char& c : dirCopy) {
			if (c == '/') {
				c = ALY_PATH_SEPARATOR[0];
			}
		}
	}
	else if (ALY_PATH_SEPARATOR[0] != '\\') {
		for (char& c : dirCopy) {
			if (c == '\\') {
				c = ALY_PATH_SEPARATOR[0];
			}
		}
	}
	//Mitigate duplicate entries
	bool found = false;
	for (std::string str : assetDirectories) {
		if (str == dirCopy) {
			found = true;
			break;
		}
	}
	if (!found) {
		assetDirectories.push_back(dirCopy);
	}
}
std::shared_ptr<Font> AlloyContext::loadFont(FontType type,
		const std::string& name, const std::string& file) {

	try {
		auto f = std::shared_ptr<Font>(new Font(name, getFullPath(file), this));
		int idx = static_cast<int>(type);
		if (idx >= (int) fonts.size()) {
			fonts.resize(idx + 1);
		}
		fonts[idx] = f;
		return fonts[static_cast<int>(type)];
	} catch (...) {
		std::cerr << "Could not load font " << type << std::endl;
	}
	return std::shared_ptr<Font>();
}
std::shared_ptr<Font> AlloyContext::loadFont(int idx, const std::string& name,
		const std::string& file) {

	try {
		auto f = std::shared_ptr<Font>(new Font(name, getFullPath(file), this));
		if (idx >= (int) fonts.size()) {
			fonts.resize(idx + 1);
		}
		fonts[idx] = f;
		return fonts[idx];
	} catch (...) {
		std::cerr << "Could not load font " << name << std::endl;
	}
	return std::shared_ptr<Font>();
}
std::shared_ptr<Font> AlloyContext::loadFont(const std::string& name,
		const std::string& file) {
	try {
		fonts.push_back(
				std::shared_ptr<Font>(new Font(name, getFullPath(file), this)));
		return fonts.back();
	} catch (...) {
		std::cerr << "Could not load font " << name << std::endl;
	}
	return std::shared_ptr<Font>();
}
int AlloyContext::getFrameBufferWidth() const {
	return getCurrentWindow()->getFrameBufferSize().x;
}
int AlloyContext::getFrameBufferHeight() const {
	return getCurrentWindow()->getFrameBufferSize().y;
}
int2 AlloyContext::getFrameBufferSize() const {
	return getCurrentWindow()->getFrameBufferSize();
}
std::string AlloyContext::getFullPath(const std::string& partialFile) {
	std::string fileName = partialFile;
	if (ALY_PATH_SEPARATOR[0] != '/') {
		for (char& c : fileName) {
			if (c == '/') {
				c = ALY_PATH_SEPARATOR[0];
			}
		}
	}
	else if (ALY_PATH_SEPARATOR[0] != '\\') {
		for (char& c : fileName) {
			if (c == '\\') {
				c = ALY_PATH_SEPARATOR[0];
			}
		}
	}
	for (std::string& dir : assetDirectories) {
		std::string fullPath = RemoveTrailingSlash(dir) + ALY_PATH_SEPARATOR+ fileName;
		if (FileExists(fullPath)) {
			if (fullPath.size()>2&&fullPath.substr(0, 2) == "..") {
				fullPath = RemoveTrailingSlash(GetCurrentWorkingDirectory()) + ALY_PATH_SEPARATOR + fullPath;
			}
			return fullPath;
		}
	}
	std::string executableDir = GetExecutableDirectory();
	for (std::string& dir : assetDirectories) {
		std::string fullPath = RemoveTrailingSlash(executableDir)
				+ ALY_PATH_SEPARATOR+ RemoveTrailingSlash(dir) + ALY_PATH_SEPARATOR + fileName;
		if (FileExists(fullPath)) {
			return fullPath;
		}
	}
	std::cout << "Could not find \"" << fileName
			<< "\"\nThis is where I looked:" << std::endl;
	for (std::string& dir : assetDirectories) {
		std::string fullPath = RemoveTrailingSlash(dir) + ALY_PATH_SEPARATOR+ fileName;
		std::cout << "\"" << fullPath << "\"" << std::endl;
		fullPath = executableDir + ALY_PATH_SEPARATOR + RemoveTrailingSlash(dir) + ALY_PATH_SEPARATOR + fileName;
		std::cout << "\"" << fullPath << "\"" << std::endl;
	}
	throw std::runtime_error(
			MakeString() << "Could not find \"" << fileName << "\"");
	return std::string("");
}
void AlloyContext::begin(const WindowPtr& window) {
	if (glfwGetCurrentContext()) {
		windowHistory.push_back(windowMap[glfwGetCurrentContext()]);
	}
	window->begin();
}
void AlloyContext::end() {
	if (windowHistory.size() > 0) {
		windowHistory.back()->setCurrent();
		windowHistory.pop_back();
	}
}
bool AlloyContext::isControlDown() const {
	GLFWwindow* window = getCurrentWindow()->handle;
	return ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)
			| glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL)) != 0);
}
bool AlloyContext::isShiftDown() const {
	GLFWwindow* window = getCurrentWindow()->handle;
	return ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)
			| glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)) != 0);
}
bool AlloyContext::isAltDown() const {
	GLFWwindow* window = getCurrentWindow()->handle;
	return ((glfwGetKey(window, GLFW_KEY_LEFT_ALT)
			| glfwGetKey(window, GLFW_KEY_RIGHT_ALT)) != 0);
}
bool AlloyContext::isSuperDown() const {
	GLFWwindow* window = getCurrentWindow()->handle;
	return ((glfwGetKey(window, GLFW_KEY_LEFT_SUPER)
			| glfwGetKey(window, GLFW_KEY_RIGHT_SUPER)) != 0);
}
void AlloyContext::setDragObject(Region* region) {
	mouseDownRegion = region;
	cursorDownPosition = cursorPosition - mouseDownRegion->getBoundsPosition();
}
bool AlloyContext::isOnTop(Region* region) const {
	return (onTopRegion != nullptr
			&& (onTopRegion == region || region->hasParent(onTopRegion)));
}
pixel2 AlloyContext::getCursorDownPosition() const {
	return cursorDownPosition
			+ ((mouseDownRegion != nullptr) ?
					mouseDownRegion->getBoundsPosition() : pixel2(0.0f));
}
bool AlloyContext::fireListeners(const InputEvent& event) {
	firingListeners = true;
	if (event.type
			== InputType::Key&& event.isDown() && event.key == GLFW_KEY_TAB) {
		if (objectFocusRegion != nullptr && objectFocusRegion->isVisible()) {
			if (event.isShiftDown()) {
				objectFocusRegion->focusPrevious();
				//objectFocusRegion->printFocus();
				return true;
			} else {
				objectFocusRegion->focusNext();
				//objectFocusRegion->printFocus();
				return true;
			}
		}
	}
	for (auto iter = listeners.rbegin(); iter != listeners.rend(); iter++) {
		if (firingListeners) {
			EventHandler* handler = iter->first;
			try {
				if (handler->onEventHandler(this, event))
					return true;
			} catch (std::exception& e) {
				std::cerr << "Error occurred while firing events for "
						<< iter->second << ": " << e.what() << std::endl;
				return false;
			}
			if (!firingListeners) {
				return false;
			}
		} else {
			return false;
		}
	}
	return false;
}
void AlloyContext::interruptListeners() {
	firingListeners = false;
}
void AlloyContext::addListener(EventHandler* region) {
	firingListeners = false;
	auto pos = listeners.find(region);
	if (pos == listeners.end()) {
		listeners[region] = region->getName();
	}
}
bool AlloyContext::hasListener(EventHandler* region) const {
	auto pos = listeners.find((EventHandler*) region);
	return (pos != listeners.end());
}
void AlloyContext::removeListener(const EventHandler* region) {
	firingListeners = false;
	auto pos = listeners.find((EventHandler*) region);
	if (pos != listeners.end()) {
		listeners.erase(pos);
	}
}
EventHandler::~EventHandler() {
	Application::removeListener(this);
}
void AlloyContext::setOnTopRegion(Region* region) {
	if (region == nullptr)
		throw std::runtime_error(
				"On top region cannot be null. use removeOnTopRegion() instead.");
	if (onTopRegion != nullptr) {
		if (onTopRegion->onRemoveFromOnTop)
			onTopRegion->onRemoveFromOnTop();
	}
	onTopRegion = region;
	region->setVisible(true);
}
void AlloyContext::removeOnTopRegion(Region* region) {
	if (region == nullptr)
		throw std::runtime_error("Remove on top region cannot be null.");
	if (region == onTopRegion) {
		if (onTopRegion->onRemoveFromOnTop)
			onTopRegion->onRemoveFromOnTop();
		onTopRegion = nullptr;
	}
}
WindowPtr AlloyContext::addWindow(const std::string& name, int width,
		int height) {
	WindowPtr win(new Window(title, width, height));
	windowMap[win->handle] = win;
	windows.push_back(win);
	return win;
}
bool AlloyContext::remove(Window* w) {
	for (auto iter = windows.begin(); iter != windows.end(); iter++) {
		WindowPtr win = *iter;
		if (win.get() == w) {
			windows.erase(iter);
			windowMap.erase(windowMap.find(win->handle));
			return true;
		}
	}
	return false;
}
AlloyContext::AlloyContext(const std::string& title, int width, int height,
		const Theme& theme) :
		theme(theme), title(title), firingListeners(false) {
	if (glfwInit() != GL_TRUE) {
		throw std::runtime_error("Could not initialize GLFW.");
	}
	glfwSetErrorCallback([](int error, const char* desc) {
		std::cout << "GLFW Error [" << error << "] " << desc << std::endl;
	});
	WindowPtr mainWindow = addWindow(title, width, height);
	int widthMM, heightMM;
	numMonitors = 0;
	glfwGetMonitors(&numMonitors);
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (monitor == nullptr)
		throw std::runtime_error("Could not find monitor.");
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (mode == nullptr)
		throw std::runtime_error("Could not find video monitor.");
	glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
	dpmm = double2(mode->width / (double) widthMM,
			mode->height / (double) heightMM);
	pixelRatio = mainWindow->getPixelRatio();
	lastAnimateTime = std::chrono::steady_clock::now();
	lastCursorTime = std::chrono::steady_clock::now();
	lastUpdateTime = std::chrono::steady_clock::now();
	cursor = &Cursor::Normal;
}
int AlloyContext::getMonitorCount() const {
	return numMonitors;
}
int2 AlloyContext::getMonitorPhysicalSize(int idx) const {
	int widthMM, heightMM;
	int num = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&num);
	if (idx < num) {
		glfwGetMonitorPhysicalSize(monitors[idx], &widthMM, &heightMM);
		return int2(widthMM, heightMM);
	} else {
		return int2(0);
	}
}
int2 AlloyContext::getMonitorSize(int idx) const {
	int mon = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&mon);
	if (idx < mon) {
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[idx]);
		return int2(mode->width, mode->height);
	} else {
		return int2(0);
	}
}
double2 AlloyContext::getMonitorDpmm(int idx) const {
	int widthMM, heightMM;
	int mon = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&mon);
	if (idx < mon) {
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[idx]);
		glfwGetMonitorPhysicalSize(monitors[idx], &widthMM, &heightMM);
		return double2(mode->width / (double) widthMM,
				mode->height / (double) heightMM);
	} else {
		return double2(0.0);
	}
}
void AlloyContext::addDeferredTask(const std::function<void()>& func,
		bool block) {
	std::lock_guard<std::mutex> guard(taskLock);
	deferredTasks.push_back(func);
	if (block) {
		std::thread::id currentThread = std::this_thread::get_id();
		if (currentThread != threadId) {
			std::this_thread::yield();
			while (deferredTasks.size() > 0) {
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		} else {
			throw std::runtime_error(
					"Cannot block and wait for deferred task on same thread as Alloy context.");
		}
	}
}
bool AlloyContext::executeDeferredTasks() {
	std::lock_guard<std::mutex> guard(taskLock);
	if (deferredTasks.size() > 0) {
		for (std::function<void()>& func : deferredTasks) {
			func();
		}
		deferredTasks.clear();
		return true;
	}
	return false;
}
bool AlloyContext::isMouseContainedIn(Region* region) const {
	return (region->getBounds().contains(cursorPosition));
}
bool AlloyContext::isMouseContainedIn(const box2px& box) const {
	return (box.contains(cursorPosition));
}
bool AlloyContext::isMouseContainedIn(const pixel2& pos,
		const pixel2& dims) const {
	return ((box2px(pos, dims)).contains(cursorPosition));
}
bool AlloyContext::isMouseOver(Region* region, bool includeParent) const {
	if (includeParent) {
		return (region != nullptr
				&& (mouseOverRegion == region
						|| (mouseOverRegion != nullptr
								&& mouseOverRegion->hasParent(region))));
	} else {
		return (mouseOverRegion == region && region != nullptr);
	}
}
void AlloyContext::setMouseDownObject(Region* region) {
	if (region != nullptr && mouseDownRegion != nullptr) {
		cursorDownPosition = cursorDownPosition
				+ mouseDownRegion->getBoundsPosition()
				- region->getBoundsPosition();
	}
	mouseDownRegion = region;
}
bool AlloyContext::isMouseDown(Region* region, bool includeParent) const {
	if (includeParent) {
		return (mouseDownRegion == region
				|| (mouseDownRegion != nullptr
						&& mouseDownRegion->hasParent(region)));
	} else {
		return (mouseDownRegion == region);
	}
}
std::shared_ptr<Composite>& AlloyContext::getGlassPane() {
	return getCurrentWindow()->glass;
}
void AlloyContext::clearEvents() {
	mouseOverRegion = nullptr;
	mouseDownRegion = nullptr;
	cursorFocusRegion = nullptr;
	objectFocusRegion = nullptr;
	onTopRegion = nullptr;
	firingListeners = false;
}
void AlloyContext::clearEvents(Region* region) {
	for (auto win : windows) {
		win->resetLocator();
	}
	if (mouseOverRegion != nullptr
			&& (region == mouseOverRegion || mouseOverRegion->hasParent(region)))
		mouseOverRegion = nullptr;
	if (mouseDownRegion != nullptr
			&& (region == mouseDownRegion || mouseDownRegion->hasParent(region)))
		mouseDownRegion = nullptr;
	if (cursorFocusRegion != nullptr
			&& (region == cursorFocusRegion
					|| cursorFocusRegion->hasParent(region)))
		cursorFocusRegion = nullptr;
	if (objectFocusRegion != nullptr
			&& (region == objectFocusRegion
					|| objectFocusRegion->hasParent(region)))
		objectFocusRegion = nullptr;
	if (onTopRegion != nullptr
			&& (region == onTopRegion || onTopRegion->hasParent(region)))
		onTopRegion = nullptr;

}

bool AlloyContext::isOnScreenRender() const {
	return (getCurrentWindow()->isVisible());
}
bool AlloyContext::isOffScreenRender() const {
	return (!getCurrentWindow()->isVisible());
}

bool AlloyContext::isCursorFocused(const Region* region) {
	if (cursorFocusRegion != nullptr) {
		if (cursorFocusRegion->isVisible()) {
			return (region == cursorFocusRegion);
		} else {
			cursorFocusRegion = nullptr;
			return false;
		}
	}
	return false;
}
bool AlloyContext::isObjectFocused(const Region* region) {
	if (objectFocusRegion != nullptr) {
		if (objectFocusRegion->isVisible()) {
			return (region == objectFocusRegion);
		} else {
			objectFocusRegion = nullptr;
			return false;
		}
	}
	return false;
}
void AlloyContext::update(Window* win) {
	endTime = std::chrono::steady_clock::now();
	double updateElapsed = std::chrono::duration<double>(
			endTime - lastUpdateTime).count();
	double animateElapsed = std::chrono::duration<double>(
			endTime - lastAnimateTime).count();
	double cursorElapsed = std::chrono::duration<double>(
			endTime - lastCursorTime).count();
	if (deferredTasks.size() > 0) {
		executeDeferredTasks();
		win->ui->updateCursor(win->resetLocator());
		win->dirtyLocator = false;
		mouseOverRegion = win->locate(cursorPosition);
		dirtyCursor = false;
		win->dirtyUI= true;
	}
	if (updateElapsed > UPDATE_LOCATOR_INTERVAL_SEC) {
		if (win->dirtyLocator) {
			win->ui->updateCursor(win->resetLocator());
			win->dirtyLocator = false;
			mouseOverRegion = win->locate(cursorPosition);
			dirtyCursor = false;
		}
		lastUpdateTime = endTime;
	}
	if (cursorElapsed >= UPDATE_CURSOR_INTERVAL_SEC) { //Dont try to animate faster than 60 fps.
		if (dirtyCursor && !win->dirtyLocator) {
			mouseOverRegion = win->locate(cursorPosition);
			dirtyCursor = false;
		}
		dirtyUI = true;
		lastCursorTime = endTime;
	}
	if (animateElapsed >= ANIMATE_INTERVAL_SEC) { //Dont try to animate faster than 60 fps.
		lastAnimateTime = endTime;
		if (animator.step(animateElapsed)) {
			win->dirtyUI= true;
			dirtyUI = true;
		}
	}
	if (win->dirtyUI) {
		win->ui->pack(this);
		animator.firePostEvents();
		win->dirtyLocator = true;
		win->dirtyUI = false;
	}

}

void AlloyContext::forceDestroy() {
	for (auto win : windows) {
		win->handle = nullptr;
	}
}

void AlloyContext::setCursor(const Cursor* cursor) {
	this->cursor = cursor;
}
const Cursor* AlloyContext::getCursor() const {
	return cursor;
}
std::shared_ptr<AlloyContext>& AlloyContext::getDefaultContext() {
	return defaultContext;
}
pixel2 AlloyContext::getCursorPosition() const {
	return cursorPosition;
}
std::mutex& AlloyContext::getLock() {
	return glLock;
}
const std::mutex& AlloyContext::getLock() const {
	return glLock;
}
box2px AlloyContext::getViewport() const {
	return box2px(pixel2(0.0f, 0.0f),
			pixel2(getCurrentWindow()->getFrameBufferSize()));
}
NVGcontext* AlloyContext::getNVG() const {
	return getCurrentWindow()->nvg;
}
int AlloyContext::getScreenWidth() const {
	return getCurrentWindow()->getScreenSize().x;
}
int2 AlloyContext::getScreenSize() const {
	return getCurrentWindow()->getScreenSize();
}
int AlloyContext::getScreenHeight() const {
	return getCurrentWindow()->getScreenSize().y;
}
WindowPtr AlloyContext::getCurrentWindow() const {
	auto ptr = glfwGetCurrentContext();
	if (ptr != nullptr) {
		return windowMap.at(ptr);
	} else {
		throw std::runtime_error("No current window available.");
		return WindowPtr();
	}
}
pixel2 AlloyContext::getRelativeCursorDownPosition() const {
	return cursorDownPosition;
}
bool AlloyContext::hasDeferredTasks() const {
	return (deferredTasks.size() > 0);
}

AlloyContext::~AlloyContext() {
	windows.clear();
	windowMap.clear();
	glfwTerminate();
}
}

