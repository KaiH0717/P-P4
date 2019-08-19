#pragma once

#include "fbxsdk.h"
#include "Mesh.h"

class Model
{
private:
	char* name;
	// Model attributes
	std::vector<Mesh*> meshes;
	// Model's position relative to the world
	XMFLOAT4X4 world;
	XMMATRIX worldMatrix;
public:
	Model();
	~Model();

	//////////////////////////////
	// getters
	//////////////////////////////
	char* GetName();
	std::vector<Mesh*>& GetMeshes();
	XMMATRIX GetWorldMatrix();

	//////////////////////////////
	// setters
	//////////////////////////////
	void SetName(char* name);
	void SetWorldMatrix(const XMMATRIX& world);

	void AddMesh(Mesh* mesh);

	//////////////////////////////
	// draw all meshes
	//////////////////////////////
	void Draw(ID3D11DeviceContext* context);
	//////////////////////////////
	// draw a mesh on a specified index
	//////////////////////////////
	void Draw(ID3D11DeviceContext* context, unsigned int index);
};
