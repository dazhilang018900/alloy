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

#ifndef ALLOYCONTEXT_H_
#define ALLOYCONTEXT_H_
//Include cereal here to avoid name conflict with Xlib definition of "Bool"
#include "common/cereal/archives/json.hpp"
#define GLFW_INCLUDE_GLU
#ifndef __gl_h_
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/glew.h>
#else
#include <GL/glew.h>
#if !defined(__APPLE__) || !defined(__MACH__)
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#endif
#endif
#include <GLFW/glfw3.h>
#include <mutex>
#include <memory>
#include <list>
#include <map>
#include <thread>
#include <string>
#include "common/nanovg.h"
#include "math/AlloyVecMath.h"
#include "common/AlloyUnits.h"
#include "image/AlloyImage.h"
#include "ui/AlloyAnimator.h"
#include "ui/AlloyEnum.h"
#include "ui/AlloyEvent.h"
#include "AlloyCursorLocator.h"
int printOglError(const char *file, int line);
#define CHECK_GL_ERROR() printOglError(__FILE__, __LINE__)

namespace aly {
struct ImageVAO {
	GLuint vao = 0;
	GLuint positionBuffer = 0;
	GLuint uvBuffer = 0;
};
class Application;
class Composite;
class Font;
class Window {
protected:
	CursorLocator locator;
	bool visible;
	std::string name;
	bool hideOnClose;
public:
	GLFWwindow* handle;
	NVGcontext* nvg;
	ImageVAO vao;
	bool dirtyLocator;
	bool dirtyUI;
	std::vector<std::shared_ptr<Font>> fonts;
	Application* app;
	std::shared_ptr<Composite> ui;
	std::shared_ptr<Composite> glass;
	std::string getName() const {
		return name;
	}
	bool isHideOnClose() const;
	void setLocation(int x, int y);
	int2 getLocation() const;
	void requestPack();
	void requestCursorUpdate();
	Window(const std::string& title, int width, int height,bool hideOnClose=true);
	Window();
	int2 getScreenSize() const;
	int2 getFrameBufferSize() const;
	box2px getViewport() const;
	float getPixelRatio() const;
	void swapBuffers() const;
	void setSwapInterval(int interval) const;
	void init(const std::string& title, int width, int height);
	void setVisible(bool v);
	void registerCallbacks(Application* app);
	bool isVisible() const;
	CursorLocator* resetLocator();
	void setCurrent() const;
	void setFocused() const;
	void begin() const;
	bool shouldClose() const;
	void add(Region* region);
	Region* locate(const pixel2& cursor) const;
	~Window();
};
typedef std::shared_ptr<Window> WindowPtr;

const RGBA COLOR_NONE(0, 0, 0, 0);
const RGBA COLOR_BLACK(0, 0, 0, 255);
const RGBA COLOR_WHITE(255, 255, 255, 255);

class AlloyContext;
struct Font {
private:
	NVGcontext* nvg;
public:
	int handle;
	const std::string name;
	const std::string file;
	int getCursorPosition(const std::string & text, float fontSize,
			int xCoord) const;
	Font(const std::string& name,NVGcontext* nvg, const std::string& file,
			AlloyContext* context);
};
struct Glyph {
	const std::string name;
	const GlyphType type;
	pixel width;
	pixel height;
	Glyph(const std::string& name, GlyphType type, pixel w, pixel h) :
			name(name), type(type), width(w), height(h) {

	}
	virtual inline ~Glyph() {
	}
	;
	virtual void draw(const box2px& bounds, const Color& fgColor,
			const Color& bgColor, AlloyContext* context) = 0;
};
struct ImageGlyph: public Glyph {
	int handle;
	const std::string file;
	ImageGlyph(const std::string& file, AlloyContext* context, bool mipmap =
			false);
	ImageGlyph(const ImageRGBA& rgba, AlloyContext* context,
			bool mipmap = false);
	ImageGlyph(const ImageRGB& rgb, AlloyContext* context, bool mipmap = false);
	void draw(const box2px& bounds, const Color& fgColor, const Color& bgColor,
			AlloyContext* context) override;
	void set(const ImageRGBA& rgba, AlloyContext* context);
	void set(const ImageRGB& rgb, AlloyContext* context);
	void set(const ImageRGBAf& rgba, AlloyContext* context);
	~ImageGlyph();
};
struct CheckerboardGlyph: public Glyph {
	int handle;
	CheckerboardGlyph(int width, int height, int horizTiles, int vertTiles,
			AlloyContext* context, bool mipmap = false);
	void draw(const box2px& bounds, const Color& fgColor, const Color& bgColor,
			AlloyContext* context) override;
};
struct AwesomeGlyph: public Glyph {
	const int codePoint;
	const FontStyle style;
	AwesomeGlyph(int codePoint, AlloyContext* context, const FontStyle& style =
			FontStyle::Normal, pixel fontHeight = 32);
	void draw(const box2px& bounds, const Color& fgColor, const Color& bgColor,
			AlloyContext* context) override;
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Font & v) {
	return ss << "Font: " << v.name << "[" << v.handle << "]: \"" << v.file
			<< "\"";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const ImageGlyph & v) {
	return ss << "ImageGlyph: " << v.name << "[" << v.handle
			<< "]: dimensions= (" << v.width << ", " << v.height << ")";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const AwesomeGlyph & v) {
	return ss << "Awesome Glyph: " << v.name << "[" << v.codePoint
			<< "]: dimensions= (" << v.width << ", " << v.height << ")";
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Glyph & v) {
	return ss << "Glyph: " << v.name << ": dimensions= (" << v.width << ", "
			<< v.height << ")";
}

struct Cursor {
	static const Cursor Normal, Horizontal, Vertical, Position, Hand, Rotate,
			Grab, SlantUp, SlantDown, TextInsert, CrossHairs, Hidden;
	std::string codeString;
	FontType fontType;
	float fontSize;
	float angle;
	int align;
	pixel2 nudge;
	Cursor(int code, float fontSize, int align = NVG_ALIGN_TOP | NVG_ALIGN_LEFT,
			const FontType& fontType = FontType::Icon, float angle = 0.0f,
			const pixel2& nudge = pixel2(0.0f)) :
			codeString(CodePointToUTF8(code)), fontType(fontType), fontSize(
					fontSize), angle(angle), align(align), nudge(nudge) {
	}
	void draw(AlloyContext* context) const;
};
class Composite;
class Region;
class AlloyContext {
private:
	std::string title;
	std::thread::id threadId;
	std::mutex taskLock;
	std::mutex glLock;
	std::list<std::string> assetDirectories;
	std::list<WindowPtr> windowHistory;
	std::map<GLFWwindow*, WindowPtr> windowMap;
	std::vector<WindowPtr> windows;
	bool dirtyUI = true;
	bool dirtyCursor = false;
	bool enableDebugInterface = false;
	Animator animator;
	//CursorLocator cursorLocator;
	const double ANIMATE_INTERVAL_SEC = 1.0 / 30.0;
	const double UPDATE_LOCATOR_INTERVAL_SEC = 1.0 / 30.0;
	const double UPDATE_CURSOR_INTERVAL_SEC = 1.0 / 90.0;
	bool leftMouseButton = false;
	bool middleMouseButton = false;
	bool rightMouseButton = false;
	std::chrono::steady_clock::time_point endTime;
	std::chrono::steady_clock::time_point lastAnimateTime;
	std::chrono::steady_clock::time_point lastUpdateTime;
	std::chrono::steady_clock::time_point lastCursorTime;
	Region* mouseOverRegion = nullptr;
	Region* mouseDownRegion = nullptr;
	Region* cursorFocusRegion = nullptr;
	Region* objectFocusRegion = nullptr;
	Region* onTopRegion = nullptr;
	const Cursor* cursor = nullptr;
	std::map<EventHandler*, std::string> listeners;
	std::shared_ptr<Composite> glassPane;
	std::list<std::function<void()>> deferredTasks;
	static std::shared_ptr<AlloyContext> defaultContext;
	//int2 viewSize;
	//int2 screenSize;
	bool firingListeners;
	int numMonitors;
	void addListener(EventHandler* region);
	void removeListener(const EventHandler* region);
	bool hasListener(EventHandler* region) const;
public:
	friend class Application;
	//NVGcontext* nvgContext;
	//GLFWwindow* window;
	//GLFWwindow* offscreenWindow;
	const Theme theme;
	ImageVAO vaoImageOffScreen;
	pixel2 cursorPosition = pixel2(-1, -1);
	pixel2 cursorDownPosition = pixel2(-1, -1);
	int doubleClickTime = 300;
	double2 dpmm;
	Window* focusedWindow=nullptr;
	const std::vector<WindowPtr>& getWindows() const {
		return windows;
	}
	std::vector<WindowPtr>& getWindows() {
		return windows;
	}
	int getMonitorCount() const;
	int2 getMonitorSize(int idx) const;
	double2 getMonitorDpmm(int idx) const;
	int2 getMonitorPhysicalSize(int idx) const;
	bool isOnScreenRender() const;
	bool isOffScreenRender() const;
	void clearEvents(Region* region);
	void clearEvents();
	void forceDestroy();
	WindowPtr getMainWindow() const{
		return windows[0];
	}
	WindowPtr addWindow(const std::string& name,int width,int height,bool hideOnClose=true);
	bool remove(Window* win);
	void begin(const WindowPtr& window);
	inline void begin(){
		begin(getCurrentWindow());
	}
	void end();
	void setCursor(const Cursor* cursor);
	const Cursor* getCursor() const;
	static std::shared_ptr<AlloyContext>& getDefaultContext();
	pixel2 getCursorPosition() const;
	std::mutex& getLock();
	const std::mutex& getLock() const;
	box2px getViewport() const;
	NVGcontext* getNVG() const;
	int getScreenWidth() const;
	int getScreenHeight() const;
	int2 getScreenSize() const;
	WindowPtr getCurrentWindow() const;
	pixel2 getRelativeCursorDownPosition() const;
	bool hasDeferredTasks() const;
	void addDeferredTask(const std::function<void()>& func, bool block = false);
	bool executeDeferredTasks();
	std::shared_ptr<Composite>& getGlassPane();
	pixel2 getCursorDownPosition() const;
	bool fireListeners(const InputEvent& event);
	void interruptListeners();
	void setDragObject(Region* region);
	bool isMouseOver(Region* region, bool includeParent = false) const;
	bool isMouseDown(Region* region, bool includeParent = false) const;
	bool isCursorFocused(const Region* region);
	bool isObjectFocused(const Region* region);
	bool isControlDown() const;
	bool isShiftDown() const;
	bool isAltDown() const;
	bool isSuperDown() const;
	void setMouseDownObject(Region* region);
	Region* getMouseDownObject() const {
		return mouseDownRegion;
	}
	inline void setCursorFocus(Region* region) {
		cursorFocusRegion = region;
	}
	inline Region* getCursorFocus() const {
		return cursorFocusRegion;
	}
	inline void setObjectFocus(Region* region) {
		objectFocusRegion = region;
	}
	inline Region* getObjectFocus() const {
		return objectFocusRegion;
	}
	void setOnTopRegion(Region* region);
	void removeOnTopRegion(Region* region);

