#ifndef _cMeshObject_HG_
#define _cMeshObject_HG_
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class cMeshObject
{
public:
	cMeshObject();
	std::string meshName;
	std::string friendlyName;		// Can look it up
	glm::vec3 pos;			// 0,0,0 (origin)
	// glm::vec4 colour
	glm::vec4 diffuseColour;		// 0,0,0,1 (black)
	glm::vec4 specularColour;		// 0,0,0,1 (black)
	glm::vec3 orientation;	// aka rotation 
	float scale;		// 1.0f  
	bool isWireframe;		// false

	bool isVisible;		// True by default

	static const unsigned int NUMBEROFTEXTURES = 4;
	std::string textureNames[NUMBEROFTEXTURES];
	float textureBlendRatio[NUMBEROFTEXTURES];

	// Alpha texture as transparency
	std::string alphaBlendTexture;
	bool bUseTextureAlpha;

	// NOT on the exam. 
	// Will discard textures that aren't being used
	// (used to make a better explosion)
	bool bIsImposterUseDiscard; 
};

#endif 