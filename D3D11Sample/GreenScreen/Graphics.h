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

// include my own files and other helpful files
#include "Camera.h"
#include "Model.h"
#include "Light.h"
// Lari's time class
#include "XTime.h"

struct Matrix_ConstantBuffer
{
	XMFLOAT4X4 world[5];
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	// camera position
	XMFLOAT4 cameraPosition;
};

struct Light_ConstantBuffer
{
	// [0] = directional lighting, [1] = point lighting, [2] = spot lighting
	XMFLOAT4 lightPos[3];
	XMFLOAT4 lightNormal[3];
	XMFLOAT4 lightColor[3];
	// x = radius, y = rotation, z = time, w = wavy toggle
	XMFLOAT4 lightRadius;
	// x = inner cone ratio, y = outer cone ratio, z = black and white toggle
	XMFLOAT4 coneRatio;
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
	ID3D11BlendState* blendState = nullptr;
	ID3D11Buffer* matrix_id3d11buffer;
	ID3D11Buffer* light_id3d11buffer;
	ID3D11InputLayout* inputLayout = nullptr;
	// constant buffers
	Matrix_ConstantBuffer matrix_cb;
	Light_ConstantBuffer light_cb;

	bool lockOn = false;
	XMVECTOR cameraTarget;

	float radius = 10000.0f;
	Camera camera;
	Camera camera1;
	Light dLight;
	Light pLight;
	Light sLight;

	XTime time;
	double elapsedTime;
	bool wave = false;
	bool normalMapping = false;
	bool multiTexture = false;

	Model skyBox;
	Model ywing;
	Model spaceStation;
	Model venatorStarDestroyer;
	Model acclamatorStarDestroyer;
	Model sun;
	Model earth;
	Model moon;
	Model lucrehulk;
	Model rock;
	Model astroid;
	Model planet01;
	Model plane;

	std::vector<XMVECTOR> modelPositions;
	unsigned int modelID = 0;

	unsigned int wndWidth = 0;
	unsigned int wndHeight = 0;
public:
	// Init constructor
	Graphics(GW::SYSTEM::GWindow* attatchPoint);
	// Shutdown
	~Graphics();
	// Draw
	void Render(GW::SYSTEM::GWindow* attatchPoint);
	// Init function
	HRESULT InitializeDevice();
	void CleanDevice();
	HRESULT CreateBuffer(ID3D11Device* device,ID3D11Buffer** buffer, UINT bindFlag, UINT byteWidth, const void* pSysMem);
	void KeyboardHandle(float delta);
	void ConstantBufferSetUp(const XMMATRIX& worldMatrix, Camera& camera);
};
