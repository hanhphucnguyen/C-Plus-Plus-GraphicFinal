#include "cVAOManager.h"

#include "globalStuff.h"

#include <fstream>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

#include <sstream>

sModelDrawInfo::sModelDrawInfo()
{
	this->VAO_ID = 0;

	this->VertexBufferID = 0;
	this->VertexBuffer_Start_Index = 0;
	this->numberOfVertices = 0;

	this->IndexBufferID = 0;
	this->IndexBuffer_Start_Index = 0;
	this->numberOfIndices = 0;
	this->numberOfTriangles = 0;

	// The "local" (i.e. "CPU side" temporary array)
	this->pVertices = 0;	// or NULL
	this->pIndices = 0;		// or NULL

	// You could store the max and min values of the 
	//  vertices here (determined when you load them):
	glm::vec3 maxValues;
	glm::vec3 minValues;

	//	scale = 5.0/maxExtent;		-> 5x5x5
	float maxExtent;

	// FOR NOW, it's true
	this->GenerateSphericalUVs = true;

	// 
	this->FileHasUVs = false;

	return;
}


bool cVAOManager::LoadModelIntoVAO(
	std::string fileName,
	sModelDrawInfo &drawInfo,
	unsigned int shaderProgramID)

{
	// Load the model from file
	// (We do this here, since if we can't load it, there's 
	//	no point in doing anything else, right?)

	drawInfo.meshName = fileName;

	if (!this->m_LoadTheModel(fileName, drawInfo))
	{
		this->m_AppendTextToLastError("Didn't load model", true);
		return false;
	}

	// Generate UVs if needed
	if (drawInfo.GenerateSphericalUVs)
	{
		this->m_GenTextureCoordsSpherical(drawInfo, POSITIVE_X,
			POSITIVE_Y, false, 1.0f, false);
	}


	// 
	// Model is loaded and the vertices and indices are in the drawInfo struct
	// 

	// Create a VAO (Vertex Array Object), which will 
	//	keep track of all the 'state' needed to draw 
	//	from this buffer...

	// Ask OpenGL for a new buffer ID...
	glGenVertexArrays(1, &(drawInfo.VAO_ID));
	// "Bind" this buffer:
	// - aka "make this the 'current' VAO buffer
	glBindVertexArray(drawInfo.VAO_ID);

	// Now ANY state that is related to vertex or index buffer
	//	and vertex attribute layout, is stored in the 'state' 
	//	of the VAO... 


	// NOTE: OpenGL error checks have been omitted for brevity
//	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &(drawInfo.VertexBufferID));

	//	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, drawInfo.VertexBufferID);
	// sVert vertices[3]
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sVert) * drawInfo.numberOfVertices,	// ::g_NumberOfVertsToDraw,	// sizeof(vertices), 
		(GLvoid*)drawInfo.pVertices,							// pVertices,			//vertices, 
		GL_STATIC_DRAW);


	// Copy the index buffer into the video card, too
	// Create an index buffer.
	glGenBuffers(1, &(drawInfo.IndexBufferID));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawInfo.IndexBufferID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,			// Type: Index element array
		sizeof(unsigned int) * drawInfo.numberOfIndices,
		(GLvoid*)drawInfo.pIndices,
		GL_STATIC_DRAW);

	// Set the vertex attributes.

	GLint vpos_location = glGetAttribLocation(shaderProgramID, "vPos");	
	GLint vcol_location = glGetAttribLocation(shaderProgramID, "vCol");	
	GLint vNormal_location = glGetAttribLocation(shaderProgramID, "vNormal");	
	GLint vUVx2_location = glGetAttribLocation(shaderProgramID, "vUVx2");	// NEW

	// Set the vertex attributes for this shader
	glEnableVertexAttribArray(vpos_location);	// vPos
	glVertexAttribPointer(vpos_location, 4,		// vPos
		GL_FLOAT, GL_FALSE,
		sizeof(sVert),					//  sizeof(float) * 6,
		(void*)offsetof(sVert, x));		//  (void*)0);

	glEnableVertexAttribArray(vcol_location);	// vCol
	glVertexAttribPointer(vcol_location, 4,		// vCol
		GL_FLOAT, GL_FALSE,
		sizeof(sVert),					//  sizeof(float) * 6,
		(void*)offsetof(sVert, r));		//  (void*)(sizeof(float) * 3));
	
	glEnableVertexAttribArray(vNormal_location);	// vNormal
	glVertexAttribPointer(vNormal_location, 4,		// vNormal
		GL_FLOAT, GL_FALSE,
		sizeof(sVert),					//  sizeof(float) * 6,
		(void*)offsetof(sVert, nx));		//  (void*)(sizeof(float) * 3));
	
	// Where  is it going to find the UVs?? 
	glEnableVertexAttribArray(vUVx2_location);		// UVs
	glVertexAttribPointer(vUVx2_location, 4,		// UVs
		GL_FLOAT, GL_FALSE,
		sizeof(sVert),					
		(void*)offsetof(sVert, u0));		
	

	// Now that all the parts are set up, set the VAO to zero
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(vpos_location);
	glDisableVertexAttribArray(vcol_location);
	glDisableVertexAttribArray(vNormal_location);
	glDisableVertexAttribArray(vUVx2_location);


	// Store the draw information into the map
	this->m_map_ModelName_to_VAOID[drawInfo.meshName] = drawInfo;


	return true;
}


