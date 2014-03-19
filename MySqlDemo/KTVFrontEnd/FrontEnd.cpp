#include "FrontEnd.h"
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

BEGIN_EVENT_TABLE(FrontEnd, wxFrame)
    EVT_IDLE(FrontEnd::OnIdle)
    
    EVT_BUTTON(ID_SINGER_SEARCH, FrontEnd::OnSingerSearch)
    EVT_BUTTON(ID_SONG_SEARCH, FrontEnd::OnSongSearch)
    EVT_BUTTON(ID_SINGER_MALE, FrontEnd::OnSingerMale)
    EVT_BUTTON(ID_SINGER_FEMALE, FrontEnd::OnSingerFemale)
    EVT_BUTTON(ID_SINGER_BAND, FrontEnd::OnSingerBand)
    EVT_BUTTON(ID_SINGER_RETURN, FrontEnd::OnSingerReturn)
    EVT_BUTTON(ID_SONG_RETURN, FrontEnd::OnSongReturn)
    EVT_BUTTON(ID_NEXT_SONG, FrontEnd::OnNextSong)

    EVT_MENU(ID_FILE_EXIT, FrontEnd::OnExit)
    EVT_MENU(ID_HELP_ABOUT, FrontEnd::OnAbout)

    EVT_LISTBOOK_PAGE_CHANGING(wxID_ANY, FrontEnd::OnListbook)

    EVT_LIST_ITEM_SELECTED(ID_LIST_SINGER, FrontEnd::OnSingerSelected)
    EVT_LIST_ITEM_SELECTED(ID_LIST_SONG, FrontEnd::OnSongSelected)
END_EVENT_TABLE()

FrontEnd::FrontEnd(wxWindow* parent, int id, const wxString& title):
    wxFrame(parent, id, title)//, pos, size, wxDEFAULT_FRAME_STYLE)
{
    //_CrtSetBreakAlloc(3446);
    //_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
    ////////////////////////////////////////////////////
    // books creation
    m_panel    = NULL;
    m_bookCtrl = NULL;

    //// create a dummy image list with a few icons
    //const wxSize imageSize(32, 32);

    //m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    //m_imageList->
    //    Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
    //m_imageList->
    //    Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
    //m_imageList->
    //    Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
    //m_imageList->
    //    Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));

    // Panel
    m_panel = new wxPanel(this);

    // Set sizers
    m_sizerFrame = new wxBoxSizer(wxVERTICAL);

    // Create list book
    int flags;
    flags = wxBK_LEFT;
    m_bookCtrl = new wxListbook(m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags);
    wxListView* list = ((wxListbook*)m_bookCtrl)->GetListView();
    list->SetColumnWidth(0, 140);

    if ( !m_bookCtrl )
        assert(0);
    m_bookCtrl->Hide();

    //m_bookCtrl->SetImageList(m_imageList);

    init_pages(m_bookCtrl);

    m_sizerFrame->Add(m_bookCtrl, wxSizerFlags(5).Expand().Border());
    //m_sizerFrame->Insert(0, m_bookCtrl, wxSizerFlags(15).Expand().Border());
    
    m_sizerFrame->Show(m_bookCtrl);
    m_sizerFrame->Layout();
    ////////////////////////////////////////////////////

    m_btnNextSong = new wxButton(m_panel, ID_NEXT_SONG, "Next Song");
    m_sizerFrame->Add(m_btnNextSong, wxSizerFlags(1).Expand());

    m_panel->SetSizer(m_sizerFrame);
    
    m_sizerFrame->SetSizeHints(this);

    InitMenus();
    InitProperties();    

    if(!DBConnect("KTVAdmin", "admin", "localhost", "ktv"))
        wxMessageBox("Cannot connect to the database£¡", "Error");
}

void FrontEnd::InitProperties()
{
    SetSize(wxSize(500,400));

    Centre(wxBOTH);
}

void FrontEnd::InitMenus()
{
    wxMenu* menu_file = new wxMenu;
    menu_file->Append(ID_FILE_EXIT, "Exit");

    wxMenu* menu_help = new wxMenu;
    menu_help->Append(ID_HELP_ABOUT, "About this program");

    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append(menu_file, "File");
    menu_bar->Append(menu_help, "Help");

    SetMenuBar(menu_bar);
}

FrontEnd::~FrontEnd(void)
{

}

