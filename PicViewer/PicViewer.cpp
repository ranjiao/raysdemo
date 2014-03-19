#include "DXUT.h"
#include "PicViewer.h"
#include <windows.h>
#include <sstream>
#include <fstream>

// NOTE: 1. 现在接受的输入路径为 c:/path/to/file 形式
//       2. 略缩图的长宽必须相等

using namespace std;

static HRESULT result;

const wchar_t thumbDir[] = L".thumbnail";
const int MAX_QUEUE_NUM = 100;
const int MAX_BUF_LENGTH = 2048;

PicViewer::PicViewer(wstring path, int winWidth, int winHeight):
m_path(path), m_pDevice(NULL), m_isOK(false),
m_thumbWidth(150), m_thumbHeight(150), m_gapX(10), m_gapY(10), m_textHeight(15),
m_winWidth(winWidth), m_winHeight(winHeight),
m_sprite(NULL), m_pFont(NULL), m_pFont2(NULL), m_hLoadThread(NULL), m_hSaveThread(NULL)
{
    // mutex object
    m_hInfoMutex = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    m_hD3dxMutex = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    assert(m_hInfoMutex);
    assert(m_hSemaphore);
}

void PicViewer::Create(IDirect3DDevice9* pDevice)
{
    m_isOK = true;
    m_pDevice = pDevice;

    // create sprite
    // FIXME: memory leak here. Is the window is resized then the calling 
    // below will cause memory leak, looks like D3DX's fault
    D3DXCreateSprite(m_pDevice, &m_sprite);  

    // create directx fonts
    HDC hDC = GetDC( NULL );
    int nLogPixelsY = GetDeviceCaps( hDC, LOGPIXELSY );
    ReleaseDC( NULL, hDC );
    
    int nHeight = -MulDiv( 8, nLogPixelsY, 72 );
    result = D3DXCreateFont( m_pDevice, nHeight, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
                     OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                     L"Arial", &m_pFont );
    result = D3DXCreateFont( m_pDevice, 6, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                     OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                     L"System", &m_pFont2 );
}

PicViewer::~PicViewer(void)
{
    Destroy();

    CloseHandle(m_hInfoMutex);
}

void PicViewer::Destroy()
{
    SAFE_RELEASE( m_sprite );
    SAFE_RELEASE( m_pFont );
    SAFE_RELEASE( m_pFont2 );

    cleanUp();
}

bool PicViewer::listFiles(wstring path)
{
    bool result = true;

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError;
    wstring DirSpec = path + L"*";

    // Find the first file in the directory.
    hFind = FindFirstFile(DirSpec.c_str(), (LPWIN32_FIND_DATAW)&FindFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        MessageBoxA(NULL, "Error while listing files", "Error", MB_OK);
        result = false;
    } 
    else 
    {
        // List all the other files in the directory.
        while (FindNextFile(hFind, &FindFileData) != 0) 
        {
            if( fileFilter(FindFileData.cFileName) )
            {
                //// already in the thumbnail db, skip
                //if ( m_pics.find( FindFileData.cFileName ) != m_pics.end() )
                //    continue;

                PicInfo info;
                info.filename = FindFileData.cFileName;
                info.size = FindFileData.nFileSizeLow;
                info.texture = NULL;
                info.loaded = false;
                
                // check if there's already a thumbnail
                findThumbnail(info);
                m_pics[info.filename] = info;
            }
        }

        dwError = GetLastError();
        FindClose(hFind);
        if (dwError != ERROR_NO_MORE_FILES) 
        {
            MessageBoxA(NULL, "Error while listing files", "Error", MB_OK);
            result = false;
        }
    }
    return result;
}

DWORD WINAPI loadAllPictures(LPVOID param);
DWORD WINAPI saveAllPictures(LPVOID param);

void PicViewer::LoadPictureFolder(std::wstring path)
{
    // 如果长宽不相等，则需要修改下面对于scaleX和scaleY的计算
    assert( m_thumbWidth == m_thumbHeight );
    assert( !path.empty() );

    wstring strTemp = path.substr(path.length(), path.length()-1);
    if( strTemp.compare(L"/") )
        path.append(L"/");

    m_path = path;
    
    cleanUp();

    if( !listFiles(path) )
        return;

    m_hLoadThread = CreateThread(NULL, 0, loadAllPictures, this, 0, NULL); 

    assert( m_hLoadThread );
}

