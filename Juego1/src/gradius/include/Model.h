#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include <vector>

class Model {
	// model data
	int numberMeshes;
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	void loadModel (string path);
	void processNode (aiNode *node, const aiScene *scene);
	Mesh processMesh (aiMesh *mesh, const aiScene *scene);
	vector<Texture> loadMaterialTextures (aiMaterial *mat, aiTextureType type,
		string typeName);
public:
	vec3 massCenter;
	vec4 AABB;

	vec4 AABBWithOffset (mat4 offset);
	vector<Mesh> getMeshes ();
	Model (string path, bool gamma = false) : gammaCorrection (gamma) {
		massCenter = vec3 (0.0f);
		numberMeshes = 0;
		loadModel (path);
	}
	void Draw (Shader *shader, int amount = 0);
};

#endif

