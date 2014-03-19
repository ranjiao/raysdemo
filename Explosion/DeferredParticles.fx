//--------------------------------------------------------------------------------------
// File: BasicHLSL9.fx
//
// The effect file for the BasicHLSL sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

#define MAX_INSTANCES 200
#define MAX_GLOWLIGHTS 8

cbuffer cbInstancedGlobals
{
	float4x4 g_mWorldInst[MAX_INSTANCES];
	float4x4 g_mViewProj;
};

cbuffer cbPerFrame
{
	float  g_fTime;   
	float3 g_LightDir;
	float3 g_vEyePt;
	float3 g_vRight;
	float3 g_vUp;
	float3 g_vForward;
	float4x4 g_mWorldViewProjection;   
	float4x4 g_mInvViewProj;
	float4x4 g_mWorld;
};

cbuffer cbglowlights
{
	uint   g_NumGlowLights;
	float4 g_vGlowLightPosIntensity[MAX_GLOWLIGHTS];
	float4 g_vGlowLightColor[MAX_GLOWLIGHTS];
	
	float3  g_vGlowLightAttenuation;
	float3  g_vMeshLightAttenuation;
};

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
Texture2D g_txMeshTexture;          // Color texture for mesh
//Texture2D g_txParticleColor;        // Particle color buffer

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = 0;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DepthRead
{
    DepthEnable = TRUE;
    DepthWriteMask = 0;
    DepthFunc = LESS_EQUAL;
};
//
//	Blend State
//
BlendState DeferredBlending
{
	AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    BlendEnable[1] = TRUE;
    SrcBlend = ONE;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ONE;
    DestBlendAlpha = INV_SRC_ALPHA;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
};

BlendState ForwardBlending
{
	AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    BlendEnable[1] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
};

BlendState CompositeBlending
{
	AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    BlendEnable[1] = FALSE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
};

BlendState DisableBlending
{
	AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
    BlendEnable[1] = FALSE;
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
};

RasterizerState RSWireframe
{
	FillMode = Wireframe;
};

