#include "Graphics.h"

// shader includes
#include "VertexShader.csh"
#include "PixelShader.csh"

#define RAND_COLOR XMFLOAT4(rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX),1.0f)

Graphics::Graphics(GW::SYSTEM::GWindow* attatchPoint)
{
	if (attatchPoint) // valid window?
	{
		// Create surface, will auto attatch to GWindow
		if (G_SUCCESS(GW::GRAPHICS::CreateGDirectX11Surface(attatchPoint, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT, &mySurface)))
		{
			// Grab handles to all DX11 base interfaces
			mySurface->GetDevice((void**)& myDevice);
			mySurface->GetSwapchain((void**)& mySwapChain);
			mySurface->GetContext((void**)& myContext);

			// Initalize device
			HRESULT hr = InitializeDevice();
			camera.SetPosition(0.0f, 5.0f, -20.0f);
			camera.SetProjection(90.0f, 1.0f, 0.1f, 500.0f);
			// directional light init
			dLight.SetPosition(0.0f, 0.0f, 0.0f);
			dLight.SetNormal(-0.577f, -0.577f, 0.577f);
			dLight.SetColor(0.75f, 0.75f, 0.94f, 1.0f);
			// point light init
			pLight.SetPosition(0.0f, 5.0f, 0.0f);
			pLight.SetNormal(0.0f, 0.0f, 0.0f);
			pLight.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
			// spot light init
			sLight.SetPosition(0.0f, 50.0f, 0.0f);
			sLight.SetNormal(-0.577f, -0.577f, 0.577f);
			sLight.SetColor(0.541f, 0.168f, 0.886f, 1.0f);

			time.Restart();
			elapsedTime = 0.0;
			wave = false;
			blackWhite = false;
		}
	}
}

Graphics::~Graphics()
{
	// Release DX Objects aquired from the surface
	myDevice->Release();
	mySwapChain->Release();
	myContext->Release();

	// TODO: "Release()" more stuff here!
	CleanDevice();

	if (mySurface) // Free Gateware Interface
	{
		mySurface->DecrementCount(); // reduce internal count (will auto delete on Zero)
		mySurface = nullptr; // the safest way to fly
	}
}

void Graphics::Render()
{
	if (mySurface) // valid?
	{
		// this could be changed during resolution edits, get it every frame
		ID3D11RenderTargetView* myRenderTargetView = nullptr;
		ID3D11DepthStencilView* myDepthStencilView = nullptr;
		if (G_SUCCESS(mySurface->GetRenderTarget((void**)& myRenderTargetView)))
		{
			// Grab the Z Buffer if one was requested
			if (G_SUCCESS(mySurface->GetDepthStencilView((void**)& myDepthStencilView)))
			{
				myContext->ClearDepthStencilView(myDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); // clear it to Z exponential Far.
				myDepthStencilView->Release();
			}

			// Set active target for drawing, all array based D3D11 functions should use a syntax similar to below
			ID3D11RenderTargetView* const targets[] = { myRenderTargetView };
			myContext->OMSetRenderTargets(1, targets, myDepthStencilView);
			// Clear screen
			const float bg_Color[] = { 0.4f, 0.4f, 0.4f, 1.0f };
			myContext->ClearRenderTargetView(myRenderTargetView, bg_Color);

			// keyboard inputs
			time.Signal();
			elapsedTime += time.Delta();
			KeyboardHandle((float)time.SmoothDelta());

			// setup pipeline
			UINT strides = sizeof(Vertex);
			UINT offsets = 0;
			myContext->IASetInputLayout(inputLayout);
			myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// draw
			XMMATRIX temp = XMMatrixMultiply(XMMatrixRotationY(-160.0f * 0.01f), XMMatrixTranslation(50.0f, 0.0f, 0.0f));
			corvette.SetWorldMatrix(temp);
			ConstantBufferSetUp(corvette.GetWorldMatrix());
			corvette.Draw(myContext);

			//temp = XMMatrixMultiply(XMMatrixTranslation(-20.0f, -20.0f, 0.0f), XMMatrixRotationAxis(XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f), 90.0f));
			temp = XMMatrixTranslation(-50.0f, 0.0f, 0.0f);
			arc170.SetWorldMatrix(temp);
			ConstantBufferSetUp(arc170.GetWorldMatrix());
			arc170.Draw(myContext);

			temp = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
			plane.SetWorldMatrix(temp);
			ConstantBufferSetUp(plane.GetWorldMatrix());
			plane.Draw(myContext);

			//temp = XMMatrixRotationY(elapsedTime);
			//spaceStation.SetWorldMatrix(temp);
			//ConstantBufferSetUp(spaceStation.GetWorldMatrix());
			//spaceStation.Draw(myContext);

			temp = XMMatrixMultiply(XMMatrixRotationX(30.0f /** venatorStarDestroyer.GetMeshes()[0]->scale*/), XMMatrixTranslation(0.0f, 1.9f, 0.0f));
			venatorStarDestroyer.SetWorldMatrix(temp);
			ConstantBufferSetUp(venatorStarDestroyer.GetWorldMatrix());
			venatorStarDestroyer.Draw(myContext);

			// Present Backbuffer using Swapchain object
			// Framerate is currently unlocked, we suggest "MSI Afterburner" to track your current FPS and memory usage.
			mySwapChain->Present(1, 0); // set first argument to 1 to enable vertical refresh sync with display
			// Free any temp DX handles aquired this frame
			myRenderTargetView->Release();
		}
	}
}

