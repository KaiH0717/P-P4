#include "Graphics.h"

// shader includes
#include "VertexShader.csh"
#include "PixelShader.csh"
#include "SkyboxVertexShader.csh"
#include "SkyboxPixelShader.csh"
#include "HyperSpeedVertexShader.csh"
#include "HyperSpeedPixelShader.csh"
#include "NormalMappingVertexShader.csh"
#include "NormalMappingPixelShader.csh"
#include "AcclamatorVertexShader.csh"
#include "AcclamatorPixelShader.csh"

#define RAND_XMFLOAT4 XMFLOAT4(rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX),1.0f)
#define USE_SECOND_VIEWPORT 0

Graphics::Graphics(GW::SYSTEM::GWindow * attatchPoint)
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
			camera.SetPosition(0.0f, 20.0f, -70.0f);
			camera.SetProjection(90.0f, 1.0f, 0.1f, 500.0f);

			camera1.SetPosition(0.0f, 50.0f, -50.0f);
			camera1.SetProjection(90.0f, 1.0f, 0.1f, 500.0f);
			// directional light init
			dLight.SetPosition(0.0f, 0.0f, 0.0f);
			dLight.SetNormal(-0.577f, -0.577f, 0.577f);
			dLight.SetColor(0.45f, 0.45f, 0.64f, 1.0f);
			// point light init
			pLight.SetPosition(0.0f, 0.0f, 0.0f);
			pLight.SetNormal(0.0f, 0.0f, 0.0f);
			pLight.SetColor(1.0f, 0.858f, 0.0f, 1.0f);
			// spot light init
			sLight.SetPosition(0.0f, 0.0f, 0.0f);
			sLight.SetNormal(-0.577f, -0.577f, 0.577f);
			sLight.SetColor(0.541f, 0.168f, 0.886f, 1.0f);

			time.Restart();
			elapsedTime = 0.0;
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