// We don't want to return an int, likely
bool cVAOManager::FindDrawInfoByModelName(
	std::string filename,
	sModelDrawInfo &drawInfo)
{
	std::map< std::string /*model name*/,
		sModelDrawInfo /* info needed to draw*/ >::iterator
		itDrawInfo = this->m_map_ModelName_to_VAOID.find(filename);

	// Find it? 
	if (itDrawInfo == this->m_map_ModelName_to_VAOID.end())
	{
		// Nope
		return false;
	}

	// Else we found the thing to draw
	// ...so 'return' that information
	drawInfo = itDrawInfo->second;
	return true;
}


bool cVAOManager::m_LoadTheModel(std::string fileName,
	sModelDrawInfo &drawInfo)
{
	// Open the file. 
	// Read until we hit the word "vertex"
	// Read until we hit the word "face"
	// Read until we hit the word "end_header"

	std::ifstream thePlyFile(fileName.c_str());
	if (!thePlyFile.is_open())
	{	// Something is wrong...
		std::stringstream ssError;
		ssError << "Can't open >" << fileName << "< file." << std::endl;
		this->m_AppendTextToLastError(ssError.str());
		return false;
	}

	std::string temp;
	while (thePlyFile >> temp)
	{
		if (temp == "vertex")
		{
			break;
		}
	};
	// read the number of vertices...
	thePlyFile >> drawInfo.numberOfVertices;	//::g_NumberOfVertices;

	while (thePlyFile >> temp)
	{
		if (temp == "face")
		{
			break;
		}
	};
	// read the number of triangles...
	thePlyFile >> drawInfo.numberOfTriangles;		// ::g_NumberOfTriangles;


	while (thePlyFile >> temp)
	{
		if (temp == "end_header")
		{
			break;
		}
	};

	// And now, we start reading vertices... Hazzah!

	// This is set up to match the ply (3d model) file. 
	// NOT the shader. 
	struct sVertPly
	{
		glm::vec3 pos;
		glm::vec4 colour;
		glm::vec4 normal;
		// UV coords, too
		glm::vec2 UVs;		// File has only UV (but shader has vec4)
	};

	std::vector<sVertPly> vecTempPlyVerts;

	sVertPly tempVert;
	// Load the vertices...
	for (unsigned int index = 0; index != drawInfo.numberOfVertices; // ::g_NumberOfVertices; 
		index++)
	{
		// This line is where you match it to the actual file.
		thePlyFile >> tempVert.pos.x >> tempVert.pos.y >> tempVert.pos.z;
		thePlyFile >> tempVert.normal.x >> tempVert.normal.y >> tempVert.normal.z;

		// Are we going to load UVs, too?
		if (drawInfo.FileHasUVs)
		{
			thePlyFile >> tempVert.UVs.x >> tempVert.UVs.y;
		}


//		tempVert.pos.x /= 10.0f;
//		tempVert.pos.y /= 10.0f;
//		tempVert.pos.z /= 10.0f;

		// Add too... what? 
		vecTempPlyVerts.push_back(tempVert);
	}

	// Create a local vertex array
	// Note: The format the file (ply) is DIFFERENT from this array:
	// - sVertPly was made to match the file format
	// - sVert was made to match the shader vertex attrib format

	drawInfo.pVertices = new sVert[drawInfo.numberOfVertices];

	// Optional clear array to zero 
	//memset( drawInfo.pVertices, 0, sizeof(sVert) * drawInfo.numberOfVertices);

	for (unsigned int index = 0; index != drawInfo.numberOfVertices; index++)
	{
		drawInfo.pVertices[index].x = vecTempPlyVerts[index].pos.x;
		drawInfo.pVertices[index].y = vecTempPlyVerts[index].pos.y;
		drawInfo.pVertices[index].z = vecTempPlyVerts[index].pos.z;
		drawInfo.pVertices[index].w = 1.0f;

		drawInfo.pVertices[index].r = vecTempPlyVerts[index].colour.r;
		drawInfo.pVertices[index].g = vecTempPlyVerts[index].colour.g;
		drawInfo.pVertices[index].b = vecTempPlyVerts[index].colour.b;
		drawInfo.pVertices[index].a = 1.0f;

		drawInfo.pVertices[index].nx = vecTempPlyVerts[index].normal.x;
		drawInfo.pVertices[index].ny = vecTempPlyVerts[index].normal.y;
		drawInfo.pVertices[index].nz = vecTempPlyVerts[index].normal.z;
		drawInfo.pVertices[index].nw = 1.0f;

		// For now, we are ignoring the UVs (setting them to zeros)
		drawInfo.pVertices[index].u0 = vecTempPlyVerts[index].UVs.x;
		drawInfo.pVertices[index].v0 = vecTempPlyVerts[index].UVs.y;
		drawInfo.pVertices[index].u1 = 0.0f;
		drawInfo.pVertices[index].v1 = 0.0f;

	}// for ( unsigned int index...


	struct sTriPly
	{
		unsigned int vindex[3];		// the 3 indices
		sVertPly verts[3];			// The actual vertices
	};

	std::vector<sTriPly> vecTempPlyTriangles;
	sTriPly tempTriangle;
	unsigned int discard = 0;
	for (unsigned int index = 0; index != drawInfo.numberOfTriangles;	// ::g_NumberOfTriangles; 
		index++)
	{
		// 3 5622 5601 5640
		thePlyFile >> discard						// the "3" at the start
			>> tempTriangle.vindex[0]
			>> tempTriangle.vindex[1]
			>> tempTriangle.vindex[2];

		//// Look up the vertex that matches the triangle index values.
		//tempTriangle.verts[0] = vecTempPlyVerts[ tempTriangle.vindex[0] ];
		//tempTriangle.verts[1] = vecTempPlyVerts[ tempTriangle.vindex[1] ];
		//tempTriangle.verts[2] = vecTempPlyVerts[ tempTriangle.vindex[2] ];

		vecTempPlyTriangles.push_back(tempTriangle);
	}//for ( unsigned int index...


	// NOW, we need to put them into the vertex array buffer that 
	//	will be passed to OpenGL. Why? 
	// Because we called glDrawArrays(), that's why. 

//	::g_NumberOfVertsToDraw = ::g_NumberOfTriangles * 3;	// 3 because "triangles"

	drawInfo.numberOfIndices = drawInfo.numberOfTriangles * 3;

	// sVert* pVertices = 0;
//	pVertices = new sVert[::g_NumberOfVertsToDraw];

	drawInfo.pIndices = new unsigned int[drawInfo.numberOfIndices];

	// Optional clear array to zero 
	//memset( drawInfo.pIndices, 0, sizeof(unsigned int) * drawInfo.numberOfIndices);

	// Allocate on the HEAP, so infinite size... 
	// delete pVertices			/// error!
	// delete [] pVertices		/// correct!!
	unsigned int indexBufferIndex = 0;
	for (unsigned int triIndex = 0;
		triIndex != drawInfo.numberOfTriangles;		// ::g_NumberOfTriangles; 
		triIndex++, indexBufferIndex += 3)
	{
		sTriPly& curTri = vecTempPlyTriangles[triIndex];

		// This is taking an array of triangles vertex indices 
		// (which is a 2D array of indices, really) and converting
		//  it into a 1D array of indices...)
		drawInfo.pIndices[indexBufferIndex + 0] = curTri.vindex[0];
		drawInfo.pIndices[indexBufferIndex + 1] = curTri.vindex[1];
		drawInfo.pIndices[indexBufferIndex + 2] = curTri.vindex[2];

		//pVertices[ vertIndex + 0 ].x = curTri.verts[0].pos.x;
		//pVertices[ vertIndex + 0 ].y = curTri.verts[0].pos.y;
		//pVertices[ vertIndex + 0 ].z = curTri.verts[0].pos.z;
		//pVertices[ vertIndex + 0 ].r = curTri.verts[0].colour.x;
		//pVertices[ vertIndex + 0 ].g = curTri.verts[0].colour.y;
		//pVertices[ vertIndex + 0 ].b = curTri.verts[0].colour.z;
//
		//pVertices[ vertIndex + 1 ].x = curTri.verts[1].pos.x;
		//pVertices[ vertIndex + 1 ].y = curTri.verts[1].pos.y;
		//pVertices[ vertIndex + 1 ].z = curTri.verts[1].pos.z;
		//pVertices[ vertIndex + 1 ].r = curTri.verts[1].colour.x;
		//pVertices[ vertIndex + 1 ].g = curTri.verts[1].colour.y;
		//pVertices[ vertIndex + 1 ].b = curTri.verts[1].colour.z;
//
		//pVertices[ vertIndex + 2 ].x = curTri.verts[2].pos.x;
		//pVertices[ vertIndex + 2 ].y = curTri.verts[2].pos.y;
		//pVertices[ vertIndex + 2 ].z = curTri.verts[2].pos.z;
		//pVertices[ vertIndex + 2 ].r = curTri.verts[2].colour.x;
		//pVertices[ vertIndex + 2 ].g = curTri.verts[2].colour.y;
		//pVertices[ vertIndex + 2 ].b = curTri.verts[2].colour.z;
//
	}// for ( unsigned int triIndex = 0...

	return true;
}


