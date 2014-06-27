#include <GL\glew.h>
#include <GL\wglew.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>
#include <string>
#include <stdio.h>

// Hardcoded settings
#define TR_WINDOW_NAME	"30 Rays per Second"
#define TR_WIDTH		1024
#define TR_HEIGHT		768
#define TR_FULLSCREEN	false
#define TR_VSYNC		false

// Macroses
#define TR_CHECK_OPENGL() { GLenum error = glGetError(); if (error != GL_NO_ERROR) { char buffer[1024]; _snprintf(buffer, 1024, "OpenGL error (0x%X) on %i line: %s\n", error, __LINE__, glewGetErrorString(error)); MessageBoxA(gWindow ? glfwGetWin32Window(gWindow) : nullptr, buffer, "OpenGL Error!", MB_OK); } }
#define glCheck(func) { func; TR_CHECK_OPENGL(); }


GLFWwindow* gWindow = nullptr;

int main()
{
	if (!glfwInit())
		return -1;

	gWindow = glfwCreateWindow(TR_WIDTH, TR_HEIGHT, TR_WINDOW_NAME, TR_FULLSCREEN ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	if (!gWindow)
	{
		glfwTerminate();
		return -1;
	}

	ShowWindow(glfwGetWin32Window(gWindow), SW_SHOWMAXIMIZED);
	glfwMakeContextCurrent(gWindow);

	printf("%s\n", (char*)glGetString(GL_VERSION));

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		return -1;
	wglSwapIntervalEXT(TR_VSYNC ? 1 : 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	TR_CHECK_OPENGL();


	while (!glfwWindowShouldClose(gWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT);


		// Render here!


		glFinish();
		glfwSwapBuffers(gWindow);
		glfwPollEvents();
		TR_CHECK_OPENGL();
	}


	glfwTerminate();
	return 0;
}