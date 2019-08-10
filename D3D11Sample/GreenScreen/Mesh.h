#pragma once

// Include DirectX11 for interface access
#include <d3d11.h>

#include <DirectXMath.h>
using namespace DirectX;

struct Vertex
{
	XMFLOAT4 position;
	XMFLOAT2 texture;
	XMFLOAT3 normal;
	XMFLOAT4 color;
};

class Mesh
{
private:
	// attributes
	char* name;
	Vertex* vertices;
	void* indices;
	unsigned int vertexCount;
	unsigned int indexCount;
	// storage type
	XMFLOAT4X4 world;
	// speed type
	XMMATRIX worldMatrix;
public:
	Mesh(char* name, void* indices, unsigned int vertexCount, unsigned int indexCount);
	~Mesh();

	char* GetName() const;
	Vertex* GetVertices() const;
	void* GetIndices() const;
	unsigned int GetVertexCount() const;
	unsigned int GetIndexCount() const;
	const XMMATRIX GetWorldMatrix() const;

	//void SetName(char* name);
	//void SetVertices(Vertex* vertices);
	//void SetIndices(void* indices);
	//void SetVertexCount(unsigned int vertexCount);
	//void SetIndexCount(unsigned int indexCount);
	void SetWorldMatrix(const XMMATRIX& world);
};