HRESULT Graphics::InitializeDevice()
{
	HRESULT hr = S_OK;
	// define input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	// create input layout
	hr = myDevice->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShader, sizeof(VertexShader), &inputLayout);

	// load model data
	corvette.AddMesh(new Mesh());
	corvette.GetMeshes()[0]->LoadVertices("../../My assets/Corvette_Data", "Corvette", 0.01f, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	hr = corvette.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = corvette.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = corvette.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = corvette.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = corvette.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Corvette/Corvette_Diffuse_Tex.dds");
	hr = corvette.GetMeshes()[0]->CreateSamplerState(myDevice);

	arc170.AddMesh(new Mesh());
	arc170.GetMeshes()[0]->LoadVertices("../../My assets/ARC170_Data", "ARC170", 0.01f, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	hr = arc170.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = arc170.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = arc170.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = arc170.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = arc170.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/ARC170_Tex.dds");
	hr = arc170.GetMeshes()[0]->CreateSamplerState(myDevice);

	plane.AddMesh(new Mesh());
	plane.GetMeshes()[0]->LoadVertices("../../My assets/Plane_Data", "Plane", 5.0f, XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f));
	hr = plane.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = plane.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = plane.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = plane.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = plane.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Box_Circuit.dds");
	hr = plane.GetMeshes()[0]->CreateSamplerState(myDevice);

	spaceStation.AddMesh(new Mesh());
	spaceStation.GetMeshes()[0]->LoadVertices("../../My assets/SpaceStation_Data", "SpaceStation", 0.5f, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	hr = spaceStation.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = spaceStation.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = spaceStation.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = spaceStation.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = spaceStation.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/SpaceStation01/RT_2D_Station2_Spec.dds");
	hr = spaceStation.GetMeshes()[0]->CreateSamplerState(myDevice);

	venatorStarDestroyer.AddMesh(new Mesh());
	venatorStarDestroyer.GetMeshes()[0]->LoadVertices("../../My assets/Venator_Data", "VenatorStarDestroyer", 0.001f, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = venatorStarDestroyer.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/VenatorStarDestroyer/ReV_venator.dds");
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateSamplerState(myDevice);


	// describe constant variables and create constant buffer
	hr = CreateBuffer(myDevice, &matrix_id3d11buffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(Matrix_ConstantBuffer), nullptr);
	hr = CreateBuffer(myDevice, &light_id3d11buffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(Light_ConstantBuffer), nullptr);
	return hr;
}

void Graphics::CleanDevice()
{
	inputLayout->Release();
	matrix_id3d11buffer->Release();
	light_id3d11buffer->Release();
}

HRESULT Graphics::CreateBuffer(ID3D11Device* device, ID3D11Buffer** buffer, UINT bindFlag, UINT byteWidth, const void* pSysMem)
{
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subRsrcData;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&subRsrcData, sizeof(D3D11_SUBRESOURCE_DATA));
	bufferDesc.BindFlags = bindFlag;
	bufferDesc.ByteWidth = byteWidth;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	subRsrcData.pSysMem = pSysMem;
	switch (bindFlag)
	{
	case D3D11_BIND_VERTEX_BUFFER:
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		return device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
	case D3D11_BIND_INDEX_BUFFER:
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		return device->CreateBuffer(&bufferDesc, &subRsrcData, buffer);
	case D3D11_BIND_CONSTANT_BUFFER:
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		return device->CreateBuffer(&bufferDesc, nullptr, buffer);
	case D3D11_BIND_SHADER_RESOURCE:
		return E_NOTIMPL;
	case D3D11_BIND_STREAM_OUTPUT:
		return E_NOTIMPL;
	case D3D11_BIND_RENDER_TARGET:
		return E_NOTIMPL;
	case D3D11_BIND_DEPTH_STENCIL:
		return E_NOTIMPL;
	case D3D11_BIND_UNORDERED_ACCESS:
		return E_NOTIMPL;
	case D3D11_BIND_DECODER:
		return E_NOTIMPL;
	case D3D11_BIND_VIDEO_ENCODER:
		return E_NOTIMPL;
	default:
		return S_FALSE;
	}
}

void Graphics::KeyboardHandle(float delta)
{
	float offset = 3.5f;
	offset = 6.0f;
	// move forward
	if (GetAsyncKeyState('W')) { camera.MoveZ(offset * delta); }
	// move left
	if (GetAsyncKeyState('A')) { camera.MoveX(-offset * delta); }
	// move backwards
	if (GetAsyncKeyState('S')) { camera.MoveZ(-offset * delta); }
	// move right
	if (GetAsyncKeyState('D')) { camera.MoveX(offset * delta); }
	// move up
	if (GetAsyncKeyState('Q')) { camera.MoveY(offset * delta); }
	// move down
	if (GetAsyncKeyState('E')) { camera.MoveY(-offset * delta); }
	// yaw left
	if (GetAsyncKeyState('J')) { camera.Yaw(-offset * delta); }
	// yaw right
	if (GetAsyncKeyState('L')) { camera.Yaw(offset * delta); }
	// pitch up
	if (GetAsyncKeyState('I')) { camera.Pitch(-offset * delta); }
	// pitch down
	if (GetAsyncKeyState('K')) { camera.Pitch(offset * delta); }
	// roll left
	if (GetAsyncKeyState('U')) { camera.Roll(offset * delta); }
	// roll right
	if (GetAsyncKeyState('O')) { camera.Roll(-offset * delta); }
	// increase fov
	if (GetAsyncKeyState('1')) { camera.IncreaseFOV((offset + 25.0f) * delta); }
	// decrease fov
	if (GetAsyncKeyState('2')) { camera.DecreaseFOV((offset + 25.0f) * delta); }
	// increase near plane
	if (GetAsyncKeyState('3')) { camera.IncreaseNearPlane((offset + 10.0f) * delta); }
	// decrease near plane
	if (GetAsyncKeyState('4')) { camera.DecreaseNearPlane((offset + 10.0f) * delta); }
	// increase far plane
	if (GetAsyncKeyState('5')) { camera.IncreaseFarPlane((offset + 10.0f) * delta); }
	// decrease far plane
	if (GetAsyncKeyState('6')) { camera.DecreaseFarPlane((offset + 10.0f) * delta); }
	// waviness
	if (GetAsyncKeyState('F') & 0x1) { wave = !wave; }
	// black and white
	if (GetAsyncKeyState('G') & 0x1) { blackWhite = !blackWhite; }
	// reset camera
	if (GetAsyncKeyState('R') & 0x1) { camera.SetPosition(0.0f, 0.0f, -20.0f); camera.SetProjection(90.0f, 1.0f, 0.1f, 500.0f); }
	camera.UpdateView();
}

void Graphics::ConstantBufferSetUp(const XMMATRIX& worldMatrix)
{
	// lighting
	static float rot = 0.0f; rot += 0.1f;
	// directional lighting
	dLight.SetWorldMatrix(XMMatrixRotationY(0.01f));
	dLight.UpdatePosition();
	XMStoreFloat4(&light_cb.lightPos[0], dLight.GetPositionVector());
	XMStoreFloat4(&light_cb.lightNormal[0], dLight.GetNormalVectorNormalized());
	XMStoreFloat4(&light_cb.lightColor[0], dLight.GetColor());
	// point light
	pLight.SetWorldMatrix(XMMatrixRotationY(0.01f));
	pLight.UpdatePosition();
	XMStoreFloat4(&light_cb.lightPos[1], pLight.GetPositionVector());
	XMStoreFloat4(&light_cb.lightNormal[1], pLight.GetNormalVectorNormalized());
	XMStoreFloat4(&light_cb.lightColor[1], pLight.GetColor());
	// spot light
	sLight.SetPosition(radius * 5.0f, 15.0f, radius * 5.0f);
	sLight.SetWorldMatrix(XMMatrixRotationY(0.01f));
	sLight.UpdatePosition();
	XMVECTOR coneRatio = { 0.98f, 0.88f, (float)blackWhite, 0.0f };
	XMStoreFloat4(&light_cb.coneRatio, coneRatio);
	XMStoreFloat4(&light_cb.lightPos[2], sLight.GetPositionVector());
	XMStoreFloat4(&light_cb.lightNormal[2], sLight.GetNormalVectorNormalized());
	XMStoreFloat4(&light_cb.lightColor[2], sLight.GetColor());
	// point light radius
	if (radius > 10.0f)
		shrink = true;
	else if (radius <= 1.0f)
		shrink = false;
	if (shrink)
		radius -= 0.03f;
	else
		radius += 0.03f;
	XMVECTOR lightRad = { radius, rot, (float)elapsedTime, (float)wave };
	XMStoreFloat4(&light_cb.lightRadius, lightRad);

	// world
	XMStoreFloat4x4(&matrix_cb.world, worldMatrix);
	// view
	XMStoreFloat4x4(&matrix_cb.view, camera.GetViewMatrix());
	// projection
	float ar = 0.0f;
	mySurface->GetAspectRatio(ar);
	camera.SetAspectRatio(ar);
	XMStoreFloat4x4(&matrix_cb.projection, camera.GetProjectionMatrix());
	// camera position
	XMStoreFloat4(&matrix_cb.cameraPosition, camera.GetWorldViewMatrix().r[3]);

	// map constant buffer
	ID3D11Buffer* constantBuffer[] = { matrix_id3d11buffer, light_id3d11buffer };
	D3D11_MAPPED_SUBRESOURCE gpuBuffer;
	myContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	memcpy(gpuBuffer.pData, &matrix_cb, sizeof(Matrix_ConstantBuffer));
	myContext->Unmap(constantBuffer[0], 0);
	myContext->Map(constantBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	memcpy(gpuBuffer.pData, &light_cb, sizeof(Light_ConstantBuffer));
	myContext->Unmap(constantBuffer[1], 0);
	// set constant buffer in memory
	myContext->VSSetConstantBuffers(0, 2, constantBuffer);
	myContext->PSSetConstantBuffers(0, 2, constantBuffer);
}