RasterizerState RSSolid
{
	FillMode = Solid;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_PARTICLEINPUT
{
	float4 Position   		: POSITION;
	float2 TextureUV  		: TEXCOORD0;
	float3 vLifeAndRot      : NORMAL; 
	float4 Color	  		: COLOR0;
};

struct VS_PARTICLEOUTPUT
{
    float4 Position   		: POSITION; // vertex position 
    float3 TextureUVI 		: TEXCOORD0;   // vertex texture coords
    float3 SinCosThetaLife 	: TEXCOORD1;
    float4 Color	  		: COLOR0;
};

struct VS_SCREENOUTPUT
{
    float4 Position   : SV_POSITION; // vertex position  
};

//--------------------------------------------------------------------------------------
// Render particle information into the particle buffer
//--------------------------------------------------------------------------------------
VS_PARTICLEOUTPUT RenderParticlesVS( VS_PARTICLEINPUT input )
{
    VS_PARTICLEOUTPUT Output;
    
    // Standard transform
    Output.Position = mul(input.Position, g_mWorldViewProjection);
    Output.TextureUVI.xy = input.TextureUV; 
    //Output.TextureUVI.xy = input.vLifeAndRot.xy; 
    // modify by yanglp
    Output.Color = input.Color;
    
    
    // Get the world position
    float3 WorldPos = mul( input.Position, g_mWorld ).xyz;

	// Loop over the glow lights (from the explosions) and light our particle
	float runningintensity = 0;
	uint count = g_NumGlowLights;
	for( uint i=0; i<count; i++ )
	{
		float3 delta = g_vGlowLightPosIntensity[i].xyz - WorldPos;
		float distSq = dot(delta,delta);
		float3 d = float3(1,/*sqrt(distSq)*/0,distSq);
		
		float fatten = 1.0 / dot( g_vGlowLightAttenuation, d );
		
		float intensity = fatten * g_vGlowLightPosIntensity[i].w * g_vGlowLightColor[i].w;
		runningintensity += intensity;
		Output.Color += intensity * float4(0.9, 0.1, 0.1, 0.9);//g_vGlowLightColor[i];
	}
	
	Output.Color = float4(1,0,0,0);
	Output.TextureUVI.z = runningintensity;
    
    // Rotate our texture coordinates
    float fRot = -input.vLifeAndRot.y;					//这里本来是fRot的，但用vLifeAndRot的第二个值传进来
    //float fRot = 0.5;
    Output.SinCosThetaLife.x = sin( fRot );
    Output.SinCosThetaLife.y = cos( fRot );
    Output.SinCosThetaLife.z = input.vLifeAndRot.x;		//这里本来是fLife的，但用vLifeAndRot的第一个值传进来
    //Output.SinCosThetaLife.z = 0.5;
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// Render particle information into the particle buffer
//--------------------------------------------------------------------------------------
struct PBUFFER_OUTPUT
{
	float4 color0 : SV_TARGET0;
	float4 color1 : SV_TARGET1;
};

//--------------------------------------------------------------------------------------
// Render particle information into the screen
//--------------------------------------------------------------------------------------
float4 RenderParticlesPS( VS_PARTICLEOUTPUT input ) : COLOR
{ 	
	float4 diffuse = g_txMeshTexture.Sample( g_samLinear, input.TextureUVI.xy );
	
	// unbias
	float3 norm = diffuse.xyz * 2 - 1;

	// rotate
	float3 rotnorm;
	float fSinTheta = input.SinCosThetaLife.x;
	float fCosTheta = input.SinCosThetaLife.y;
	
	rotnorm.x = fCosTheta * norm.x - fSinTheta * norm.y;
	rotnorm.y = fSinTheta * norm.x + fCosTheta * norm.y;
	rotnorm.z = norm.z;
	
	// rebias
	norm = rotnorm;
	
	// Fade, modified by yanglp
	float alpha = max(0.0f, diffuse.a * (1.0f - input.SinCosThetaLife.z));
	//float alpha = diffuse.a * 0.5;
	
	// rebias	
	float intensity = input.TextureUVI.z * alpha;
	
	// move normal into world space
    float3 worldnorm;
    worldnorm = -norm.x * g_vRight;
    worldnorm += norm.y * g_vUp;
    worldnorm += -norm.z * g_vForward;
    
    //modified by yanglp
    float lighting = max( 0.1, dot( worldnorm, g_LightDir ) );
    //float lighting = 0.5;
    
    float3 flashcolor = input.Color.xyz * intensity;
    float3 lightcolor = input.Color.xyz * lighting;
    float3 lerpcolor = lerp( lightcolor, flashcolor, intensity );
    float4 color = float4( lerpcolor, alpha );
    //color = float4(flashcolor, alpha);
	
	//float4 color1 = float4(1.0f,1.0f,0.0f,0.3f);
	return color;
}

//--------------------------------------------------------------------------------------
// Renders scene to render target using D3D9 Techniques
//--------------------------------------------------------------------------------------
technique RenderParticles
{
    pass P0
    {
        //SetVertexShader( CompileShader( vs_4_0, RenderParticlesVS( ) ) );
        //SetPixelShader( CompileShader( ps_4_0, RenderParticlesPS( ) ) );

		VertexShader = compile vs_3_0 RenderParticlesVS( );
		PixelShader = compile ps_3_0 RenderParticlesPS( );
		
		//以下这些状态在vs,ps_3_0里不知道怎么设置，但可以在程序里设置这些状态
		//SetBlendState( ForwardBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		
		//AlphaToCoverageEnable = FALSE;
		//BlendEnable[0] = TRUE ;
		//BlendEnable[1] = TRUE;
		
 		AlphaBlendEnable = TRUE;
  		SrcBlend = SRCALPHA;
  		DestBlend = INVSRCALPHA;
  		BlendOp = ADD;
 		SrcBlendAlpha = SRCALPHA;// ZERO;
 		DestBlendAlpha = DESTALPHA;//ZERO;
 		BlendOpAlpha = ADD;
		//RenderTargetWriteMask[0] = 0x0F;
		//RenderTargetWriteMask[1] = 0x0F;
		
		//SetDepthStencilState( DepthRead, 0 );
 		//DepthEnable = TRUE;
 		//StencilWriteMask = 0;
 		//StencilFunc = LESSEQUAL;
	    
        //SetRasterizerState( RSSolid );
        //FillMode = SOLID;
    }
}