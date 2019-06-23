#include "globalStuff.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>		
#include <algorithm>
#include "cShaderManager.h"
#include "cVAOManager.h"
#include "HelpfulFunctions.h"

cVAOManager* g_pVAOManager = NULL;
cMeshObject* g_pBall = NULL;
GLFWwindow* g_pWindow = NULL;
cBasicTextureManager* g_pTextureManager = NULL;
cLightManager* g_pLightManager = NULL;
cMeshObject* pCurrent = NULL;
void AddOnlyNonTransparent(std::vector<cMeshObject*> &vecNonTransparent, std::vector<cMeshObject*> &vecAllTheObjects);

void AddOnlyTransparent(std::vector<cMeshObject*> &vecTransparent,std::vector<cMeshObject*> &vecAllTheObjects);

void SortTransparentBackToFront(std::vector<cMeshObject*> &vecTransparent,glm::vec3 cameraEye);

// Set default camera here
glm::vec3 g_cameraEye = glm::vec3(0.0f, 3.69999862f, +12.3000011f);
glm::vec3 g_cameraTarget = glm::vec3(0.0f, 2.29999995f, -2.99999952f);

// C++ side variable for colour (so we can change it externally)
glm::vec3 g_triangleColour = glm::vec3(0.5f, 0.1f, 0.75f);

//Set default light index and show debug sphere
unsigned int g_currentSelectedLightIndex = 0;
bool g_showDebugLightSpheres = true;

// Called by the draw call... 
void DrawMeshObject(cMeshObject* pCurMesh, glm::mat4 matModel, GLint shaderProgID);

// Draws a number of spheres showing the brightness of the lights at certain distances
// (helpful for managing the attenuation values)
void DrawLightAttenuationSpheres(unsigned int lightIndex, GLint shaderProgID);

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

bool isShiftDownAlone(int mods)
{
	if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT)
	{
		return true;
	}
	return false;
}

bool isShiftDown(int mods)
{
	if (mods & GLFW_MOD_SHIFT)
	{
		return true;
	}
	return false;
}

