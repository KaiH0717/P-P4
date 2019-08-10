#pragma once

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
	char* name;
	Vertex* vertices;
	void* indices;
	unsigned int vertexCount;
	unsigned int indexCount;
public:
	Mesh(char* name, void* indices, unsigned int vertexCount, unsigned int indexCount);
	~Mesh();

	char* GetName() const;
	Vertex* GetVertices() const;
	void* GetIndices() const;
	unsigned int GetVertexCount() const;
	unsigned int GetIndexCount() const;

	//void SetName(char* name);
	//void SetVertices(Vertex* vertices);
	//void SetIndices(void* indices);
	//void SetVertexCount(unsigned int vertexCount);
	//void SetIndexCount(unsigned int indexCount);
};
