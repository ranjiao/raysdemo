#include "DXUT.h"
#include "DrawBox.h"
#include <sstream>

const DWORD red = 0xffff0000;
const DWORD green = 0xff00ff00;
const DWORD blue = 0xff0000ff;
const float axisLength = 10.f;
const float axisWidth = 1.0;
const float axisHeight = 0.5;

#define vec2length(x,y) sqrt( (x)*(x) + (y)*(y) )

struct DRAWBOX_CUSTOMVERTEX
{
    FLOAT x, y, z;
    DWORD color;
    void set(float xx, float yy, float zz, DWORD ccolor)
    {
        x = xx; y = yy; z = zz; color = ccolor;
    }
};

#define D3DFVF_DRAWBOX_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

CDrawBox::CDrawBox(IDirect3DDevice9* pd3dDevice, CModelViewerCamera* camera)
:m_pDevice(pd3dDevice), m_height(0), m_color(0xffff0000), 
m_crtAxis(AXIS_NONE), m_oldState(NOT_STARTED), m_axeLength(0.01),
m_offSet(0,0,0), m_scale(0,0,0), m_rotate(0,0,0), m_oldRotate(0,0,0), 
m_camera(camera)
{
    m_ptLastCursor.x = m_ptLastCursor.y = 0;
    m_state = NOT_STARTED;
    m_cbOnCreate = m_cbOnSizeChange = m_cbOnRotateChange = m_cbOnPosChange = NULL;

    createScaleAxes();
    createMoveAxes();
    createBox();

#ifdef _DEBUG
    D3DXCreateFont( m_pDevice, -11, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
                     OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                     L"Arial", &m_pFont );
    m_debugMsg = L"Ready";
#endif
}

CDrawBox::~CDrawBox(void)
{
    SAFE_RELEASE(m_pVBMoveAxes);
    SAFE_RELEASE(m_pVBBox);
    SAFE_RELEASE(m_pIBBox);
    SAFE_RELEASE(m_pVBScaleAxes);
    SAFE_RELEASE(m_pIBScaleAxes);
    //SAFE_RELEASE(m_pIBScaleFaces);

#ifdef _DEBUG
    SAFE_RELEASE( m_pFont );
#endif
}

void CDrawBox::createBox()
{
    m_pDevice->CreateVertexBuffer( 8 * sizeof( DRAWBOX_CUSTOMVERTEX ),
          0, D3DFVF_DRAWBOX_CUSTOMVERTEX,
          D3DPOOL_DEFAULT, &m_pVBBox, NULL );

    DRAWBOX_CUSTOMVERTEX* pVertices;
    m_pVBBox->Lock( 0, sizeof( DRAWBOX_CUSTOMVERTEX ) * 8, ( void** )&pVertices, 0 );

    pVertices[0].set( 1, 1, 1, red);    
    pVertices[1].set(-1, 1, 1, red);
    pVertices[2].set(-1, 1,-1, red);
    pVertices[3].set( 1, 1,-1, red);

    pVertices[4].set( 1,-1, 1, red);
    pVertices[5].set(-1,-1, 1, red);
    pVertices[6].set(-1,-1,-1, red);
    pVertices[7].set( 1,-1,-1, red);

    m_pVBBox->Unlock();    

    m_pDevice->CreateIndexBuffer( 24*sizeof(WORD),
        D3DUSAGE_WRITEONLY,
        D3DFMT_INDEX16,
        D3DPOOL_MANAGED,
        &m_pIBBox,
        NULL);

    WORD* indices = NULL;
    m_pIBBox->Lock(0,0, (void**)&indices, 0);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 1;
    indices[3] = 2;
    indices[4] = 2;
    indices[5] = 3;
    indices[6] = 3;
    indices[7] = 0;

    indices[8]  = 4;
    indices[9]  = 5;
    indices[10] = 5;
    indices[11] = 6;
    indices[12] = 6;
    indices[13] = 7;
    indices[14] = 7;
    indices[15] = 4;

    indices[16] = 0;
    indices[17] = 4;
    indices[18] = 1;
    indices[19] = 5;
    indices[20] = 2;
    indices[21] = 6;
    indices[22] = 3;
    indices[23] = 7;

    m_pIBBox->Unlock();
}