bool isAltDownAlone(int mods)
{
	if ((mods & GLFW_MOD_ALT) == GLFW_MOD_ALT)
	{
		return true;
	}
	return false;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	// WASD
	// A & D =  x axis (left and right)
	// W & S =  z axis (towards and away)
	// Q & E =  y axis (up and down)

	//Set camera speed here
	const float CAMERASPEED = 0.5f;

	if (isShiftDownAlone(mods))
	{
		
		if (key == GLFW_KEY_A) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.x += CAMERASPEED; }
		if (key == GLFW_KEY_D) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.x -= CAMERASPEED; }

		if (key == GLFW_KEY_W) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.z += CAMERASPEED; }
		if (key == GLFW_KEY_S) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.z -= CAMERASPEED; }

		if (key == GLFW_KEY_Q) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.y += CAMERASPEED; }
		if (key == GLFW_KEY_E) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.y -= CAMERASPEED; }

		// Change atteuation
		if (key == GLFW_KEY_1) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].atten.y *= 0.99f; } // decrease 1%
		if (key == GLFW_KEY_2) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].atten.y *= 1.01f; } // Increase 1%

		// Change atteuation
		if (key == GLFW_KEY_3) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].atten.z *= 0.99f; } // decrease 0.1%
		if (key == GLFW_KEY_4) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].atten.z *= 1.01f; } // Increase 0.1%

		if (key == GLFW_KEY_MINUS) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].TurnOffLight(); }
		if (key == GLFW_KEY_EQUAL) { ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].TurnOnLight(); }

		std::cout << "curent light pos x:" << ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.x << std::endl;
		std::cout << "curent light pos y:" << ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.y << std::endl;
		std::cout << "curent light pos z:" << ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].position.z << std::endl;

		if (key == GLFW_KEY_5)
		{
			float angleInner = ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].GetInnerAngle();
			float angleOuter = ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].GetOuterAngle();

			angleInner -= 1.0f;
			angleOuter -= 1.0f;

			::g_pLightManager->vecLights[::g_currentSelectedLightIndex].SetSpotConeAngles(angleInner, angleOuter);
			std::cout << "current angleInner" << angleInner << "current angleOuter" << angleOuter << std::endl;
		}
		if (key == GLFW_KEY_6)
		{
			float angleInner = ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].GetInnerAngle();
			float angleOuter = ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].GetOuterAngle();

			angleInner += 1.0f;
			angleOuter += 1.0f;

			::g_pLightManager->vecLights[::g_currentSelectedLightIndex].SetSpotConeAngles(angleInner, angleOuter);
			std::cout << "current angleInner" << angleInner << "current angleOuter" << angleOuter << std::endl;
		}

		if (key == GLFW_KEY_L) { ::g_showDebugLightSpheres = true; }
		if (key == GLFW_KEY_K) { ::g_showDebugLightSpheres = false; }

		// Note: we are using the "action" as well, so we don't get multiple key presses each time
		if (key == GLFW_KEY_0 && action == GLFW_PRESS) { ::g_currentSelectedLightIndex++; }
		if (key == GLFW_KEY_9 && action == GLFW_PRESS) { ::g_currentSelectedLightIndex--; }

		// Check if the selected light has gone outside of the range
		if ( ::g_currentSelectedLightIndex< 0) 
		{ 
			::g_currentSelectedLightIndex = (unsigned int)::g_pLightManager->vecLights.size() - 1; 
		}
		if (::g_currentSelectedLightIndex >= (unsigned int)::g_pLightManager->vecLights.size()) 
		{ 
			::g_currentSelectedLightIndex = 0; 
		}

		//Show info on window title
		std::stringstream ssTitle;
		ssTitle << "Light #" << ::g_currentSelectedLightIndex;

		if (::g_pLightManager->vecLights[::g_currentSelectedLightIndex].isOn())
		{
			ssTitle << " (ON)";
		}
		else
		{
			ssTitle << " (OFF)";
		}

		ssTitle  
			<< " Linear Atten: " 
			<< ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].atten.y
			<< " quad: " 
			<< ::g_pLightManager->vecLights[::g_currentSelectedLightIndex].atten.z << std::endl;

		glfwSetWindowTitle( ::g_pWindow, ssTitle.str().c_str() );

		return;
	}

	// Move camera target
	if (isAltDownAlone(mods))
	{
		if (key == GLFW_KEY_A) { ::g_cameraTarget.x += CAMERASPEED; }
		if (key == GLFW_KEY_D) { ::g_cameraTarget.x -= CAMERASPEED; }

		if (key == GLFW_KEY_W) { ::g_cameraTarget.z += CAMERASPEED; }
		if (key == GLFW_KEY_S) { ::g_cameraTarget.z -= CAMERASPEED; }

		if (key == GLFW_KEY_Q) { ::g_cameraTarget.y += CAMERASPEED; }
		if (key == GLFW_KEY_E) { ::g_cameraTarget.y -= CAMERASPEED; }

		std::cout << "camera target x:" << ::g_cameraTarget.x << std::endl;
		std::cout << "camera target y:" << ::g_cameraTarget.y << std::endl;
		std::cout << "camera target z:" << ::g_cameraTarget.z << std::endl;

		return;
	}

	// Move camera eye
	if (key == GLFW_KEY_A) { ::g_cameraEye.x += CAMERASPEED; }
	if (key == GLFW_KEY_D) { ::g_cameraEye.x -= CAMERASPEED; }

	if (key == GLFW_KEY_W) { ::g_cameraEye.z += CAMERASPEED; }
	if (key == GLFW_KEY_S) { ::g_cameraEye.z -= CAMERASPEED; }

	if (key == GLFW_KEY_Q) { ::g_cameraEye.y += CAMERASPEED; }
	if (key == GLFW_KEY_E) { ::g_cameraEye.y -= CAMERASPEED; }

	std::cout << "camera eye x:" << ::g_cameraEye.x << std::endl;
	std::cout << "camera eye y:" << ::g_cameraEye.y << std::endl;
	std::cout << "camera eye z:" << ::g_cameraEye.z << std::endl;

	// Move select object 
	if (key == GLFW_KEY_J) { ::pCurrent->pos.x += CAMERASPEED; }
	if (key == GLFW_KEY_L) { ::pCurrent->pos.x -= CAMERASPEED; }

	if (key == GLFW_KEY_I) { ::pCurrent->pos.z += CAMERASPEED; }
	if (key == GLFW_KEY_K) { ::pCurrent->pos.z -= CAMERASPEED; }

	if (key == GLFW_KEY_U) { ::pCurrent->pos.y += CAMERASPEED; }
	if (key == GLFW_KEY_O) { ::pCurrent->pos.y -= CAMERASPEED; }

	//scale
	if (key == GLFW_KEY_Z) { ::pCurrent->scale += CAMERASPEED; }
	if (key == GLFW_KEY_X) { ::pCurrent->scale -= CAMERASPEED; }

	std::cout << "Current Object pos x:" << ::pCurrent->pos.x << std::endl;
	std::cout << "Current Object pos y:" << ::pCurrent->pos.y << std::endl;
	std::cout << "Current Object pos z:" << ::pCurrent->pos.z << std::endl;

	// Move to preset camera positions
	if (key == GLFW_KEY_N)
	{
		::g_cameraEye = glm::vec3(0.0f, 3.69999862f, +12.3000011f);
		::g_cameraTarget = glm::vec3(0.0f, 2.29999995f, -2.99999952f);
	}
	if (key == GLFW_KEY_M)
	{
		::g_cameraEye = glm::vec3(3.69999862f, 3.09999919f, -12.4000216f);
		::g_cameraTarget = glm::vec3(0.0f, 2.29999995f, -2.99999952f);
	}

	const float COLOURCHANGERATE = 0.01f;
	if (key == GLFW_KEY_1) { ::g_triangleColour.x += COLOURCHANGERATE; }
	if (key == GLFW_KEY_2) { ::g_triangleColour.x -= COLOURCHANGERATE; }

	if (key == GLFW_KEY_3) { ::g_triangleColour.y += COLOURCHANGERATE; }
	if (key == GLFW_KEY_4) { ::g_triangleColour.y -= COLOURCHANGERATE; }

	if (key == GLFW_KEY_5) { ::g_triangleColour.z += COLOURCHANGERATE; }
	if (key == GLFW_KEY_6) { ::g_triangleColour.z -= COLOURCHANGERATE; }

}

