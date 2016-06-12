#include "Model.h"

#include <iostream>
#include <fstream>
#include <IL/il.h>

Model::~Model()
{
	textureIdMap.clear();

	// clear meshes stuff
	for (unsigned int i = 0; i < meshes.size(); ++i)
	{
		glDeleteVertexArrays(1, &(meshes[i].vao));
		glDeleteTextures(1, &(meshes[i].texIndex));
		glDeleteBuffers(1, &(meshes[i].uniformBlockIndex));
	}
}

void Model::get_bounding_box_for_node(const aiNode* nd,
                                      aiVector3D* min,
                                      aiVector3D* max)

{
	//aiMatrix4x4 prev;
	unsigned int n = 0, t;

	for (; n < nd->mNumMeshes; ++n)
	{
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t)
		{
			aiVector3D tmp = mesh->mVertices[t];

			min->x = aisgl_min(min->x, tmp.x);
			min->y = aisgl_min(min->y, tmp.y);
			min->z = aisgl_min(min->z, tmp.z);

			max->x = aisgl_max(max->x, tmp.x);
			max->y = aisgl_max(max->y, tmp.y);
			max->z = aisgl_max(max->z, tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n)
	{
		get_bounding_box_for_node(nd->mChildren[n], min, max);
	}
}


void Model::get_bounding_box(aiVector3D* min, aiVector3D* max)
{
	min->x = min->y = min->z = 1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode, min, max);
}


bool Model::Import3DFromFile()
{
	std::string pFile = dirName + modelname;
	//check if file exists
	std::ifstream fin(pFile.c_str());
	if (!fin.fail())
	{
		fin.close();
	}
	else
	{
		printf("Couldn't open file: %s\n", pFile.c_str());
		printf("%s\n", importer.GetErrorString());
		return false;
	}

	scene = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Quality);

	// If the import failed, report it
	if (!scene)
	{
		printf("%s\n", importer.GetErrorString());
		return false;
	}

	// Now we can access the file's contents.
	printf("Import of scene %s succeeded.\n", pFile.c_str());

	aiVector3D scene_min, scene_max; // , scene_center;
	get_bounding_box(&scene_min, &scene_max);
	float tmp;
	tmp = scene_max.x - scene_min.x;
	tmp = scene_max.y - scene_min.y > tmp ? scene_max.y - scene_min.y : tmp;
	tmp = scene_max.z - scene_min.z > tmp ? scene_max.z - scene_min.z : tmp;
	scaleFactor = 1.f / tmp;

	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}


