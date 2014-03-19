#pragma once

#include <string>
#include <map>
#include <queue>

struct PicInfo
{
    int                     width, height;
    float                   scaleX, scaleY;
    std::wstring            filename;
    size_t                  size;

    LPDIRECT3DTEXTURE9      texture;
    bool                    saved;
    bool                    loaded;
    std::wstring            strSize;
    std::wstring            strRect;

    std::wstring GetSizeString();
    std::wstring GetRectString();

    PicInfo()
    {
        saved = false;
        filename = strSize = L"";
    }
};

typedef std::map<std::wstring, PicInfo>             INFO_MAP;
typedef std::map<std::wstring, PicInfo>::iterator   INFO_MAP_ITER;

class PicViewer
{
private:
    friend DWORD WINAPI loadAllPictures(LPVOID param);
    friend DWORD WINAPI saveAllPictures(LPVOID param);
    int m_winWidth, m_winHeight;
    int m_thumbWidth, m_thumbHeight;
    int m_gapX, m_gapY;
    int m_textHeight;
    bool m_isOK;
private:
    // directx variables
    ID3DXSprite*        m_sprite;
    IDirect3DDevice9*   m_pDevice;
    ID3DXFont*          m_pFont, *m_pFont2;

    // thread variables
    HANDLE              m_hInfoMutex, m_hD3dxMutex; 
    HANDLE              m_hSemaphore;
    HANDLE              m_hLoadThread, m_hSaveThread;

    //WORKLIST            m_worklist;
private:
    void cleanUp();
    bool fileFilter(std::wstring filename);
    bool listFiles(std::wstring path);

    static void loadTexture(PicInfo& info, std::wstring, int thumbWidth, 
        int thumbHeight, IDirect3DDevice9* pDevice, HANDLE hD3dxMutex);

    // save thumbnails
    static bool saveTexture(PicInfo& info, std::wstring folder);

    bool findThumbnail(PicInfo& info);

    static bool checkReturnValue(HRESULT result);
protected:
    INFO_MAP m_pics;
    std::wstring m_path;
protected:
    void draw();
public:
    PicViewer(std::wstring path=L"",
        int winWidth = 600, int winHeight = 800);
    ~PicViewer(void);

    void Create(LPDIRECT3DDEVICE9 pDevice);
    void Destroy();

    void LoadPictureFolder(std::wstring path);

    void OnDraw(float fElapsedTime);
    void OnResetDevice(LPDIRECT3DDEVICE9 pDevice);
    void OnLostDevice();
};