int main(void)
{

	GLuint vertex_buffer;
	GLint vpos_location, vcol_location;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	::g_pWindow = glfwCreateWindow(1024, 768, "Final Exam", NULL, NULL);
	if ( !::g_pWindow )
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	
	glfwMakeContextCurrent(::g_pWindow);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	// Make a shader manager
	cShaderManager* pMyShaderManager = new cShaderManager();  // HEAP

	cShaderManager::cShader vertShader;
	vertShader.fileName = "vertexShader.glsl";

	cShaderManager::cShader fragShader;
	fragShader.fileName = "fragmentShader.glsl";

	if (pMyShaderManager->createProgramFromFile(
		"simpleShader", vertShader, fragShader))
	{
		std::cout << "Shaders are OK" << std::endl;
	}
	else
	{
		std::cout << "ERROR: " << std::endl;
		std::cout << pMyShaderManager->getLastError() << std::endl;
	}

	// Get the shader program ID from the one we compiled.
	GLuint shaderProgID = pMyShaderManager->getIDFromFriendlyName("simpleShader");

	::g_pVAOManager = new cVAOManager();
	//Load all the model
	LoadTheModelsIntoScene(shaderProgID);

	// Moving object
	pCurrent = pFindByFriendlyName("current");

	glfwSetKeyCallback(::g_pWindow, key_callback);

	//Load light
	::g_pLightManager = new cLightManager();
	::g_pLightManager->LoadUniformLocations(shaderProgID);

	// Set initial light information
	::g_pLightManager->vecLights[0].position = glm::vec4(1.90000284f, 11.6000109f, 1.10000002f, 1.0f);
	::g_pLightManager->vecLights[0].atten = glm::vec4( 0.0f, 0.007552f, 0.001f, 1.0f);
	::g_pLightManager->vecLights[0].TurnOnLight();
	::g_pLightManager->vecLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	// Light colour
	::g_pLightManager->vecLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	::g_pLightManager->vecLights[0].SetLightType(sLight::POINT_LIGHT);

	//Spot light example
	::g_pLightManager->vecLights[1].TurnOffLight();	
	// Point the spot light along the z axis...
	::g_pLightManager->vecLights[1].direction = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	::g_pLightManager->vecLights[1].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	::g_pLightManager->vecLights[1].SetLightType(sLight::SPOT_LIGHT);
	::g_pLightManager->vecLights[1].SetSpotConeAngles(
		-42.0f,		// inner angle
		-42.0f);		// outer angle

	/* Set light same position with object

	// Lights for bullet blaster things
	::g_pLightManager->vecLights[4].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);//Red
	cMeshObject* pBullet = pFindByFriendlyName("bullet1");
	//pBullet->pos = glm::vec4(-2.10000014f, 2.39999986f, 12.9000158f, 1.0f);
	::g_pLightManager->vecLights[4].position = glm::vec4(pBullet->pos, 1.0f);
	*/


	GLint matView_loc = glGetUniformLocation(shaderProgID, "matView");
	GLint matProj_loc = glGetUniformLocation(shaderProgID, "matProj");
	GLint matModel_loc = glGetUniformLocation(shaderProgID, "matModel");
	GLint matModelInvTran_loc = glGetUniformLocation(shaderProgID, "matModelInvTrans");

	GLint theRed_loc = glGetUniformLocation(shaderProgID, "theRed");
	GLint theGreen_loc = glGetUniformLocation(shaderProgID, "theGreen");
	GLint theBlue_loc = glGetUniformLocation(shaderProgID, "theBlue");

	while (!glfwWindowShouldClose(::g_pWindow))
	{
		float ratio;
		int width, height;

		//glm::mat4 matModel;				// now moved into DrawObject()
		glm::mat4 matProj;
		glm::mat4 matView;
		//glm::mat4 matModelInvTrans;		// now moved into DrawObject()

		// Immediate keyboard (mouse or joystick)
		if (glfwGetKey(::g_pWindow, GLFW_KEY_F) == GLFW_PRESS)
		{
			::g_cameraEye.x -= 0.02f;
		}
		if (glfwGetKey(::g_pWindow, GLFW_KEY_G) == GLFW_PRESS)
		{
			::g_cameraEye.x += 0.02f;
		}
		/*
		// Example Animate the falcon
		cMeshObject* pFalcon2 = pFindByFriendlyName("Falcon");
		pFalcon2->pos.x += 0.001f;
		pFalcon2->orientation.x += 0.001f;
		pFalcon2->orientation.y += 0.0007f;
		*/

		glfwGetFramebufferSize(::g_pWindow, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		// The background clear colour (white for S2)
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);

		/*
		// Change the texture over time
		cMeshObject* pFalcon = pFindByFriendlyName("Falcon");
		float text0Blend = pFalcon->textureBlendRatio[0];		
		text0Blend += 0.01f;								
		if (text0Blend > 1.0f) { text0Blend = 0.0f; }
		float text1Blend = 1.0f - text0Blend;			   
		pFalcon->textureBlendRatio[0] = text0Blend;		
		pFalcon->textureBlendRatio[1] = text1Blend;		
		*/

		// *****************************************
		// Set the things that stay constant for the drawing the scene
		// We DON'T change the camera, the projection matrix, or the lighting
		// Select the shader we want to use
		glUseProgram(shaderProgID);
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		// was "p"
		matProj = glm::perspective(0.6f,
			ratio,
			0.1f,
			1000.0f);

		matView = glm::mat4(1.0f);			// was "v"

		//glm::vec3 cameraEye = glm::vec3( 0.0, 0.0, +12.0f ); 
		//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

		matView = glm::lookAt(
			g_cameraEye,
			g_cameraTarget,
			upVector);

		// ********* Passing lighting info ***********
		::g_pLightManager->CopyLightValuesToShader();
		GLint eye_loc = glGetUniformLocation(shaderProgID, "eyeLocation");
		glUniform4f(eye_loc, ::g_cameraEye.x, ::g_cameraEye.y, ::g_cameraEye.z, 1.0f);

		glUniformMatrix4fv(matView_loc, 1, GL_FALSE, glm::value_ptr(matView));
		glUniformMatrix4fv(matProj_loc, 1, GL_FALSE, glm::value_ptr(matProj));

		// *******************************************
		// For transparency:
		// - Draw all non-transparent things first
		std::vector<cMeshObject*> vecNonTransparent;
		std::vector<cMeshObject*> vecTransparent;

		// Fill the vector with non transparent object to draw
		AddOnlyNonTransparent(vecNonTransparent, ::g_vecObjectsToDraw);

		AddOnlyTransparent(vecTransparent, ::g_vecObjectsToDraw);

		SortTransparentBackToFront(vecTransparent, g_cameraEye);
		// Which could be a single pass of the bubble sort

		// - Sort transparent things from FAR to NEAR RELATIVE TO THE CAMERA
		// - Then draw them (in that order)

		// Draw all NON transparent objects
		// Sort transparent objects from FAR to NEAR (from the camera)
		// Draw transparent objects last, in that order

		for (std::vector< cMeshObject* >::iterator itCurMesh = vecNonTransparent.begin();
			itCurMesh != vecNonTransparent.end(); itCurMesh++)
		{
			cMeshObject* pCurrentMesh = *itCurMesh;

			glm::mat4 matModel = glm::mat4(1.0f);

			DrawMeshObject(pCurrentMesh, matModel, shaderProgID);
		}
		for (std::vector< cMeshObject* >::iterator itCurMesh = vecTransparent.begin();
			itCurMesh != vecTransparent.end(); itCurMesh++)
		{
			cMeshObject* pCurrentMesh = *itCurMesh;

			glm::mat4 matModel = glm::mat4(1.0f);

			DrawMeshObject(pCurrentMesh, matModel, shaderProgID);
		}

		//for (std::vector< cMeshObject* >::iterator itCurMesh = ::g_vecObjectsToDraw.begin();
		//	itCurMesh != ::g_vecObjectsToDraw.end(); itCurMesh++)
		//{
		//	cMeshObject* pCurrentMesh = *itCurMesh;

		//	glm::mat4 matModel = glm::mat4(1.0f);

		//	DrawMeshObject(pCurrentMesh, matModel, shaderProgID);

		//	// end of the draw loop
		//}

		// ****************************************
		// Now draw a bunch of spheres where the light attenuation values are
		if (::g_showDebugLightSpheres)
		{
			DrawLightAttenuationSpheres( ::g_currentSelectedLightIndex, shaderProgID);
		}

		glfwSwapBuffers(::g_pWindow);
		glfwPollEvents();
	}
	glfwDestroyWindow(::g_pWindow);
	glfwTerminate();

	// Get rid of stuff
	delete pMyShaderManager;

	exit(EXIT_SUCCESS);
}