void Graphics::Render(GW::SYSTEM::GWindow* attatchPoint)
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

			// viewport stuff
			D3D11_VIEWPORT viewPort[2];
			attatchPoint->GetClientWidth(wndWidth);
			attatchPoint->GetClientHeight(wndHeight);
			viewPort[0].Width = (FLOAT)wndWidth;
			viewPort[0].Height = (FLOAT)wndHeight;
			viewPort[0].MinDepth = 0.0f;
			viewPort[0].MaxDepth = 1.0f;
			viewPort[0].TopLeftX = 0;
			viewPort[0].TopLeftY = 0;
			viewPort[1].Width = 300.0f;
			viewPort[1].Height = 300.0f;
			viewPort[1].MinDepth = 0.0f;
			viewPort[1].MaxDepth = 0.9f;
			viewPort[1].TopLeftX = 0;
			viewPort[1].TopLeftY = 0;

			// keyboard inputs
			time.Signal();
			elapsedTime += time.Delta();
			KeyboardHandle((float)time.SmoothDelta());

			// draw
			XMMATRIX temp = XMMatrixIdentity();
			skyBox.SetWorldMatrix(XMMatrixTranslation(camera.GetWorldPosition().x, camera.GetWorldPosition().y, camera.GetWorldPosition().z));
			ConstantBufferSetUp(skyBox.GetWorldMatrix(), camera);
			skyBox.DrawIndexed(myContext, &viewPort[0]);

			// Grab the Z Buffer if one was requested
			if (G_SUCCESS(mySurface->GetDepthStencilView((void**)& myDepthStencilView)))
			{
				myContext->ClearDepthStencilView(myDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); // clear it to Z exponential Far.
				myDepthStencilView->Release();
			}

			temp = XMMatrixMultiply(XMMatrixRotationY(30.0f), XMMatrixMultiply(XMMatrixRotationX((float)elapsedTime * 0.5f), XMMatrixTranslation(50.0f, 0.0f, 0.0f)));
			ywing.SetWorldMatrix(temp);
			XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationY(30.0f), XMMatrixMultiply(XMMatrixRotationX((float)elapsedTime * 0.5f), XMMatrixTranslation(30.0f, 0.0f, 10.0f))));
			XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationY(30.0f), XMMatrixMultiply(XMMatrixRotationX((float)elapsedTime * 0.5f), XMMatrixTranslation(30.0f, 0.0f, -10.0f))));
			XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationY(30.0f), XMMatrixMultiply(XMMatrixRotationX((float)elapsedTime * 0.5f), XMMatrixTranslation(50.0f, 0.0f, 30.0f))));
			XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationY(30.0f), XMMatrixMultiply(XMMatrixRotationX((float)elapsedTime * 0.5f), XMMatrixTranslation(50.0f, 0.0f, -30.0f))));
			ConstantBufferSetUp(ywing.GetWorldMatrix(), camera);
			ywing.DrawIndexInstanced(myContext, &viewPort[0], 5);
			modelPositions[0] = ywing.GetWorldMatrix().r[3];

			temp = XMMatrixMultiply(XMMatrixRotationY((float)elapsedTime * 0.5f), XMMatrixTranslation(-200.0f, 30.0f, 0.0f));
			spaceStation.SetWorldMatrix(temp);
			ConstantBufferSetUp(spaceStation.GetWorldMatrix(), camera);
			spaceStation.DrawIndexed(myContext, &viewPort[0]);
			modelPositions[1] = spaceStation.GetWorldMatrix().r[3];

			temp = XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(70.0f, 30.0f, 0.0f));
			venatorStarDestroyer.SetWorldMatrix(temp);
			XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 15.0f, 40.0f)));
			XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, -15.0f, -40.0f)));
			XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-80.0f, 15.0f, 80.0f)));
			XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-80.0f, -15.0f, -80.0f)));
			ConstantBufferSetUp(venatorStarDestroyer.GetWorldMatrix(), camera);
			venatorStarDestroyer.DrawIndexInstanced(myContext, &viewPort[0], 5);
			modelPositions[2] = venatorStarDestroyer.GetWorldMatrix().r[3];

			temp = XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(0.0f, 30.0f, 0.0f));
			acclamatorStarDestroyer.SetWorldMatrix(temp);
			XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 20.0f, 30.0f)));
			XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-50.0f, 25.0f, -50.0f)));
			XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 30.0f, 60.0f)));
			XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 35.0f, -25.0f)));
			ConstantBufferSetUp(acclamatorStarDestroyer.GetWorldMatrix(), camera);
			acclamatorStarDestroyer.DrawIndexInstanced(myContext, &viewPort[0], 5);
			modelPositions[3] = acclamatorStarDestroyer.GetWorldMatrix().r[3];

			temp = XMMatrixRotationY((float)elapsedTime * 0.5f);
			sun.SetWorldMatrix(temp);
			ConstantBufferSetUp(sun.GetWorldMatrix(), camera);
			sun.DrawIndexed(myContext, &viewPort[0]);
			modelPositions[4] = sun.GetWorldMatrix().r[3];

			temp = XMMatrixMultiply(XMMatrixRotationY((float)elapsedTime * 0.5f), XMMatrixTranslation(100.0f, 0.0f, 0.0f));
			temp = XMMatrixMultiply(temp, XMMatrixRotationY((float)elapsedTime * 0.5f));
			earth.SetWorldMatrix(temp);
			ConstantBufferSetUp(earth.GetWorldMatrix(), camera);
			earth.DrawIndexed(myContext, &viewPort[0]);
			modelPositions[5] = earth.GetWorldMatrix().r[3];

			temp = XMMatrixTranslationFromVector(earth.GetWorldMatrix().r[3]);
			temp = XMMatrixMultiply(XMMatrixRotationY((float)elapsedTime * 0.5f), temp);
			temp = XMMatrixMultiply(temp, XMMatrixTranslation(20.0f, 0.0f, 0.0f));
			moon.SetWorldMatrix(temp);
			ConstantBufferSetUp(moon.GetWorldMatrix(), camera);
			moon.DrawIndexed(myContext, &viewPort[0]);
			modelPositions[6] = moon.GetWorldMatrix().r[3];

			temp = XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f));
			temp = XMMatrixMultiply(temp, XMMatrixTranslation(200.0f, 0.0f, 0.0f));
			lucrehulk.SetWorldMatrix(temp);
			XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(150.0f, 50.0f, 150.0f)));
			XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(250.0f, -50.0f, -150.0f)));
			XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(200.0f, 100.0f, 100.0f)));
			XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(200.0f, -100.0f, 100.0f)));
			ConstantBufferSetUp(lucrehulk.GetWorldMatrix(), camera);
			lucrehulk.DrawIndexInstanced(myContext, &viewPort[0], 5);
			modelPositions[7] = lucrehulk.GetWorldMatrix().r[3];

			temp = XMMatrixTranslation(0.0f, -200.0f, 0.0f);
			rock.SetWorldMatrix(temp);
			ConstantBufferSetUp(rock.GetWorldMatrix(), camera);
			rock.DrawIndexed(myContext, &viewPort[0]);
			modelPositions[9] = rock.GetWorldMatrix().r[3];

			temp = XMMatrixTranslation(0.0f, -190.0f, 0.0f);
			astroid.SetWorldMatrix(temp);
			ConstantBufferSetUp(astroid.GetWorldMatrix(), camera);
			astroid.DrawIndexed(myContext, &viewPort[0]);
			modelPositions[10] = astroid.GetWorldMatrix().r[3];

			temp = XMMatrixMultiply(XMMatrixRotationY((float)elapsedTime * 0.5f), XMMatrixTranslation(50.0f, 0.0f, 0.0f));
			temp = XMMatrixMultiply(temp, XMMatrixRotationY(-(float)elapsedTime * 0.5f));
			planet01.SetWorldMatrix(temp);
			ConstantBufferSetUp(planet01.GetWorldMatrix(), camera);
			planet01.DrawIndexed(myContext, &viewPort[0]);
			modelPositions[11] = planet01.GetWorldMatrix().r[3];

