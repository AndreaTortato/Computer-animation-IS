#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT

#include "nuklear.h"
#include "nuklear_glfw_gl3.h"
#include "math/vec2.h"

class Application {
private:
	Application(const Application&);
	Application& operator=(const Application&);

public:
	bool close = false;
	vec2 mousePosition;
	vec2 lastMousePosition;

	Application() { };
	inline virtual ~Application() { };
	inline virtual void init() { };
	inline virtual void update(float dt) { };
	inline virtual void render(float aspectRatio) { };
	inline virtual void ImGui(nk_context* inContext) { };
	inline virtual void shutdown() { };

	inline virtual void onKeyDown(int key, int scancode) { };
	inline virtual void onKeyUp(int key, int scancode) { };
	inline virtual void onRightMouseButtonDown() { };
	inline virtual void onRightMouseButtonUp() { };
	inline virtual void onLeftMouseButtonDown() { };
	inline virtual void onLeftMouseButtonUp() { };
	inline virtual void onMiddleMouseButtonDown() { };
	inline virtual void onMiddleMouseButtonUp() { };
	inline virtual void onScroll(double xOffset, double yOffset) { };
};