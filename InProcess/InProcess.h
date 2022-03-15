#pragma once

#include "resource.h"
#include "InProcessViewer.h"

#define	WINDOW_W 300
#define	WINDOW_H 200

#define	WM_FILTER_WRITE_FRAME_INFO (WM_APP + 2021)
#define	WM_FILTER_WRITE_VIDEO_INFO (WM_APP + 2022)

typedef std::shared_ptr<CInProcessViewer> CInProcessViewerPtr;
typedef std::map<HWND, CInProcessViewerPtr> CInProcessViewerContainer;

class CInProcessApp
{
public:

	TCHAR m_name[MAX_PATH];
	TCHAR m_information[MAX_PATH];

	HINSTANCE m_instance;
	HWND m_dialog;
	HFONT m_font;
	PROCESS_INFORMATION m_pi;
	CInProcessViewerContainer m_viewers;

	int m_vram_w;
	int m_vram_h;
	int m_vram_size;
	BOOL m_do_draw_white;
	auls::CMemref m_memref;

public:

	CInProcessApp();
	~CInProcessApp();

	void dllInit(HINSTANCE instance);
	BOOL init(FILTER *fp);
	BOOL exit(FILTER *fp);
	BOOL createDialog(FILTER *fp);
	BOOL createSubProcess();
	void recalcLayout();
	BOOL writeFrameInfo(FILTER* fp, void* editp);
	BOOL writeFrameInfo(FILTER* fp, void* editp, HWND hwnd);
	BOOL writeVideoInfo(FILTER* fp, void* editp, HWND hwnd);

	void showViewers(int show);
	void onViewerCreated(HWND hwnd);
	void onViewerDestroyed(HWND hwnd);
	void onViewerChanged(HWND hwnd);

	static HWND getCurrentViewer(HWND hdlg);
	static LRESULT CALLBACK wndProc(HWND hwnd,UINT umsg,WPARAM wparam, LPARAM lparam);

	void postMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		::PostThreadMessage(m_pi.dwThreadId, message, wParam, lParam);
	}
};

extern CInProcessApp theApp;
