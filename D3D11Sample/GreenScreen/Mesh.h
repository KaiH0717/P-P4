#pragma once

// Include DirectX11 for interface access
#include <d3d11.h>

#include <DirectXMath.h>
using namespace DirectX;

struct Vertex
{
	XMFLOAT4 position;
	XMFLOAT4 color;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
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
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
public:
	Mesh();
	Mesh(char* name, void* indices, unsigned int vertexCount, unsigned int indexCount);
	~Mesh();

	const char* GetName() const;
	Vertex* GetVertices() const;
	const void* GetIndices() const;
	const unsigned int& GetVertexCount() const;
	const unsigned int& GetIndexCount() const;
	const XMMATRIX& GetWorldMatrix() const;

	void SetName(char* name);
	void SetVertices(Vertex* vertices);
	void SetIndices(void* indices);
	void SetVertexCount(unsigned int vertexCount);
	void SetIndexCount(unsigned int indexCount);
	void SetWorldMatrix(const XMMATRIX& world);
};
