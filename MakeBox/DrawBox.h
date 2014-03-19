#pragma once

#include <string>

#include "DXUTcamera.h"
#include "IDrawBox.h"

struct Ray
{
    D3DXVECTOR3 origin;
    D3DXVECTOR3 direction;
};

struct Plane
{
    D3DXVECTOR3 point;
    D3DXVECTOR3 normal;
};

class CDrawBox: public IDrawBox
{
private:
#ifdef _DEBUG
    ID3DXFont* m_pFont;
    std::wstring    m_debugMsg;
#endif
protected:
    LPDIRECT3DVERTEXBUFFER9 m_pVBMoveAxes, m_pVBScaleAxes, m_pVBRotateAxes;
    LPDIRECT3DINDEXBUFFER9  m_pIBScaleAxes, m_pIBScaleFaces;
    LPDIRECT3DINDEXBUFFER9  m_pIBRotateAxes, m_pIBRotateFaces;
    
    LPDIRECT3DVERTEXBUFFER9 m_pVBBox;
    LPDIRECT3DINDEXBUFFER9  m_pIBBox;

    CModelViewerCamera*     m_camera;

    D3DXVECTOR3 m_oldRotate;
    D3DXVECTOR3 m_offSet, m_scale, m_rotate;
    D3DXVECTOR3 m_pos, m_eyePos;
    DWORD       m_color;

    // 由y值相同的两点和一个高度确定这个盒子的大小
    D3DXVECTOR3 point1;
    D3DXVECTOR3 point2;
    float       m_height, m_width, m_depth;
    float       m_axeLength;
    POINT       m_ptCursor, m_ptLastCursor;

    enum AXIS
    {
        AXIS_NONE = 0,
        AXIS_X = 0x01,
        AXIS_Y = 0x02,
        AXIS_Z = 0x04,
    };
    DWORD m_crtAxis;

    enum BOX_STATE
    {
        NOT_STARTED = 0,
        START_PICK,
        POINT1_PICKED,
        POINT2_PICKED,
        BOX_START_MOVING,
        BOX_MOVING,
        BOX_START_ROTATING,
        BOX_ROTATING,
        BOX_START_SCALING,
        BOX_SCALING,
        BOX_STATE_COUNT,
    } m_state, m_oldState;

    IDirect3DDevice9* m_pDevice;

    DrawBoxCallback m_cbOnCreate;
    DrawBoxCallback m_cbOnSizeChange;
    DrawBoxCallback m_cbOnRotateChange;
    DrawBoxCallback m_cbOnPosChange;
protected:
    Ray         calPickRay(int x, int y);
    D3DXVECTOR3 calPickPoint(int x, int y, Plane p1);
    D3DXVECTOR3 calPickPoint(POINT cursor, Plane p1);

protected:
    void createMoveAxes();
    void createScaleAxes();
    void createRotateAxes();
    void createBox();

    void render1(D3DXVECTOR3 p1, D3DXVECTOR3 p2);
    void render2(D3DXVECTOR3 p1, D3DXVECTOR3 p2, float m_height);
    void renderBox(float widht, float height, float depth);    
    float calRenderRatio();
    void renderMoveAxes();
    void renderScaleAxes();
    void renderRotateAxes();
    
    void onAxisPick();              // 判断坐标轴是否被拾取
    void onMoveAxisPick(D3DXVECTOR3 p1, D3DXVECTOR3 p2, D3DXVECTOR3 p3, float errorDist);
    void onScaleAxisPick(D3DXVECTOR3 p1, D3DXVECTOR3 p2, D3DXVECTOR3 p3, float errorDist);
    void onRotateAxisPick(D3DXVECTOR3 p1, D3DXVECTOR3 p2, D3DXVECTOR3 p3, float errorDist);

    void processState();            // 处理不同状态下的逻辑
    void traceMouse(D3DXVECTOR3* offset);
    void moveBox();
    void scaleBox();
    void rotateBox();

    bool created(){ return m_height != 0; };
    bool isCreating(){ return m_state >= START_PICK && m_state <= POINT2_PICKED; };
    bool isMoving(){ return m_state >= BOX_START_MOVING && m_state <= BOX_MOVING; };
    bool isRotating(){ return m_state >= BOX_START_ROTATING && m_state <= BOX_ROTATING; };
    bool isScaling(){ return m_state >= BOX_START_SCALING && m_state <= BOX_SCALING; };
public:
    CDrawBox(IDirect3DDevice9* pd3dDevice, CModelViewerCamera* camera);
    ~CDrawBox(void);

    bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void OnRender(float elapsedTime );

    // 设置盒子的颜色
    void SetColor(DWORD color)      { m_color = color; };

    float GetWidth(){ return m_width; };
    float GetHeight(){ return m_height; };
    float GetDepth(){ return m_depth; };
    D3DXVECTOR3 GetSize(){ return D3DXVECTOR3(m_width, m_height, m_depth); };

    void SetWidth(float width);
    void SetHeight(float height);
    void SetDepth(float depth);
    void SetSize(D3DXVECTOR3 size);

    D3DXVECTOR3 GetYawPitchRoll(){ return m_oldRotate;};
    void SetYawPitchRoll(D3DXVECTOR3 rotate);

    // 回调函数
    void SetOnCreate(DrawBoxCallback cb);
    void SetOnSizeChange(DrawBoxCallback cb);
    void SetOnPosChange(DrawBoxCallback cb);
    void SetOnRotateChange(DrawBoxCallback cb);
};
