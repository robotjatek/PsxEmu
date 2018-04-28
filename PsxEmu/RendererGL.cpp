#include "RendererGL.h"
#include <cstdlib>
#include <string>
#include <Windows.h>
#include "Memory.h"
#include <plog\Log.h>

_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

void RendererGL::WindowCloseCallback(GLFWwindow* window)
{
	glfwDestroyWindow(window);
}

RendererGL::RendererGL(const Memory* memory)
{
	this->memory = memory;
	glfwInit();
	glfwWindowHint(GLFW_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_VERSION_MINOR, 4);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	this->window = glfwCreateWindow(1024, 512, "PSX", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glfwSetWindowCloseCallback(window, this->WindowCloseCallback);
	
	glewExperimental = true;
	glewInit();

	glClearColor(0, 0, 0, 0);

	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(this->vertexArrayId);

	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glGenBuffers(1, &colorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferId);

	shaderId = LoadShaders();
	glUseProgram(shaderId);
}

RendererGL::~RendererGL()
{
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &colorBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
	glfwTerminate();
}

void RendererGL::SwapBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	if (vertices.size())
	{
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()/2);
	}

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		memory->StopSystem();
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	glfwSwapBuffers(window);
	glfwPollEvents();
	vertices.clear();
	colors.clear();
}

GLuint RendererGL::LoadShaders()
{
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertexShaderCode = R"(
										#version 330
										layout(location = 0) in vec2 position;
										layout(location = 1) in vec3 color;
										
										out vec3 fragment_color;
										uniform int offsetX;
										uniform int offsetY;

										void main()
										{
											gl_Position.x = position.x/512 - 1.0 + offsetX;
											gl_Position.y = 1.0-(position.y/256) + offsetY;
											gl_Position.z = 0.0;
											gl_Position.w = 1.0;
											fragment_color = color;
										}
)";


	std::string fragmentShaderCode = R"(
										#version 330
										in vec3 fragment_color;
										out vec3 color_out;

										void main()
										{
											color_out = fragment_color/255.0;
										}
)";

	const char *vp = vertexShaderCode.c_str();
	const char *fp = fragmentShaderCode.c_str();

	glShaderSource(vertexShaderId, 1, &vp, nullptr);
	glCompileShader(vertexShaderId);

	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(vertexShaderId, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		LOG_FATAL << "Error while compiling vertex shader";// << VertexShaderErrorMessage;
	}

	glShaderSource(fragmentShaderId, 1, &fp, nullptr);
	glCompileShader(fragmentShaderId);

	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(fragmentShaderId, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		LOG_FATAL << "Error while compiling fragment shader";// << VertexShaderErrorMessage;
	}

	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);

	glDetachShader(programId, vertexShaderId);
	glDetachShader(programId, fragmentShaderId);
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	return programId;
}

void RendererGL::PushPolygons(const PolygonData& polygon, int numberOfPolygons)
{
	if (numberOfPolygons == 3)
	{
		for (int i = 0; i < numberOfPolygons; i++)
		{
			vertices.push_back(polygon.vertices[i].x);
			vertices.push_back(polygon.vertices[i].y);
			colors.push_back(polygon.vertices[i].r);
			colors.push_back(polygon.vertices[i].g);
			colors.push_back(polygon.vertices[i].b);
		}
	}
	else if (numberOfPolygons == 4)
	{
		vertices.push_back(polygon.vertices[0].x);
		vertices.push_back(polygon.vertices[0].y);
		colors.push_back(polygon.vertices[0].r);
		colors.push_back(polygon.vertices[0].g);
		colors.push_back(polygon.vertices[0].b);

		vertices.push_back(polygon.vertices[1].x);
		vertices.push_back(polygon.vertices[1].y);
		colors.push_back(polygon.vertices[1].r);
		colors.push_back(polygon.vertices[1].g);
		colors.push_back(polygon.vertices[1].b);

		vertices.push_back(polygon.vertices[2].x);
		vertices.push_back(polygon.vertices[2].y);
		colors.push_back(polygon.vertices[2].r);
		colors.push_back(polygon.vertices[2].g);
		colors.push_back(polygon.vertices[2].b);

		vertices.push_back(polygon.vertices[2].x);
		vertices.push_back(polygon.vertices[2].y);
		colors.push_back(polygon.vertices[2].r);
		colors.push_back(polygon.vertices[2].g);
		colors.push_back(polygon.vertices[2].b);

		vertices.push_back(polygon.vertices[3].x);
		vertices.push_back(polygon.vertices[3].y);
		colors.push_back(polygon.vertices[3].r);
		colors.push_back(polygon.vertices[3].g);
		colors.push_back(polygon.vertices[3].b);

		vertices.push_back(polygon.vertices[1].x);
		vertices.push_back(polygon.vertices[1].y);
		colors.push_back(polygon.vertices[1].r);
		colors.push_back(polygon.vertices[1].g);
		colors.push_back(polygon.vertices[1].b);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 2, GL_UNSIGNED_INT, GL_FALSE, 0, nullptr);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLuint), &vertices[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colorBufferId);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 0, nullptr);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLuint), &colors[0], GL_DYNAMIC_DRAW);
}

void RendererGL::UpdateDrawingOffset(const GLint offsetX, const GLint offsetY)
{
	GLuint location;
	location = glGetUniformLocation(shaderId, "offsetX");
	glUniform1i(location, offsetX);
	location = glGetUniformLocation(shaderId, "offsetY");
	glUniform1i(location, offsetY);
}