	inline Region* getOnTopRegion() const {
		return onTopRegion;
	}
	bool isMouseContainedIn(const box2px& box) const;
	bool isMouseContainedIn(const pixel2& pos, const pixel2& dims) const;
	bool isMouseContainedIn(Region* region) const;
	bool isLeftMouseButtonDown() const {
		return leftMouseButton;
	}
	bool isMiddleMouseButtonDown() const {
		return middleMouseButton;
	}
	bool isRightMouseButtonDown() const {
		return rightMouseButton;
	}
	void setLeftMouseButtonDown(bool f) {
		leftMouseButton = f;
	}
	void setMiddleMouseButtonDown(bool f) {
		middleMouseButton = f;
	}
	void setRightMouseButtonDown(bool f) {
		rightMouseButton = f;
	}
	inline bool isMouseDown() const {
		return (mouseDownRegion != nullptr);
	}
	bool isOnTop(Region* region) const;
	inline bool isMouseDrag(Region* region) {
		return (region != nullptr && mouseDownRegion == region
				&& leftMouseButton);
	}
	inline bool isMouseDrag() {
		return (mouseDownRegion != nullptr && leftMouseButton);
	}
	pixel pixelRatio;
	void update(Window* win);
	void requestPack() {
		getCurrentWindow()->dirtyUI=true;
	}
	void requestUpdateCursor() {
		dirtyCursor = true;
	}
	void requestUpdateCursorLocator() {
		getCurrentWindow()->dirtyLocator=true;
	}
	inline void setDebug(bool enabled) {
		enableDebugInterface = enabled;
	}
	inline bool toggleDebug() {
		return enableDebugInterface = !enableDebugInterface;
	}
	inline bool isDebugEnabled() {
		return enableDebugInterface;
	}
	std::list<std::string>& getAssetDirectories() {
		return assetDirectories;
	}
	void addAssetDirectory(const std::string& dir);
	std::shared_ptr<Font> loadFont(FontType type, const std::string& name,
			const std::string& partialFile);
	std::shared_ptr<Font> loadFont(int idx, const std::string& name,
			const std::string& partialFile);
	std::shared_ptr<Font> loadFont(const std::string& name,
			const std::string& partialFile);
	std::string getFullPath(const std::string& partialFile);
	int getFrameBufferWidth() const;
	int getFrameBufferHeight() const;
	int2 getFrameBufferSize() const;
	const char* getFontName(FontType type,Window* win) const;
	int getFontHandle(FontType type,Window* win) const ;
	const char* getFontName(FontType type) const;
	int getFontHandle(FontType type) const ;
	inline std::shared_ptr<ImageGlyph> createImageGlyph(
			const std::string& fileName, bool mipmap = false) {
		return std::shared_ptr<ImageGlyph>(new ImageGlyph(fileName, this));
	}
	inline std::shared_ptr<ImageGlyph> createImageGlyph(const ImageRGBA& img,
			bool mipmap = false) {
		return std::shared_ptr<ImageGlyph>(new ImageGlyph(img, this));
	}
	inline std::shared_ptr<ImageGlyph> createImageGlyph(const ImageRGB& img,
			bool mipmap = false) {
		return std::shared_ptr<ImageGlyph>(new ImageGlyph(img, this));
	}
	inline std::shared_ptr<AwesomeGlyph> createAwesomeGlyph(int codePoint,
			const FontStyle& style = FontStyle::Normal, pixel height = 32) {
		std::shared_ptr<AwesomeGlyph> g = std::shared_ptr<AwesomeGlyph>(
				new AwesomeGlyph(codePoint, this, style, height));
		return g;
	}std::shared_ptr<Font>& getFont(FontType type);
	template<class A> std::shared_ptr<Tween>& addTween(AColor& out,
			const Color& start, const Color& end, double duration, const A& a =
					Linear()) {
		return animator.add(out, start, end, duration, a);
	}
	template<class A> std::shared_ptr<Tween>& addTween(AUnit2D& out,
			const AUnit2D& start, const AUnit2D& end, double duration,
			const A& a = Linear()) {
		return animator.add(out, start, end, duration, a);
	}
	template<class A> std::shared_ptr<Tween>& addTween(AUnit1D& out,
			const AUnit1D& start, const AUnit1D& end, double duration,
			const A& a = Linear()) {
		return animator.add(out, start, end, duration, a);
	}
	AlloyContext(const std::string& title,int width=1920, int height=1080,
			const Theme& theme = Theme::Default);
	void repaintUI() {
		dirtyUI = true;
	}
	~AlloyContext();
};
inline std::shared_ptr<AlloyContext>& AlloyDefaultContext() {
	return AlloyContext::getDefaultContext();
}
typedef std::shared_ptr<Font> FontPtr;
typedef std::shared_ptr<Glyph> GlyphPtr;
typedef std::shared_ptr<AwesomeGlyph> AwesomeGlyphPtr;
typedef std::shared_ptr<ImageGlyph> ImageGlyphPtr;

}
#endif /* ALLOYCONTEXT_H_ */
