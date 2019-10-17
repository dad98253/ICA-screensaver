
// ICA branded screensaver based on:
// greensquare.cpp is an open-source example
// screensaver by Rachel Grey, lemming@alum.mit.edu.
// All changes made to these two examples are:
// copyright(c) John Kuras 2019
// The terms and conditions for all of these changes are WTFPL



#define _USE_MATH_DEFINES
#include <math.h>

#include <windows.h>
#include  <scrnsave.h>
#include <gl/glew.h>
//#include <GL/glcorearb.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include "screensaver1\resource.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "screensaver1\shader.hpp"
#include "screensaver1\texture.hpp"

//get rid of these warnings:
//truncation from const double to float
//conversion from double to float
#pragma warning(disable: 4305 4244) 


//Define a Windows timer

#define TIMER 1
#define IDC_TUMBLE 2019




//These forward declarations are just for readability,
//so the big three functions can come first 

void InitGL(HWND hWnd, HDC& hDC, HGLRC& hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void GetConfig();
void WriteConfig(HWND hDlg);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();
void OnTimer(HDC hDC);
int LoadGLTextures();


int Width, Height; //globals for size of screen
// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
static const GLfloat g_vertex_buffer_data[] = {
-1.0f,-1.0f,-1.0f,
-1.0f,-1.0f, 1.0f,
-1.0f, 1.0f, 1.0f,
-1.0f,-1.0f,-1.0f,
-1.0f, 1.0f, 1.0f,
-1.0f, 1.0f,-1.0f,

 1.0f, 1.0f, 1.0f,
 1.0f,-1.0f,-1.0f,
 1.0f, 1.0f,-1.0f,
 1.0f,-1.0f,-1.0f,
 1.0f, 1.0f, 1.0f,
 1.0f,-1.0f, 1.0f,

 1.0f,-1.0f, 1.0f,
-1.0f,-1.0f,-1.0f,
 1.0f,-1.0f,-1.0f,
 1.0f,-1.0f, 1.0f,
-1.0f,-1.0f, 1.0f,
-1.0f,-1.0f,-1.0f,

 1.0f, 1.0f, 1.0f,
 1.0f, 1.0f,-1.0f,
-1.0f, 1.0f,-1.0f,
 1.0f, 1.0f, 1.0f,
-1.0f, 1.0f,-1.0f,
-1.0f, 1.0f, 1.0f,

 1.0f, 1.0f,-1.0f,
-1.0f,-1.0f,-1.0f,
-1.0f, 1.0f,-1.0f,
 1.0f, 1.0f,-1.0f,
 1.0f,-1.0f,-1.0f,
-1.0f,-1.0f,-1.0f,

-1.0f, 1.0f, 1.0f,
-1.0f,-1.0f, 1.0f,
 1.0f,-1.0f, 1.0f,
 1.0f, 1.0f, 1.0f,
-1.0f, 1.0f, 1.0f,
 1.0f,-1.0f, 1.0f
};


// Two UV coordinatesfor each vertex.
static const GLfloat g_uv_buffer_data[] = {
	0.0f, 0.0f ,
	1.0f, 0.0f ,
	1.0f, 1.0f ,
	0.0f, 0.0f ,
	1.0f, 1.0f ,
	0.0f, 1.0f ,

	1.0f, 1.0f ,
	0.0f, 0.0f ,
	1.0f, 0.0f ,
	0.0f, 0.0f ,
	1.0f, 1.0f ,
	0.0f, 1.0f ,

	1.0f, 1.0f ,
	0.0f, 0.0f ,
	1.0f, 0.0f ,
	1.0f, 1.0f ,
	0.0f, 1.0f ,
	0.0f, 0.0f ,

	1.0f, 1.0f ,
	0.0f, 1.0f ,
	0.0f, 0.0f ,
	1.0f, 1.0f ,
	0.0f, 0.0f ,
	1.0f, 0.0f ,

	1.0f, 1.0f ,
	0.0f, 0.0f ,
	1.0f, 0.0f ,
	1.0f, 1.0f ,
	0.0f, 1.0f ,
	0.0f, 0.0f ,

	0.0f, 1.0f ,
	0.0f, 0.0f ,
	1.0f, 0.0f ,
	1.0f, 1.0f ,
	0.0f, 1.0f ,
	1.0f, 0.0f
};

GLuint vertexbuffer;
GLuint uvbuffer;
GLuint TextureID;
GLuint Texture;
GLuint programID;
GLuint MatrixID;
GLuint VertexArrayID;
glm::mat4 Projection;
glm::mat4 View;
glm::mat4 ViewTemp;
glm::mat4 Model;
glm::mat4 ModelTemp;
glm::mat4 MVP;

const char* vs_source =
"#version 330 core\n"
"// Input vertex data, different for all executions of this shader.\n"
"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
"layout(location = 1) in vec2 vertexUV;\n"
"// Output data ; will be interpolated for each fragment.\n"
"out vec2 UV;\n"
"// Values that stay constant for the whole mesh.\n"
"uniform mat4 MVP;\n"
"void main() {\n"
"// Output position of the vertex, in clip space : MVP * position\n"
"gl_Position = MVP * vec4(vertexPosition_modelspace, 1);\n"
"// UV of the vertex. No special space for this one.\n"
"UV = vertexUV;\n"
"}\n";


const char* fs_source =
"#version 330 core/n"
"// Interpolated values from the vertex shaders/n"
"in vec2 UV;/n"
"// Ouput data/n"
"out vec3 color;/n"
"// Values that stay constant for the whole mesh./n"
"uniform sampler2D myTextureSampler;/n"
"void main() {/n"
"// Output color = color of the texture at the specified UV/n"
"color = texture(myTextureSampler, UV).rgb;/n"
"}/n";


inline float Deg2Rad(float degrees) {
	return degrees * (float)(M_PI / 180.0f);
}


//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////


// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	static HDC hDC;
	static HGLRC hRC;
	static RECT rect;

	switch (message) {

	case WM_CREATE:
		// get window dimensions
		GetClientRect(hWnd, &rect);
		Width = rect.right;
		Height = rect.bottom;

		//get configuration from registry
		GetConfig();

		// setup OpenGL, then animation
		InitGL(hWnd, hDC, hRC);
		SetupAnimation(Width, Height);

		//set timer to tick every 10 ms
		SetTimer(hWnd, TIMER, 10, NULL);
		return 0;

	case WM_DESTROY:
		KillTimer(hWnd, TIMER);
		CleanupAnimation();
		CloseGL(hWnd, hDC, hRC);
		return 0;

	case WM_TIMER:
		OnTimer(hDC);       //animate!      
		return 0;

	}

	return DefScreenSaverProc(
		hWnd, message, wParam, lParam);

}

