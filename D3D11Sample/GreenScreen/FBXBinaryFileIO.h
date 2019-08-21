#pragma once
#include "Vertex.h"
#include <iostream>
#include <fstream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// REMARKS: 
// MUST SET DATA BEFORE WRITING IT OUT,
// MUST READ DATA TO POPULATE VECTOR
////////////////////////////////////////////////////////////////////////////////
class FBXBinaryFileIO
{
public:
	std::vector<unsigned int> indices;
	std::vector<PER_VERTEX_DATA> vertices;
	unsigned int index = 0;
	PER_VERTEX_DATA vertex;

	unsigned int vertexCount = 0;
	unsigned int indexCount = 0;
public:
	////////////////////////////////////////
	// Setter functions
	////////////////////////////////////////
	void SetVertexPosition(float* position)
	{
		vertex.position[0] = position[0];
		vertex.position[1] = position[1];
		vertex.position[2] = position[2];
		vertex.position[3] = position[3];
	}
	void SetVertexNormal(float* normal)
	{
		vertex.normal[0] = normal[0];
		vertex.normal[1] = normal[1];
		vertex.normal[2] = normal[2];
	}
	void SetVertexTexture(float* texture)
	{
		vertex.texture[0] = texture[0];
		vertex.texture[1] = texture[1];
	}
	void SetVertexData(float* position, float* normal, float* texture)
	{
		SetVertexPosition(position);
		SetVertexNormal(normal);
		SetVertexTexture(texture);
	}
	////////////////////////////////////////
	// File IO functions
	////////////////////////////////////////
	void Write(const char* fileName)
	{
		std::fstream bfout;
		bfout.open(fileName, std::ios::out | std::ios::binary | std::ios::trunc);
		if (bfout.is_open())
		{
			// write vertex data
			vertexCount = static_cast<unsigned int>(vertices.size());
			bfout.write((char*)& vertexCount, sizeof(unsigned int));
			bfout.write((char*)vertices.data(), (sizeof(PER_VERTEX_DATA) * vertexCount));
			// write index data
			indexCount = static_cast<unsigned int>(indices.size());
			bfout.write((char*)& indexCount, sizeof(unsigned int));
			bfout.write((char*)indices.data(), (sizeof(unsigned int) * indexCount));

			bfout.close();
		}
		else return;
	}
	void Read(const char* fileName)
	{
		vertexCount = 0;
		indexCount = 0;
		this->vertices.clear();
		this->indices.clear();
		std::fstream bfin;
		bfin.open(fileName, std::ios::in | std::ios::binary);
		if (bfin.is_open())
		{
			// read vertex data
			bfin.read((char*)& vertexCount, sizeof(unsigned int)); // read vertex count
			for (unsigned int i = 0; i < vertexCount; i++)
			{
				// populate vertex vector
				bfin.read((char*)& vertex, sizeof(PER_VERTEX_DATA));
				vertices.push_back(vertex);
			}
			// read index data
			bfin.read((char*)& indexCount, sizeof(unsigned int)); // read index count
			for (unsigned int i = 0; i < indexCount; i++)
			{
				// populate index vector
				bfin.read((char*)& index, sizeof(unsigned int));
				indices.push_back(index);
			}

			bfin.close();
		}
		else return;
	}
};
