#ifndef FRONTEND_H
#define FRONTEND_H

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <wx/listbook.h>
#include <wx/listctrl.h>
#include "../common/DBOperator.h"

// mysql admin: KTVUser passwd: user

class FrontEnd: public wxFrame, public DBOperator
{
public:
    FrontEnd(wxWindow* parent, int id, const wxString& title);
    ~FrontEnd(void);

    void OnListbook(wxListbookEvent& event);
    void OnIdle(wxIdleEvent& event);
    wxBookCtrlBase *GetCurrentBook() const { return m_bookCtrl; }

    void OnSingerSearch(wxCommandEvent& event);
    void OnSongSearch(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSingerBand(wxCommandEvent& event);
    void OnSingerMale(wxCommandEvent& event);
    void OnSingerFemale(wxCommandEvent& event);
    void OnSingerReturn(wxCommandEvent& event);
    void OnSingerSelected(wxListEvent& event);
    void OnSongSelected(wxListEvent& event);
    void OnSongReturn(wxCommandEvent& event);
    void OnNextSong(wxCommandEvent& event);
    void InitProperties();
    void InitMenus();
private:
    void set_properties();
    void do_layout();
    
    void init_pages(wxBookCtrlBase* book);
    
    wxPanel* create_page_singer_gender(wxBookCtrlBase* book);
    wxPanel* create_page_singer_pinyin(wxBookCtrlBase* book, 
        bool btnReturn,
        wxListCtrl** list);
    wxPanel* create_page_song_pinyin(wxBookCtrlBase* book);
    wxPanel* create_page_song_played(wxBookCtrlBase* book);
    wxPanel* create_page_song_queued(wxBookCtrlBase* book);
    wxPanel* create_page_song_all(wxBookCtrlBase* book, 
        bool btnReturn,
        wxListCtrl** list);

    wxPanel* create_panel_search_result(wxWindow* parent, wxListCtrl** list, int id = -1);

protected:
    wxBookCtrlBase *m_bookCtrl;
    wxBoxSizer *m_sizerFrame;
    wxPanel *m_panel;
    //wxImageList *m_imageList;

    wxButton *m_btnNextSong;

    //wxPanel* m_singerPanels[3];

    wxListCtrl* m_listSongAll;
    wxListCtrl* m_listSongQueued;
    wxListCtrl* m_listSongPlayed;
    wxListCtrl* m_listSongPinyin;
    wxListCtrl* m_listSongSinger;
    wxListCtrl* m_listSingerAll;
    wxListCtrl* m_listSingerBand;
    wxListCtrl* m_listSingerFemale;
    wxListCtrl* m_listSingerMale;

    wxTextCtrl* m_textSingerPinyin;
    wxTextCtrl* m_textSong;
    DECLARE_EVENT_TABLE()
};

enum ID_COMMANDS    
{
    ID_SINGER_MALE,
    ID_SINGER_FEMALE,
    ID_SINGER_BAND,
    ID_SINGER_SEARCH,
    ID_SINGER_RETURN,
    ID_SONG_SEARCH,
    ID_SONG_RETURN,
    ID_NEXT_SONG,

    ID_FILE_EXIT,
    ID_HELP_ABOUT,

    ID_LIST_SINGER,
    ID_LIST_SONG,
};

#endif