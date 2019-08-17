#include "Model.h"

Model::Model()
{
	worldMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&world, worldMatrix);
}

Model::~Model()
{
}

void Model::Draw(ID3D11DeviceContext* context)
{
	if (context)
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			context->DrawIndexed(meshes[i]->indexCount, 0, 0);
		}
	}
}

void Model::Draw(ID3D11DeviceContext* context, unsigned int index)
{
	if (context)
	{
		context->DrawIndexed(meshes[index]->vertexCount, 0, 0);
	}
}