void DrawLightAttenuationSpheres(unsigned int lightIndex, GLint shaderProgID)
{
	// Turn on the "draw without lighting" thing in the shader
	//uniform vec4 debugColour;
	//uniform bool bUseDebugColourNoLight;		// float 0.0f 1.0f

	GLint debugColour_UniformLocation = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bUseDebugColourNoLight_UniLoc = glGetUniformLocation(shaderProgID, "bUseDebugColourNoLight");

	// Odd, but this sets a boolean in the shader
	glUniform1f(bUseDebugColourNoLight_UniLoc, (float)GL_TRUE);

	// Increase the line width 
	glLineWidth(1.0f);

	cMeshObject* pMeshBall = pFindByFriendlyName("meshball");

	// Draw the ball where the light is
	glm::mat4 matModel = glm::mat4(1.0f);
	pMeshBall->isVisible = true;
	pMeshBall->pos = ::g_pLightManager->vecLights[lightIndex].position;
	pMeshBall->scale = 0.25f;
//	pMeshBall->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4f(debugColour_UniformLocation,
		1.0f, 1.0f, 1.0f, 1.0f);
	DrawMeshObject(pMeshBall, matModel, shaderProgID);
	pMeshBall->isVisible = false;


	float sizeOf95PercentSphere = ::g_pLightManager->vecLights[0].lightHelper.calcApproxDistFromAtten(
		0.95,		// Target light level
		0.01f,		// Accuracy
		cLightHelper::DEFAULTINFINITEDISTANCE,
		::g_pLightManager->vecLights[lightIndex].atten.x,	// Const
		::g_pLightManager->vecLights[lightIndex].atten.y,	// Linear
		::g_pLightManager->vecLights[lightIndex].atten.z,	// Quadratic 
		cLightHelper::DEFAULTMAXITERATIONS);

	matModel = glm::mat4(1.0f);
	pMeshBall->isVisible = true;
	pMeshBall->pos = ::g_pLightManager->vecLights[lightIndex].position;
	pMeshBall->scale = sizeOf95PercentSphere;
