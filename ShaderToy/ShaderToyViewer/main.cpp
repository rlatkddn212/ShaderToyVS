#pragma warning(disable:4996)
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")

#include <GL/glew.h>	
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
using namespace std;

GLFWwindow* window;

void initGLFW()
{
	// GLFW 초기화
	if (!glfwInit())
	{
		cerr << "ERROR : init GLFW\n";
		return exit(-1);
	}

	// GLFW 설정
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// window 생성
	window = glfwCreateWindow(1024, 768, "ShaderToy Viewer", NULL, NULL);

	if (window == NULL)
	{
		cerr << "Error : open GLFW\n";
		glfwTerminate();
		return exit(-1);
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}

void initGLEW()
{
	// GLEW 초기화
	glfwMakeContextCurrent(window);
	glewExperimental = true;

	if (glewInit() != GLEW_OK)
	{
		cerr << "ERROR : init GLEW\n";
		return exit(-1);
	}
}

GLuint LoadShaders(const char* vertexFilePath, const char* fragmentFilePath)
{
	static const string shaderInputStr =
		"#version 420 core\n					\
		uniform vec3 iResolution;\n				\
		uniform float iTime; \n					\
		uniform float iTimeDelta; \n			\
		uniform float iFrame;\n					\
		uniform float iChannelTime[4]; \n		\
		uniform vec4 iMouse; \n					\
		uniform vec4 iDate; \n					\
		uniform float iSampleRate; \n			\
		uniform vec3 iChannelResolution[4];\n	\
		uniform sampler%s iChannel0; \n			\
		uniform sampler%s iChannel1; \n			\
		uniform sampler%s iChannel2; \n			\
		uniform sampler%s iChannel3; \n";

	static const string mainFuncStr =
		"layout(location = 0) out vec4 fragColor;\n				\
		void main() \n											\
		{\n														\
		   mainImage(fragColor.xyzw, gl_FragCoord.xy);\n		\
		}";

	// 쉐이더들 생성
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// 버텍스 쉐이더 코드를 파일에서 읽기
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertexFilePath, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else
	{
		//printf("파일 %s 를 읽을 수 없음.\n", vertexFilePath);
		getchar();
		return 0;
	}

	// 프래그먼트 쉐이더 코드를 파일에서 읽기
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragmentFilePath, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = shaderInputStr + sstr.str() + mainFuncStr;
		
		char* tmp = new char[FragmentShaderCode.size() + 1024];
		memset(tmp, 0, FragmentShaderCode.size() + 1024);
		sprintf(tmp, FragmentShaderCode.c_str(), "2D", "2D", "2D", "2D");
		FragmentShaderCode = tmp;
		delete[] tmp;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// 버텍스 쉐이더를 컴파일
	printf("Compiling shader : %s\n", vertexFilePath);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// 버텍스 쉐이더를 검사
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// 프래그먼트 쉐이더를 컴파일
	printf("Compiling shader : %s\n", fragmentFilePath);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// 프래그먼트 쉐이더를 검사
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// 프로그램에 링크
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// 프로그램 검사
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
struct UniformData
{
	GLint iResolution;
	GLint iTime;
	GLint iTimeDelta;
	GLint iFrame;
	GLint iChannelTime[4];
	GLint iMouse;
	GLint iDate;
	GLint iSampleRate;
	GLint iChannelResolution[4];
	GLint iChannel0;
	GLint iChannel1;
	GLint iChannel2;
	GLint iChannel3;
};

void setUniformData(GLuint programID, UniformData& data)
{
	data.iResolution = glGetUniformLocation(programID, "iResolution");
	data.iTime = glGetUniformLocation(programID, "iTime");
	data.iTimeDelta = glGetUniformLocation(programID, "iTimeDelta");
	data.iFrame = glGetUniformLocation(programID, "iFrame");
	data.iChannelTime[0] = glGetUniformLocation(programID, "iChannelTime[0]");
	data.iChannelTime[1] = glGetUniformLocation(programID, "iChannelTime[1]");
	data.iChannelTime[2] = glGetUniformLocation(programID, "iChannelTime[2]");
	data.iChannelTime[3] = glGetUniformLocation(programID, "iChannelTime[3]");
	data.iMouse = glGetUniformLocation(programID, "iMouse");
	data.iDate = glGetUniformLocation(programID, "iDate");
	data.iSampleRate = glGetUniformLocation(programID, "iSampleRate");
	data.iChannelResolution[0] = glGetUniformLocation(programID, "iChannelResolution[0]");
	data.iChannelResolution[1] = glGetUniformLocation(programID, "iChannelResolution[1]");
	data.iChannelResolution[2] = glGetUniformLocation(programID, "iChannelResolution[2]");
	data.iChannelResolution[3] = glGetUniformLocation(programID, "iChannelResolution[3]");
}

int main()
{
	initGLFW();
	initGLEW();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	static const GLfloat g_vertex_buffer_data[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	static const GLfloat uv_buffer_data[] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f,  1.0f,
		1.0f,  1.0f,
	};

	// 버텍스 버퍼에 핸들
	GLuint vertexbuffer;
	GLuint uvBuffer;

	// 버퍼를 생성
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// 버텍스들을 OpenGL로
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

	GLuint programID = LoadShaders("vertex.glsl", "Image.glsl");

	glUseProgram(programID);
	UniformData data;
	setUniformData(programID, data);

	GLfloat iResolution[] = { 1024, 768, 0 };
	clock_t startTime = clock();
	clock_t curTime;
	float diffTime = 0;

	do
	{
		time_t time = 0;
		tm *date = localtime(&time);
		curTime = clock();
		// drawing
		static const GLfloat blue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, blue);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

		glUniform3fv(data.iResolution, 1, &iResolution[0]);
		glUniform1f(data.iTime, (curTime - startTime) / 1000.0f);
		//glUniform1f(data.iTimeDelta, playtime_in_second);
		//glUniform1f(data.iFrame, playtime_in_second);
		// glUniform1f(data.iChannelTime, );
		glUniform4f(data.iMouse, 0.0f, 0.0f, 0.0f, 0.0f);
		int sec = 3600 * date->tm_hour + 60 * date->tm_min + date->tm_sec;
		glUniform4f(data.iDate, date->tm_year, date->tm_mon, date->tm_mday, sec);
		//glUniform1f(data.iSampleRate, playtime_in_second);
		
		glVertexAttribPointer(
			0,                  // 0번째 속성(attribute).
			3,                  // 크기(size)
			GL_FLOAT,           // 타입(type)
			GL_FALSE,           // 정규화(normalized)?
			0,                  // 다음 요소 까지 간격(stride)
			(void*)0            // 배열 버퍼의 오프셋(offset; 옮기는 값)
		);


		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glVertexAttribPointer(
			1,					// 1번째 속성(attribute).
			2,                  // 크기(size)
			GL_FLOAT,           // 타입(type)
			GL_FALSE,           // normalized
			0,                  // 다음 요소 까지 간격
			(void*)0            // 배열 버퍼의 오프셋
		);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// swap buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glfwTerminate();
	return 0;
}