bool bTumble = true;


BOOL WINAPI
ScreenSaverConfigureDialog(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{

	//InitCommonControls();  
	//would need this for slider bars or other common controls

	HWND aCheck;

	switch (message)
	{

	case WM_INITDIALOG:
		LoadString(hMainInstance, IDS_DESCRIPTION, szAppName, 40);

		GetConfig();

		aCheck = GetDlgItem(hDlg, IDC_TUMBLE);
		SendMessage(aCheck, BM_SETCHECK,
			bTumble ? BST_CHECKED : BST_UNCHECKED, 0);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDC_TUMBLE:
			bTumble = (IsDlgButtonChecked(hDlg, IDC_TUMBLE) == BST_CHECKED);
			return TRUE;

			//cases for other controls would go here

		case IDOK:
			WriteConfig(hDlg);      //get info from controls
			EndDialog(hDlg, LOWORD(wParam) == IDOK);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam) == IDOK);
			return TRUE;
		}

	}     //end command switch

	return FALSE;
}



// needed for SCRNSAVE.LIB
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
	return TRUE;
}


/////////////////////////////////////////////////
////   INFRASTRUCTURE ENDS, SPECIFICS BEGIN   ///
////                                          ///
////    In a more complex scr, I'd put all    ///
////     the following into other files.      ///
/////////////////////////////////////////////////


// Initialize OpenGL
static void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof pfd);
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	//pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;

	hDC = GetDC(hWnd);

	int i = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, i, &pfd);

	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);

	LoadGLTextures();


}

// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);

	ReleaseDC(hWnd, hDC);
}


void SetupAnimation(int Width, int Height)
{
	//window resizing stuff
	glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-300, 300, -240, 240, 25, 75);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	//camera xyz, the xyz to look at, and the up vector (+y is up)

//background
	glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black


	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glShadeModel(GL_SMOOTH);

	//no need to initialize any objects
	//but this is where I'd do it

	//glColor3f(0.1, 1.0, 0.3); //green

}

static GLfloat spin = 0;   //a global to keep track of the square's spinning


void OnTimer(HDC hDC) //increment and display
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Initialize GLEW
	/*glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return;
	}*/
	spin = mod(spin + 1.0f,360.0f);
