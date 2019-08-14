#pragma once

#include "fbxsdk.h"
#include "Mesh.h"
#include <vector>

class Model
{
private:
	// storage type
	XMFLOAT4X4 world;
	// speed type
	XMMATRIX worldMatrix;
	FbxManager* fbxManager;
public:
};