//	pMeshBall->diffuseColour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(debugColour_UniformLocation,
		1.0f, 0.0f, 0.0f, 1.0f);
	DrawMeshObject(pMeshBall, matModel, shaderProgID);
	pMeshBall->isVisible = false;


	float sizeOf50PercentSphere = ::g_pLightManager->vecLights[lightIndex].lightHelper.calcApproxDistFromAtten(
		0.50,		// Target light level
		0.01f,		// Accuracy
		cLightHelper::DEFAULTINFINITEDISTANCE,
		::g_pLightManager->vecLights[lightIndex].atten.x,	// Const
		::g_pLightManager->vecLights[lightIndex].atten.y,	// Linear
		::g_pLightManager->vecLights[lightIndex].atten.z,	// Quadratic 
		cLightHelper::DEFAULTMAXITERATIONS);
	matModel = glm::mat4(1.0f);
	pMeshBall->isVisible = true;
	pMeshBall->pos = ::g_pLightManager->vecLights[lightIndex].position;
	pMeshBall->scale = sizeOf50PercentSphere;
//	pMeshBall->diffuseColour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glUniform4f(debugColour_UniformLocation,
		0.0f, 1.0f, 0.0f, 1.0f);
	DrawMeshObject(pMeshBall, matModel, shaderProgID);
	pMeshBall->isVisible = false;

	float sizeOf25PercentSphere = ::g_pLightManager->vecLights[0].lightHelper.calcApproxDistFromAtten(
		0.25,		// Target light level
		0.01f,		// Accuracy
		cLightHelper::DEFAULTINFINITEDISTANCE,
		::g_pLightManager->vecLights[lightIndex].atten.x,	// Const
		::g_pLightManager->vecLights[lightIndex].atten.y,	// Linear
		::g_pLightManager->vecLights[lightIndex].atten.z,	// Quadratic 
		cLightHelper::DEFAULTMAXITERATIONS);
	matModel = glm::mat4(1.0f);
	pMeshBall->isVisible = true;
	pMeshBall->pos = ::g_pLightManager->vecLights[lightIndex].position;
	pMeshBall->scale = sizeOf25PercentSphere;
//	pMeshBall->diffuseColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	glUniform4f(debugColour_UniformLocation,
		0.0f, 1.0f, 1.0f, 1.0f);
	DrawMeshObject(pMeshBall, matModel, shaderProgID);
	pMeshBall->isVisible = false;


	float sizeOf5PercentSphere = ::g_pLightManager->vecLights[lightIndex].lightHelper.calcApproxDistFromAtten(
		0.05,		// Target light level
		0.01f,		// Accuracy
		cLightHelper::DEFAULTINFINITEDISTANCE,
		::g_pLightManager->vecLights[lightIndex].atten.x,	// Const
		::g_pLightManager->vecLights[lightIndex].atten.y,	// Linear
		::g_pLightManager->vecLights[lightIndex].atten.z,	// Quadratic 
		cLightHelper::DEFAULTMAXITERATIONS);
	matModel = glm::mat4(1.0f);
	pMeshBall->isVisible = true;
	pMeshBall->pos = ::g_pLightManager->vecLights[lightIndex].position;
	pMeshBall->scale = sizeOf5PercentSphere;
