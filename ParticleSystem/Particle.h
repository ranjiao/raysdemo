#ifndef __PARTICLE_H__
#define __PARTICLE_H__

using namespace std;

class Emitter;

class Particle
{
    friend class Emitter;
    double          m_crtAge, m_originAge;
    D3DXVECTOR3     m_position, m_speed;
    Emitter*        m_emitter;
    int             m_type;
public:
    Particle(Emitter* e){ m_emitter = e; };
    void SetParam(D3DXVECTOR3 position, D3DXVECTOR3 speed, float age = 5.0f, int type = 0);
    bool Update(double timeElapsed);   // return true if it need to be recycled
    bool IsActive(){ return m_crtAge > 0; };
    void Reset();
};

#endif