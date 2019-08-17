#pragma once
// This file can be used to contain very basic DX11 Loading, Drawing & Clean Up. (Start Here, or Make your own set of classes)
#pragma once
// Include our DX11 middle ware 
#include "../../Gateware/Interface/G_Graphics/GDirectX11Surface.h"
#include "../../Gateware/Interface/G_Math/GVector.h"
// Other possible Gateware libraries:
// GMatrix, GAudio, GController, GInput, GLog, GFile... +more
// While these libraries won't directly add any points to your project they can be used to make them more "gamelike"
// To add a library just include the relevant "Interface" & add any related *.cpp files from the "Source" folder to the Gateware filter->.
// Gateware comes with MSDN style docs too: "Gateware\Documentation\html\index.html"

// Include DirectX11 for interface access
#include <d3d11.h>

// include my own files and other helpful files
#include "FBXBinaryFileIO.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
// Lari's time class
#include "XTime.h"

struct ConstantBuffer
{
	// storage type for permenant use
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	// [0] = directional lighting, [1] = point lighting
	XMFLOAT4 lightPos[3];
	XMFLOAT4 lightNormal[3];
	XMFLOAT4 lightColor[3];
	XMFLOAT4 lightRadius;
};

class Graphics
{
	// variables here
	GW::GRAPHICS::GDirectX11Surface* mySurface = nullptr;
	// Gettting these handles from GDirectX11Surface will increase their internal refrence counts, be sure to "Release()" them when done!
	ID3D11Device* myDevice = nullptr;
	IDXGISwapChain* mySwapChain = nullptr;
	ID3D11DeviceContext* myContext = nullptr;

	// D3D11 resource variables
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	ID3D11InputLayout* vertexLayout = nullptr;
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	// textures
	ID3D11ShaderResourceView* shaderRV = nullptr;
	ID3D11SamplerState* sampler = nullptr;

	std::vector<Mesh*> meshes;
	ConstantBuffer cb;
	bool shrink = false;
	float radius = 5.0f;
	float PS_Unique = 0.0f;
	Camera camera;
	Light dLight;
	Light pLight;
	Light sLight;
	Mesh hub;
	// model class
	// vertex, index buffer
	// shader, texture
	XTime time;
	double elapsedTime;
	bool wave;

	FBXBinaryFileIO fileIO;
public:
	// Init constructor
	Graphics(GW::SYSTEM::GWindow* attatchPoint);
	// Shutdown
	~Graphics();
	// Draw
	void Render();
	// Init function
	HRESULT InitializeDevice();
	void CleanDevice();
	HRESULT CreateBuffer(ID3D11Device* device,ID3D11Buffer** buffer, UINT bindFlag, UINT byteWidth, const void* pSysMem);
	void KeyboardHandle(float delta);
};
