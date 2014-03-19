#include <wx/wx.h>
#include "FrontEnd.h"

class MyApp: public wxApp
{
public:
    bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();
    FrontEnd* frame_main = new FrontEnd(NULL, wxID_ANY, "VOD System");
    SetTopWindow(frame_main);
    frame_main->Show();
    return true;
}