DWORD WINAPI loadAllPictures(LPVOID param)
{
    INFO_MAP_ITER iter;
    DWORD dwWaitResult; 
    HANDLE hDir;
    assert(param);
    PicViewer* viewer = reinterpret_cast<PicViewer*>(param);

    // TODO: lock these line ?
    int thumbWidth = viewer->m_thumbWidth;
    int thumbHeight = viewer->m_thumbHeight;
    IDirect3DDevice9* pDevice = viewer->m_pDevice;

    wstring path = viewer->m_path; 
    wstring dirName = path + thumbDir;


    // test if the directory already exists
    hDir = CreateFile(dirName.c_str(), NULL, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if ( hDir == INVALID_HANDLE_VALUE )
    {
        // create it if not
        result = CreateDirectory(dirName.c_str(), NULL);
        if( result == NULL )
            MessageBox(NULL, L"Error while creating thunbnail directory", L"error", MB_OK);
    }
    else
        CloseHandle(hDir);

    iter = viewer->m_pics.begin();
    while(iter != viewer->m_pics.end())
    {
        PicInfo info = iter->second;

        viewer->loadTexture(info, path, thumbWidth, thumbHeight, 
            pDevice, viewer->m_hD3dxMutex);

        // save the thumbnail
        if( !info.saved )
            viewer->saveTexture( info, path );

        // save the info
        dwWaitResult = WaitForSingleObject( viewer->m_hInfoMutex, 2000 );
        switch (dwWaitResult) 
        {
            // The thread got ownership of the mutex
            case WAIT_OBJECT_0: 
                iter->second = info;
                iter++;
                ReleaseMutex(viewer->m_hInfoMutex);
                break; 

            // The thread got ownership of an abandoned mutex
            case WAIT_ABANDONED: 
                return 1; 
        }
    }
    return 0;
}

void PicViewer::loadTexture(PicInfo& info, wstring path, int thumbWidth,
                            int thumbHeight, IDirect3DDevice9* pDevice,
                            HANDLE hD3dxMutex)
{
    HRESULT result;
    LPDIRECT3DTEXTURE9 pTexture;
    D3DXIMAGE_INFO imageInfo;
    int width, height;
    wstring filename;

    assert( !path.empty() );
    assert( thumbWidth );
    assert( thumbHeight );
    assert( pDevice );
    assert( hD3dxMutex );
        
    // 计算纹理的尺寸
    filename = path + info.filename;
    D3DXGetImageInfoFromFile( filename.c_str(), &imageInfo);

    if(imageInfo.Width > imageInfo.Height)
    {
        width = thumbWidth;
        height = static_cast<int>( thumbHeight * imageInfo.Height / (float)imageInfo.Width );
    }
    else
    {
        height = thumbHeight;
        width = static_cast<int>(  thumbWidth * imageInfo.Width / (float)imageInfo.Height );
    }

    // info中保存的仍然是原始图片的大小
    info.width = imageInfo.Width;
    info.height = imageInfo.Height;

    // 如果已有略缩图，则读取略缩图
    if(info.saved)
        filename = path + thumbDir + L"/" + info.filename;

    DWORD dwWaitResult = WaitForSingleObject( hD3dxMutex, INFINITE );
        if( dwWaitResult == WAIT_ABANDONED )
            return;
        result = D3DXCreateTextureFromFileEx(
          pDevice,
          filename.c_str(),
          width,
          height,
          D3DX_DEFAULT,     // mips level
          D3DUSAGE_DYNAMIC, // usage
          D3DFMT_UNKNOWN,   // format
          D3DPOOL_DEFAULT,  // pool
          D3DX_DEFAULT,     // filter
          D3DX_DEFAULT,     // mip filter
          NULL,             // transparent color
          NULL,             // info
          NULL,
          &pTexture
        );
    ReleaseMutex( hD3dxMutex );

    if(info.width > info.height)
    {
        info.scaleX = 1.0;
        info.scaleY = (float)info.height / (float)info.width ;

    }
    else
    {
        info.scaleX = (float)info.width / (float)info.height;
        info.scaleY = 1.0;
    }

    if ( checkReturnValue(result) )
    {
        info.texture = pTexture;
        info.loaded = true;
    }
    else
    {
        info.texture = NULL;
    }
}

bool PicViewer::saveTexture(PicInfo& info, wstring folder)
{
    assert( !folder.empty() );

    bool done = false;
    wstring thumbFilename = folder + thumbDir + L"/" + info.filename;

    result = D3DXSaveTextureToFile(
        thumbFilename.c_str(),
        D3DXIFF_JPG,
        info.texture,
        NULL
    );
    done = result== D3D_OK;
    if(done)
        info.saved = true;

    return done;
}

bool PicViewer::checkReturnValue(HRESULT result)
{
    switch(result)
    {
    case D3D_OK:
        return true;
        break;
    case D3DERR_NOTAVAILABLE:
        MessageBoxA(NULL, "Not avaliable", "Error while loading texture", MB_OK);
        return false;
        break;
    case D3DERR_OUTOFVIDEOMEMORY:
        MessageBoxA(NULL, "Out of video memory", "Error while loading texture", MB_OK);
        return false;
        break;
    case D3DERR_INVALIDCALL:
        MessageBoxA(NULL, "Invalid call", "Error while loading texture", MB_OK);
        return false;
        break;
    case D3DXERR_INVALIDDATA:
        MessageBoxA(NULL, "Invalid data", "Error while loading texture", MB_OK);
        return false;
        break;
    default:
        MessageBoxA(NULL, "Unknow error", "Error while loading texture", MB_OK);
        return false;
        break;
    }
    
}

bool PicViewer::fileFilter(wstring filename)
{
    assert( !filename.empty() );

    const int extCount = 9;
    const wchar_t ext[extCount][5] = { 
        L".bmp", L".dds", L".dib", 
        L".hdr", L".jpg", L".pfm", 
        L".png", L".ppm", L".tga" };

    if (filename.length() < 4)
        return false;
    wstring fileExt = filename.substr( filename.length() - 4, 4);
    
    bool result = false;
    for(int i=0; i<extCount; i++)
    {
        if( _wcsnicmp(fileExt.c_str(), ext[i], 1024) == 0 )
        {
            result = true;
            break;
        }
    }
    return result;
}

void PicViewer::cleanUp()
{
    INFO_MAP_ITER iter;
    for(iter = m_pics.begin();
        iter != m_pics.end();
        iter++)
    {
        SAFE_RELEASE(iter->second.texture);
    }

    m_pics.clear();
}

void PicViewer::OnDraw(float fElapsedTime)
{
    if( !m_isOK )
        return;
    draw();
}

void PicViewer::draw()
{
    HRESULT result;
    D3DXVECTOR3 center = D3DXVECTOR3(0.f,0.f,0.f);
    D3DXVECTOR3 pos = D3DXVECTOR3((float)m_gapX, (float)m_gapY, 0.f);
    D3DXVECTOR3 realPos;
    RECT fontRect;

    if( !m_sprite )
        return;

    m_sprite->Begin(D3DXSPRITE_DONOTSAVESTATE);

    INFO_MAP_ITER iter = m_pics.begin();
    while(iter != m_pics.end())
    {
        PicInfo info;

        if( pos.x + m_thumbWidth + m_gapX > m_winWidth )
        {   // 换行
            pos.x = (float) m_gapX;
            pos.y += (float) m_thumbHeight + m_gapY + m_textHeight * 2;
        }

        // 当图片宽高比不为1时，将图片移动到中央显示
        realPos.x = pos.x + (1.0f-iter->second.scaleX)*m_thumbWidth / 2.0f;
        realPos.y = pos.y + (1.0f-iter->second.scaleY)*m_thumbHeight / 2.0f;
        realPos.z = 0.0f;

        // mutex
        WaitForSingleObject( m_hInfoMutex, INFINITE );
            info = iter->second;
            iter++;
        ReleaseMutex( m_hInfoMutex );

        // draw the texture
        if(info.texture)
            result = m_sprite->Draw(info.texture, NULL, NULL, &realPos, 0xFFFFFFFF);

        // 标题
        SetRect(&fontRect, (int)pos.x, (int)pos.y + m_thumbHeight, 
            (int)pos.x + m_thumbWidth, (int)pos.y + m_thumbHeight + m_textHeight);

        m_pFont2->DrawText( NULL, info.filename.c_str(), -1, &fontRect, 
            DT_CENTER , 0xFF000000 );

        // 文件大小
        SetRect(&fontRect, (int)pos.x, (int)pos.y + m_thumbHeight + m_textHeight, 
            (int)pos.x + m_thumbWidth, (int)pos.y + m_thumbHeight + m_textHeight*2);

        m_pFont->DrawText( NULL, 
            (info.GetSizeString() + L"  " + info.GetRectString()).c_str(), 
            -1, &fontRect, 
            DT_CENTER , 0xFF000000 );

        pos += D3DXVECTOR3( (float)m_thumbWidth + m_gapX, 0, 0);
    }

    m_sprite->End();
}

void PicViewer::OnResetDevice(LPDIRECT3DDEVICE9 pDevice)
{
    m_pDevice = pDevice;
    m_sprite->OnResetDevice();
    m_pFont->OnResetDevice();
    m_pFont2->OnResetDevice();
}

void PicViewer::OnLostDevice()
{
    if( m_pFont )
        m_pFont->OnLostDevice();
    if( m_pFont2 )
        m_pFont2->OnLostDevice();
}

// 检查是否存在对应的略缩图
// TODO: 还要处理当图片更新时，相应更新略缩图的情况
bool PicViewer::findThumbnail(PicInfo& info)
{
    wstring thumbFilename;
    HANDLE hDir;

    thumbFilename = m_path + thumbDir + L"/" + info.filename;

    // test if the directory already exists
    hDir = CreateFile(thumbFilename.c_str(), NULL, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL, NULL);

    if ( hDir == INVALID_HANDLE_VALUE )
    {
        return false;
    }
    CloseHandle(hDir);

    info.saved = true;

    return true;
}

std::wstring PicInfo::GetSizeString()
{
    wstringstream stream;
    
    if( !strSize.empty() )
        return strSize;

    if( size < 2000 )               // size < 2k
    {
        stream << size << "B"; 
    }
    else
    {
        if( size < 2000000 )        // 2k < size < 2M
        {
            stream << (size / 1000) << "KB";
        }
        else                        // 2M < size
        {
            stream << (size / 1000000) << "MB";
        }
    }
    strSize = stream.str();

    return strSize;
}

std::wstring PicInfo::GetRectString()
{
    wstringstream stream;

    if( !loaded )
        return L"loading...";

    if( !strRect.empty() )
        return strRect;

    stream << width << L"x" << height;
    strRect = stream.str();
    loaded = true;
    return strRect;
}
