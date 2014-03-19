//--------------------------------------------------------------------------------------
// File: Explosion.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
//#include "BreakableWall.h"
#include "ParticleSystem.h"
#include "resource.h"


#define MAX_MUSHROOM_CLOUDS 8
#define MAX_GROUND_BURSTS 23
#define MAX_PARTICLE_SYSTEMS 30
#define MAX_FLASH_LIGHTS 8
#define MAX_INSTANCES 200

extern UINT        g_NumUsedParticles;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CParticleSystem**                   g_ppParticleSystem = NULL;
ID3DXEffect*						g_pEffect = NULL;       // D3DX effect interface
CModelViewerCamera                  g_Camera;               // A model viewing camera
IDirect3DVertexBuffer9*				g_pParticleBuffer = NULL;
CDXUTDirectionWidget                g_LightControl;
D3DXHANDLE							g_technique = NULL ;
D3DXHANDLE							g_TextureHandle = NULL;
IDirect3DTexture9*					g_pTexture;

LPDIRECT3DVERTEXDECLARATION9    g_pVertDecl = NULL;// Vertex decl for the sample
D3DVERTEXELEMENT9 g_aVertDecl[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,   0 },
	{ 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
	D3DDECL_END()
};


D3DXVECTOR3                         g_vWindVel( -2.0f,10.0f,0 );
D3DXVECTOR3                         g_vGravity( 0,-9.8f,0.0f );
float                               g_fGroundPlane = 0.5f;
float                               g_fLightRaise = 1.0f;
float                               g_fFlashLife = 0.50f;
float                               g_fWorldBounds = 100.0f;
float                               g_fFlashIntensity = 1000.0f;
float                               g_fGroundBurstStartSpeed = 100.0f;
float                               g_fLandMineStartSpeed = 250.0f;
D3DXVECTOR4                         g_vFlashAttenuation( 0,0.0f,3.0f,0 );
D3DXVECTOR4                         g_vMeshLightAttenuation( 0,0,1.5f,0 );

UINT                                g_NumParticles = 200;
UINT                                g_NumParticlesToDraw = 0;
float                               g_fStartSize = 0.0f;
float                               g_fEndSize = 10.0f;
float                               g_fSizeExponent = 128.0f;
float                               g_fSpread = 4.0f;
float                               g_fMushroomStartSpeed = 20.0f;
float                               g_fStalkStartSpeed = 50.0f;
float                               g_fMushroomCloudLifeSpan = 10.0f;
float                               g_fEndSpeed = 4.0f;
float                               g_fSpeedExponent = 32.0f;
float                               g_fFadeExponent = 4.0f;
float                               g_fGroundBurstLifeSpan = 9.0f;
float                               g_fRollAmount = 0.2f;
float                               g_fWindFalloff = 20.0f;
D3DXVECTOR3                         g_vPosMul( 1,1,1 );
D3DXVECTOR3                         g_vDirMul( 1,1,1 );
float                               g_fPopperLifeSpan = 9.0f;


