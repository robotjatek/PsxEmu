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
	std::vector<GLuint> vertices;
public:
	RendererGL();
	virtual ~RendererGL();
	void SwapBuffers();
	GLuint LoadShaders();
	void PushPolygons(PolygonData polygon, int numberOfPolygons);
};

