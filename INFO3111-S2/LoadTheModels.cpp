#include "globalStuff.h"
#include "cMeshObject.h"

#include <glm/glm.hpp>
#include <iostream>

// Here's the actual variable (the one and only)
std::vector< cMeshObject* > g_vecObjectsToDraw;

void LoadTheModelsIntoScene(unsigned int shaderProgID)
{

	::g_pTextureManager = new cBasicTextureManager();
	::g_pTextureManager->SetBasePath("Textures");

	// load a texture
	::g_pTextureManager->Create2DTextureFromBMPFile("RapsRule.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Yellow.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Brick.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("SpidermanUV_square.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("SpotsAlphaTexture.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Rock.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Explosion2.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Explosion3.bmp", true);
	//check texture load
	if (!::g_pTextureManager->Create2DTextureFromBMPFile("HEYOpenGL.bmp", true))
	{
		std::cout << "Didn't load texture" << std::endl;
	}
	else
	{
		std::cout << "Texture loaded ok" << std::endl;
	}

	
	//// A "height map example"
	//::g_pTextureManager->Create2DTextureFromBMPFile("MountainTextureHeight.bmp", true);
	
	sModelDrawInfo ballModel;
	::g_pVAOManager->LoadModelIntoVAO("smoothbal_inverted_xyz_n.ply",
		ballModel,
		shaderProgID);

	sModelDrawInfo spiderMan;
	spiderMan.FileHasUVs = true;				    //******NOTE****/
	spiderMan.GenerateSphericalUVs = false;		//******NOTE****/
	::g_pVAOManager->LoadModelIntoVAO("legospiderman_All.ply",
		spiderMan,
		shaderProgID);

	sModelDrawInfo falcon;
	falcon.GenerateSphericalUVs = true;
	::g_pVAOManager->LoadModelIntoVAO("MilleniumFalcon_xyz_n.ply",
		falcon,
		shaderProgID);

	sModelDrawInfo bullet;
	::g_pVAOManager->LoadModelIntoVAO("TearDropBullet_inverted_normals_xyz_n.ply",
		bullet,
		shaderProgID);

	sModelDrawInfo smallQuad;
	smallQuad.FileHasUVs = true;
	smallQuad.GenerateSphericalUVs = false;
	if (::g_pVAOManager->LoadModelIntoVAO("1x1_2Tri_Quad_2_Sided_xyz_n_uv_2.ply",
		smallQuad,
		shaderProgID))
	{
		std::cout << "loaded the small quad" << std::endl;
	}
	/*
	sModelDrawInfo asteroidGroup;
	asteroidGroup.GenerateSphericalUVs = true;
	asteroidGroup.FileHasUVs = false;
	if (::g_pVAOManager->LoadModelIntoVAO("asteroid_group_375084_verts.ply",
		asteroidGroup,
		shaderProgID))
	{
		std::cout << "Asteroids loaded" << std::endl;
	}
	*/

	
	sModelDrawInfo cowModel;
	cowModel.GenerateSphericalUVs = true;
	cowModel.FileHasUVs = false;				
	::g_pVAOManager->LoadModelIntoVAO("cow_xyzN.ply",
		cowModel,
		shaderProgID);
	
	
	/*sModelDrawInfo largeQuad;
	largeQuad.FileHasUVs = true;
	largeQuad.GenerateSphericalUVs = false;
	if (::g_pVAOManager->LoadModelIntoVAO("Flat_Quad_xyz_n_uv.ply",
		largeQuad,
		shaderProgID))
	{
		std::cout << "loaded the large quad" << std::endl;
	}*/

	//*************************************************
	//*************************************************
	// Add the objects to the scene I want to draw
	//*************************************************
	//*************************************************

	//Must have
	cMeshObject* g_pBall = new cMeshObject();
	g_pBall->meshName = "smoothbal_inverted_xyz_n.ply";
	g_pBall->friendlyName = "meshball";
	g_pBall->pos = glm::vec3(0.0f, 0.0f, 0.0f);
	g_pBall->scale = 1.0f;
	g_pBall->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	g_pBall->isWireframe = true;
	g_pBall->isVisible = false;
	::g_vecObjectsToDraw.push_back(g_pBall);

	cMeshObject* cow = new cMeshObject();
	cow->meshName = "cow_xyzN.ply";
	cow->pos = glm::vec3(0.0f, 0.0f, 0.0f);
	cow->scale = 5.0f;
	cow->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 0.4f);
	::g_vecObjectsToDraw.push_back(cow);

	cMeshObject* cow2 = new cMeshObject();
	cow2->meshName = "cow_xyzN.ply";
	cow2->pos = glm::vec3(0.0f, 0.0f, 10.0f);
	cow2->scale = 5.0f;
	cow2->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 0.4f);
	::g_vecObjectsToDraw.push_back(cow2);

	cMeshObject* cow3 = new cMeshObject();
	cow3->meshName = "cow_xyzN.ply";
	cow3->pos = glm::vec3(0.0f, 0.0f, 5.0f);
	cow3->scale = 5.0f;
	cow3->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 0.4f);
	::g_vecObjectsToDraw.push_back(cow3);

	/*cMeshObject* pAss = new cMeshObject();
	pAss->meshName = "asteroid_group_375084_verts.ply";
	pAss->pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pAss->scale = 0.1f;
	pAss->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pAss->textureNames[0] = "Rock.bmp";
	pAss->textureBlendRatio[0] = 1.0f;
	::g_vecObjectsToDraw.push_back(pAss);*/

	cMeshObject* pSpiderman = new cMeshObject();
	pSpiderman->meshName = "legospiderman_All.ply";
	pSpiderman->friendlyName = "current";
	pSpiderman->pos = glm::vec3(0.0f, 5.0f, 0.0f);
	pSpiderman->scale = 1.0f;
	pSpiderman->textureNames[0] = "SpidermanUV_square.bmp";
	pSpiderman->textureBlendRatio[0] = 1.0f;		// All raptors
	pSpiderman->textureNames[1] = "Brick.bmp";
	pSpiderman->textureBlendRatio[1] = 0.0f;		// All raptors
	pSpiderman->alphaBlendTexture = "SpotsAlphaTexture.bmp";
	pSpiderman->bUseTextureAlpha = true;
	::g_vecObjectsToDraw.push_back(pSpiderman);

	cMeshObject* pFalcon = new cMeshObject();
	pFalcon->meshName = "MilleniumFalcon_xyz_n.ply";
	pFalcon->friendlyName = "Falcon";
	pFalcon->pos = glm::vec3(-3.0f, 3.0f, 0.0f);
	pFalcon->scale = 5.0f;
	pFalcon->orientation = glm::vec3( 
		glm::radians(-90.0f), 
		glm::radians(170.0f),
		glm::radians(20.0f));
	pFalcon->diffuseColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0);
	pFalcon->textureNames[0] = "RapsRule.bmp";
	pFalcon->textureBlendRatio[0] = 0.0f;		// All raptors
	pFalcon->textureNames[1] = "Brick.bmp";
	pFalcon->textureBlendRatio[1] = 1.0f;		// All raptors
	pFalcon->alphaBlendTexture = "HEYOpenGL.bmp";
	pFalcon->bUseTextureAlpha = true;
	::g_vecObjectsToDraw.push_back(pFalcon);

	cMeshObject* pBullet1 = new cMeshObject();
	pBullet1->meshName = "TearDropBullet_inverted_normals_xyz_n.ply";
	pBullet1->friendlyName = "bullet1";
	pBullet1->pos = glm::vec3(0.0f, 0.5f, 5.0f);
	pBullet1->scale = 0.1f;
	pBullet1->orientation = glm::vec3( 0.0f, glm::radians(-180.0f), 0.0f);
	pBullet1->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	::g_vecObjectsToDraw.push_back(pBullet1);

	/*cMeshObject* pLargeFlatQuad = new cMeshObject();
	pLargeFlatQuad->friendlyName = "heightMappedTerrain";
	pLargeFlatQuad->meshName = "Flat_Quad_xyz_n_uv.ply";
	pLargeFlatQuad->pos = glm::vec3(0.0f, -15.0f, 0.0f);
	pLargeFlatQuad->scale = 0.5f;
	pLargeFlatQuad->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pLargeFlatQuad->textureNames[0] = "RapsRule.bmp";
	pLargeFlatQuad->textureBlendRatio[0] = 1.0f;
	pLargeFlatQuad->isWireframe = false;
	::g_vecObjectsToDraw.push_back(pLargeFlatQuad);*/

	cMeshObject* pSmallQuad = new cMeshObject();
	pSmallQuad->meshName = "1x1_2Tri_Quad_2_Sided_xyz_n_uv_2.ply";
	pSmallQuad->pos = glm::vec3(0.0f, 10.0f, 0.0f);
	pSmallQuad->scale = 2.0f;
	pSmallQuad->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pSmallQuad->textureNames[0] = "Explosion3.bmp";
	pSmallQuad->textureBlendRatio[0] = 1.0f;
	// *************************************
	pSmallQuad->bIsImposterUseDiscard = true;
	// *************************************
	::g_vecObjectsToDraw.push_back(pSmallQuad);

	// DO initial, expensive, FULL sort here
	// (then do a much less expensive sort per frame)

	//NOTE: If we want to have a range of specular (shiny) objects, 
	//      we would change the POWER value of the specular colour.
	//      And, unlike other colour things, it STARTS at 1.0f, and 
	//      goes to REALLY large numbers, like 10,000. 
	//    10 is sort of shiny
	//    100 is more shiny (like a coffee cup)
	//    1,000 is REALLY shiny.

	return;
}