//	pMeshBall->diffuseColour = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	glUniform4f(debugColour_UniformLocation,
		0.0f, 0.0f, 1.0f, 1.0f);
	DrawMeshObject(pMeshBall, matModel, shaderProgID);
	pMeshBall->isVisible = false;

	// Set the line width where it was
	glLineWidth(1.0f);

	// ****************************************

		// Odd, but this sets a boolean in the shader
	glUniform1f(bUseDebugColourNoLight_UniLoc, (float)GL_FALSE);


	return;
}


glm::vec2 g_schoothTexture = glm::vec2(0.0f, 0.0f);

void DrawMeshObject(cMeshObject* pCurMesh, 
					glm::mat4 matModel, 
					GLint shaderProgID)
{
	// Is is visible? 
	if (!pCurMesh->isVisible)
	{
		return;
	}

	// ***********************************************************************
	// Set up the "texture bindings"
	GLint bUseTexture_UniLoc = glGetUniformLocation(shaderProgID, "bUseTexture");
	glUniform1f(bUseTexture_UniLoc, (float)GL_TRUE);


	GLint texSamp2D_00_UniLoc = glGetUniformLocation(shaderProgID, "texSamp2D_00");
	GLint texSamp2D_01_UniLoc = glGetUniformLocation(shaderProgID, "texSamp2D_01");
	GLint texSamp2D_02_UniLoc = glGetUniformLocation(shaderProgID, "texSamp2D_02");
	GLint texSamp2D_03_UniLoc = glGetUniformLocation(shaderProgID, "texSamp2D_03");
	GLint textureBlendRatio_UniLoc = glGetUniformLocation(shaderProgID, "textureBlendRatio");

	GLuint texture00Number
		= ::g_pTextureManager->getTextureIDFromName(pCurMesh->textureNames[0]);

	GLuint texture01Number
		= ::g_pTextureManager->getTextureIDFromName(pCurMesh->textureNames[1]);

	GLuint texture02Number
		= ::g_pTextureManager->getTextureIDFromName(pCurMesh->textureNames[2]);

	GLuint texture03Number
		= ::g_pTextureManager->getTextureIDFromName(pCurMesh->textureNames[3]);

	// Texture binding...
	glActiveTexture(GL_TEXTURE0);	// GL_TEXTURE0 = 33984
	glBindTexture(GL_TEXTURE_2D, texture00Number);
	glUniform1i(texSamp2D_00_UniLoc, 0 /*texture unit number*/);

	glActiveTexture(GL_TEXTURE1);	// GL_TEXTURE0 = 33984
	glBindTexture(GL_TEXTURE_2D, texture01Number);
	glUniform1i(texSamp2D_01_UniLoc, 1 /*texture unit number*/);

	glActiveTexture(GL_TEXTURE2);	// GL_TEXTURE0 = 33984
	glBindTexture(GL_TEXTURE_2D, texture02Number);
	glUniform1i(texSamp2D_02_UniLoc, 2 /*texture unit number*/);

	glActiveTexture(GL_TEXTURE3);	// GL_TEXTURE0 = 33984
	glBindTexture(GL_TEXTURE_2D, texture03Number);
	glUniform1i(texSamp2D_03_UniLoc, 3 /*texture unit number*/);

	// Set up the blending
	glUniform4f(textureBlendRatio_UniLoc,
		pCurMesh->textureBlendRatio[0],	
		pCurMesh->textureBlendRatio[1], 
		pCurMesh->textureBlendRatio[2], 
		pCurMesh->textureBlendRatio[3]); 
	// ***********************************************************************
	// Alpha transparency texture
	// uniform sampler2D texAphaBlendSampler;

	// uniform bool bUseTextureAlpha;	
	GLint texAphaBlendSampler_UniLoc = glGetUniformLocation(shaderProgID, "texAphaBlendSampler");
	GLint bUseTextureAlpha_UniLoc = glGetUniformLocation(shaderProgID, "bUseTextureAlpha");

	if (pCurMesh->bUseTextureAlpha)
	{
		glUniform1f(bUseTextureAlpha_UniLoc, (float)GL_TRUE);

		GLuint alphaTextureID
			= ::g_pTextureManager->getTextureIDFromName(pCurMesh->alphaBlendTexture);

		glActiveTexture(GL_TEXTURE20);	// GL_TEXTURE0 = 33984
		glBindTexture(GL_TEXTURE_2D, alphaTextureID);
		glUniform1i(texAphaBlendSampler_UniLoc, 20 /*texture unit number*/);

	}
	else
	{
		glUniform1f(bUseTextureAlpha_UniLoc, (float)GL_FALSE);
	}

	// ***********************************************************************
	// HACK:
	GLint bUseMapTexture_UniLoc = glGetUniformLocation(shaderProgID, "bUseMapTexture");
	glUniform1f(bUseMapTexture_UniLoc, (float)GL_FALSE);

	if (pCurMesh->friendlyName == "heightMappedTerrain")
	{
		//uniform sampler2D heightMapTexture;
		//uniform bool bUseMapTexture;
		glUniform1f(bUseMapTexture_UniLoc, (float)GL_TRUE);

		GLuint heightMapTextureID
			= ::g_pTextureManager->getTextureIDFromName("MountainTextureHeight.bmp");
		// Pick a texture unit that we aren't using
		glActiveTexture(GL_TEXTURE25);	// GL_TEXTURE0 = 33984
		glBindTexture(GL_TEXTURE_2D, heightMapTextureID);

		GLint heightMapTexture_UniLoc = glGetUniformLocation(shaderProgID, "heightMapTexture");
		glUniform1i(heightMapTexture_UniLoc, 25 /*Same as above*/);

		// The move the texture over time
		GLint scootchTexture_UniLoc = glGetUniformLocation(shaderProgID, "scootchTexture");
		glUniform2f( scootchTexture_UniLoc, 
			         ::g_schoothTexture.x, 
			         ::g_schoothTexture.y);

		::g_schoothTexture.x += 0.001f;

	}

	// ***********************************************************************
	// A "discard" imposter....

	GLint bIsImposterUseDiscard_UniLoc = glGetUniformLocation(shaderProgID, "bIsImposterUseDiscard");
	if (pCurMesh->bIsImposterUseDiscard)
	{
		glUniform1f(bIsImposterUseDiscard_UniLoc, (float)GL_TRUE);
	}
	else
	{
		glUniform1f(bIsImposterUseDiscard_UniLoc, (float)GL_FALSE);
	}
	// ***********************************************************************

	// Enables "alpha blending"
	glEnable(GL_BLEND);
	// Straight forward "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Here's where the drawing for each object starts
	//         mat4x4_identity(m);
	matModel = glm::mat4(1.0f);

	// ********************************
	glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
		glm::vec3(pCurMesh->pos.x, pCurMesh->pos.y, pCurMesh->pos.z));
	matModel = matModel * matTranslate;
	// ********************************
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
		pCurMesh->orientation.z,
		glm::vec3(0.0f, 0.0, 1.0f));
	matModel = matModel * rotateZ;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
		pCurMesh->orientation.y,
		glm::vec3(0.0f, 1.0, 0.0f));
	matModel = matModel * rotateY;

	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
		pCurMesh->orientation.x,
		glm::vec3(1.0f, 0.0, 0.0f));
	matModel = matModel * rotateX;
	// ********************************

	// Note scale is passed the same on all axes
	glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurMesh->scale, pCurMesh->scale, pCurMesh->scale));
	matModel = matModel * matScale;
	// ********************************

	GLint matModel_loc = glGetUniformLocation(shaderProgID, "matModel");
	GLint matModelInvTran_loc = glGetUniformLocation(shaderProgID, "matModelInvTrans");

	// Now pass the three matrix variables one at a time
	glUniformMatrix4fv(matModel_loc, 1, GL_FALSE, glm::value_ptr(matModel));
	//glUniformMatrix4fv(matView_loc, 1, GL_FALSE, glm::value_ptr(matView));
	//glUniformMatrix4fv(matProj_loc, 1, GL_FALSE, glm::value_ptr(matProj));

	// Calculate the inverse transpose 
	glm::mat4 matModelInvTrans = glm::inverse(glm::transpose(matModel));

	glUniformMatrix4fv(matModelInvTran_loc, 1, GL_FALSE, glm::value_ptr(matModelInvTrans));

	//// ********* Passing lighting info ***********
	//::g_pLightManager->CopyLightValuesToShader();

