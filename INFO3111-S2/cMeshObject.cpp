#include "cMeshObject.h"

// c'tor
cMeshObject::cMeshObject()
{
	this->pos = glm::vec3(0.0f,0.0f,0.0f);				// 0,0,0 (origin)
	//this->colour = glm::vec4(0.0f,0.0f,0.0f,1.0f);		// 0,0,0,1 (black)
	this->diffuseColour = glm::vec4(0.0f,0.0f,0.0f,1.0f);		// 0,0,0,1 (black)

	// RGB are the COLOUR of the highlight (0 to 1)
	// W (4th) value is the POWER of the highlight
	//       (which is from 1 to 10,000) 
	this->specularColour = glm::vec4(1.0f, 1.0f, 1.0f, 10.0f);

	this->orientation = glm::vec3(0.0f, 0.0f, 0.0f);
	this->scale = 1.0f;
	this->isWireframe = false;

	this->isVisible = true;

	this->textureBlendRatio[0] = 0.0f;
	this->textureBlendRatio[1] = 0.0f;
	this->textureBlendRatio[2] = 0.0f;
	this->textureBlendRatio[3] = 0.0f;

	this->bUseTextureAlpha = false;

	this->bIsImposterUseDiscard = false;

	return;
}