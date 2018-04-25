#include "RendererGL.h"
#include <cstdlib>
#include <string>
#include <Windows.h>

_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

RendererGL::RendererGL()
{

	glfwInit();
	glfwWindowHint(GLFW_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_VERSION_MINOR, 4);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	this->window = glfwCreateWindow(1024, 512, "PSX", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	
	glewExperimental = true;
	glewInit();

	glClearColor(0, 0, 0, 0);

	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(this->vertexArrayId);

	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	GLuint shader = LoadShaders();
	glUseProgram(shader);
}

RendererGL::~RendererGL()
{
	glfwTerminate();
}

void RendererGL::SwapBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 2, GL_UNSIGNED_INT, GL_FALSE, 0, nullptr);
	if (vertices.size())
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLuint), &vertices[0], GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()/2);
	}
	glDisableVertexAttribArray(0);

	glfwSwapBuffers(window);
	glfwPollEvents();
	vertices.clear();
}

GLuint RendererGL::LoadShaders()
{
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertexShaderCode = R"(
										#version 330
										layout(location = 0) in vec2 position;

										void main()
										{
											gl_Position.x = position.x/512 - 1.0;
											gl_Position.y = 1.0-(position.y/256);
											gl_Position.z = 0.0;
											gl_Position.w = 1.0;
										}
)";


	std::string fragmentShaderCode = R"(
										#version 330
										out vec3 color;

										void main()
										{
											color = vec3(1,0,0);
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
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	glShaderSource(fragmentShaderId, 1, &fp, nullptr);
	glCompileShader(fragmentShaderId);

	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(fragmentShaderId, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
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

void RendererGL::PushPolygons(PolygonData polygon, int numberOfPolygons)
{
	for (int i = 0; i < numberOfPolygons; i++)
	{
		vertices.push_back(polygon.vertices[i].x);
		vertices.push_back(polygon.vertices[i].y);
	}
}