//		GLint light0_position_loc = glGetUniformLocation(shaderProgID, "theLights[0].position");
//		GLint light0_diffuse_loc = glGetUniformLocation(shaderProgID, "theLights[0].diffuse");
//		GLint light0_specular_loc = glGetUniformLocation(shaderProgID, "theLights[0].specular");
//		GLint light0_atten_loc = glGetUniformLocation(shaderProgID, "theLights[0].atten");
//		GLint light0_direction_loc = glGetUniformLocation(shaderProgID, "theLights[0].direction");
//		GLint light0_param1_loc = glGetUniformLocation(shaderProgID, "theLights[0].param1");
//		GLint light0_param2_loc = glGetUniformLocation(shaderProgID, "theLights[0].param2");

//			glUniform4f(light0_position_loc, 5.0f, 5.0f, 0.0f, 1.0f);
//			glUniform4f(light0_diffuse_loc, 1.0f, 1.0f, 1.0f, 1.0f);		// White
//			glUniform4f(light0_specular_loc, 1.0f, 1.0f, 1.0f, 1.0f);		// White
//
//		// Constant 
//		// Linear
//		// Quadratic
//		//glUniform4f(light0_atten_loc, 0.0f, 0.05f, 0.0f, 1.0f);		
//		//glUniform4f(light0_atten_loc, 
//		//	0.0f, // constant atten
//		//	0.1f, // linear atten
//		//	0.001f, // quadratic
//		//	1.0f);
//
//		//glUniform4f(light0_param1_loc, 0.0f, 0.01f, 0.01f, 1.0f);		// x = 0 = point light
//		//glUniform4f(light0_param2_loc, 1.0f, 0.01f, 0.01f, 1.0f);		// x = 1 = on

			//GLint eye_loc = glGetUniformLocation(shaderProgID, "eyeLocation");
			//glUniform4f(eye_loc, ::g_cameraEye.x, ::g_cameraEye.y, ::g_cameraEye.z, 1.0f);

			// ********* Passing lighting info ***********

	GLint diffuseColour_location = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_location = glGetUniformLocation(shaderProgID, "specularColour");

			//glUniform3f(newColour_location, ::g_triangleColour.x,
			//	::g_triangleColour.y, ::g_triangleColour.z);
	glUniform4f(diffuseColour_location,
		pCurMesh->diffuseColour.r,
		pCurMesh->diffuseColour.g,
		pCurMesh->diffuseColour.b,
		pCurMesh->diffuseColour.a);		// <<----- Alpha 

	// Specular HIGHLIGHT colour (and power)
	// glUniform4f(specularColour_location, 1.0f, 1.0f, 1.0f, 10.0f);
	glUniform4f(specularColour_location,
		pCurMesh->specularColour.r,		// COLOUR of the SPECULAR highlight
		pCurMesh->specularColour.g,
		pCurMesh->specularColour.b,
		pCurMesh->specularColour.w);	// POWER of specular (higher is shinier)

	sModelDrawInfo modelInfo;

	// Is the object wireframe? 
	if (pCurMesh->isWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (::g_pVAOManager->FindDrawInfoByModelName(pCurMesh->meshName, modelInfo))
	{
		glBindVertexArray(modelInfo.VAO_ID);

		glDrawElements(GL_TRIANGLES,
			modelInfo.numberOfIndices,
			GL_UNSIGNED_INT,
			0);
	}

	return;
}


