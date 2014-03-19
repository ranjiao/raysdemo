#include "DXUT.h"
#include <assert.h>
#include "Particle.h"
#include "Emitter.h"
#include <sstream>

struct FVFParticle
{
    D3DXVECTOR3 position;
    unsigned long color;
};
#define FVF_PARTICLE_VERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

inline DWORD FtoDW( FLOAT f ){ return *((DWORD*)&f ); }
using namespace std;

static HRESULT hr;

Emitter::Emitter(LPDIRECT3DDEVICE9 device, D3DXVECTOR3 position, D3DXVECTOR3 direction, 
        unsigned int maxParticleNum, unsigned int emitSpeed)
{
    m_device = device;
    m_position = position;
    m_direction = direction;
    m_maxParticleNum = maxParticleNum;
    m_emitSpeed = emitSpeed;
    m_particleAge = 2.0;
    m_particleAgeVriety = 0.5;
    m_crtParticleNum = 0;
    m_isOK = false;

    InitParticlePool();
    LoadTexture();

    if(FAILED(hr = m_device->CreateVertexBuffer(maxParticleNum * sizeof(FVFParticle),
        D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS,
        FVF_PARTICLE_VERTEX, 
        D3DPOOL_DEFAULT,
        &m_vertexBuffer, NULL)))
        assert(false);
    if(FAILED(hr = D3DXCreateFont( m_device, -11, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_font)))
        assert(false);
}

Emitter::~Emitter()
{
    DestroyParticlePool();
    DestroyTexture();

    SAFE_RELEASE(m_vertexBuffer);
    SAFE_RELEASE(m_font);
}

void Emitter::DestroyParticlePool()
{
    m_crtParticleNum = 0;
    for(int i=0; i<MAX_PARTICLE_TYPE; i++)
        m_crtParticleNum += m_poolUsed[i].size();

    assert(m_crtParticleNum + m_poolReady.size() == m_maxParticleNum);

    set<Particle*>::iterator iter;
    for(int i=0; i<MAX_PARTICLE_TYPE; i++)
    {
        for(iter = m_poolUsed[i].begin(); iter != m_poolUsed[i].end(); iter++)
            delete *iter;
        m_poolUsed[i].clear();
    }

    for(iter = m_poolReady.begin(); iter != m_poolReady.end(); iter++)
        delete *iter;
    m_poolReady.clear();
    m_isOK = false;
};

void Emitter::InitParticlePool()
{
    assert(m_maxParticleNum >=0 && m_maxParticleNum < MAX_PARTICLE_LIMIT);
    for(int i=0; i<MAX_PARTICLE_TYPE; i++)
        assert(m_poolUsed[i].empty());
    assert(m_poolReady.empty());

    for(unsigned int i=0; i<m_maxParticleNum; i++)
    {
        Particle* particle = new Particle(this);
        particle->SetParam( m_position, m_direction );
        m_poolReady.insert(particle);
    }
    m_isOK = true;
};

void Emitter::LoadTexture()
{
    HRESULT result;

    for(int i=0; i<MAX_PARTICLE_TYPE; i++)
    {
        wstringstream ss;
        ss << L"Textures/fire";
        ss << i+1;
        ss << L".bmp";
        wstring strTmp = ss.str();
        result = D3DXCreateTextureFromFile(
            m_device, strTmp.c_str(), &m_textures[i]);
        if(result != D3D_OK)
            assert(false);
    }

    result = D3DXCreateTextureFromFile(
        m_device, L"Textures/whitefire.png", &m_textures[MAX_PARTICLE_TYPE]);
    if(result != D3D_OK)
        assert(false);
};

void Emitter::DestroyTexture()
{
    for(int i=0; i<MAX_PARTICLE_TYPE; i++)
        SAFE_RELEASE( m_textures[i] );
    SAFE_RELEASE( m_textures[MAX_PARTICLE_TYPE] );
}

void Emitter::RecycleParticle(Particle* p)
{
    p->Reset();
    m_poolReady.insert(p);
    m_poolUsed[p->m_type].erase(p);
}

