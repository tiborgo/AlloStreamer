// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <boost/thread.hpp>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title,
            const wxPoint&  pos,
            const wxSize&   size);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

enum
{
    ID_Hello = 1
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Hello, MyFrame::OnHello)
EVT_MENU(wxID_EXIT, MyFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

// We are in a shared library and have no main
IMPLEMENT_WX_THEME_SUPPORT;
IMPLEMENT_APP_NO_MAIN(MyApp);

bool MyApp::OnInit()
{
    MyFrame *frame =
        new MyFrame("Hello World", wxPoint(50, 50), wxSize(450, 340));

    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title,
                 const wxPoint&  pos,
                 const wxSize&   size)
    : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    wxMenu *menuFile = new wxMenu;

    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets' Hello world sample",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

boost::thread windowThread;

void WindowLoop()
{
    wxDISABLE_DEBUG_SUPPORT();
    int argc = 1;
    char **argv = new char *[argc];
    const char *name = "CubemapExtractionPlugin";
    argv[0] = new char[strlen(name) + 1];
    strcpy(argv[0], name);
    //int x = wxEntry(argc, argv);
    int a = 0;
}

void CreatePreviewWindow()
{
    windowThread = boost::thread(boost::bind(&WindowLoop));
}

void RepaintPreviewWindow()
{
}

void DestroyPreviewWindow()
{
}