int Model::LoadGLTextures()
{
	ILboolean success;

	/* initialization of DevIL */

	/* scan scene's materials for textures */
	for (unsigned int m = 0; m < scene->mNumMaterials; ++m)
	{
		int texIndex = 0;
		aiString path; // filename

		aiReturn texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		while (texFound == AI_SUCCESS)
		{
			//fill map with textures, OpenGL image ids set to 0
			textureIdMap[path.data] = 0;
			// more textures?
			texIndex++;
			texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		}
	}

	int numTextures = textureIdMap.size();

	/* create and fill array with DevIL texture ids */
	ILuint* imageIds = new ILuint[numTextures];
	ilGenImages(numTextures, imageIds);

	/* create and fill array with GL texture ids */
	GLuint* textureIds = new GLuint[numTextures];
	glGenTextures(numTextures, textureIds); /* Texture name generation */

	/* get iterator */
	std::unordered_map<std::string, GLuint>::iterator itr = textureIdMap.begin();
	int i = 0;
	for (; itr != textureIdMap.end(); ++i , ++itr)
	{
		//save IL image ID
		std::string filename = (*itr).first; // get filename
		filename = dirName + filename;
		(*itr).second = textureIds[i]; // save texture id for filename in map

		ilBindImage(imageIds[i]); /* Binding of DevIL image name */
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		success = ilLoadImage(filename.c_str());

		if (success)
		{
			/* Convert image to RGBA */
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

			/* Create and load textures to OpenGL */
			glBindTexture(GL_TEXTURE_2D, textureIds[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
			                          ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
			                          ilGetData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			std::cout << "Loaded texture " << filename << std::endl;
		}
		else
			printf("Couldn't load Image: %s\n", filename.c_str());
	}
	/* Because we have already copied image data into texture data
	we can release memory used by image. */
	ilDeleteImages(numTextures, imageIds);

	//Cleanup
	delete[] imageIds;
	delete[] textureIds;

	//return success;
	return true;
}


//// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
//void Color4f(const aiColor4D *color)
//{
//	glColor4f(color->r, color->g, color->b, color->a);
//}

void Model::set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

void Model::color4_to_float4(const aiColor4D* c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}


void Model::genVAOsAndUniformBuffer()
{
	Mesh aMesh;
	Material aMat;
	GLuint buffer;

	// For each mesh
	for (unsigned int n = 0; n < scene->mNumMeshes; ++n)
	{
		const aiMesh* mesh = scene->mMeshes[n];

		// create array with faces
		// have to convert from Assimp format to array
		unsigned int* faceArray;
		faceArray = static_cast<unsigned int *>(malloc(sizeof(unsigned int) * mesh->mNumFaces * 3));
		unsigned int faceIndex = 0;

		for (unsigned int t = 0; t < mesh->mNumFaces; ++t)
		{
			const aiFace* face = &mesh->mFaces[t];

			memcpy(&faceArray[faceIndex], face->mIndices, 3 * sizeof(unsigned int));
			faceIndex += 3;
		}
		aMesh.numFaces = scene->mMeshes[n]->mNumFaces;

		// generate Vertex Array for mesh
		glGenVertexArrays(1, &(aMesh.vao));
		glBindVertexArray(aMesh.vao);

		// buffer for faces
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);

		// buffer for vertex positions
		if (mesh->HasPositions())
		{
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(vertexLoc);
			glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, nullptr);
		}

		// buffer for vertex normals
		if (mesh->HasNormals())
		{
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
			glEnableVertexAttribArray(normalLoc);
			glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, nullptr);
		}

		// buffer for vertex texture coordinates
		if (mesh->HasTextureCoords(0))
		{
			float* texCoords = static_cast<float *>(malloc(sizeof(float) * 2 * mesh->mNumVertices));
			for (unsigned int k = 0; k < mesh->mNumVertices; ++k)
			{
				texCoords[k * 2] = mesh->mTextureCoords[0][k].x;
				texCoords[k * 2 + 1] = mesh->mTextureCoords[0][k].y;
			}
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
			glEnableVertexAttribArray(texCoordLoc);
			glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, 0, 0, nullptr);
		}

		// unbind buffers
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// create material uniform buffer
		aiMaterial* mtl = scene->mMaterials[mesh->mMaterialIndex];
		aiString name;
		if (AI_SUCCESS != mtl->Get(AI_MATKEY_NAME, name))
		{
			std::cerr << "Somethign happened when we try to get the material's name" << std::endl;
			throw std::runtime_error("Somethign happened when we try to get the material's name");
		}

		aiString texPath; //contains filename of texture
		if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
		{
			//bind texture
			unsigned int texId = textureIdMap[texPath.data];
			aMesh.texIndex = texId;
			aMat.texCount = 1;
		}
		else
		{
			aMat.texCount = 0;
		}

		if (materialMap.find(name.C_Str()) != materialMap.end())
		{
			aMesh.uniformBlockIndex = materialMap[name.C_Str()];
			std::cout << "Material already loaded " << name.C_Str() << std::endl;
		}
		else
		{
			auto opacity = 1.0f;
			if (AI_SUCCESS == mtl->Get(AI_MATKEY_OPACITY, opacity))
			{
				opaque = true;
			}

			float c[4];
			set_float4(c, 0.0f, 0.0f, 0.0f, opacity);
			aiColor4D diffuse;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
				color4_to_float4(&diffuse, c);
			memcpy(aMat.diffuse, c, sizeof(c));
			aMat.diffuse[3] = opacity;

			set_float4(c, 0.0f, 0.0f, 0.0f, opacity);
			aiColor4D ambient;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
				color4_to_float4(&ambient, c);
			memcpy(aMat.ambient, c, sizeof(c));
			aMat.ambient[3] = opacity;

			set_float4(c, 0.0f, 0.0f, 0.0f, opacity);
			aiColor4D specular;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
				color4_to_float4(&specular, c);
			memcpy(aMat.specular, c, sizeof(c));
			aMat.specular[3] = opacity;

			set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
			aiColor4D emission;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
				color4_to_float4(&emission, c);
			memcpy(aMat.emissive, c, sizeof(c));

			float shininess = 0.0;
			unsigned int max;
			aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
			aMat.shininess = shininess;

			glGenBuffers(1, &(aMesh.uniformBlockIndex));
			glBindBuffer(GL_UNIFORM_BUFFER, aMesh.uniformBlockIndex);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(aMat), static_cast<void *>(&aMat), GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			std::cout << "Loaded material " << name.C_Str() << std::endl;
			materialMap[name.C_Str()] = aMesh.uniformBlockIndex;
		}

		meshes.push_back(aMesh);
	}
}