#if USE_SECOND_VIEWPORT
			// second view port draw calls
			skyBox.SetWorldMatrix(XMMatrixTranslation(camera1.GetWorldPosition().x, camera1.GetWorldPosition().y, camera1.GetWorldPosition().z));
			ConstantBufferSetUp(skyBox.GetWorldMatrix(), camera1);
			skyBox.Draw(myContext, &viewPort[1]);

			// Grab the Z Buffer if one was requested
			if (G_SUCCESS(mySurface->GetDepthStencilView((void**)& myDepthStencilView)))
			{
				myContext->ClearDepthStencilView(myDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); // clear it to Z exponential Far.
				myDepthStencilView->Release();
			}

			temp = XMMatrixMultiply(XMMatrixRotationY(-160.0f * 0.01f), XMMatrixTranslation(50.0f, 0.0f, 0.0f));
			corvette.SetWorldMatrix(temp);
			ConstantBufferSetUp(corvette.GetWorldMatrix(), camera1);
			corvette.Draw(myContext, &viewPort[1]);

			temp = XMMatrixTranslation(-50.0f, 0.0f, 0.0f);
			arc170.SetWorldMatrix(temp);
			ConstantBufferSetUp(arc170.GetWorldMatrix(), camera1);
			arc170.Draw(myContext, &viewPort[1]);

			temp = XMMatrixMultiply(XMMatrixRotationY((float)elapsedTime), XMMatrixTranslation(-200.0f, 30.0f, 0.0f));
			spaceStation.SetWorldMatrix(temp);
			ConstantBufferSetUp(spaceStation.GetWorldMatrix(), camera1);
			spaceStation.Draw(myContext, &viewPort[1]);

			temp = XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(0.0f, 0.0f, 0.0f));
			venatorStarDestroyer.SetWorldMatrix(temp);
			XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 0.0f, 40.0f)));
			XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 0.0f, -40.0f)));
			XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-80.0f, 0.0f, 80.0f)));
			XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-80.0f, 0.0f, -80.0f)));
			ConstantBufferSetUp(venatorStarDestroyer.GetWorldMatrix(), camera1);
			venatorStarDestroyer.DrawInstanced(myContext, &viewPort[1], 5);

			temp = XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(0.0f, 30.0f, 0.0f));
			acclamatorStarDestroyer.SetWorldMatrix(temp);
			XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 20.0f, 30.0f)));
			XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-50.0f, 25.0f, -50.0f)));
			XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 30.0f, 60.0f)));
			XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 35.0f, -25.0f)));
			ConstantBufferSetUp(acclamatorStarDestroyer.GetWorldMatrix(), camera1);
			acclamatorStarDestroyer.DrawInstanced(myContext, &viewPort[1], 5);
