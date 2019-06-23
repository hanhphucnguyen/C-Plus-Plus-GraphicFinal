#ifndef _cVAOManager_HG_
#define _cVAOManager_HG_

// Will load the models and place them 
// into the vertex and index buffers to be drawn

#include <string>
#include <map>

// The vertex structure 
//	that's ON THE GPU (eventually) 
// So dictated from THE SHADER
struct sVert
{
	float x, y, z, w;		
	float r, g, b, a;
	float nx, ny, nz, nw;
	float u0, v0, u1, v1;		// Two texture coordinates
};


struct sModelDrawInfo
{
	sModelDrawInfo();

	std::string meshName;

	unsigned int VAO_ID;

	unsigned int VertexBufferID;
	unsigned int VertexBuffer_Start_Index;
	unsigned int numberOfVertices;

	unsigned int IndexBufferID;
	unsigned int IndexBuffer_Start_Index;
	unsigned int numberOfIndices;
	unsigned int numberOfTriangles;

	// If true, then there's no UVs in the model or we want 
	// to overwrite them (like with the asteroids)
	bool GenerateSphericalUVs;

	// This is onlly in Section 2
	bool FileHasUVs;

	// The "local" (i.e. "CPU side" temporary array)
	sVert* pVertices;	//  = 0;
	// The index buffer (CPU side)
	unsigned int* pIndices;

	//glm::vec3 max;
	//glm::vec3 min;
	//float maxExtent;


};


class cVAOManager
{
public:

	bool LoadModelIntoVAO(std::string fileName,
		sModelDrawInfo &drawInfo,
		unsigned int shaderProgramID);

	// We don't want to return an int, likely
	bool FindDrawInfoByModelName(std::string filename,
		sModelDrawInfo &drawInfo);

	std::string getLastError(bool bAndClear = true);

private:

	// Will generate UV coords
	enum enumTEXCOORDBIAS
	{
		POSITIVE_X, POSITIVE_Y, POSITIVE_Z
	};
	void m_GenTextureCoordsSpherical(sModelDrawInfo &drawInfo, 
		                             enumTEXCOORDBIAS uBias, 
		                             enumTEXCOORDBIAS vBias, 
		                             bool basedOnNormals, 
		                             float scale, 
		                             bool fast);


	std::map< std::string /*model name*/,
		sModelDrawInfo /* info needed to draw*/ >
		m_map_ModelName_to_VAOID;

	// Loads the ply model file into a temporary array
	bool m_LoadTheModel(std::string fileName,
		sModelDrawInfo &drawInfo);

	std::string m_lastErrorString;
	void m_AppendTextToLastError(std::string text, bool addNewLineBefore = true);
};

#endif	// _cVAOManager_HG_
