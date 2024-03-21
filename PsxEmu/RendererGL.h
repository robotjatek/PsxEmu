#pragma once
#include <GL\glew.h>
#include <GL\wglew.h>
#include <GLFW\glfw3.h>
#include "GPUData.h"
#include <vector>

class Memory;

class RendererGL
{
private:
	const Memory * memory;
	GLFWwindow * window;
	GLuint vertexArrayId;
	GLuint vboId;
	GLuint colorBufferId;
	GLuint shaderId;
	std::vector<GLuint> vertices;
	std::vector<GLuint> colors;	
	static void WindowCloseCallback(GLFWwindow* window);
public:
	RendererGL(const Memory* memory);
	virtual ~RendererGL();
	void SwapBuffers();
	GLuint LoadShaders();
	void PushPolygons(const PolygonData& polygon, int numberOfPolygons, bool textured);
	void UpdateDrawingOffset(const GLint offsetX, const GLint offsetY);
};

