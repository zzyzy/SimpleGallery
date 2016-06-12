#pragma once

#include <vector>
#include <unordered_map>

#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>

#include "Mesh.h"

struct Material
{
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	int texCount;
};

// Vertex Attribute Locations
static GLuint vertexLoc = 0, normalLoc = 1, texCoordLoc = 2;

struct Model
{
	std::string dirName = "models/helicopter/";
	std::string modelname = "helicopter.obj";
	bool opaque;

	~Model();

	void SetModelFile(std::string dirName, std::string modelName)
	{
		this->dirName = dirName;
		this->modelname = modelName;
		Import3DFromFile();
		LoadGLTextures();
		genVAOsAndUniformBuffer();
	}

	std::vector<Mesh> meshes;
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// the global Assimp scene object
	const aiScene* scene = nullptr;

	// scale factor for the model to fit in the window
	float scaleFactor;
	// images / texture
	// map image filenames to textureIds
	// pointer to texture Array
	std::unordered_map<std::string, GLuint> textureIdMap;
	std::unordered_map<std::string, GLuint> materialMap;

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)
private:
	void get_bounding_box_for_node(const aiNode* nd, aiVector3D* min, aiVector3D* max);


	void get_bounding_box(aiVector3D* min, aiVector3D* max);


	bool Import3DFromFile();


	int LoadGLTextures();


	void set_float4(float f[4], float a, float b, float c, float d);

	void color4_to_float4(const aiColor4D* c, float f[4]);


	void genVAOsAndUniformBuffer();
};