cMeshObject* pFindByFriendlyName(std::string name)
{
	for (unsigned int index = 0; index != g_vecObjectsToDraw.size(); index++)
	{
		//is this the one? 
		if (::g_vecObjectsToDraw[index]->friendlyName == name)
		{
			// yUp
			return ::g_vecObjectsToDraw[index];
		}
	}
	// Didn't find it
	return NULL;
}


// Fill the vector with non transparent object to draw	
void AddOnlyNonTransparent(
	std::vector<cMeshObject*> &vecNonTransparent,
	std::vector<cMeshObject*> &vecAllTheObjects)
{
	int x = 0;
	while (x < vecAllTheObjects.size()) {
		if (vecAllTheObjects[x]->diffuseColour.a < 1.0f)
		{
			// It's  transparent
			x++;
		}
		else
		{
			// it's NOT transparent
			vecNonTransparent.push_back(vecAllTheObjects[x]);
			x++;
		}
	}
}

void AddOnlyTransparent(
	std::vector<cMeshObject*> &vecTransparent,
	std::vector<cMeshObject*> &vecAllTheObjects)
{
	int x = 0;
	while (x < vecAllTheObjects.size()) {
		if (vecAllTheObjects[x]->diffuseColour.a < 1.0f)
		{
			// It's  transparent
			vecTransparent.push_back(vecAllTheObjects[x]);
			x++;
		}
		else
		{
			// it's NOT transparent
			x++;
		}
	}
}

class cIsObjectACloserToTheCameraThanB
{
public:
	cIsObjectACloserToTheCameraThanB(glm::vec3 cameraEye)
	{
		this->m_cameraEye = cameraEye;
	}
	glm::vec3 m_cameraEye;

	bool operator() (cMeshObject* pObjectA, cMeshObject* pObjectB)
	{

		float distTocameraA = glm::distance(pObjectA->pos, this->m_cameraEye);
		float distTocameraB = glm::distance(pObjectB->pos, this->m_cameraEye);

		if (distTocameraA > distTocameraB)
		{
			return true;
		}
		return false;
	}
};

void SortTransparentBackToFront(std::vector<cMeshObject*> &vecTransparent,
	glm::vec3 cameraEye)
{
	cIsObjectACloserToTheCameraThanB myCompareThing(cameraEye);
	std::sort(vecTransparent.begin(),
		vecTransparent.end(),
		myCompareThing);		// Predicate function
}