#endif
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
		{"TANGENT" , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT	  , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	// create input layout
	hr = myDevice->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShader, sizeof(VertexShader), &inputLayout);
	// setup pipeline
	myContext->IASetInputLayout(inputLayout);
	myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	myDevice->CreateBlendState(&blendDesc, &blendState);
	myContext->OMSetBlendState(blendState, 0, 0xffffffff);
	// load model data
	skyBox.AddMesh(new Mesh("SkyBox", -1.0f), "../../My assets/SkyBox_Data");
	hr = skyBox.GetMeshes()[0]->CreateVertexShader(myDevice, SkyboxVertexShader, sizeof(SkyboxVertexShader));
	hr = skyBox.GetMeshes()[0]->CreatePixelShader(myDevice, SkyboxPixelShader, sizeof(SkyboxPixelShader));
	hr = skyBox.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = skyBox.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = skyBox.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/SPAAAAAAAAAAAAAAAAAAAAACE.dds");
	hr = skyBox.GetMeshes()[0]->CreateSamplerState(myDevice);

	ywing.AddMesh(new Mesh("Y-Wing", 0.025f), "../../My assets/YWing_Data");
	hr = ywing.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = ywing.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = ywing.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = ywing.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = ywing.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Rep_BTLB_Ywing_Gold.dds");
	hr = ywing.GetMeshes()[0]->CreateSamplerState(myDevice);

	spaceStation.AddMesh(new Mesh("SpaceStation", 1.5f), "../../My assets/SpaceStation_Data");
	hr = spaceStation.GetMeshes()[0]->CreateVertexShader(myDevice, NormalMappingVertexShader, sizeof(NormalMappingVertexShader));
	hr = spaceStation.GetMeshes()[0]->CreatePixelShader(myDevice, NormalMappingPixelShader, sizeof(NormalMappingPixelShader));
	hr = spaceStation.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = spaceStation.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = spaceStation.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/SpaceStation01/RT_2D_Station2_Diffuse.dds");
	hr = spaceStation.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/SpaceStation01/RT_2D_Station2_Normal.dds");
	hr = spaceStation.GetMeshes()[0]->CreateSamplerState(myDevice);

	venatorStarDestroyer.AddMesh(new Mesh("VenatorStarDestroyer", 0.0025f), "../../My assets/Venator_Data");
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateVertexShader(myDevice, NormalMappingVertexShader, sizeof(NormalMappingVertexShader));
	hr = venatorStarDestroyer.GetMeshes()[0]->CreatePixelShader(myDevice, NormalMappingPixelShader, sizeof(NormalMappingPixelShader));
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/VenatorStarDestroyer/ReV_venator.dds");
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/VenatorStarDestroyer/StaDestroyer.dds");
	hr = venatorStarDestroyer.GetMeshes()[0]->CreateSamplerState(myDevice);

	acclamatorStarDestroyer.AddMesh(new Mesh("AcclamatorStarDestroyer", 0.0025f), "../../My assets/Acclamator_Data");
	hr = acclamatorStarDestroyer.GetMeshes()[0]->CreateVertexShader(myDevice, NormalMappingVertexShader, sizeof(NormalMappingVertexShader));
	hr = acclamatorStarDestroyer.GetMeshes()[0]->CreatePixelShader(myDevice, NormalMappingPixelShader, sizeof(NormalMappingPixelShader));
	hr = acclamatorStarDestroyer.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = acclamatorStarDestroyer.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = acclamatorStarDestroyer.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/AcclamatorStarDestroyer/ReV_Acclamator1.dds");
	hr = acclamatorStarDestroyer.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/AcclamatorStarDestroyer/NormalMap.dds");
	hr = acclamatorStarDestroyer.GetMeshes()[0]->CreateSamplerState(myDevice);

	sun.AddMesh(new Mesh("Sun", 2.5f), "../../My assets/Sun_Data");
	hr = sun.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = sun.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = sun.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = sun.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = sun.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/sun.dds");
	hr = sun.GetMeshes()[0]->CreateSamplerState(myDevice);

	earth.AddMesh(new Mesh("Earth"), "../../My assets/Earth_Data");
	hr = earth.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = earth.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = earth.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = earth.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = earth.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Earth/Planet_Diffuse.dds");
	hr = earth.GetMeshes()[0]->CreateSamplerState(myDevice);

	moon.AddMesh(new Mesh("Moon", 0.5f), "../../My assets/Moon_Data");
	hr = moon.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = moon.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = moon.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = moon.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = moon.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/moon_Diffuse.dds");
	hr = moon.GetMeshes()[0]->CreateSamplerState(myDevice);

	lucrehulk.AddMesh(new Mesh("Lucrehulk", 0.15f), "../../My assets/Lucrehulk_Data");
	hr = lucrehulk.GetMeshes()[0]->CreateVertexShader(myDevice, VertexShader, sizeof(VertexShader));
	hr = lucrehulk.GetMeshes()[0]->CreatePixelShader(myDevice, PixelShader, sizeof(PixelShader));
	hr = lucrehulk.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = lucrehulk.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = lucrehulk.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Lucrehulk/Sev_lucrehulk.dds");
	hr = lucrehulk.GetMeshes()[0]->CreateSamplerState(myDevice);

	rock.AddMesh(new Mesh("Rock", 100.0f), "../../My assets/Plane_Data");
	rock.GetMeshes()[0]->vertices[0].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	rock.GetMeshes()[0]->vertices[1].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	rock.GetMeshes()[0]->vertices[2].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	rock.GetMeshes()[0]->vertices[3].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	hr = rock.GetMeshes()[0]->CreateVertexShader(myDevice, NormalMappingVertexShader, sizeof(NormalMappingVertexShader));
	hr = rock.GetMeshes()[0]->CreatePixelShader(myDevice, NormalMappingPixelShader, sizeof(NormalMappingPixelShader));
	hr = rock.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = rock.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = rock.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Rock/Rock_Diffuse.dds");
	hr = rock.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Rock/Rock_Normal.dds");
	hr = rock.GetMeshes()[0]->CreateSamplerState(myDevice);

	astroid.AddMesh(new Mesh("Astroid", 1.5f), "../../My assets/Astroid_Data");
	hr = astroid.GetMeshes()[0]->CreateVertexShader(myDevice, NormalMappingVertexShader, sizeof(NormalMappingVertexShader));
	hr = astroid.GetMeshes()[0]->CreatePixelShader(myDevice, NormalMappingPixelShader, sizeof(NormalMappingPixelShader));
	hr = astroid.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = astroid.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = astroid.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Rock/RT_2D_Rock_Asteroid_Diffuse.dds");
	hr = astroid.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Rock/RT_2D_Rock_Asteriod_N.dds");
	hr = astroid.GetMeshes()[0]->CreateSamplerState(myDevice);

	planet01.AddMesh(new Mesh("Planet01", 0.01f), "../../My assets/Planet01_Data");
	hr = planet01.GetMeshes()[0]->CreateVertexShader(myDevice, NormalMappingVertexShader, sizeof(NormalMappingVertexShader));
	hr = planet01.GetMeshes()[0]->CreatePixelShader(myDevice, NormalMappingPixelShader, sizeof(NormalMappingPixelShader));
	hr = planet01.GetMeshes()[0]->CreateVertexBuffer(myDevice);
	hr = planet01.GetMeshes()[0]->CreateIndexBuffer(myDevice);
	hr = planet01.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Planet01/RT_2D_Planet_Diffuse.dds");
	hr = planet01.GetMeshes()[0]->CreateShaderResourceView(myDevice, L"../../My assets/Textures/Planet01/RT_2D_Planet_Normal.dds");
	hr = planet01.GetMeshes()[0]->CreateSamplerState(myDevice);

	modelPositions.push_back(ywing.GetWorldMatrix().r[3]);
	modelPositions.push_back(spaceStation.GetWorldMatrix().r[3]);
	modelPositions.push_back(venatorStarDestroyer.GetWorldMatrix().r[3]);
	modelPositions.push_back(acclamatorStarDestroyer.GetWorldMatrix().r[3]);
	modelPositions.push_back(sun.GetWorldMatrix().r[3]);
	modelPositions.push_back(earth.GetWorldMatrix().r[3]);
	modelPositions.push_back(moon.GetWorldMatrix().r[3]);
	modelPositions.push_back(lucrehulk.GetWorldMatrix().r[3]);
	modelPositions.push_back(rock.GetWorldMatrix().r[3]);
	modelPositions.push_back(astroid.GetWorldMatrix().r[3]);
	modelPositions.push_back(planet01.GetWorldMatrix().r[3]);

	XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 15.0f, 40.0f)));
	XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, -15.0f, -40.0f)));
	XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-80.0f, 15.0f, 80.0f)));
	XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-80.0f, -15.0f, -80.0f)));
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[1]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[2]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[3]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[4]).r[3]);

	XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 20.0f, 30.0f)));
	XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-50.0f, 25.0f, -50.0f)));
	XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 30.0f, 60.0f)));
	XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 35.0f, -25.0f)));
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[1]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[2]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[3]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[4]).r[3]);

	XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(150.0f, 50.0f, 150.0f)));
	XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(250.0f, -50.0f, -150.0f)));
	XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(200.0f, 100.0f, 100.0f)));
	XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixRotationY(30.0f)), XMMatrixTranslation(200.0f, -100.0f, 100.0f)));
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[1]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[2]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[3]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[4]).r[3]);

