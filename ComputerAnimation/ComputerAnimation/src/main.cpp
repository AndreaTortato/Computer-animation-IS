// Computer Animations framework
// Authors: Eva Valls and Victor Ubieto
// Reference: c++ Game Animation Programming by Gabor Szauer
 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <windows.h>

#include "application.h"
#include "labSelector.h"

// Nuklear
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT

#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

// Select the dedicated graphic card over the integrated
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Frame Timers to evaluate performance
struct FrameTimer {
	// High level timers
	double frameTime = 0.0;
	float  deltaTime = 0.0f;
	// CPU timers
	double frameUpdate = 0.0;
	double frameRender = 0.0;
	double glfwEvents = 0.0;
	double imguiLogic = 0.0;
	double imguiRender = 0.0;
	double swapBuffer = 0.0;
	// GPU timers
	double imguiGPU = 0.0;
	double appGPU = 0.0;
};

// Globals
Application* app; // instance that points to the lab to execute
LabSelector labSelector; // class to manage the lab to play
nk_context* nkContext = 0;
GLuint gVertexArrayObject = 0;
GLuint gGpuApplicationStart = 0;
GLuint gGpuApplicationStop = 0;
GLuint gGpuImguiStart = 0;
GLuint gGpuImguiStop = 0;
bool vsync = true;

static void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
static void onMouseEvent(GLFWwindow* window, int button, int action, int mods);
static void onScroll(GLFWwindow* window, double xoffset, double yoffset);

