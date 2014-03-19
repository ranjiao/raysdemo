#pragma once

class IDrawBox
{
public:
    typedef void (*DrawBoxCallback)(D3DXVECTOR3 size, D3DXVECTOR3 rotate);
public:
    virtual ~IDrawBox(){};
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    
    virtual void OnRender(float elapsedTime ) = 0;

    // 设置盒子的颜色
    virtual void SetColor(DWORD color) = 0;

    virtual float GetWidth() = 0;
    virtual float GetHeight() = 0;
    virtual float GetDepth() = 0;
    virtual D3DXVECTOR3 GetSize() = 0;

    virtual void SetWidth(float width) = 0;
    virtual void SetHeight(float height) = 0;
    virtual void SetDepth(float depth) = 0;
    virtual void SetSize(D3DXVECTOR3 size) = 0;

    virtual D3DXVECTOR3 GetYawPitchRoll() = 0;
    virtual void SetYawPitchRoll(D3DXVECTOR3 rotate) = 0;

    virtual void SetOnCreate(DrawBoxCallback cb) = 0;
    virtual void SetOnSizeChange(DrawBoxCallback cb) = 0;
    virtual void SetOnPosChange(DrawBoxCallback cb) = 0;
    virtual void SetOnRotateChange(DrawBoxCallback cb) = 0;
};