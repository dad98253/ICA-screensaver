
//greensquare.cpp is an open-source example
//screensaver by Rachel Grey, lemming@alum.mit.edu.
//Paste into an IDE to compile if desired.
//I haven't chosen to include the resource file,
//so you'd need to provide a description string
//and so forth.

#define SOIL_CHECK_FOR_GL_ERRORS 0

#include <windows.h>
#include  <scrnsave.h>
#include  <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screensaver1\resource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "screensaver1\stb_image.h"

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


int Width, Height; //globals for size of screen

GLuint	texture[1];			// Storage For One Texture


//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////

int LoadGLTextures()                                    // Load Bitmaps And Convert To Textures
{
	/* load an image file directly as a new OpenGL texture */
/*	texture[0] = SOIL_load_OGL_texture
	(
		"Data/img_test.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);

	if (texture[0] == 0)
		return false;


	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

*/

	//unsigned int texture;
	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load("img_test.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		printf("Failed to load texture\n");
	}
	stbi_image_free(data);

	return true;                                        // Return Success
}

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
		LoadGLTextures();
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


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);

	//no need to initialize any objects
	//but this is where I'd do it

	glColor3f(0.1, 1.0, 0.3); //green

}

static GLfloat spin = 0;   //a global to keep track of the square's spinning


void OnTimer(HDC hDC) //increment and display
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	spin = spin + 1;
	/*
	glPushMatrix();
	glRotatef(spin, 0.0, 0.0, 1.0);

	glPushMatrix();

	glTranslatef(150, 0, 0);

	if (bTumble)
		glRotatef(spin * -3.0, 0.0, 0.0, 1.0);
	else
		glRotatef(spin * -1.0, 0.0, 0.0, 1.0);
*/
	//draw the square (rotated to be a diamond)

	float xvals[] = { -30.0, 0.0, 30.0, 0.0 };
	float yvals[] = { 0.0, -30.0, 0.0, 30.0 };
	float txvals[] = { 0.0, 1.0, 1.0, 0.0 };
	float tyvals[] = { 0.0, 0.0, 1.0, 1.0 };

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(txvals[i], tyvals[i]);
		glVertex2f(xvals[i], yvals[i]);
	}
	glEnd();

	glPopMatrix();

	glFlush();
	SwapBuffers(hDC);
	glPopMatrix();
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
		"Software\\GreenSquare", //lpctstr
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
		"Software\\GreenSquare", //lpctstr
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