/*
	glPushMatrix();
	glRotatef(spin, 0.0, 0.0, 1.0);

	glPushMatrix();

	glTranslatef(150, 0, 0);

	if (bTumble)
		glRotatef(spin * -3.0, 0.0, 0.0, 1.0);
	else
		glRotatef(spin * -1.0, 0.0, 0.0, 1.0);

	//draw the square (rotated to be a diamond)

	float xvals[] = { -30.0, 0.0, 30.0, 0.0 };
	float yvals[] = { 0.0, -30.0, 0.0, 30.0 };

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++)
		glVertex2f(xvals[i], yvals[i]);
	glEnd();

	glPopMatrix();

	glFlush();
	SwapBuffers(hDC);
	glPopMatrix();
	*/

	// set the orientation of the cube
	if (bTumble) {
		ModelTemp = glm::rotate(Model, glm::radians(spin), glm::vec3(0.0f, 0.0f, 1.0f)); // where the vector is axis of rotation (e.g. about the z-axis)
		ModelTemp = glm::rotate(ModelTemp, glm::radians(spin), glm::vec3(0.0f, 1.0f, 0.0f)); // where the vector is axis of rotation (e.g. about the y-axis)
		ModelTemp = glm::rotate(ModelTemp, glm::radians(spin), glm::vec3(1.0f, 0.0f, 0.0f)); // where the vector is axis of rotation (e.g. about the x-axis)
	}
	else {
		ModelTemp = Model;
	}
	// set the view
	//ViewTemp = glm::translate(ViewTemp, glm::vec3(0.0f, 1.3f, 0.0f)); // where the vector is the translation (e.g. 5 units in each axis)
	ViewTemp = glm::translate(View, glm::vec3(cosf(Deg2Rad(spin)) * 10.0f, sinf(Deg2Rad(spin)) * 10.0f, 0.0f)); // where the vector is the translation (e.g. 5 units in each axis)
		
	// Our ModelViewProjection : multiplication of our 3 matrices
	MVP = Projection * ViewTemp * ModelTemp; // Remember, matrix multiplication is the other way around

	// Use our shader
	glUseProgram(programID);

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// Swap buffers
	//glfwSwapBuffers(window);
	//glfwPollEvents();
	SwapBuffers(hDC);

}

void CleanupAnimation()
{
	//didn't create any objects, so no need to clean them up
}



/////////   REGISTRY ACCESS FUNCTIONS     ///////////

void GetConfig()
{

	HKEY key;
	//DWORD lpdw;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\ICAscreensaver", //lpctstr
		0,                      //reserved
		KEY_QUERY_VALUE,
		&key) == ERROR_SUCCESS)
	{
		DWORD dsize = sizeof(bTumble);
		DWORD dwtype = 0;

		RegQueryValueEx(key, "Tumble", NULL, &dwtype,
			(BYTE*)& bTumble, &dsize);


		//Finished with key
		RegCloseKey(key);
	}
	else //key isn't there yet--set defaults
	{
		bTumble = true;
	}

}

void WriteConfig(HWND hDlg)
{

	HKEY key;
	DWORD lpdw;

	if (RegCreateKeyEx(HKEY_CURRENT_USER,
		"Software\\ICAscreensaver", //lpctstr
		0,                      //reserved
		(LPSTR)"",                     //ptr to null-term string specifying the object type of this key
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&key,
		&lpdw) == ERROR_SUCCESS)

	{
		RegSetValueEx(key, "Tumble", 0, REG_DWORD,
			(BYTE*)& bTumble, sizeof(bTumble));

		//Finished with keys
		RegCloseKey(key);
	}

}

int LoadGLTextures()                                    // Load Bitmaps And Convert To Textures
{
	/* load an image file directly as a new OpenGL texture */

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		return -1;
	}
	   	 
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
		
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
//	programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
	programID = LoadShaders(vs_source, fs_source);

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	View = glm::translate(mat4(1.0f), glm::vec3(0.0f, 0.0f, -50.0f)); // where the vector is the translation (e.g. 5 units in each axis)
		/*View = glm::lookAt(
		glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);*/
	// Model matrix : an identity matrix (model will be at the origin) scaled by a constant
	Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(5.0f, 5.0f, 5.0f)); // where the vector is the scale factor for eachaxis (e.g. .25 units in all axes)
	// Our ModelViewProjection : multiplication of our 3 matrices
	MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// Load the texture using any two methods
//	  Texture = loadBMP_custom("uvtemplate.bmp");
	Texture = loadBMP_custom("ICA-logo-on-black.bmp");
	//	GLuint Texture = loadDDS("uvtemplate.DDS");

	// Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
		
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
	
	return(0);
}