void CDrawBox::createMoveAxes()
{
    m_pDevice->CreateVertexBuffer( 30 * sizeof( DRAWBOX_CUSTOMVERTEX ),
          0, D3DFVF_DRAWBOX_CUSTOMVERTEX,
          D3DPOOL_DEFAULT, &m_pVBMoveAxes, NULL );

    DRAWBOX_CUSTOMVERTEX* pVertices;
    m_pVBMoveAxes->Lock( 0, sizeof( DRAWBOX_CUSTOMVERTEX ) * 30, ( void** )&pVertices, 0 );
    
    pVertices[0].set( 0, 0, 0,red);
    pVertices[1].set(axisLength, 0, 0,red);
    pVertices[2].set(axisLength, 0, 0,red);
    pVertices[3].set( axisLength-axisWidth, axisHeight, 0,red);
    pVertices[4].set(axisLength, 0, 0,red);
    pVertices[5].set( axisLength-axisWidth,-axisHeight, 0,red);

    pVertices[6].set( 0, 0, 0,green);
    pVertices[7].set( 0,axisLength, 0,green);
    pVertices[8].set( 0,axisLength, 0,green);
    pVertices[9].set( 0, axisLength-axisWidth, axisHeight,green);
    pVertices[10].set( 0,axisLength, 0,green);
    pVertices[11].set(0, axisLength-axisWidth,-axisHeight,green);

    pVertices[12].set( 0, 0, 0,blue);
    pVertices[13].set( 0, 0,axisLength,blue);
    pVertices[14].set( 0, 0,axisLength,blue);
    pVertices[15].set( 0, axisHeight, axisLength-axisWidth,blue);
    pVertices[16].set( 0, 0,axisLength,blue);
    pVertices[17].set( 0,-axisHeight, axisLength-axisWidth,blue);

    pVertices[18].set( 0, 1, 0,green);
    pVertices[19].set( 1, 1, 0,green);
    pVertices[20].set( 1, 1, 0,red);
    pVertices[21].set( 1, 0, 0,red);

    pVertices[22].set( 0, 0, 1,blue);
    pVertices[23].set( 0, 1, 1,blue);
    pVertices[24].set( 0, 1, 1,green);
    pVertices[25].set( 0, 1, 0,green);

    pVertices[26].set( 0, 0, 1,blue);
    pVertices[27].set( 1, 0, 1,blue);
    pVertices[28].set( 1, 0, 1,red);
    pVertices[29].set( 1, 0, 0,red);

    m_pVBMoveAxes->Unlock();    
}

void CDrawBox::createScaleAxes()
{
    DRAWBOX_CUSTOMVERTEX* pVertices;
    m_pDevice->CreateVertexBuffer( 18 * sizeof( DRAWBOX_CUSTOMVERTEX ),
          0, D3DFVF_DRAWBOX_CUSTOMVERTEX,
          D3DPOOL_DEFAULT, &m_pVBScaleAxes, NULL );
    m_pVBScaleAxes->Lock( 0, sizeof( DRAWBOX_CUSTOMVERTEX ) * 30, ( void** )&pVertices, 0 );
    pVertices[0].set( 0, 0, 0,red);
    pVertices[1].set( 0, 0, 0,green);
    pVertices[2].set( 0, 0, 0,blue);
    pVertices[3].set( 4, 0, 0,red);
    pVertices[4].set( 0, 4, 0,green);
    pVertices[5].set( 0, 0, 4,blue);
    pVertices[6].set( 6, 0, 0,red);
    pVertices[7].set( 0, 6, 0,green);
    pVertices[8].set( 0, 0, 6,blue);

    pVertices[9].set(axisLength, 0, 0,red);
    pVertices[10].set( 0,axisLength, 0,green);
    pVertices[11].set( 0, 0,axisLength,blue);
    pVertices[12].set( axisLength-axisWidth, axisHeight, 0,red);
    pVertices[13].set( axisLength-axisWidth,-axisHeight, 0,red);
    pVertices[14].set( 0, axisLength-axisWidth, axisHeight,green);
    pVertices[15].set( 0, axisLength-axisWidth,-axisHeight,green);
    pVertices[16].set( 0, axisHeight, axisLength-axisWidth,blue);
    pVertices[17].set( 0,-axisHeight, axisLength-axisWidth,blue);

    pVertices[18].set( 2, 2, 0,red);
    pVertices[19].set( 2, 2, 0,green);
    pVertices[20].set( 0, 2, 2,green);
    pVertices[21].set( 0, 2, 2,blue);
    pVertices[22].set( 2, 0, 2,red);
    pVertices[23].set( 2, 0, 2,blue);

    pVertices[24].set( 3, 3, 0,red);
    pVertices[25].set( 3, 3, 0,green);
    pVertices[26].set( 0, 3, 3,blue);
    pVertices[27].set( 0, 3, 3,green);
    pVertices[28].set( 3, 0, 3,red);
    pVertices[29].set( 3, 0, 3,blue);
    m_pVBScaleAxes->Unlock();    

    WORD* indices = NULL;
    m_pDevice->CreateIndexBuffer( 42*sizeof(WORD),
        D3DUSAGE_WRITEONLY,
        D3DFMT_INDEX16,
        D3DPOOL_MANAGED,
        &m_pIBScaleAxes,
        NULL);
    m_pIBScaleAxes->Lock(0,0, (void**)&indices, 0);
    indices[0] = 0; indices[1] = 9; 
    indices[2] = 9; indices[3] = 12; 
    indices[4] = 9; indices[5] = 13; 

    indices[6] = 1; indices[7] = 10; 
    indices[8] = 10; indices[9] = 14; 
    indices[10] = 10; indices[11] = 15; 

    indices[12] = 2; indices[13] = 11; 
    indices[14] = 11; indices[15] = 16; 
    indices[16] = 11; indices[17] = 17; 

    indices[18] = 3; indices[19] =18; indices[20] = 3; indices[21] =22; 
    indices[22] = 6; indices[23] =24; indices[24] = 6; indices[25] =28; 

    indices[26] = 4; indices[27] =19; indices[28] = 4; indices[29] =20; 
    indices[30] = 7; indices[31] =25; indices[32] = 7; indices[33] =27; 

    indices[34] = 5; indices[35] =21; indices[36] = 5; indices[37] =23; 
    indices[38] = 8; indices[39] =26; indices[40] = 8; indices[41] =29; 
    m_pIBScaleAxes->Unlock();

    //m_pDevice->CreateIndexBuffer( 24*sizeof(WORD),
    //    D3DUSAGE_WRITEONLY,
    //    D3DFMT_INDEX16,
    //    D3DPOOL_MANAGED,
    //    &m_pIBScaleFaces,
    //    NULL);

    //m_pIBScaleFaces->Lock(0,0, (void**)&indices, 0);


    //m_pIBScaleFaces->Unlock();
}