std::string cVAOManager::getLastError(bool bAndClear /*=true*/)
{
	std::string theLastError = this->m_lastErrorString;

	if (bAndClear)
	{
		this->m_lastErrorString = "";
	}

	return theLastError;
}

void cVAOManager::m_AppendTextToLastError(std::string text, bool addNewLineBefore /*=true*/)
{
	std::stringstream ssError;
	ssError << this->m_lastErrorString;

	if (addNewLineBefore)
	{
		ssError << std::endl;
	}

	ssError << text;

	this->m_lastErrorString = ssError.str();

	return;

}


// Generates texture coordinates based on a shperical projection from the 
// origin (0,0,0) location of the model. 
// NOTE: Will overwrite any existing texture coordinates 
// Inspired by: http://www.mvps.org/directx/articles/spheremap.htm
// 
//	basedOnNormals = true --> uses normals for calculation, so good for reflections
//	basedOnNormals = false --> uses model coordinates, so more 'shrink wrapped'
//  fast -> approximate or not (not uses sin() )
//  scale -> normally 1.0, but can be the limits of the texture coord
//
// The 'bias' is what coordinates are used for the generation. 
// For example, if POSITIVE_X is for u and POSITIVE_Y is for v, then:
//	u coords: based on -x to +x
//  v coords: based on -y to +y
//
void cVAOManager::m_GenTextureCoordsSpherical(sModelDrawInfo &drawInfo, enumTEXCOORDBIAS uBias, enumTEXCOORDBIAS vBias, bool basedOnNormals, float scale, bool fast)
{
	// determine the 'remaining' coordinate...
	bool xUsed = false;
	bool yUsed = false;
	bool zUsed = false;
	if (uBias == POSITIVE_X || vBias == POSITIVE_X)	xUsed = true;
	if (uBias == POSITIVE_Y || vBias == POSITIVE_Y)	yUsed = true;
	if (uBias == POSITIVE_Z || vBias == POSITIVE_Z)	yUsed = true;


	for (unsigned int index = 0; index != drawInfo.numberOfVertices; index++)
	{
		glm::vec3 xyz;
		if (basedOnNormals)
		{
			if (uBias == POSITIVE_X)		xyz.x = drawInfo.pVertices[index].nx;
			else if (uBias == POSITIVE_Y)	xyz.x = drawInfo.pVertices[index].ny;
			else if (uBias == POSITIVE_Z)	xyz.x = drawInfo.pVertices[index].nz;

			if (vBias == POSITIVE_X)		xyz.y = drawInfo.pVertices[index].nx;
			else if (vBias == POSITIVE_Y)	xyz.y = drawInfo.pVertices[index].ny;
			else if (vBias == POSITIVE_Z)	xyz.y = drawInfo.pVertices[index].nz;

			// Fill in the remaining coordinate...
			if (!xUsed)	xyz.z = drawInfo.pVertices[index].nx;
			if (!yUsed)	xyz.z = drawInfo.pVertices[index].ny;
			if (!zUsed)	xyz.z = drawInfo.pVertices[index].nz;
		}
		else
		{
			if (uBias == POSITIVE_X)		xyz.x = drawInfo.pVertices[index].x;
			else if (uBias == POSITIVE_Y)	xyz.x = drawInfo.pVertices[index].y;
			else if (uBias == POSITIVE_Z)	xyz.x = drawInfo.pVertices[index].z;

			if (vBias == POSITIVE_X)		xyz.y = drawInfo.pVertices[index].x;
			else if (vBias == POSITIVE_Y)	xyz.y = drawInfo.pVertices[index].y;
			else if (vBias == POSITIVE_Z)	xyz.y = drawInfo.pVertices[index].z;

			// Fill in the remaining coordinate...
			if (!xUsed)	xyz.z = drawInfo.pVertices[index].x;
			if (!yUsed)	xyz.z = drawInfo.pVertices[index].y;
			if (!zUsed)	xyz.z = drawInfo.pVertices[index].z;
		}

		// BUG: wasn't normalizing. Sorry about that. 
		xyz = glm::normalize(xyz);

		if (fast)
		{
			drawInfo.pVertices[index].u0 = ((xyz.x / 2.0f) + 0.5f) * scale;
			drawInfo.pVertices[index].v0 = ((xyz.y / 2.0f) + 0.5f) * scale;
		}
		else
		{
			const float PI = 3.141457f;
			drawInfo.pVertices[index].u0 = ((asin(xyz.x) / PI) + 0.5f) * scale;
			drawInfo.pVertices[index].v0 = ((asin(xyz.y) / PI) + 0.5f) * scale;
		}
		drawInfo.pVertices[index].u1 = drawInfo.pVertices[index].u0;
		drawInfo.pVertices[index].v1 = drawInfo.pVertices[index].v0;
	}
	return;
}