void FrontEnd::OnIdle( wxIdleEvent& WXUNUSED(event) )
{
    static int s_nPages = wxNOT_FOUND;
    static int s_nSel = wxNOT_FOUND;
    static wxBookCtrlBase *s_currBook = NULL;

    wxBookCtrlBase *currBook = GetCurrentBook();

    int nPages = currBook ? currBook->GetPageCount() : 0;
    int nSel = currBook ? currBook->GetSelection() : wxNOT_FOUND;

    if ( nPages != s_nPages || nSel != s_nSel || s_currBook != currBook )
    {
        s_nPages = nPages;
        s_nSel = nSel;
        s_currBook = currBook;

        wxString selection;
        if ( nSel == wxNOT_FOUND )
            selection << wxT("Ready...");
        else
            selection << currBook->GetPageText( nSel );

        
        wxString title;
        title.Printf(wxT("VOD System -- %s"), selection.c_str());
        
        SetTitle(title);
    }
}

void FrontEnd::init_pages(wxBookCtrlBase* parent)
{
    wxPanel *panel_singer = create_page_singer_gender(parent);
    parent->AddPage( panel_singer, "Singers", false);

    wxPanel *panel_singer_pinyin = create_page_singer_pinyin(parent, false, &m_listSingerAll);
    parent->AddPage( panel_singer_pinyin, "Singer Pinyin", false);

    wxPanel *panel_song_pinyin = create_page_song_pinyin(parent);
    parent->AddPage( panel_song_pinyin, "Songs Pinyin", false);

    wxPanel *panel_songs_queued = create_page_song_played(parent);
    parent->AddPage( panel_songs_queued, "Songs Played", false);

    wxPanel *panel_songs_played = create_page_song_queued(parent);
    parent->AddPage( panel_songs_played, "Songs Queued", false);

    wxPanel *panel_songs_all = create_page_song_all(parent, false, &m_listSongAll);
    parent->AddPage( panel_songs_all, "All Songs", false);
}

wxPanel* FrontEnd::create_page_singer_gender(wxBookCtrlBase* parent)
{
    wxPanel *panel = new wxPanel(parent);

    wxButton *buttonMale = new wxButton(panel, ID_SINGER_MALE, wxT("MaleSingers"));
    wxButton *buttonFemale = new wxButton(panel, ID_SINGER_FEMALE, wxT("FemaleSingers"));
    wxButton *buttonBand = new wxButton(panel, ID_SINGER_BAND, wxT("Band"));

    wxBoxSizer *sizerPanel = new wxBoxSizer(wxVERTICAL);
    sizerPanel->Add(buttonMale, 1, wxEXPAND);
    sizerPanel->Add(buttonFemale, 1, wxEXPAND);
    sizerPanel->Add(buttonBand, 1, wxEXPAND);
    panel->SetSizer(sizerPanel);

    return panel;
}

wxPanel* FrontEnd::create_page_singer_pinyin(wxBookCtrlBase* parent, bool btnReturn,
                                             wxListCtrl** listCtrl)
{
    wxPanel *panel = new wxPanel(parent);
    assert(_CrtCheckMemory());

    wxPanel *panelSearch = new wxPanel(panel);
    wxPanel *panelResult = create_panel_search_result(panel, listCtrl, ID_LIST_SINGER);

    wxStaticText    *searchLabel = NULL;
    wxButton        *searchBtn = NULL;
    wxButton        *returnBtn = NULL;
    if(!btnReturn)
    {
        searchLabel = new wxStaticText(panelSearch, -1, "Singer:");
        m_textSingerPinyin = new wxTextCtrl(panelSearch, -1);
        searchBtn = new wxButton(panelSearch, ID_SINGER_SEARCH, "Search");
    }
    else
    {
        returnBtn = new wxButton(panelSearch, ID_SINGER_RETURN, "Return");
    }

    wxBoxSizer *sizerPanel = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizerSearch = new wxBoxSizer(wxHORIZONTAL);
    
    sizerPanel->Add(panelSearch, 0, wxEXPAND);
    sizerPanel->Add(panelResult, 1, wxEXPAND);

    if(!btnReturn)
    {
        sizerSearch->Add(searchLabel, 1, wxEXPAND);
        sizerSearch->Add(m_textSingerPinyin, 5, wxEXPAND);
        sizerSearch->Add(searchBtn, 1, wxEXPAND);
    }
    else
        sizerSearch->Add(returnBtn, 1, wxEXPAND);

    panel->SetSizer(sizerPanel);
    panelSearch->SetSizer(sizerSearch);

    assert(_CrtCheckMemory());

    return panel;
}

