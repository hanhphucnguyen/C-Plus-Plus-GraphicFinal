#ifndef _globalStuff_HG_
#define _globalStuff_HG_ 

// These have to be included in the right 
// order through the ENTIRE build...

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include "cVAOManager.h"

#include "cMeshObject.h"

// Here because we might want to control the lights from outside main
#include "LightManager/cLightMangager.h"

#include "BasicTextureManager/cBasicTextureManager.h"

// Note that these variables, in the header, are all "extern"
// Never put a variable in a header that isn't extern

// Vector of things drawn in the scene
extern std::vector< cMeshObject* > g_vecObjectsToDraw;


extern cVAOManager* g_pVAOManager;

// The ball we can use to see where the lights are 
extern cMeshObject* g_pBall;

// Returns zero or NULL if we didn't find it
cMeshObject* pFindByFriendlyName(std::string name);

extern cLightManager* g_pLightManager;

// Used to show the attenuation spheres on the lights
extern unsigned int g_currentSelectedLightIndex;// = 0;
extern bool g_showDebugLightSpheres;// = true;

// The main output window
// (Moved here so we can set the title)
extern GLFWwindow* g_pWindow;

extern cBasicTextureManager* g_pTextureManager;

#endif
