#include "Model.h"

Model::Model()
{
	name = nullptr;
	worldMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&world, worldMatrix);
}

Model::~Model()
{
	for (size_t i = 0; i < meshes.size(); i++)
	{
		delete meshes[i];
	}
	meshes.clear();
}

char* Model::GetName() { return name; }
std::vector<Mesh*>& Model::GetMeshes() { return meshes; }
XMMATRIX Model::GetWorldMatrix() { return XMLoadFloat4x4(&world); }

void Model::SetName(char* name) { this->name = name; }
void Model::SetWorldMatrix(const XMMATRIX& world) { worldMatrix = world; XMStoreFloat4x4(&this->world, worldMatrix); }

void Model::AddMesh(Mesh* mesh)
{
	if (mesh)
	{
		meshes.push_back(mesh);
	}
}

void Model::Draw(ID3D11DeviceContext* context)
{
	if (context)
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			UINT strides = sizeof(Vertex);
			UINT offsets = 0;
			context->IASetVertexBuffers(0, 1, &meshes[i]->vertexBuffer, &strides, &offsets);
			context->IASetIndexBuffer(meshes[i]->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			context->VSSetShader(meshes[i]->vertexShader, nullptr, 0);
			context->PSSetShader(meshes[i]->pixelShader, nullptr, 0);
			context->PSSetShaderResources(0, 1, &meshes[i]->shaderRV);
			context->PSSetSamplers(0, 1, &meshes[i]->sampler);
			context->DrawIndexed(meshes[i]->indexCount, 0, 0);
		}
	}
}

void Model::Draw(ID3D11DeviceContext* context, unsigned int index)
{
	if (context && (index >= 0 && index < meshes.size()))
	{
		context->DrawIndexed(meshes[index]->vertexCount, 0, 0);
	}
}