#if USE_SECOND_VIEWPORT
	XMStoreFloat4x4(&matrix_cb.world[1], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-40.0f, 20.0f, 30.0f)));
	XMStoreFloat4x4(&matrix_cb.world[2], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-50.0f, 25.0f, -50.0f)));
	XMStoreFloat4x4(&matrix_cb.world[3], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 30.0f, 60.0f)));
	XMStoreFloat4x4(&matrix_cb.world[4], XMMatrixMultiply(XMMatrixRotationX(30.0f), XMMatrixTranslation(-90.0f, 35.0f, -25.0f)));
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[1]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[2]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[3]).r[3]);
	modelPositions.push_back(XMLoadFloat4x4(&matrix_cb.world[4]).r[3]);
#endif

	// describe constant variables and create constant buffer
	hr = CreateBuffer(myDevice, &matrix_id3d11buffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(Matrix_ConstantBuffer), nullptr);
	hr = CreateBuffer(myDevice, &light_id3d11buffer, D3D11_BIND_CONSTANT_BUFFER, sizeof(Light_ConstantBuffer), nullptr);
	return hr;
}

void Graphics::CleanDevice()
{
	if (inputLayout) { inputLayout->Release(); }
	if (matrix_id3d11buffer) { matrix_id3d11buffer->Release(); }
	if (light_id3d11buffer) { light_id3d11buffer->Release(); }
	if (blendState) { blendState->Release(); }
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
	float rotationSpeed = 0.35f;
	offset = 10.0f;
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
	if (GetAsyncKeyState('J')) { camera.Yaw(-offset * delta * rotationSpeed); }
	// yaw right
	if (GetAsyncKeyState('L')) { camera.Yaw(offset * delta * rotationSpeed); }
	// pitch up
	if (GetAsyncKeyState('I')) { camera.Pitch(-offset * delta * rotationSpeed); }
	// pitch down
	if (GetAsyncKeyState('K')) { camera.Pitch(offset * delta * rotationSpeed); }
	// waviness
	if (GetAsyncKeyState('F') & 0x1) { wave = !wave; }
	// enable normal mapping
	if (GetAsyncKeyState('N') & 0x1) { normalMapping = !normalMapping; }
	// reset camera
	if (GetAsyncKeyState('R') & 0x1) { camera.SetPosition(0.0f, 20.0f, -70.0f); camera.SetProjection(90.0f, 1.0f, 0.1f, 500.0f); lockOn = false; }

	if (GetAsyncKeyState(VK_SPACE) & 0x1)
	{
		++modelID;
		if (modelID == modelPositions.size())
			modelID = 0;
		lockOn = true;
	}
	if (lockOn)
	{
		cameraTarget = modelPositions[modelID];
		camera.LookAt(cameraTarget);
	}
	camera.UpdateView();
}

