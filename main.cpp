#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "glew.h"
#include <gl\GL.h>
#include <gl\GLU.h>
#include <assert.h>

#include <string>
#include <fstream>
#include <iostream>
#include <time.h>

HINSTANCE hInstance;
HDC hDc;
HGLRC hGlrc;
PIXELFORMATDESCRIPTOR pfd;
RECT wSize{ 100, 100, 800, 600 };
static wchar_t wClassName[] = L"testClassName";
static wchar_t wTitleName[] = L"testTitleName";

GLuint loadShaders(const char* vsPath, const char* fsPath)
{
	//shader creation
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertexShaderCode;
	std::string fragmentShaderCode;

	if (vsPath != " ")
	{
		std::ifstream vertexShaderStream(vsPath, std::ios::in);
		if (vertexShaderStream.is_open())
		{
			std::string line = "";
			while (std::getline(vertexShaderStream, line))
				vertexShaderCode += "\n" + line;
			vertexShaderStream.close();
		}
	}

	if (fsPath != " ")
	{
		std::ifstream fragmentShaderStream(fsPath, std::ios::in);
		if (fragmentShaderStream.is_open())
		{
			std::string line = "";
			while (std::getline(fragmentShaderStream, line))
				fragmentShaderCode += "\n" + line;
			fragmentShaderStream.close();
		}
	}

	//shader compiles
	char const *vertexShaderPointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderId, 1, &vertexShaderPointer, NULL);
	glCompileShader(vertexShaderId);

	char const *fragmentShaderPointer = fragmentShaderCode.c_str();
	glShaderSource(fragmentShaderId, 1, &fragmentShaderPointer, NULL);
	glCompileShader(fragmentShaderId);

	//check
	GLint compileResult;

	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &compileResult);
	assert(compileResult == GL_TRUE);

	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &compileResult);
	assert(compileResult == GL_TRUE);


	//program id linking
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glBindFragDataLocation(programId, 0, "outColor");
	glLinkProgram(programId);

	//chec
	glGetProgramiv(programId, GL_LINK_STATUS, &compileResult);
	assert(compileResult == GL_TRUE);

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	return programId;
}

//vertex array
GLfloat vertices[] =
{
	1.0f, 1.0f, 1.0f, 0, 0,
	1.0f, -1.0f, 0, 1.0f, 0,
	-1.0f, 1.0f, 0, 0, 1.0f,
	-1.0f, -1.0f, 1.0f, 0, 0
};

GLuint indices[] =
{
	0, 1, 2,
	1, 2, 3
};

//DRAW
void renderFrame(GLuint m_vboId, GLuint programId)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
}


//VBO creation
GLuint CreateVBO(GLfloat *vertices, int size)
{
	GLuint vboId;
	glGenBuffersARB(1, &vboId);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, vertices, GL_STATIC_DRAW_ARB);
	/*
	int bufferSize = 0;
	glGetBufferParameterivARB(GL_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE_ARB, &bufferSize);
	if (size != bufferSize)
	{
		glDeleteBuffersARB(1, &vboId);
		vboId = 0;
		MessageBox(NULL, L"VBO data size mismatch", wTitleName, NULL);
	}
	*/
	return vboId;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int pixelFormat;

	switch (uMsg)
	{
	case WM_CREATE:
		pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL, PFD_TYPE_RGBA,
			24,
			24
		};

		hDc = GetDC(hWnd);
		pixelFormat = ChoosePixelFormat(hDc, &pfd);
		SetPixelFormat(hDc, pixelFormat, &pfd);
		hGlrc = wglCreateContext(hDc);
		wglMakeCurrent(hDc, hGlrc);

		break;

	case WM_DESTROY:
		hGlrc = nullptr;
		wglDeleteContext(hGlrc);
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			SendMessage(hWnd, WM_CLOSE, NULL, NULL);
		}
		return 0;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

int main()
{
	//window create
	WNDCLASSEX window;
	MSG uMsg;
	HWND hWnd;

	bool quit = false;

	hInstance = GetModuleHandle(nullptr);

	window.cbSize = sizeof(WNDCLASSEX);
	window.style = CS_OWNDC;
	window.lpfnWndProc = WndProc;
	window.hInstance = hInstance;
	window.lpszClassName = wClassName;
	window.lpszMenuName = NULL;
	window.cbClsExtra = 0;
	window.cbWndExtra = 0;
	window.hIcon = NULL;
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.hbrBackground = NULL;
	window.hIconSm = LoadIcon(window.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	RegisterClassEx(&window);
	AdjustWindowRect(&wSize, WS_CAPTION, false);
	hWnd = CreateWindowEx(NULL, wClassName, wTitleName, WS_OVERLAPPEDWINDOW, wSize.left, wSize.top, wSize.right, wSize.bottom, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, L"Window creation failed!", wTitleName, NULL);
		return 1;
	}

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	//GLEW init
	if (glewInit() != GLEW_OK)
	{
		MessageBox(NULL, L"glewInit() failure", NULL, NULL);
		return -1;
	}

	//create VBO
	int size = sizeof(vertices);
	int m_vboId = CreateVBO(&vertices[0], size);

	GLuint programId = loadShaders("Data/VertexShader.txt", "Data/FragmentShader.txt");	
	glUseProgram(programId);

	GLuint posAttrib = glGetAttribLocation(programId, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	
	/*	not in use
	GLuint colAttrib = glGetAttribLocation(programId, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(2 * sizeof(float)));
	*/

	//main message loop
	while (!quit)
	{
		while (PeekMessage(&uMsg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (uMsg.message == WM_QUIT)
			{
				quit = true;
				break;
			}
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);

		}
		renderFrame(0, programId);
		SwapBuffers(hDc);
	}
}