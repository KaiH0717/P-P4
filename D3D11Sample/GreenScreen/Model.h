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

	//////////////////////////////
	// Methods
	//////////////////////////////
	void AddMesh(Mesh* mesh);
	void AddMesh(Mesh* mesh, const char* fileName);

	//////////////////////////////
	// draw calls
	//////////////////////////////
	void DrawIndexed(ID3D11DeviceContext* context, D3D11_VIEWPORT* viewPort);
	void DrawIndexInstanced(ID3D11DeviceContext* context, D3D11_VIEWPORT* viewPort, unsigned int numberOfInstance);
};
