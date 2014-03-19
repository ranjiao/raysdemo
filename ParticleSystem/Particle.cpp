#include "DXUT.h"
#include "Particle.h"
#include "Emitter.h"

void Particle::Reset()
{
    m_speed = m_position = D3DXVECTOR3(0,0,0);
    m_crtAge = m_originAge;
}

void Particle::SetParam(D3DXVECTOR3 position, D3DXVECTOR3 speed, float age, int type)
{
    m_position = position;
    m_speed = speed;
    m_crtAge = m_originAge = age;
    m_type = type;
}

bool Particle::Update(double timeElapsed)
{
    m_crtAge -= timeElapsed;
    
    m_position += m_speed * (float)timeElapsed;

    // recycle this partile
    if(m_crtAge <= 0)
        return true;
    return false;
}