int main(void)
{
	GLFWwindow* window;

	/* Glfw (Window API) */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1280, 720, "Computer Animation", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	if (vsync == false) {
		glfwSwapInterval(0);
	}
	
	/* Glew (OpenGL API) */
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("\nRenderer: %s", renderer);
	printf("\nOpenGL version supported %s\n\n", version);
	fflush(stdout);

	/* Nuklear (GUI API) */
	nkContext = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

	// Set up font
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&atlas);
	nk_glfw3_font_stash_end();
	// Display helpers
	nk_color defaultColor = nkContext->style.text.color;
	nk_color red = { 255, 0, 0, 255 };
	nk_color orange = { 255, 165, 0, 255 };
	char printBuffer[512];

    double prev_frame_time = 0.0;
	int32_t width, height;
    glfwGetFramebufferSize(window, &width, &height);

	/* Set the lab selector to "Running" state */
	labSelector.init();

	// GPU Timers
	bool slowFrame = false;
	bool firstRenderSample = true;
	GLint timerResultAvailable = 0;
	GLuint64 gpuStartTime = 0;
	GLuint64 gpuStopTime = 0;

	glGenQueries(1, &gGpuApplicationStart);
	glGenQueries(1, &gGpuApplicationStop);
	glGenQueries(1, &gGpuImguiStart);
	glGenQueries(1, &gGpuImguiStop);

	// Setup some OpenGL required state
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	// CPU timings
	int32_t timerFrequency = 30; // the performance profiling will update every <timeFrequency> frames
	float timerStart;
	float timerStop;
	float frameStart;
	float frameStop;
	double timerDiff;
	
	FrameTimer display;
	FrameTimer accumulator;
	memset(&display, 0, sizeof(FrameTimer));
	memset(&accumulator, 0, sizeof(FrameTimer));
	int frameCounter = 0;

	double xpos, ypos; // mouse position vars

	// Bind event callbacks
	glfwSetKeyCallback(window, onKeyEvent);
	glfwSetMouseButtonCallback(window, onMouseEvent);
	glfwSetScrollCallback(window, onScroll);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

        // Poll for and process events
		timerStart = glfwGetTime();
        glfwPollEvents();
		glfwGetCursorPos(window, &xpos, &ypos);
		timerStop = glfwGetTime();
		timerDiff = timerStop - timerStart;
		accumulator.glfwEvents += (double)timerDiff * 1000.0;

		// Start frame for the selected lab
		nk_glfw3_new_frame();
		frameStart = glfwGetTime();
		app = labSelector.getCurrentLab();

		// Update
        double curr_time = glfwGetTime();
        double delta_time = curr_time - prev_frame_time;
        prev_frame_time = curr_time;
		accumulator.deltaTime += delta_time;
		// App update
		if (labSelector.isRunning() && app != 0) {
			app->mousePosition.x = xpos;
			app->mousePosition.y = ypos;
			app->update(delta_time);
		}
		timerStop = glfwGetTime();
		timerDiff = timerStop - curr_time;
		accumulator.frameUpdate += (double)timerDiff * 1000.0;
		
		// Render
		timerStart = glfwGetTime();
        float aspect = (float)width / (float)height;
		glBindVertexArray(gVertexArrayObject);
		// Application GPU Timer
		if (!firstRenderSample) { 
			glGetQueryObjectiv(gGpuApplicationStop, GL_QUERY_RESULT, &timerResultAvailable);
			while (!timerResultAvailable) {
				std::cout << "Waiting on app GPU timer!\n";
				glGetQueryObjectiv(gGpuApplicationStop, GL_QUERY_RESULT, &timerResultAvailable);
			}
			glGetQueryObjectui64v(gGpuApplicationStart, GL_QUERY_RESULT, &gpuStartTime);
			glGetQueryObjectui64v(gGpuApplicationStop, GL_QUERY_RESULT, &gpuStopTime);
			accumulator.appGPU += (double)(gpuStopTime - gpuStartTime) / 1000000.0;
		}
		glQueryCounter(gGpuApplicationStart, GL_TIMESTAMP);
		// App render
		if (labSelector.isRunning() && app != 0) {
			app->render(aspect);
		}
		glQueryCounter(gGpuApplicationStop, GL_TIMESTAMP);
		timerStop = glfwGetTime();
		timerDiff = timerStop - timerStart;
		accumulator.frameRender += (double)timerDiff * 1000.0;

		// IMGUI Update
		timerStart = glfwGetTime();
		if (nkContext) {
			float imguiXPosition = ((float)width) - 205.0f;
			if (nk_begin(nkContext, "Display Stats", nk_rect(imguiXPosition, 5.0f, 200.0f, 65.0f), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(nkContext, 15, 200, 1);

				if (slowFrame) {
					nkContext->style.text.color = red;
				}
				else {
					nkContext->style.text.color = defaultColor;
				}

				double displayFrequency = 1000.0 / display.frameTime;
				sprintf(printBuffer, "Display frequency: %0.2f\0", displayFrequency);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				if (slowFrame) {
					nkContext->style.text.color = defaultColor;
				}

				if (vsync != 0) {
					nk_label(nkContext, "VSynch: ON", NK_TEXT_LEFT);
				}
				else {
					nk_label(nkContext, "VSynch: OFF", NK_TEXT_LEFT);
				}

				double frameBudget = (1000.0 / displayFrequency);
				sprintf(printBuffer, "Frame budget: %0.2f ms\0", frameBudget);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);
			}
			nk_end(nkContext);

			if (nk_begin(nkContext, "High Level Timers", nk_rect(imguiXPosition, 75.0f, 200.0f, 45.0f), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(nkContext, 15, 200, 1);

				if (slowFrame) {
					nkContext->style.text.color = red;
				}
				else {
					nkContext->style.text.color = defaultColor;
				}

				sprintf(printBuffer, "Frame Time: %0.5f ms\0", display.frameTime);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				sprintf(printBuffer, "Delta Time: %0.5f ms\0", display.deltaTime);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				if (slowFrame) {
					nkContext->style.text.color = defaultColor;
				}
			}
			nk_end(nkContext);

			if (nk_begin(nkContext, "GPU Timers", nk_rect(imguiXPosition, 125.0f, 200.0f, 45.0f), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(nkContext, 15, 200, 1);

				sprintf(printBuffer, "Game GPU: %0.5f ms\0", display.appGPU);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				sprintf(printBuffer, "IMGUI GPU: %0.5f ms\0", display.imguiGPU);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);
			}
			nk_end(nkContext);

			if (nk_begin(nkContext, "CPU Timers", nk_rect(imguiXPosition, 175.0f, 200.0f, 120.0f), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(nkContext, 15, 200, 1);

				sprintf(printBuffer, "GLFW Events: %0.5f ms\0", display.glfwEvents);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				sprintf(printBuffer, "Game Update: %0.5f ms\0", display.frameUpdate);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				sprintf(printBuffer, "Game Render: %0.5f ms\0", display.frameRender);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				sprintf(printBuffer, "IMGUI logic: %0.5f ms\0", display.imguiLogic);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				sprintf(printBuffer, "IMGUI render: %0.5f ms\0", display.imguiRender);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);

				sprintf(printBuffer, "Swap Buffers: %0.5f ms\0", display.swapBuffer);
				nk_label(nkContext, printBuffer, NK_TEXT_LEFT);
			}
			nk_end(nkContext);

			if (labSelector.isRunning()) {
				if (app != 0) {
					app->ImGui(nkContext);
				}
				labSelector.ImGui(nkContext, imguiXPosition);
			}
		}
		timerStop = glfwGetTime();
		timerDiff = timerStop - timerStart;
		accumulator.imguiLogic += (double)timerDiff * 1000.0;

		// Imgui Render
		timerStart = glfwGetTime();
		if (nkContext != 0) {
			if (!firstRenderSample) { // Imgui GPU Timer
				glGetQueryObjectiv(gGpuImguiStop, GL_QUERY_RESULT, &timerResultAvailable);
				while (!timerResultAvailable) {
					std::cout << "Waiting on imgui GPU timer!\n";
					glGetQueryObjectiv(gGpuImguiStop, GL_QUERY_RESULT, &timerResultAvailable);
				}
				glGetQueryObjectui64v(gGpuImguiStart, GL_QUERY_RESULT, &gpuStartTime);
				glGetQueryObjectui64v(gGpuImguiStop, GL_QUERY_RESULT, &gpuStopTime);
				accumulator.imguiGPU += (double)(gpuStopTime - gpuStartTime) / 1000000.0;
			}
			glQueryCounter(gGpuImguiStart, GL_TIMESTAMP);
			nk_glfw3_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
			glQueryCounter(gGpuImguiStop, GL_TIMESTAMP);
		}
		timerStop = glfwGetTime();
		timerDiff = timerStop - timerStart;
		accumulator.imguiRender += (double)timerDiff * 1000.0;

		// Wait for GPU
		timerStart = glfwGetTime();
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		timerStop = glfwGetTime();
		timerDiff = timerStop - timerStart;
		accumulator.swapBuffer += (double)timerDiff * 1000.0;

		frameStop = glfwGetTime();
		timerDiff = frameStop - frameStart;
		double frameTime = (double)timerDiff * 1000.0;
		accumulator.frameTime += frameTime;

		// Profiling house keeping
		firstRenderSample = false;
		if (++frameCounter >= timerFrequency) {
			frameCounter = 0;

			display.glfwEvents = accumulator.glfwEvents / timerFrequency;
			display.frameUpdate = accumulator.frameUpdate / timerFrequency;
			display.frameRender = accumulator.frameRender / timerFrequency;
			display.imguiLogic = accumulator.imguiLogic / timerFrequency;
			display.imguiRender = accumulator.imguiRender / timerFrequency;
			display.swapBuffer = accumulator.swapBuffer / timerFrequency;
			display.frameTime = accumulator.frameTime / timerFrequency;
			display.deltaTime = accumulator.deltaTime / timerFrequency;
			display.appGPU = accumulator.appGPU / timerFrequency;
			display.imguiGPU = accumulator.imguiGPU / timerFrequency;

			memset(&accumulator, 0, sizeof(FrameTimer));
			slowFrame = display.frameTime >= 33.33; // slowframe < 30fps
		}

		if (app != 0 && app->close == true)
			break;
    }

    // clean data
    if (app != 0) {
        std::cout << "Expected application to be null on exit\n";
        delete app;
    }

	nk_glfw3_shutdown();
    glfwTerminate();
    return 0;
}

static void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (app != 0) {

		if (action == GLFW_PRESS) {
			app->onKeyDown(key, scancode);
		}
		if (action == GLFW_RELEASE) {
			app->onKeyUp(key, scancode);
		}
	}
}

static void onMouseEvent(GLFWwindow* window, int button, int action, int mods) {
	if (app != 0) {

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)
				app->onRightMouseButtonDown();
			else if (action == GLFW_RELEASE) {
				app->onRightMouseButtonUp();
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS)
				app->onLeftMouseButtonDown();
			else if (action == GLFW_RELEASE) {
				app->onLeftMouseButtonUp();
			}
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (action == GLFW_PRESS)
				app->onMiddleMouseButtonDown();
			else if (action == GLFW_RELEASE) {
				app->onMiddleMouseButtonUp();
			}
		}
	}
}

static void onScroll(GLFWwindow* window, double xoffset, double yoffset) {
	if (app != 0) {
		app->onScroll(xoffset, yoffset);
	}
}