#define MAX_FLASH_COLORS 4
D3DXVECTOR4 g_vFlashColor[MAX_FLASH_COLORS] =
{
	D3DXVECTOR4( 1.0f, 0.5f, 0.00f, 0.9f ),
	D3DXVECTOR4( 1.0f, 0.3f, 0.05f, 0.9f ),
	D3DXVECTOR4( 1.0f, 0.4f, 0.00f, 0.9f ),
	D3DXVECTOR4( 0.8f, 0.3f, 0.05f, 0.9f )
};

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext )
{
    // Typically want to skip back buffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
	HRESULT hr;

	// Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
	// shader debugger. Debugging vertex shaders requires either REF or software vertex 
	// processing, and debugging pixel shaders requires REF.  The 
	// D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
	// shader debugger.  It enables source level debugging, prevents instruction 
	// reordering, prevents dead code elimination, and forces the compiler to compile 
	// against the next higher available software target, which ensures that the 
	// unoptimized shaders do not exceed the shader model limitations.  Setting these 
	// flags will cause slower rendering since the shaders will be unoptimized and 
	// forced into software.  See the DirectX documentation for more information about 
	// using the shader debugger.
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

#ifdef DEBUG_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"DeferredParticles.fx" ) );

	LPD3DXBUFFER pBuffer;
	// If this fails, there should be debug output as to 
	// they the .fx file failed to compile
	D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags,
		NULL, &g_pEffect, &pBuffer );
	if (pBuffer)
	{
		::MessageBoxA(0,(char*)pBuffer->GetBufferPointer(),0,0);
		SAFE_RELEASE(pBuffer);
		return S_FALSE;
	}
	V_RETURN( pd3dDevice->CreateVertexDeclaration( g_aVertDecl, &g_pVertDecl ) );


	// World transform to identity
	D3DXMATRIXA16 mIdent;
	D3DXMatrixIdentity( &mIdent );
	V_RETURN( pd3dDevice->SetTransform( D3DTS_WORLD, &mIdent ) );

	//粒子系统
	// Particle system
	UINT NumStalkParticles = 500;
	UINT NumGroundExpParticles = 345;
	UINT NumLandMineParticles = 125;
	UINT MaxParticles = MAX_MUSHROOM_CLOUDS * ( g_NumParticles + NumStalkParticles ) +
		( MAX_GROUND_BURSTS - MAX_MUSHROOM_CLOUDS ) * NumGroundExpParticles +
		( MAX_PARTICLE_SYSTEMS - MAX_GROUND_BURSTS ) * NumLandMineParticles;
	V_RETURN( CreateParticleArray( MaxParticles ) );

	D3DXVECTOR4 vColor0( 1.0f,1.0f,1.0f,1 );
	D3DXVECTOR4 vColor1( 0.6f,0.6f,0.6f,1 );

	srand( timeGetTime() );
	g_ppParticleSystem = new CParticleSystem*[MAX_PARTICLE_SYSTEMS];
	g_NumParticlesToDraw = 0;
	for( UINT i = 0; i < MAX_MUSHROOM_CLOUDS; i += 2 )
	{
		D3DXVECTOR3 vLocation;
		vLocation.x = RPercent() * 50.0f;
		vLocation.y = g_fGroundPlane;
		vLocation.z = RPercent() * 50.0f;

		g_ppParticleSystem[i] = new CMushroomParticleSystem();
		g_ppParticleSystem[i]->CreateParticleSystem( g_NumParticles );
		g_ppParticleSystem[i]->SetSystemAttributes( vLocation,
			g_fSpread, g_fMushroomCloudLifeSpan, g_fFadeExponent,
			g_fStartSize, g_fEndSize, g_fSizeExponent,
			g_fMushroomStartSpeed, g_fEndSpeed, g_fSpeedExponent,
			g_fRollAmount, g_fWindFalloff,
			1, 0, D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ),
			vColor0, vColor1,
			g_vPosMul, g_vDirMul );

		g_NumParticlesToDraw += g_NumParticles;

		g_ppParticleSystem[i + 1] = new CStalkParticleSystem();
		g_ppParticleSystem[i + 1]->CreateParticleSystem( NumStalkParticles );
		g_ppParticleSystem[i + 1]->SetSystemAttributes( vLocation,
			15.0f, g_fMushroomCloudLifeSpan, g_fFadeExponent * 2.0f,
			g_fStartSize * 0.5f, g_fEndSize * 0.5f, g_fSizeExponent,
			g_fStalkStartSpeed, -1.0f, g_fSpeedExponent,
			g_fRollAmount, g_fWindFalloff,
			1, 0, D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ),
			vColor0, vColor1,
			D3DXVECTOR3( 1, 0.1f, 1 ), D3DXVECTOR3( 1, 0.1f, 1 ) );

		g_NumParticlesToDraw += NumStalkParticles;
	}

	for( UINT i = MAX_MUSHROOM_CLOUDS; i < MAX_GROUND_BURSTS; i++ )
	{
		D3DXVECTOR3 vLocation;
		vLocation.x = RPercent() * 50.0f;
		vLocation.y = g_fGroundPlane;
		vLocation.z = RPercent() * 50.0f;

		g_ppParticleSystem[i] = new CGroundBurstParticleSystem();
		g_ppParticleSystem[i]->CreateParticleSystem( NumGroundExpParticles );
		g_ppParticleSystem[i]->SetSystemAttributes( vLocation,
			1.0f, g_fGroundBurstLifeSpan, g_fFadeExponent,
			0.5f, 8.0f, 1.0f,
			g_fGroundBurstStartSpeed, g_fEndSpeed, 4.0f,
			g_fRollAmount, 1.0f,
			30, 100.0f, D3DXVECTOR3( 0, 0.5f, 0 ), D3DXVECTOR3( 1.0f, 0.5f,
			1.0f ),
			vColor0, vColor1,
			g_vPosMul, g_vDirMul );

		g_NumParticlesToDraw += NumGroundExpParticles;
	}

	for( UINT i = MAX_GROUND_BURSTS; i < MAX_PARTICLE_SYSTEMS; i++ )
	{
		D3DXVECTOR3 vLocation;
		vLocation.x = RPercent() * 50.0f;
		vLocation.y = g_fGroundPlane;
		vLocation.z = RPercent() * 50.0f;

		g_ppParticleSystem[i] = new CLandMineParticleSystem();
		g_ppParticleSystem[i]->CreateParticleSystem( NumLandMineParticles );
		g_ppParticleSystem[i]->SetSystemAttributes( vLocation,
			1.5f, g_fPopperLifeSpan, g_fFadeExponent,
			1.0f, 6.0f, 1.0f,
			g_fLandMineStartSpeed, g_fEndSpeed, 2.0f,
			g_fRollAmount, 4.0f,
			0, 70.0f, D3DXVECTOR3( 0, 0.8f, 0 ), D3DXVECTOR3( 0.3f, 0.2f,
			0.3f ),
			vColor0, vColor1,
			g_vPosMul, g_vDirMul );

		g_NumParticlesToDraw += NumGroundExpParticles;
	}

	pd3dDevice->CreateVertexBuffer( sizeof( PARTICLE_VERTEX ) * 6 * g_NumParticlesToDraw,
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_MANAGED,
		&g_pParticleBuffer,
		0);

	//得到shader里的常量
	D3DXCreateTextureFromFile(pd3dDevice, L"DeferredParticle.dds", &g_pTexture);
	g_pEffect->SetTexture("g_txMeshTexture", g_pTexture);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
	HRESULT hr;

	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );


	if( g_pEffect )
		V_RETURN( g_pEffect->OnResetDevice() );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// Update the camera's position based on user input 
	g_Camera.FrameMove( fElapsedTime );
	if (fElapsedTime  > 0.1f ) fElapsedTime = 0.1f;

	D3DXVECTOR3 vEye;
	D3DXMATRIX mView;
	vEye = *g_Camera.GetEyePt();
	mView = *g_Camera.GetViewMatrix();
	D3DXVECTOR3 vRight( mView._11, mView._21, mView._31 );
	D3DXVECTOR3 vUp( mView._12, mView._22, mView._32 );
	D3DXVECTOR3 vFoward( mView._13, mView._23, mView._33 );

	D3DXVec3Normalize( &vRight, &vRight );
	D3DXVec3Normalize( &vUp, &vUp );
	D3DXVec3Normalize( &vFoward, &vFoward );

	g_pEffect->SetVector("g_vRight",&(D3DXVECTOR4(vRight.x,vRight.y,vRight.z,0.0f)));
	g_pEffect->SetVector("g_vEyePt",&(D3DXVECTOR4(vFoward.x, vFoward.y, vFoward.z, 0.0f)));
	g_pEffect->SetVector("g_vUp",&(D3DXVECTOR4(vUp.x, vUp.y, vUp.z, 0.0f)));


	UINT NumActiveSystems = 0;
	D3DXVECTOR4 vGlowLightPosIntensity[MAX_PARTICLE_SYSTEMS];
	D3DXVECTOR4 vGlowLightColor[MAX_PARTICLE_SYSTEMS];

	// Advance the system
	for( UINT i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
	{
		g_ppParticleSystem[i]->AdvanceSystem( ( float )fTime, fElapsedTime, vRight, vUp, g_vWindVel, g_vGravity );
	}

	PARTICLE_VERTEX* pVerts = NULL;

	g_pParticleBuffer->Lock( 0, 0, ( void** )&pVerts, 0 );

	CopyParticlesToVertexBuffer( pVerts, vEye, vRight, vUp );

	g_pParticleBuffer->Unlock();


	for( UINT i = 0; i < MAX_MUSHROOM_CLOUDS; i += 2 )
	{
		float fCurrentTime = g_ppParticleSystem[i]->GetCurrentTime();
		float fLifeSpan = g_ppParticleSystem[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			D3DXVECTOR3 vCenter;
			vCenter.x = RPercent() * g_fWorldBounds;
			vCenter.y = g_fGroundPlane;
			vCenter.z = RPercent() * g_fWorldBounds;
			float fStartTime = -fabs( RPercent() ) * 4.0f;
			D3DXVECTOR4 vFlashColor = g_vFlashColor[ rand() % MAX_FLASH_COLORS ];

			g_ppParticleSystem[i]->SetCenter( vCenter );
			g_ppParticleSystem[i]->SetStartTime( fStartTime );
			g_ppParticleSystem[i]->SetFlashColor( vFlashColor );
			g_ppParticleSystem[i]->Init();

			g_ppParticleSystem[i + 1]->SetCenter( vCenter );
			g_ppParticleSystem[i + 1]->SetStartTime( fStartTime );
			g_ppParticleSystem[i + 1]->SetFlashColor( vFlashColor );
			g_ppParticleSystem[i + 1]->Init();
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < g_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = g_ppParticleSystem[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = g_ppParticleSystem[i]->GetFlashColor();

			float fIntensity = g_fFlashIntensity * ( ( g_fFlashLife - fCurrentTime ) / g_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + g_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Ground bursts
	for( UINT i = MAX_MUSHROOM_CLOUDS; i < MAX_GROUND_BURSTS; i++ )
	{
		float fCurrentTime = g_ppParticleSystem[i]->GetCurrentTime();
		float fLifeSpan = g_ppParticleSystem[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			D3DXVECTOR3 vCenter;
			vCenter.x = RPercent() * g_fWorldBounds;
			vCenter.y = g_fGroundPlane;
			vCenter.z = RPercent() * g_fWorldBounds;
			float fStartTime = -fabs( RPercent() ) * 4.0f;
			D3DXVECTOR4 vFlashColor = g_vFlashColor[ rand() % MAX_FLASH_COLORS ];

			float fStartSpeed = g_fGroundBurstStartSpeed + RPercent() * 30.0f;
			g_ppParticleSystem[i]->SetCenter( vCenter );
			g_ppParticleSystem[i]->SetStartTime( fStartTime );
			g_ppParticleSystem[i]->SetStartSpeed( fStartSpeed );
			g_ppParticleSystem[i]->SetFlashColor( vFlashColor );
			g_ppParticleSystem[i]->Init();
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < g_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = g_ppParticleSystem[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = g_ppParticleSystem[i]->GetFlashColor();

			float fIntensity = g_fFlashIntensity * ( ( g_fFlashLife - fCurrentTime ) / g_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + g_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Land mines
	for( UINT i = MAX_GROUND_BURSTS; i < MAX_PARTICLE_SYSTEMS; i++ )
	{
		float fCurrentTime = g_ppParticleSystem[i]->GetCurrentTime();
		float fLifeSpan = g_ppParticleSystem[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			D3DXVECTOR3 vCenter;
			vCenter.x = RPercent() * g_fWorldBounds;
			vCenter.y = g_fGroundPlane;
			vCenter.z = RPercent() * g_fWorldBounds;
			float fStartTime = -fabs( RPercent() ) * 4.0f;
			D3DXVECTOR4 vFlashColor = g_vFlashColor[ rand() % MAX_FLASH_COLORS ];

			float fStartSpeed = g_fLandMineStartSpeed + RPercent() * 100.0f;
			g_ppParticleSystem[i]->SetCenter( vCenter );
			g_ppParticleSystem[i]->SetStartTime( fStartTime );
			g_ppParticleSystem[i]->SetStartSpeed( fStartSpeed );
			g_ppParticleSystem[i]->SetFlashColor( vFlashColor );
			g_ppParticleSystem[i]->Init();
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < g_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = g_ppParticleSystem[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = g_ppParticleSystem[i]->GetFlashColor();

			float fIntensity = g_fFlashIntensity * ( ( g_fFlashLife - fCurrentTime ) / g_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + g_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Setup light variables
	g_pEffect->SetInt("g_NumGlowLights", NumActiveSystems);
	g_pEffect->SetVectorArray("g_vGlowLightPosIntensity", vGlowLightPosIntensity, NumActiveSystems);
	g_pEffect->SetVectorArray("g_vGlowLightColor", vGlowLightColor, NumActiveSystems);
	g_pEffect->SetVector("g_vGlowLightAttenuation", &g_vFlashAttenuation);
	g_pEffect->SetVector("g_vMeshLightAttenuation", &g_vMeshLightAttenuation);
}


//--------------------------------------------------------------------------------------
// Render particles
//--------------------------------------------------------------------------------------
void RenderParticles( IDirect3DDevice9* pd3dDevice, ID3DXEffect* pRenderTechnique )
{
	//绑定顶点结构
	
	//设置那些本来在shader里设置的一些状态
	//pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	//pd3dDevice->SetRenderState(D3DBLEND_BLENDFACTOR);

	//pd3dDevice->SetRenderState(d3drs_);
	g_pEffect->SetTechnique("RenderParticles");
	// Render the scene
    pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	//pd3dDevice->SetRenderState(D3DRS_ZENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	UINT iPass,cPass;
	g_pEffect->Begin(&cPass, 0);
	for (iPass = 0; iPass<cPass; iPass++)
	{
		g_pEffect->BeginPass(iPass);

		//pd3dDevice->SetFVF(PARTICLE_VERTEX::FVF);
		pd3dDevice->SetVertexDeclaration(g_pVertDecl);
		pd3dDevice->SetStreamSource(0, g_pParticleBuffer, 0, sizeof(PARTICLE_VERTEX));
		pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2*g_NumUsedParticles);

		g_pEffect->EndPass();
	}
	g_pEffect->End();
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
	
	D3DXVECTOR3 vEyePt;
	D3DXMATRIX mWorldViewProjection;
	D3DXVECTOR4 vLightDir;
	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mViewProj;
	D3DXMATRIX mInvViewProj;

	// Get the projection & view matrix from the camera class
	D3DXMatrixIdentity( &mWorld );
	vEyePt = *g_Camera.GetEyePt();
	mProj = *g_Camera.GetProjMatrix();
	mView = *g_Camera.GetViewMatrix();

	mWorldViewProjection = mView * mProj;
	mViewProj = mView * mProj;
	D3DXMatrixInverse( &mInvViewProj, NULL, &mViewProj );
	D3DXMATRIX mSceneWorld;
	D3DXMatrixScaling( &mSceneWorld, 20, 20, 20 );
	D3DXMATRIX mSceneWVP = mSceneWorld * mViewProj;
	vLightDir = D3DXVECTOR4( g_LightControl.GetLightDirection(), 1 );

	// Per frame variables
	g_pEffect->SetMatrix("g_mWorldViewProjection", &mSceneWVP);
	g_pEffect->SetMatrix("g_mWorld", &mSceneWorld);
	g_pEffect->SetVector("g_LightDir", &vLightDir);
	g_pEffect->SetMatrix("g_mInvViewProj", &mInvViewProj);
	g_pEffect->SetFloat("g_fTime", fTime);
	g_pEffect->SetVector("g_vEyePt", &(D3DXVECTOR4(vEyePt)));
	g_pEffect->SetMatrix("g_mViewProj", &mViewProj);

	g_pEffect->SetMatrix("g_mWorldViewProjection",&mWorldViewProjection);
	g_pEffect->SetMatrix("g_mWorld", &mWorld);
	//g_technique = g_pEffect->GetTechniqueByName("RenderParticles");


    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 255,255, 255 ), 1.0f, 0 ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		RenderParticles( pd3dDevice, g_pEffect );
        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
	// Pass all remaining windows messages to camera so it can respond to user input
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	g_LightControl.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
}



//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
	SAFE_RELEASE(g_pEffect);
	for( UINT i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
	{
		SAFE_DELETE( g_ppParticleSystem[i] );
	}
	SAFE_DELETE_ARRAY(g_ppParticleSystem);
	SAFE_RELEASE( g_pParticleBuffer );
	SAFE_RELEASE( g_pTexture );
	SAFE_RELEASE( g_pVertDecl);

	DestroyParticleArray();
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions
    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // TODO: Perform any application-level initialization here

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Explosion" );
    DXUTCreateDevice( true, 640, 480 );


	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye( 0.0f, 150.0f, 336.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );
	g_Camera.SetViewParams( &vecEye, &vecAt );

	//set the light
	D3DXVECTOR3 vDir( 1,1,0 );
	D3DXVec3Normalize( &vDir, &vDir );
	g_LightControl.SetLightDirection( vDir );
    // Start the render loop
    DXUTMainLoop();

    // TODO: Perform any application-level cleanup here

    return DXUTGetExitCode();
}