wxPanel* FrontEnd::create_page_song_pinyin(wxBookCtrlBase* parent)
{
    assert(_CrtCheckMemory());
    wxPanel *panel = new wxPanel(parent);
    wxPanel *panelSearch = new wxPanel(panel);
    wxPanel *panelResult = create_panel_search_result(panel, &m_listSongPinyin, ID_LIST_SONG);

    wxStaticText    *searchLabel = new wxStaticText(panelSearch, -1, "Song Name:");
    m_textSong = new wxTextCtrl(panelSearch, -1);
    wxButton        *searchBtn = new wxButton(panelSearch, ID_SONG_SEARCH, "Search");

    wxBoxSizer *sizerPanel = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizerSearch = new wxBoxSizer(wxHORIZONTAL);
    
    sizerPanel->Add(panelSearch, 0, wxEXPAND);
    sizerPanel->Add(panelResult, 1, wxEXPAND);

    sizerSearch->Add(searchLabel, 1, wxEXPAND);
    sizerSearch->Add(m_textSong, 5, wxEXPAND);
    sizerSearch->Add(searchBtn, 1, wxEXPAND);

    panel->SetSizer(sizerPanel);
    panelSearch->SetSizer(sizerSearch);

    assert(_CrtCheckMemory());
    return panel;
}

wxPanel* FrontEnd::create_page_song_all(wxBookCtrlBase* parent, bool btnReturn, wxListCtrl** list)
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* sizer_panel = new wxBoxSizer(wxVERTICAL);

    wxButton* button = NULL;
    if(btnReturn)
        button = new wxButton(panel, ID_SONG_RETURN, "Return");

    wxPanel* panel_result = create_panel_search_result(panel, list, ID_LIST_SONG);
    m_listSongAll->ClearAll();

    if(btnReturn)
        sizer_panel->Add(button, 0, wxEXPAND);
    sizer_panel->Add(panel_result, 1, wxEXPAND);

    panel->SetSizer(sizer_panel);

    return panel;
}

wxPanel* FrontEnd::create_panel_search_result(wxWindow* parent, wxListCtrl** listResult, int id)
{
    wxPanel* panel = new wxPanel(parent);

    wxListCtrl* list = new wxListCtrl(panel, id);
    list->SetWindowStyleFlag(wxLC_LIST);
    if(listResult)
        *listResult = list;

    wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);

    sizerPanel->Add(list, 5, wxEXPAND);

    panel->SetSizer(sizerPanel);

    return panel;
}

wxPanel* FrontEnd::create_page_song_played(wxBookCtrlBase* parent)
{
    wxPanel* result = create_panel_search_result(parent, &m_listSongPlayed);

    return result;
}

wxPanel* FrontEnd::create_page_song_queued(wxBookCtrlBase* parent)
{
    wxPanel* result = create_panel_search_result(parent, &m_listSongQueued);

    return result;
}

void FrontEnd::OnSingerSearch(wxCommandEvent& event)
{
    Songs songs;
    string stmt = "select * from songs where singer = '" + m_textSingerPinyin->GetValue() + "'";
    if( !SearchSong(stmt, songs) )
        wxMessageBox("Can't find such singer.", "Message");

    for(SongsIter iter = songs.begin();
        iter != songs.end();
        iter++)
    {
        m_listSingerAll->InsertItem(0, iter->name);
    }
}

void FrontEnd::OnSongSearch(wxCommandEvent& event)
{
    Songs songs;
    string stmt = "select * from songs where name = '" + m_textSong->GetValue() + "'";
    if( !SearchSong(stmt, songs) )
        wxMessageBox("Can't find such singer.", "Message");

    for(SongsIter iter = songs.begin();
        iter != songs.end();
        iter++)
    {
        m_listSongPinyin->InsertItem(0, iter->name);
    }
}

void FrontEnd::OnSingerBand(wxCommandEvent& event)
{
    wxPanel *panel_singer = create_page_singer_pinyin(m_bookCtrl, true, &m_listSingerBand);
    m_bookCtrl->DeletePage(0);
    m_bookCtrl->InsertPage(0, panel_singer, "Singers", true);

    m_listSingerBand->ClearAll();
    Songs result;
    SearchSong("select * from songs where gender = 'Band'", result);
    for(SongsIter iter = result.begin();
        iter != result.end();
        iter ++)
    {
        m_listSingerBand->InsertItem(0, iter->singer);
    }
}

void FrontEnd::OnSingerMale(wxCommandEvent& event)
{
    wxPanel *panel_singer = create_page_singer_pinyin(m_bookCtrl, true, &m_listSingerMale);
    m_bookCtrl->DeletePage(0);
    m_bookCtrl->InsertPage(0, panel_singer, "Singers", true);

    m_listSingerMale->ClearAll();
    Songs result;
    SearchSong("select * from songs where gender = 'Male'", result);
    for(SongsIter iter = result.begin();
        iter != result.end();
        iter ++)
    {
        m_listSingerMale->InsertItem(0, iter->singer);
    }
}

