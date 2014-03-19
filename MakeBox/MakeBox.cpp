//--------------------------------------------------------------------------------------
// File: MakeBox.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "resource.h"
#include "DrawBox.h"

struct CUSTOMVERTEX
{
    FLOAT x, y, z; // The transformed position for the vertex
    DWORD color;   // The vertex color
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

ID3DXMesh *box1 = NULL;

CModelViewerCamera          g_Camera;                // A model viewing camera
IDirect3DDevice9*           g_pDevice;
bool                        g_picking = true;
CDrawBox*                   g_drawBox;

LPDIRECT3DVERTEXBUFFER9     g_pVB = NULL;

void InitMesh(IDirect3DDevice9* pd3dDevice)
{
    pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
    pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    D3DXCreateBox(pd3dDevice, 1, 1, 1, &box1, NULL);

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye( 10.0f, 10.0f, 10.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );
    g_Camera.SetProjParams( 90, 1, 1, 1000 );

    // level plane
    if( !g_pVB )
    {
        g_pDevice->CreateVertexBuffer( 200 * sizeof( CUSTOMVERTEX ),
              0, D3DFVF_CUSTOMVERTEX,
              D3DPOOL_DEFAULT, &g_pVB, NULL );
        
        CUSTOMVERTEX* pVertices;
        if( FAILED( g_pVB->Lock( 0, sizeof( CUSTOMVERTEX ) * 44, ( void** )&pVertices, 0 ) ) )
            return;   
        DWORD color = 0xffaaaa99;
        for(int i=0; i<11; i++)
        {
            pVertices[ 2*i ].x = 0 - 25.f;
            pVertices[ 2*i ].y = 0;
            pVertices[ 2*i ].z = i * 5.f  - 25.f;
            pVertices[ 2*i ].color = color;
            pVertices[ 2*i +1 ].x = 50.f - 25.f;
            pVertices[ 2*i +1 ].y = 0.f;
            pVertices[ 2*i +1 ].z = i * 5.f - 25.f;
            pVertices[ 2*i +1 ].color = color;
        }
        for(int i=0; i<11; i++)
        {
            pVertices[ 2*i + 20 ].x = i * 5.f - 25.f;
            pVertices[ 2*i + 20 ].y = 0;
            pVertices[ 2*i + 20 ].z = 0 - 25.f;
            pVertices[ 2*i + 20 ].color = color;
            pVertices[ 2*i +1 + 20 ].x = i * 5.f - 25.f;
            pVertices[ 2*i +1 + 20 ].y = 0;
            pVertices[ 2*i +1 + 20 ].z = 50 - 25.f;
            pVertices[ 2*i +1 + 20 ].color = color;
        }
        g_pVB->Unlock();
    }
}

void DestroyMesh()
{
    SAFE_RELEASE(box1);
    SAFE_RELEASE(g_pVB);
}

void SetupMatrices(IDirect3DDevice9* pd3dDevice)
{
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mWorldViewProjection;

    mWorld = *g_Camera.GetWorldMatrix();
    mView = *g_Camera.GetViewMatrix();
    mProj = *g_Camera.GetProjMatrix();

    mWorldViewProjection = mWorld * mView * mProj;

    pd3dDevice->SetTransform( D3DTS_WORLD, &mWorld );
    pd3dDevice->SetTransform( D3DTS_VIEW, &mView );
    pd3dDevice->SetTransform( D3DTS_PROJECTION, &mProj );
}

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
    g_pDevice = pd3dDevice;
    g_drawBox = new CDrawBox(pd3dDevice, &g_Camera);
    InitMesh(pd3dDevice);
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    g_Camera.FrameMove( fElapsedTime );
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) );

    SetupMatrices(pd3dDevice);
    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        // draw it
        g_pDevice->SetStreamSource( 0, g_pVB, 0, sizeof( CUSTOMVERTEX ) );
        g_pDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        g_pDevice->DrawPrimitive( D3DPT_LINELIST, 0, 44 );

        if(g_drawBox)
            g_drawBox->OnRender(fElapsedTime );
        //box1->DrawSubset(0);
        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    if( g_drawBox && !g_drawBox->OnMessage( uMsg, wParam, lParam ) )
            return 0;
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
    return 0;
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
    SAFE_DELETE(g_drawBox);
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
    DestroyMesh();
    g_pDevice = NULL;

    SAFE_DELETE(g_drawBox);
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
    DXUTCreateWindow( L"MakeBox" );
    DXUTCreateDevice( true, 1024, 768 );

    // Start the render loop
    DXUTMainLoop();

    // TODO: Perform any application-level cleanup here

    return DXUTGetExitCode();
}


