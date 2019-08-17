#pragma once

#include "fbxsdk.h"
#include "Mesh.h"

class Model
{
private:
	// Model attributes
	std::vector<Mesh*> meshes;
	// Model's position relative to the world
	XMFLOAT4X4 world;
	XMMATRIX worldMatrix;
public:
	Model();
	~Model();

	//////////////////////////////
	// draw all meshes
	//////////////////////////////
	void Draw(ID3D11DeviceContext* context);
	//////////////////////////////
	// draw a mesh on a specified index
	//////////////////////////////
	void Draw(ID3D11DeviceContext* context, unsigned int index);
};
