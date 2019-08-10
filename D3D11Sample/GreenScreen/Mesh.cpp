#include "Mesh.h"

Mesh::Mesh(char* name, void* indices,unsigned int vertexCount, unsigned int indexCount)
{
	this->name = name;
	this->vertexCount = vertexCount;
	this->indexCount = indexCount;
	vertices = new Vertex[this->vertexCount];
	this->indices = indices;
}

Mesh::~Mesh()
{
	delete[] vertices;
	vertices = nullptr;
}

char* Mesh::GetName() const { return name; }
Vertex* Mesh::GetVertices() const { return vertices; }
void* Mesh::GetIndices() const { return indices; }
unsigned int Mesh::GetVertexCount() const { return vertexCount; }
unsigned int Mesh::GetIndexCount() const { return indexCount; }

//void Mesh::SetName(char* name) { this->name = name; }
//void Mesh::SetVertices(Vertex* vertices) { this->vertices = vertices; }
//void Mesh::SetIndices(void* indices) { this->indices = indices; }
//void Mesh::SetVertexCount(unsigned int vertexCount) { this->vertexCount = vertexCount; }
//void Mesh::SetIndexCount(unsigned int indexCount) { this->indexCount = indexCount; }
