#ifndef __EMITTER_H__
#define __EMITTER_H__

#include <set>
#include "Particle.h"

using namespace std;

#define MAX_PARTICLE_LIMIT  2048
#define MAX_PARTICLE_TYPE   4

class Particle;

typedef std::set<Particle*>     ParticleVec;
typedef ParticleVec::iterator   ParticleVecIter;

class Emitter
{
private:
    D3DXVECTOR3 m_position, m_direction;    
    double m_particleAge;
    float m_particleAgeVriety;
    unsigned int m_maxParticleNum;
    unsigned int m_crtParticleNum;
    unsigned int m_emitSpeed;
    bool         m_isOK;

    LPDIRECT3DDEVICE9   m_device;
    LPDIRECT3DTEXTURE9  m_textures[MAX_PARTICLE_TYPE+1];
    LPDIRECT3DVERTEXBUFFER9 m_vertexBuffer;
    ID3DXFont*          m_font;
    std::wstring        m_dbgMsg;
protected:
    ParticleVec m_poolUsed[MAX_PARTICLE_TYPE], m_poolReady;
protected:
    void DestroyParticlePool();
    void InitParticlePool();

    void LoadTexture();
    void DestroyTexture();
public:
    Emitter(LPDIRECT3DDEVICE9 device, D3DXVECTOR3 position, D3DXVECTOR3 direction, 
        unsigned int maxParticleNum = 100, unsigned int emitSpeed = 30);
    ~Emitter();

    void Update(double timeElapsed);
    void Render();
    void RecycleParticle(Particle* p);
};

#endif