void FrontEnd::OnSingerFemale(wxCommandEvent& event)
{
    wxPanel *panel_singer = create_page_singer_pinyin(m_bookCtrl, true, &m_listSingerFemale);
    m_bookCtrl->DeletePage(0);
    m_bookCtrl->InsertPage(0, panel_singer, "Singers", true);

    m_listSingerFemale->ClearAll();
    Songs result;
    SearchSong("select * from songs where gender = 'Female'", result);
    for(SongsIter iter = result.begin();
        iter != result.end();
        iter ++)
    {
        m_listSingerFemale->InsertItem(0, iter->singer);
    }
}

void FrontEnd::OnSingerReturn(wxCommandEvent& event)
{
    wxPanel *panel_singer = create_page_singer_gender(m_bookCtrl);
    m_bookCtrl->DeletePage(0);
    m_bookCtrl->InsertPage(0, panel_singer, "Singers", true);
}

void FrontEnd::OnListbook(wxBookCtrlEvent &event)
{
    wxBookCtrlBase* book = GetCurrentBook();
    const int idx = event.GetSelection();
    if(idx != wxNOT_FOUND && book)
    {
        if(book->GetPageText(idx) == "All Songs")
        {
            m_listSongAll->ClearAll();

            Songs songs;
            SearchSong("select * from songs", songs);

            for(SongsIter iter = songs.begin();
                iter != songs.end();
                iter++)
            {
                m_listSongAll->InsertItem(0, iter->name);
            }
        }
        else if(book->GetPageText(idx) == "Songs Queued")
        {
            m_listSongQueued->ClearAll();

            Songs songs;
            SearchSong("select * from songs where state = 1", songs);

            for(SongsIter iter = songs.begin();
                iter != songs.end();
                iter++)
            {
                m_listSongQueued->InsertItem(0, iter->name);
            }
        }
        else if(book->GetPageText(idx) == "Songs Played")
        {
            m_listSongPlayed->ClearAll();

            Songs songs;
            SearchSong("select * from songs where state = 2", songs);

            for(SongsIter iter = songs.begin();
                iter != songs.end();
                iter++)
            {
                m_listSongPlayed->InsertItem(0, iter->name);
            }
        }
        else if(book->GetPageText(idx) == "Singer Pinyin")
        {
            m_listSingerAll->ClearAll();
        }
    }
}

void FrontEnd::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
                    _T("VOD System Client\n")
                    _T("Using wxWidget %s\n")
                    _T("Now running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _T("About this program"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void FrontEnd::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void FrontEnd::OnSongSelected(wxListEvent& event)
{
    string song = event.GetItem().GetText();
    string stat = "update songs set state = 1 where name = '" + song +"'";
    if( DBExecute(stat) )
        wxMessageBox("Song added.");
}

void FrontEnd::OnSingerSelected(wxListEvent& event)
{
    string singer = event.GetItem().GetText();
    string stat = "select * from songs where singer = '" + singer + "'";

    Songs songs;
    SearchSong(stat, songs);

    wxPanel *panel_songs = create_page_song_all(m_bookCtrl, true, &m_listSongSinger);
    m_bookCtrl->DeletePage(0);
    m_bookCtrl->InsertPage(0, panel_songs, "Singers", true);
    
    m_listSingerAll->ClearAll();
    for(SongsIter iter = songs.begin();
        iter != songs.end();
        iter++)
    {
        m_listSongSinger->InsertItem(0, iter->name);
    }
}

void FrontEnd::OnSongReturn(wxCommandEvent& event)
{
    wxPanel *panel_singer = create_page_singer_gender(m_bookCtrl);

    const int idx = event.GetSelection();
    if(m_bookCtrl->GetPageText(idx) == "Singers")
    {
        m_bookCtrl->DeletePage(idx);
        m_bookCtrl->InsertPage(idx, panel_singer, "Singers", true);
    }
}

void FrontEnd::OnNextSong(wxCommandEvent& event)
{
    Songs result;
    SearchSong("select * from songs where state = 1", result);
    if(result.empty())
    {
        wxMessageBox("No songs in queue for now.", "Info");
        return;
    }
    string stmt = "update songs set state = 0 where name = '" + result[0].name + "'";
    DBExecute(stmt);
    wxMessageBox("Switched to next song", "Info");
}