#pragma once
#include <GL\glew.h>
#include <GL\wglew.h>
#include <GLFW\glfw3.h>
#include "GPUData.h"
#include <vector>

class RendererGL
{
private:
	GLFWwindow * window;
	GLuint vertexArrayId;
	GLuint vboId;
	GLuint colorBufferId;
	GLuint shaderId;
	std::vector<GLuint> vertices;
	std::vector<GLuint> colors;	
public:
	RendererGL();
	virtual ~RendererGL();
	void SwapBuffers();
	GLuint LoadShaders();
	void PushPolygons(const PolygonData& polygon, int numberOfPolygons);
	void UpdateDrawingOffset(const GLint offsetX, const GLint offsetY);
};

