#include <GL\glew.h>
#include <GL\wglew.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>
#include <string>
#include <stdio.h>
#include "Mathem.h"

// Hardcoded settings
#define TR_WINDOW_NAME	"30 Rays per Second"
#define TR_WIDTH		800
#define TR_HEIGHT		600
#define TR_FULLSCREEN	false
#define TR_VSYNC		false

// Macroses
#define TR_CHECK_OPENGL() { GLenum error = glGetError(); if (error != GL_NO_ERROR) { char buffer[1024]; _snprintf(buffer, 1024, "OpenGL error (0x%X) on %i line: %s\n", error, __LINE__, glewGetErrorString(error)); MessageBoxA(gWindow ? glfwGetWin32Window(gWindow) : nullptr, buffer, "OpenGL Error!", MB_OK); } }
#define glCheck(func) { func; TR_CHECK_OPENGL(); }

//------------------------------------------------------------------------------------

GLuint gRenderResultTexID = 0;

void trCreateRenderResultTexture()
{
	glGenTextures(1, &gRenderResultTexID);
	glBindTexture(GL_TEXTURE_2D, gRenderResultTexID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TR_WIDTH, TR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void trDestroyRenderResultTexture()
{
	glDeleteTextures(1, &gRenderResultTexID);
}

//------------------------------------------------------------------------------------

GLuint gProgramID = 0;

char* trLoadFile(const char* fileName)
{
	FILE* f = fopen(fileName, "rb");
	if (!f)
		return nullptr;

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* buffer = new char[size + 1];
	fread(buffer, 1, size, f);
	buffer[size] = 0;
	fclose(f);

	return buffer;
}

GLuint trCreateShader(GLenum shaderType, const char* source)
{
	GLuint shader = glCreateShader(shaderType);

	const char* str = source;
	glShaderSource(shader, 1, (const GLchar**)&str, nullptr);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[1024];
		glGetShaderInfoLog(shader, 1024, nullptr, buffer);
		MessageBoxA(nullptr, buffer, "OpenGL Shader Error!", MB_OK);
		return 0;
	}

	return shader;
}

bool trCreateShaderProgram()
{
	gProgramID = glCreateProgram();
	//GLuint vs = trCreateShader(GL_VERTEX_SHADER, trLoadFile("rays.vs.glsl"));
	//GLuint fs = trCreateShader(GL_FRAGMENT_SHADER, trLoadFile("rays.fs.glsl"));
	GLuint cs = trCreateShader(GL_COMPUTE_SHADER, trLoadFile("../Data/rays.cs.glsl"));

	//glAttachShader(gProgramID, vs);
	//glAttachShader(gProgramID, fs);
	glAttachShader(gProgramID, cs);

	glLinkProgram(gProgramID);

	GLint status;
	glGetProgramiv(gProgramID, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[1024];
		glGetProgramInfoLog(gProgramID, 1024, nullptr, buffer);
		MessageBoxA(nullptr, buffer, "OpenGL Shader Error!", MB_OK);
		return false;
	}

	glDeleteShader(cs);
	//glDeleteShader(fs);
	//glDeleteShader(vs);

	return true;
}

void trDestroyShaderProgram()
{
	glDeleteProgram(gProgramID);
	gProgramID = 0;
}

//------------------------------------------------------------------------------------

struct trQuad
{
	vec3 a;
	float _align1;
	vec3 b;
	float _align2;
	vec3 c;
	float _align3;
	vec3 d;
	float _align4;
	vec3 center;
	float _align5;

	// Material
	vec3 diffuse;
	float _align6;

	// Required for intersection test
	vec3 N;
	float D;
	vec3 N1;
	float D1;
	vec3 N2;
	float D2;
	vec3 N3;
	float D3;
	vec3 N4;
	float D4;

	trQuad() : D(0), D1(0), D2(0), D3(0), D4(0) {}
	trQuad(const vec3& a, const vec3& b, const vec3& c, const vec3& d, const vec3& diffuse) : a(a), b(b), c(c), d(d), diffuse(diffuse)
	{
		center = (a + b + c + d) / 4.0f;

		N = normalize(cross(b - a, c - a));
		D = -dot(N, a);

		N1 = normalize(cross(N, b - a));
		D1 = -dot(N1, a);

		N2 = normalize(cross(N, c - b));
		D2 = -dot(N2, b);

		N3 = normalize(cross(N, d - c));
		D3 = -dot(N3, c);

		N4 = normalize(cross(N, a - d));
		D4 = -dot(N4, d);
	}
};

const int gNumQuads = 2;
GLuint gQuadBufID = 0;

void trCreateStorageBuffer()
{
	trQuad ar[gNumQuads];

	// NOTE: CCW
	const float val = 0.5f;
	ar[0] = trQuad(vec3(-val, val, 0), vec3(-val, -val, 0), vec3(val, -val, 0), vec3(val, val, 0), vec3(0.9f, 0.9f, 0.9f)); // wall
	ar[1] = trQuad(vec3(-val * 5, -0.5f, -val * 5), vec3(-val * 5, -0.5f, val * 5), vec3(val * 5, -0.5f, val * 5), vec3(val * 5, -0.5f, -val * 5), vec3(0.3f, 1.0f, 0.3f)); // land

	glGenBuffers(1, &gQuadBufID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gQuadBufID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gQuadBufID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ar), ar, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void trDestroyStorageBuffer()
{
	glDeleteBuffers(1, &gQuadBufID);
}

//------------------------------------------------------------------------------------

GLFWwindow* gWindow = nullptr;
double gDeltaTime = 0.0, gPrevTime = 0.0;
double gCursorPosX = 0.0, gCursorPosY = 0.0, gPrevCursorPosX = 0.0, gPrevCursorPosY = 0.0;
vec3 gCameraPos = vec3(0, 0, 3);
vec3 gCameraDir = vec3(0, 0, -1);
double gCameraRotX = 0.0, gCameraRotY = 0.0;
mat3 gCameraRot;

void OnResize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

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

	glfwSetWindowSizeCallback(gWindow, OnResize);
	glfwMakeContextCurrent(gWindow);
	ShowWindow(glfwGetWin32Window(gWindow), SW_SHOWMAXIMIZED);

	printf("OpenGL %s; %s; %s\n", (char*)glGetString(GL_VERSION), (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER));

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		return -1;
	wglSwapIntervalEXT(TR_VSYNC ? 1 : 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);

	trCreateRenderResultTexture();
	if (!trCreateShaderProgram()) return -1;
	trCreateStorageBuffer();
	glfwGetCursorPos(gWindow, &gCursorPosX, &gCursorPosY);
	TR_CHECK_OPENGL();


	while (!glfwWindowShouldClose(gWindow))
	{
		// Update Delta Time
		{
			double curTime = glfwGetTime();
			gDeltaTime = curTime - gPrevTime;
			gPrevTime = curTime;
		}

		// Update Cursor Pos
		gPrevCursorPosX = gCursorPosX;
		gPrevCursorPosY = gCursorPosY;
		glfwGetCursorPos(gWindow, &gCursorPosX, &gCursorPosY);


		// Handle Input
		{
			if (glfwGetMouseButton(gWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				double rotateSpeed = 0.2;
				gCameraRotX += (gCursorPosX - gPrevCursorPosX) * rotateSpeed;
				gCameraRotY += (gCursorPosY - gPrevCursorPosY) * rotateSpeed;
				gCameraRot = RotationMatrix(-gCameraRotY, vec3(1, 0, 0)) * RotationMatrix(gCameraRotX, vec3(0, 1, 0));
				gCameraDir = gCameraRot * vec3(0, 0, -1); // update camera dir
			}

			float moveSpeed = 2.0f; // meters in second
			if (glfwGetKey(gWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) moveSpeed *= 2.0f;
			if (glfwGetKey(gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) moveSpeed *= 0.5f;
			if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS) gCameraPos += gCameraDir * moveSpeed * gDeltaTime;
			if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS) gCameraPos -= gCameraDir * moveSpeed * gDeltaTime;
			if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS) { vec3 left = cross(gCameraDir, vec3(0, 1, 0)); gCameraPos += left * moveSpeed * gDeltaTime; }
			if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS) { vec3 left = cross(gCameraDir, vec3(0, 1, 0)); gCameraPos -= left * moveSpeed * gDeltaTime; }
			if (glfwGetKey(gWindow, GLFW_KEY_E) == GLFW_PRESS) gCameraPos += vec3(0, 1, 0) * moveSpeed * gDeltaTime;
			if (glfwGetKey(gWindow, GLFW_KEY_Q) == GLFW_PRESS) gCameraPos -= vec3(0, 1, 0) * moveSpeed * gDeltaTime;
		}


		// Rendering
		
		glClear(GL_COLOR_BUFFER_BIT);

		// Compute Lighting
		glUseProgram(gProgramID);
		glBindImageTexture(0, gRenderResultTexID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glUniform1i(glGetUniformLocation(gProgramID, "gNumQuads"), gNumQuads);
		glUniform3fv(glGetUniformLocation(gProgramID, "gCameraPos"), 1, &gCameraPos.x);
		glUniformMatrix3fv(glGetUniformLocation(gProgramID, "gCameraRot"), 1, GL_FALSE, gCameraRot.m);
		const int WorkGroup = 8; // TODO: make configurable in shader
		glDispatchCompute(TR_WIDTH / WorkGroup, TR_HEIGHT / WorkGroup, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT); // TODO: GL_TEXTURE_FETCH_BARRIER_BIT is right for 'imageStore'?
		glUseProgram(0);

		// Fullscreen Quad
		glBindTexture(GL_TEXTURE_2D, gRenderResultTexID);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(-1, 1);
		glTexCoord2f(0, 0);
		glVertex2f(-1, -1);
		glTexCoord2f(1, 0);
		glVertex2f(1, -1);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glEnd();

		// Present
		glFlush();
		glfwSwapBuffers(gWindow);
		glfwPollEvents();
		TR_CHECK_OPENGL();
	}


	trDestroyStorageBuffer();
	trDestroyShaderProgram();
	trDestroyRenderResultTexture();
	glfwTerminate();
	return 0;
}