void Graphics::ConstantBufferSetUp(const XMMATRIX& worldMatrix, Camera& camera)
{
	// lighting
	static float rot = 0.0f; rot += 0.1f;
	// directional lighting
	//dLight.SetWorldMatrix(XMMatrixRotationY(0.01f));
	dLight.SetWorldMatrix(XMMatrixIdentity());
	dLight.UpdatePosition();
	XMStoreFloat4(&light_cb.lightPos[0], dLight.GetPositionVector());
	XMStoreFloat4(&light_cb.lightNormal[0], dLight.GetNormalVectorNormalized());
	XMStoreFloat4(&light_cb.lightColor[0], dLight.GetColor());
	// point light
	pLight.SetWorldMatrix(XMMatrixIdentity());
	pLight.UpdatePosition();
	XMStoreFloat4(&light_cb.lightPos[1], pLight.GetPositionVector());
	XMStoreFloat4(&light_cb.lightNormal[1], pLight.GetNormalVectorNormalized());
	XMStoreFloat4(&light_cb.lightColor[1], pLight.GetColor());
	// spot light
	sLight.SetPosition(radius * 5.0f, 1.0f, radius * 5.0f);
	sLight.SetWorldMatrix(XMMatrixRotationY(0.001f));
	sLight.UpdatePosition();
	XMVECTOR coneRatio = XMVectorSet(0.78f, 0.48f, (float)normalMapping, 0.0f);
	XMStoreFloat4(&light_cb.coneRatio, coneRatio);
	XMStoreFloat4(&light_cb.lightPos[2], sLight.GetPositionVector());
	XMStoreFloat4(&light_cb.lightNormal[2], sLight.GetNormalVectorNormalized());
	XMStoreFloat4(&light_cb.lightColor[2], sLight.GetColor());
	XMVECTOR lightRad = XMVectorSet(radius, rot, (float)elapsedTime, (float)wave);
	XMStoreFloat4(&light_cb.lightRadius, lightRad);

	// world
	XMStoreFloat4x4(&matrix_cb.world[0], worldMatrix);
	// view
	XMStoreFloat4x4(&matrix_cb.view, camera.GetViewMatrix());
	// projection
	float ar = 0.0f;
	mySurface->GetAspectRatio(ar);
	camera.SetAspectRatio(ar);
	XMStoreFloat4x4(&matrix_cb.projection, camera.GetProjectionMatrix());
	// camera position
	XMStoreFloat4(&matrix_cb.cameraPosition, XMMatrixInverse(nullptr, camera.GetViewMatrix()).r[3]);

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
