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
private:
	void SetPipeline(ID3D11DeviceContext* context, unsigned int index);
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
	// draw calls
	//////////////////////////////
	void Draw(ID3D11DeviceContext* context, D3D11_VIEWPORT* viewPort);
	void Draw(ID3D11DeviceContext* context, unsigned int index, D3D11_VIEWPORT* viewPort);
	void DrawInstanced(ID3D11DeviceContext* context, D3D11_VIEWPORT* viewPort, unsigned int numberOfInstance);
};