void Emitter::Update(double timeElapsed)
{
    if(!m_isOK)
        return;

    m_crtParticleNum = 0;
    for(int i=0; i<MAX_PARTICLE_TYPE; i++)
        m_crtParticleNum += m_poolUsed[i].size();

    assert(m_crtParticleNum + m_poolReady.size() == m_maxParticleNum);

    if(m_crtParticleNum < m_maxParticleNum)
    {
        // emit new particles
        int newParticleNum = min( (unsigned int)(m_emitSpeed * timeElapsed * 10 ),
                                           m_maxParticleNum - m_crtParticleNum );
        for(int i=0; i<newParticleNum; i++)
        {
            int particleType = int(rand() / (float) RAND_MAX / 0.25f);
            const float randRadius = 2.2;
            Particle* p = *m_poolReady.begin();
            D3DXVECTOR3 newPos = m_position + D3DXVECTOR3( 
                randRadius * rand() / RAND_MAX, 
                0,
                randRadius * rand() / RAND_MAX );
            float newAge = (float)m_particleAge * (0.6f + 0.4f*rand() / RAND_MAX);

            p->SetParam( newPos, m_direction * 10.0, newAge, particleType );

            m_poolReady.erase(m_poolReady.begin());
            m_poolUsed[particleType].insert(p);
        }
    }

    for(int i=0; i<MAX_PARTICLE_TYPE; i++)
    {
        ParticleVecIter iter = m_poolUsed[i].begin();
        while(iter != m_poolUsed[i].end())
        {
            Particle* p = *iter;
            if( (*iter)->Update(timeElapsed) )
            {
                iter++;
                RecycleParticle(p);
            }
            else
                iter++;
        }
    }
}

void Emitter::Render()
{
    m_device->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );       // Turn on point sprites
    m_device->SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );       // Allow sprites to be scaled with distance
    m_device->SetRenderState( D3DRS_POINTSIZE,     FtoDW(0.3) ); // Float value that specifies the size to use for point size computation in cases where point size is not specified for each vertex.
    m_device->SetRenderState( D3DRS_POINTSIZE_MIN, FtoDW(0.0f) );    // Float value that specifies the minimum size of point primitives. Point primitives are clamped to this size during rendering. 
    m_device->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(1.0f) );   
    m_device->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(1.0f) );   
    m_device->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(0.0f) );   

    m_device->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    m_device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    m_device->SetStreamSource(0, m_vertexBuffer, 0, sizeof(FVFParticle) );
    m_device->SetFVF( FVF_PARTICLE_VERTEX );

    FVFParticle *pVertices = NULL;
    if(FAILED(hr = m_vertexBuffer->Lock(0, m_maxParticleNum * sizeof(FVFParticle),
        (void**)&pVertices, D3DLOCK_DISCARD)))
        assert(false);
    
    for(int i=0; i<4; i++)
    {
        int j=0;
        m_device->SetTexture(0, m_textures[i]);

        for(ParticleVecIter iter = m_poolUsed[i].end();
            iter != m_poolUsed[i].begin(); )
        {
            iter --;
            pVertices[j].position = (*iter)->m_position;
            float transparentRatio = (float) (*iter)->m_crtAge / (float)(*iter)->m_originAge;
            pVertices[j].color = D3DXCOLOR(1.0, 1.0, 1.0, 1.0) * transparentRatio;
            j++;
        }
 
        m_vertexBuffer->Unlock();
        m_device->DrawPrimitive(D3DPT_POINTLIST, 0, m_poolUsed[i].size());
    }

    std::wstringstream ss;
    ss << L"Particle: ";
    ss << m_crtParticleNum;
    
    m_dbgMsg = ss.str();
    RECT fontRect;
    SetRect(&fontRect, 10, 10, 610, 110);
    m_font->DrawText(NULL, m_dbgMsg.c_str(), -1, &fontRect, DT_LEFT, 0xFFFF0000);

    // Restore render states...
    //
    m_device->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
    m_device->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );

    m_device->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}