// m_state只应该在这里变化。整个程序的其他地方根据m_state的变化而改变行为
bool CDrawBox::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool result = true;
    switch(uMsg)
    {
    case WM_KEYDOWN:
        switch( wParam )
        {
        case 'c':
        case 'C':
            if( m_state == NOT_STARTED )
            {
                m_state = START_PICK;
                m_height = m_width = m_depth = 0.0f;
            }
            else if ( isCreating() )
                m_state = NOT_STARTED;
            break;
        case 'g':
        case 'G':
            if( m_state == NOT_STARTED && created() )
                m_state = BOX_START_MOVING;
            else if ( isMoving() )
                m_state = NOT_STARTED;
            break;
        case 'r':
        case 'R':
            if( m_state == NOT_STARTED && created() )
                m_state = BOX_START_ROTATING;
            else if ( isRotating() )
                m_state = NOT_STARTED;
            break;
        case 's':
        case 'S':
            if( m_state == NOT_STARTED && created() )
                m_state = BOX_START_SCALING;
            else if ( isScaling() )
                m_state = NOT_STARTED;
            break;
        default:
            break;
        }
        break;
    case WM_KEYUP:
        break;
    case WM_RBUTTONDOWN:
        switch(m_state)
        {
        case START_PICK:
        case POINT1_PICKED:
        case POINT2_PICKED:
            // 创建box时拾取一个点
            if( m_state <= POINT2_PICKED )
                m_state = static_cast<BOX_STATE>((m_state + 1) % (POINT2_PICKED+1) );
            break;
        default:
            break;
        }
    case WM_RBUTTONUP:
        break;
    case WM_LBUTTONDOWN:
        switch(m_state)
        {
        case BOX_START_MOVING:
        case BOX_MOVING:
        case BOX_START_SCALING:
        case BOX_SCALING:
        case BOX_START_ROTATING:
        case BOX_ROTATING:      // 鼠标选取坐标轴
            if( created()  )
            {
                onAxisPick();
                if( m_crtAxis != AXIS_NONE && 
                    (m_state == BOX_START_MOVING || 
                    m_state == BOX_START_ROTATING || 
                    m_state == BOX_START_SCALING) )
                {
                    m_state = static_cast<BOX_STATE>(m_state + 1);
                    result = false;
                }
            }
            break;

        default:
            break;
        }
        break;
    case WM_LBUTTONUP:
        switch(m_state)
        {
        case BOX_MOVING:
        case BOX_SCALING:
        case BOX_ROTATING: 
            m_state = static_cast<BOX_STATE>(m_state - 1);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return result;
}

void CDrawBox::onMoveAxisPick(D3DXVECTOR3 p1, D3DXVECTOR3 p2, 
                              D3DXVECTOR3 p3, float errorDist)
{
    float s = calRenderRatio();

    if( p1.z > 1.0*s && p1.z < 10.0*s && fabs(p1.y) < errorDist*s )
        m_crtAxis = AXIS_Z;
    else if( p1.y > 1.0*s && p1.y < 10.0*s && fabs(p1.z) < errorDist*s )
        m_crtAxis = AXIS_Y;
    else if( p2.x > 1.0*s && p2.x < 10.0*s && fabs(p2.z) < errorDist*s )
        m_crtAxis = AXIS_X;
    else if( p1.y <=1.0*s && p1.y > 0.0 && p1.z <=1.0*s && p1.z > 0.0 )
        m_crtAxis = AXIS_Y | AXIS_Z;
    else if( p2.x <=1.0*s && p2.x > 0.0 && p2.z <=1.0*s && p2.z > 0.0 )
        m_crtAxis = AXIS_X | AXIS_Z;
    else if( p3.x <=1.0*s && p3.x > 0.0 && p3.y <=1.0*s && p3.y > 0.0 )
        m_crtAxis = AXIS_X | AXIS_Y;
    else
        m_crtAxis = AXIS_NONE;
}

void CDrawBox::onScaleAxisPick(D3DXVECTOR3 p1, D3DXVECTOR3 p2, 
                               D3DXVECTOR3 p3, float errorDist)
{
    float s = calRenderRatio();

    if( p1.z > 6.0*s && p1.z < 10.0*s && fabs(p1.y) < errorDist*s )
        m_crtAxis = AXIS_Z;
    else if( p1.y > 6.0*s && p1.y < 10.0*s && fabs(p1.z) < errorDist*s )
        m_crtAxis = AXIS_Y;
    else if( p2.x > 6.0*s && p2.x < 10.0*s && fabs(p2.z) < errorDist*s )
        m_crtAxis = AXIS_X;
    else if( p1.y + p1.z < 6.0*s && p1.y + p1.z >4.0*s )
        m_crtAxis = AXIS_Y | AXIS_Z;
    else if( p2.x + p2.z < 6.0*s && p2.x + p2.z >4.0*s )
        m_crtAxis = AXIS_X | AXIS_Z;
    else if( p3.x + p3.y < 6.0*s && p3.x + p3.y >4.0*s )
        m_crtAxis = AXIS_X | AXIS_Y;
    else if( p1.y < 4.0*s && p1.z < 4.0*s && p2.x < 4.0*s )
        m_crtAxis = AXIS_X | AXIS_Y | AXIS_Z;
}

void CDrawBox::onRotateAxisPick(D3DXVECTOR3 p1, D3DXVECTOR3 p2, 
                                D3DXVECTOR3 p3, float errorDist)
{
    float s = calRenderRatio();

    if( p1.z > 1.0*s && p1.z < 10.0*s && fabs(p1.y) < errorDist*s )
        m_crtAxis = AXIS_Z;
    else if( p1.y > 1.0*s && p1.y < 10.0*s && fabs(p1.z) < errorDist*s )
        m_crtAxis = AXIS_Y;
    else if( p2.x > 1.0*s && p2.x < 10.0*s && fabs(p2.z) < errorDist*s )
        m_crtAxis = AXIS_X;
    else if( p1.y <=1.0*s && p1.y > 0.0 && p1.z <=1.0*s && p1.z > 0.0 )
        m_crtAxis = AXIS_Y | AXIS_Z;
    else if( p2.x <=1.0*s && p2.x > 0.0 && p2.z <=1.0*s && p2.z > 0.0 )
        m_crtAxis = AXIS_X | AXIS_Z;
    else if( p3.x <=1.0*s && p3.x > 0.0 && p3.y <=1.0*s && p3.y > 0.0 )
        m_crtAxis = AXIS_X | AXIS_Y;
    else
        m_crtAxis = AXIS_NONE;
}

// 判断有哪些坐标轴被鼠标拾取。由于移动、旋转和缩放的坐标轴形状
// 并不相同，所以再在onXXXAxisPick中具体判断哪些坐标轴被选中
void CDrawBox::onAxisPick()
{
    const float errorDist = 1.0f;

    Plane pl1 = { m_pos, D3DXVECTOR3(1,0,0) }; // YOZ
    Plane pl2 = { m_pos, D3DXVECTOR3(0,1,0) }; // XOZ
    Plane pl3 = { m_pos, D3DXVECTOR3(0,0,1) }; // XOZ

    D3DXVECTOR3 p1 = calPickPoint(m_ptCursor, pl1);
    p1.x = -p1.x; p1.z = -p1.z;
    D3DXVECTOR3 p2 = calPickPoint(m_ptCursor, pl2);
    p2.x = -p2.x; p2.z = -p2.z;
    D3DXVECTOR3 p3 = calPickPoint(m_ptCursor, pl3);
    p3.x = -p3.x; p3.z = -p3.z;

    p1 = p1 - m_pos;
    p2 = p2 - m_pos;
    p3 = p3 - m_pos;

    if( isMoving() )
        onMoveAxisPick(p1, p2, p3, errorDist);
    else if( isScaling() )
        onScaleAxisPick(p1, p2, p3, errorDist);
    else if( isRotating() )
        onRotateAxisPick(p1, p2, p3, errorDist);

#ifdef _DEBUG
    std::wstringstream stream;
    stream << L"p1=(" << p1.x << "," << p1.y << "," << p1.z << ")";
    stream << L" p2=(" << p2.x << "," << p2.y << "," << p2.z << ")";
    switch(m_crtAxis)
    {
    case AXIS_X:
        stream << "  X selected";
        break;
    case AXIS_Y:
        stream << "  Y selected";
        break;
    case AXIS_Z:
        stream << "  Z selected";
        break;
    case AXIS_X | AXIS_Y:
        stream << " X & Y selected";
        break;
    case AXIS_Z | AXIS_Y:
        stream << " Y & Z selected";
        break;
    case AXIS_X | AXIS_Z:
        stream << " X & Z selected";
        break;
    case AXIS_X | AXIS_Y | AXIS_Z:
        stream << " X & Y & Z selected";
    default:
        break;
    }
    m_debugMsg = stream.str();
#endif
}

// 由鼠标的坐标计算拾取射线
Ray CDrawBox::calPickRay(int x, int y)
{
    float px = 0.0f, py = 0.0f;

    D3DVIEWPORT9 vp;
    m_pDevice->GetViewport(&vp);
    D3DXMATRIX proj;
    m_pDevice->GetTransform( D3DTS_PROJECTION, &proj );
    
    px = ((( 2.0f*(x) ) / vp.Width) - 1.0f) / proj(0, 0);
    py = -(((2.0f*(y) ) / vp.Height) - 1.0f) / proj(1, 1);

    Ray ray;
    ray.origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    ray.direction = D3DXVECTOR3(px, py, 1.0f);

    D3DXMATRIX view, world, worldView;
    m_pDevice->GetTransform( D3DTS_VIEW, &view );
    m_pDevice->GetTransform( D3DTS_WORLD, &world );
    worldView = world * view;
    D3DXMATRIX mInverse;
    D3DXMatrixInverse( &mInverse, NULL, &worldView );

    D3DXVec3TransformCoord( &ray.origin, &ray.origin, &mInverse);
    D3DXVec3TransformNormal( &ray.direction, &ray.direction, &mInverse);
    D3DXVec3Normalize( &ray.direction, &ray.direction );

    return ray;
}

D3DXVECTOR3 CDrawBox::calPickPoint(POINT cursor, Plane pl)
{
    return calPickPoint(cursor.x, cursor.y, pl);
}

// 由鼠标的坐标确定该点在平面pl上拾取的坐标
D3DXVECTOR3 CDrawBox::calPickPoint(int x, int y, Plane pl)
{
    Ray ray = calPickRay(x, y);

    D3DXVECTOR3 temp = (pl.point - ray.origin);
    float t = D3DXVec3Dot( &temp, &pl.normal ) / 
        D3DXVec3Dot( &ray.direction, &pl.normal );
    temp = ray.origin + t*ray.direction;
    return D3DXVECTOR3(-temp.x, temp.y, -temp.z);
}

// 给定两个点，画出矩形
void CDrawBox::render1(D3DXVECTOR3 p1, D3DXVECTOR3 p2)
{
    float tempWidth = fabs(p1.x - p2.x);
    float tempDepth = fabs(p1.z - p2.z);
    D3DXMATRIX scale, translate, origin;
    D3DXMatrixScaling(&scale, 
        tempWidth / 2.0f, 
        1.0f, 
        tempDepth / 2.0f);
    D3DXMatrixTranslation(&translate,
        -(p1.x + p2.x)/2.0f,
        p1.y - 1.0f,
        -(p1.z + p2.z)/2.0f);

    m_pDevice->GetTransform(D3DTS_WORLD, &origin);
    
    translate = scale * translate * origin;
    m_pDevice->SetTransform(D3DTS_WORLD, &translate);

    m_pDevice->SetStreamSource( 0, m_pVBBox, 0, sizeof( DRAWBOX_CUSTOMVERTEX ) );
    m_pDevice->SetIndices( m_pIBBox );
    m_pDevice->DrawIndexedPrimitive(D3DPT_LINELIST,0, 0, 8, 0, 4);

    m_pDevice->SetTransform(D3DTS_WORLD, &origin);

}

// 由底面的对角两点和高度画出盒子
void CDrawBox::render2(D3DXVECTOR3 p1, D3DXVECTOR3 p2, float height)
{
    float tempWidth = p1.x - p2.x;
    float tempHeight = height;
    float tempDepth = p1.z - p2.z;
    D3DXMATRIX scale, translate, origin;
    D3DXMatrixScaling(&scale, 
        tempWidth / 2.0f, 
        tempHeight/ 2.0f,
        tempDepth / 2.0f);
    D3DXMatrixTranslation(&translate,
        -(p1.x + p2.x) / 2.0f,
        p1.y + height / 2.0f,
        -(p1.z + p2.z) / 2.0f);

    m_pDevice->GetTransform(D3DTS_WORLD, &origin);
    
    translate = scale * translate * origin;
    m_pDevice->SetTransform(D3DTS_WORLD, &translate);

    m_pDevice->SetStreamSource( 0, m_pVBBox, 0, sizeof( DRAWBOX_CUSTOMVERTEX ) );
    m_pDevice->SetIndices( m_pIBBox );
    m_pDevice->DrawIndexedPrimitive(D3DPT_LINELIST,0, 0, 8, 0, 12);

    m_pDevice->SetTransform(D3DTS_WORLD, &origin);
}

// 由长宽高渲染盒子
void CDrawBox::renderBox(float width, float height, float depth)
{
    if( !created() )
        return;

    D3DXMATRIX rotate, scale, translate, origin;
    D3DXMatrixScaling( &scale, 
        width / 2.0f * (m_scale.x+1.0f), 
        height / 2.0f * (m_scale.y+1.0f), 
        depth / 2.0f * (m_scale.z+1.0f) );
    D3DXMatrixRotationYawPitchRoll( &rotate,
        m_oldRotate.y + m_rotate.y,     // TODO: x和y轴是反的？？？
        m_oldRotate.x + m_rotate.x,
        m_oldRotate.z + m_rotate.z );
    D3DXMatrixTranslation( &translate, 
        m_pos.x + m_offSet.x * (m_crtAxis & AXIS_X ? 1 : 0), 
        m_pos.y + m_offSet.y * (m_crtAxis & AXIS_Y ? 1 : 0), 
        m_pos.z + m_offSet.z * (m_crtAxis & AXIS_Z ? 1 : 0) );
    m_pDevice->GetTransform( D3DTS_WORLD, &origin );
    translate = rotate * scale * translate * origin;

    m_pDevice->SetTransform( D3DTS_WORLD, &translate );
    m_pDevice->SetStreamSource( 0, m_pVBBox, 0, sizeof( DRAWBOX_CUSTOMVERTEX ) );
    m_pDevice->SetIndices( m_pIBBox );
    m_pDevice->DrawIndexedPrimitive( D3DPT_LINELIST,
        0, 0, 8, 0, 12 );

    m_pDevice->SetTransform( D3DTS_WORLD, &origin );
}

// 处理状态变化引发的行为
void CDrawBox::processState()
{
    static BOX_STATE oldState = NOT_STARTED;
    static POINT ptStartCursor;
    static float startY = 0;

    {
        D3DXVECTOR3 point0;
        point0.x = min(point1.x, point2.x);
        point0.y = 0;
        point0.z = min(point1.z, point2.z);

        Plane pl1 = { D3DXVECTOR3(0,0,0), D3DXVECTOR3(0,1,0) }; // zox面

        switch(m_state)
        {
        case NOT_STARTED:
            if( oldState == POINT2_PICKED )
            {
                // 刚创建完长方体。计算长方体的尺寸和位置
                Plane pl2 = { point0, D3DXVECTOR3(1,0,0) }; // zoy面
                m_height = calPickPoint( m_ptCursor.x, m_ptCursor.y, pl2).y - startY;
                m_width = max(point1.x - point0.x, point2.x - point0.x);
                m_depth = max(point1.z - point0.z, point2.z - point0.z);

                m_pos = D3DXVECTOR3(
                    -( point1.x + point2.x ) / 2.0f,
                    m_height/2.0f, 
                    -( point1.z + point2.z ) / 2.0f );
                
                if( m_cbOnCreate )
                    m_cbOnCreate( D3DXVECTOR3(m_width, m_height, m_depth), m_oldRotate );
                if( m_cbOnSizeChange )
                    m_cbOnSizeChange( D3DXVECTOR3(m_width, m_height, m_depth), m_oldRotate );
            }
            renderBox(m_width, m_height, m_depth);
            break;
        case BOX_START_MOVING:
            if( oldState == BOX_MOVING )
            {
                m_crtAxis = AXIS_NONE;

                // 移动操作完毕，更新盒子的位置
                m_pos += m_offSet;
                m_offSet.x = m_offSet.y = m_offSet.z = 0.f;

                if( m_cbOnPosChange )
                    m_cbOnPosChange( D3DXVECTOR3(m_width, m_height, m_depth), m_oldRotate );
            }
        case BOX_MOVING:
            moveBox();
            renderBox( m_width, m_height, m_depth );
            renderMoveAxes();
            break;
        case BOX_START_ROTATING:
            if( oldState == BOX_ROTATING )
            {
                m_crtAxis = AXIS_NONE;

                // 完成一次旋转，记录下旋转的角度
                m_oldRotate += m_rotate;
                m_rotate.x = m_rotate.y = m_rotate.z = 0.0f;

                if( m_cbOnRotateChange )
                    m_cbOnRotateChange( D3DXVECTOR3(m_width, m_height, m_depth), m_oldRotate );
            }
        case BOX_ROTATING:
            rotateBox();
            renderBox(m_width, m_height, m_depth);
            renderRotateAxes();
            break;
        case BOX_START_SCALING:
            if( oldState == BOX_SCALING )
            {
                m_crtAxis = AXIS_NONE;

                // 计算缩放完毕后的新尺寸
                m_width *= fabs( m_scale.x + 1.0f );
                m_height *= fabs( m_scale.y + 1.0f );
                m_depth *= fabs( m_scale.z + 1.0f );
                m_scale.x = m_scale.y = m_scale.z = 0.0f;

                if( m_cbOnSizeChange )
                    m_cbOnSizeChange( D3DXVECTOR3(m_width, m_height, m_depth), m_oldRotate );
            }
        case BOX_SCALING:
            scaleBox();
            renderBox( m_width, m_height, m_depth );
            renderScaleAxes();
            break;
        case POINT1_PICKED:
            if( oldState != START_PICK )
            {
                // 第一个点的位置已记录，在水平面绘制一个长方形
                render1( point1, calPickPoint( m_ptCursor.x, m_ptCursor.y, pl1 ) );
            }
            else
            {
                // 刚点下第一个点，记录第一个点的位置
                point1 = calPickPoint( m_ptCursor.x, m_ptCursor.y, pl1 );
            }
            break;
        case POINT2_PICKED:
            if ( oldState != POINT1_PICKED )
            {
                // 第二个点的位置已知，绘制一个高度随鼠标移动而变化的长方体
                Plane pl2 = { point0, D3DXVECTOR3(1,0,0) }; // zoy面
                float dy = calPickPoint( m_ptCursor.x, m_ptCursor.y, pl2).y - startY;
                render2( point1, point2, dy );
            }
            else
            {
                // 记录第二个点的位置
                ptStartCursor = m_ptCursor;
                point2 = calPickPoint( m_ptCursor.x, m_ptCursor.y, pl1 );

                D3DXVECTOR3 point0;
                point0.x = min( point1.x, point2.x );
                point0.y = 0;
                point0.z = min(point1.z, point2.z);
                Plane pl2 = { point0, D3DXVECTOR3(1,0,0) }; // zoy面
                startY = calPickPoint( m_ptCursor.x, m_ptCursor.y, pl2 ).y;
            }
            break;
        }
    }
    oldState = m_state;
}

// 计算鼠标在三维空间的移动，在缩放、移动、旋转时用到
void CDrawBox::traceMouse(D3DXVECTOR3* offset)
{
    static POINT ptStartCursor;

    // 正在缩放或移动或旋转，同时选中了至少一个轴
    if( !( (isScaling() || isMoving() || isRotating() )
        && (m_crtAxis & AXIS_X || m_crtAxis & AXIS_Y || m_crtAxis & AXIS_Z) ) )
            return;

    if( (m_oldState == BOX_START_MOVING &&
        m_state == BOX_MOVING ) ||
        (m_oldState == BOX_START_ROTATING &&
        m_state == BOX_ROTATING ) ||
        (m_oldState == BOX_START_SCALING &&
        m_state == BOX_SCALING ) )
    {
        ptStartCursor = m_ptCursor;
    }

    Plane pl1 = { m_pos, D3DXVECTOR3(1,0,0) };
    Plane pl2 = { m_pos, D3DXVECTOR3(0,1,0) };
    Plane pl3 = { m_pos, D3DXVECTOR3(0,0,1) };

    D3DXVECTOR3 crt1 = calPickPoint(m_ptCursor, pl1);
    D3DXVECTOR3 crt2 = calPickPoint(m_ptCursor, pl2);
    D3DXVECTOR3 crt3 = calPickPoint(m_ptCursor, pl3);
    
    D3DXVECTOR3 old1 = calPickPoint(ptStartCursor, pl1);
    D3DXVECTOR3 old2 = calPickPoint(ptStartCursor, pl2);
    D3DXVECTOR3 old3 = calPickPoint(ptStartCursor, pl3);

    crt1.x = crt2.y = old1.x = old2.y = 0.f;

    switch(m_crtAxis)
    {
    case AXIS_X:
        offset->x = -(crt2.x - old2.x);
        offset->z = offset->y = 0;
        break;
    case AXIS_Y:
        offset->y = crt3.y - old3.y;
        offset->x = offset->z = 0;
        break;
    case AXIS_Z:
        offset->z = -(crt2 .z - old2.z);
        offset->x = offset->y = 0;
        break;
    case AXIS_X | AXIS_Y:
        offset->x = -(crt3.x - old3.x);
        offset->y = crt3.y - old3.y;
        offset->z = 0.0f;
        break;
    case AXIS_X | AXIS_Z:
        offset->x = -(crt2.x - old2.x);
        offset->z = -(crt2.z - old2.z);
        offset->y = 0.0f;
        break;
    case AXIS_Y | AXIS_Z:
        offset->y = crt1.y - old1.y;
        offset->z = -(crt1.z - old1.z);
        offset->x = 0.0f;
        break;
    }

}

void CDrawBox::moveBox()
{
    traceMouse(&m_offSet);

#ifdef _DEBUG
    std::wstringstream stream;
    stream << L"moving offSet=(" << m_offSet.x << "," << m_offSet.y << "," << m_offSet.z << ")";
    
    if( m_crtAxis )
    {
        if(m_crtAxis & AXIS_X )
            stream << " X";
        if(m_crtAxis & AXIS_Y )
            stream << " Y";
        if(m_crtAxis & AXIS_Z )
            stream << " Z";
        stream << " axis selected";
    }
    
    m_debugMsg = stream.str();
#endif
}

void CDrawBox::scaleBox()
{
    traceMouse(&m_scale);

#ifdef _DEBUG
    std::wstringstream stream;
    stream << L"scaling offSet=(" << m_scale.x << "," << m_scale.y << "," << m_scale.z << ")";
    
    if( m_crtAxis )
    {
        if(m_crtAxis & AXIS_X )
            stream << " X";
        if(m_crtAxis & AXIS_Y )
            stream << " Y";
        if(m_crtAxis & AXIS_Z )
            stream << " Z";
        stream << " axis selected";
    }
    
    m_debugMsg = stream.str();
#endif
}

void CDrawBox::rotateBox()
{
    traceMouse(&m_rotate);

#ifdef _DEBUG
    std::wstringstream stream;
    stream << L"rotating offSet=(" << m_rotate.x << "," << m_rotate.y << "," << m_rotate.z << ")";
    
    if( m_crtAxis )
    {
        if(m_crtAxis & AXIS_X )
            stream << " X";
        if(m_crtAxis & AXIS_Y )
            stream << " Y";
        if(m_crtAxis & AXIS_Z )
            stream << " Z";
        stream << " axis selected";
    }
    
    m_debugMsg = stream.str();
#endif
}

void CDrawBox::OnRender(float elapsedTime)
{
    // 获取鼠标在窗口坐标系下的位置
    GetCursorPos( &m_ptCursor );
    ScreenToClient( DXUTGetHWND(), &m_ptCursor );

    processState();

    m_ptLastCursor = m_ptCursor;
    m_oldState = m_state;

#ifdef _DEBUG
    RECT fontRect;
    SetRect(&fontRect, 10, 10, 610, 110);

    m_pFont->DrawText( NULL, m_debugMsg.c_str(), -1, &fontRect, 
        DT_LEFT , 0xFFFF0000 );
#endif
}

// 计算视点的位置，函数的结果用于缩放坐标轴，使坐标轴的大小不随
// 视点的移动而变化
float CDrawBox::calRenderRatio()
{
    const D3DXMATRIX matWorld = *m_camera->GetWorldMatrix();
    const D3DXMATRIX matView = *m_camera->GetViewMatrix();
    D3DXMATRIX matWorldView = matWorld * matView;
    D3DXMATRIX m;
    D3DXMatrixInverse(&m, NULL, &matWorldView);
    D3DXVECTOR3 eyePos;
    eyePos.x = m._41;
    eyePos.y = m._42;
    eyePos.z = m._43;

    return D3DXVec3Length(&eyePos) * m_axeLength;
}

void CDrawBox::renderMoveAxes()
{
    D3DXMATRIX origin, translate, scale, world;

    D3DXMatrixTranslation(&translate, 
        m_pos.x + m_offSet.x * (m_crtAxis & AXIS_X ? 1 : 0), 
        m_pos.y + m_offSet.y * (m_crtAxis & AXIS_Y ? 1 : 0), 
        m_pos.z + m_offSet.z * (m_crtAxis & AXIS_Z ? 1 : 0));

    float s = calRenderRatio();
    D3DXMatrixScaling(&scale, s, s, s);

    m_pDevice->GetTransform(D3DTS_WORLD, &origin);

    world = scale * translate * origin;
    m_pDevice->SetTransform(D3DTS_WORLD, &world);
    m_pDevice->SetStreamSource( 0, m_pVBMoveAxes, 0, sizeof( DRAWBOX_CUSTOMVERTEX ) );
    m_pDevice->SetFVF( D3DFVF_DRAWBOX_CUSTOMVERTEX );
    m_pDevice->DrawPrimitive( D3DPT_LINELIST , 0, 15 );
    m_pDevice->SetTransform(D3DTS_WORLD, &origin);
}

void CDrawBox::renderScaleAxes()
{
    D3DXMATRIX translate, origin, scale, world;

    D3DXMatrixTranslation(&translate, 
        m_pos.x + m_offSet.x * (m_crtAxis & AXIS_X ? 1 : 0), 
        m_pos.y + m_offSet.y * (m_crtAxis & AXIS_Y ? 1 : 0), 
        m_pos.z + m_offSet.z * (m_crtAxis & AXIS_Z ? 1 : 0));

    float s = calRenderRatio();
    D3DXMatrixScaling(&scale, s, s, s);

    m_pDevice->GetTransform(D3DTS_WORLD, &origin);
    world = scale * translate * origin;

    m_pDevice->SetTransform(D3DTS_WORLD, &world);
    m_pDevice->SetStreamSource( 0, m_pVBScaleAxes, 0, sizeof( DRAWBOX_CUSTOMVERTEX ) );
    m_pDevice->SetIndices( m_pIBScaleAxes );
    m_pDevice->DrawIndexedPrimitive(D3DPT_LINELIST,
        0, 0, 8, 0, 21);

    m_pDevice->SetTransform(D3DTS_WORLD, &origin);
}

void CDrawBox::renderRotateAxes()
{
    renderMoveAxes();
}

void CDrawBox::SetWidth(float width)
{
    if( m_state != NOT_STARTED )
        return;
    m_width = width;
}

void CDrawBox::SetHeight(float height)
{
    if( m_state != NOT_STARTED )
        return;
    m_height = height;
}

void CDrawBox::SetDepth(float depth)
{
    if( m_state != NOT_STARTED )
        return;
    m_depth = depth;
}

void CDrawBox::SetSize(D3DXVECTOR3 size)
{
    if( m_state != NOT_STARTED )
        return;

    m_width = size.x;
    m_height = size.y;
    m_depth = size.z;
}

void CDrawBox::SetYawPitchRoll(D3DXVECTOR3 rotate)
{
    if( m_state != NOT_STARTED )
        return;
    m_oldRotate = rotate;
}

void CDrawBox::SetOnCreate(DrawBoxCallback cb)
{
    m_cbOnCreate = cb;
}

void CDrawBox::SetOnSizeChange(DrawBoxCallback cb)
{
    m_cbOnSizeChange = cb;
}

void CDrawBox::SetOnPosChange(DrawBoxCallback cb)
{
    m_cbOnPosChange = cb;
}

void CDrawBox::SetOnRotateChange(DrawBoxCallback cb)
{
    m_cbOnRotateChange = cb;
}
