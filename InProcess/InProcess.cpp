#include "pch.h"
#include "InProcess.h"

CInProcessApp theApp;

void ___outputLog(LPCTSTR text, LPCTSTR output)
{
}

CInProcessApp::CInProcessApp()
{
	::ZeroMemory(m_name, sizeof(m_name));
	::ZeroMemory(m_information, sizeof(m_information));
	m_instance = 0;
	m_dialog = 0;
	m_font = 0;
	::ZeroMemory(&m_pi, sizeof(m_pi));

	m_do_draw_white = FALSE;
	m_vram_w = 0;
	m_vram_h = 0;
	m_vram_size = 0;
}

CInProcessApp::~CInProcessApp()
{
}

void CInProcessApp::dllInit(HINSTANCE instance)
{
	MY_TRACE(_T("CInProcessApp::dllInit(0x%08p)\n"), instance);

	m_instance = instance;
	MY_TRACE_HEX(m_instance);

	::GetModuleFileName(m_instance, m_name, MAX_PATH);
	::PathStripPath(m_name);
	::PathRemoveExtension(m_name);
	MY_TRACE_STR(m_name);

	::StringCbCopy(m_information, sizeof(m_information), _T("複数ビューア version 1.0.0 by 蛇色"));
	MY_TRACE_STR(m_information);
}

BOOL CInProcessApp::init(FILTER *fp)
{
	MY_TRACE(_T("CInProcessApp::init()\n"));

	if (!m_memref.Init(fp))
	{
		MY_TRACE(_T("m_memref.Init(fp) failed.\n"));

		return FALSE;
	}

	if (!createDialog(fp))
		return FALSE;

	if (!createSubProcess())
		return FALSE;

	return TRUE;
}

BOOL CInProcessApp::exit(FILTER *fp)
{
	MY_TRACE(_T("CInProcessApp::exit()\n"));

	::DestroyWindow(m_dialog), m_dialog = 0;
	::CloseHandle(m_pi.hThread);
	::CloseHandle(m_pi.hProcess);

	return TRUE;
}

BOOL CInProcessApp::createDialog(FILTER *fp)
{
	MY_TRACE(_T("CInProcessApp::createDialog(0x%08p)\n"), fp);

	WNDCLASSEX wc = { sizeof(wc) };
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
	wc.lpfnWndProc   = wndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = IN_PROCESS_WND_EXTRA_MAX_SIZE;
	wc.hInstance     = m_instance;
	wc.hCursor       = (HCURSOR)::LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = _T("InProcess");

	if (!::RegisterClassEx(&wc))
	{
		MY_TRACE(_T("::RegisterClassEx() failed.\n"));

		return FALSE;
	}

	m_dialog = ::CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_NOPARENTNOTIFY,
		_T("InProcess"),
		_T("InProcess"),
		WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, 100, 100, fp->hwnd, 0, m_instance, 0);

	MY_TRACE_HEX(m_dialog);

	if (!m_dialog)
	{
		MY_TRACE(_T("::CreateWindowEx() failed.\n"));

		return FALSE;
	}

	{
		SYS_INFO si = {};
		fp->exfunc->get_sys_info(0, &si);
		m_vram_w = si.vram_w;
		m_vram_h = si.vram_h;
		m_vram_size = si.vram_w * si.vram_h * 3;

		MY_TRACE_NUM(m_vram_w);
		MY_TRACE_NUM(m_vram_h);
		MY_TRACE_NUM(m_vram_size);

		::SetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_W, (LONG)m_vram_w);
		::SetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_H, (LONG)m_vram_h);
		::SetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_SIZE, (LONG)m_vram_size);

		MY_TRACE_NUM(::GetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_W));
		MY_TRACE_NUM(::GetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_H));
		MY_TRACE_NUM(::GetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_SIZE));
	}

	recalcLayout();

	::ShowWindow(m_dialog, SW_SHOW);

	return TRUE;
}

BOOL CInProcessApp::createSubProcess()
{
	MY_TRACE(_T("CInProcessApp::createSubProcess()\n"));

	TCHAR path[MAX_PATH] = {};
	::GetModuleFileName(m_instance, path, MAX_PATH);
	::PathRemoveExtension(path);
	::PathAppend(path, m_name);
	::PathAddExtension(path, _T(".exe"));

	MY_TRACE_STR(path);

	TCHAR args[MAX_PATH] = {};
	::StringCbPrintf(args, sizeof(args), _T("0x%08p"), m_dialog);

	MY_TRACE_STR(args);

	STARTUPINFO si = { sizeof(si) };

	if (!::CreateProcess(
		path,           // No module name (use command line)
		args,           // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&m_pi))         // Pointer to PROCESS_INFORMATION structur
	{
		MY_TRACE(_T("::CreateProcess() failed.\n"));

		return FALSE;
	}

	return TRUE;
}

void CInProcessApp::recalcLayout()
{
	MY_TRACE(_T("CInProcessApp::recalcLayout()\n"));

	if (!m_dialog)
		return;

	const int BUTTON_W = 64;
	const int BUTTON_H = 24;
	const int PADDING = 4;

	HWND parent = ::GetParent(m_dialog);
	RECT rc; ::GetClientRect(parent, &rc);
	int x = rc.left;
	int y = rc.top;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	::MoveWindow(m_dialog, x, y, w, h, TRUE);

	::GetClientRect(m_dialog, &rc);
	x = rc.left;
	y = rc.top;
	w = rc.right - rc.left;
	h = rc.bottom - rc.top - BUTTON_H - PADDING;
	::MoveWindow(::GetDlgItem(m_dialog, IDC_VIEWER_LIST), x, y, w, h, TRUE);

	x = rc.left;
	y = rc.bottom - BUTTON_H - PADDING;
	w = BUTTON_W;
	h = BUTTON_H;
	::MoveWindow(::GetDlgItem(m_dialog, IDC_CREATE_VIEWER), x, y, w, h, TRUE);

	x += BUTTON_W + PADDING;
	::MoveWindow(::GetDlgItem(m_dialog, IDC_EDIT_VIEWER), x, y, w, h, TRUE);

	x += BUTTON_W + PADDING;
	::MoveWindow(::GetDlgItem(m_dialog, IDC_DESTROY_VIEWER), x, y, w, h, TRUE);
}

BOOL CInProcessApp::writeFrameInfo(FILTER* fp, void* editp)
{
	MY_TRACE(_T("CInProcessApp::writeFrameInfo()\n"));

	if (!fp) return FALSE;
	if (!fp->exfunc->is_editing(editp)) return FALSE; // 編集中でなければ終了

	for (auto it = theApp.m_viewers.begin(); it != theApp.m_viewers.end(); it++)
	{
		auto viewer = it->second;

		if (!writeFrameInfo(fp, editp, viewer->m_hwnd))
			return FALSE;
	}

	return TRUE;
}

BOOL CInProcessApp::writeFrameInfo(FILTER* fp, void* editp, HWND hwnd)
{
	MY_TRACE(_T("CInProcessApp::onWriteFrameInfo(0x%08p)\n"), hwnd);

	int frameNumber = fp->exfunc->get_frame(editp);
	int frameMaxNumber = fp->exfunc->get_frame_n(editp);

	MY_TRACE_NUM(frameNumber);
	MY_TRACE_NUM(frameMaxNumber);

	::SetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_FRAME_NUMBER, (LONG)frameNumber);
	::SetWindowLong(m_dialog, IN_PROCESS_WND_EXTRA_FRAME_MAX_NUMBER, (LONG)frameMaxNumber);

	theApp.postMessage(WM_MANY_VIEW_READ_FRAME_INFO, (WPARAM)hwnd, 0);

	return TRUE;
}

BOOL CInProcessApp::writeVideoInfo(FILTER* fp, void* editp, HWND hwnd)
{
	MY_TRACE(_T("CInProcessApp::onWriteVideoInfo(0x%08p)\n"), hwnd);

	auto it = m_viewers.find(hwnd);
	if (it == m_viewers.end())
		return FALSE;

	return it->second->writeVideoInfo(fp, editp);
}

void CInProcessApp::showViewers(int show)
{
	MY_TRACE(_T("CInProcessApp::showViewers(%d)\n"), show);

	theApp.postMessage(WM_MANY_VIEW_SHOW_VIEWERS, show, 0);
}

void CInProcessApp::onViewerCreated(HWND hwnd)
{
	MY_TRACE(_T("CInProcessApp::onViewerCreated(0x%08p)\n"), hwnd);

	// ビューアオブジェクトを作成し、コンテナに追加する。

	CInProcessViewerPtr viewer(new CInProcessViewer());
	m_viewers.insert(CInProcessViewerContainer::value_type(hwnd, viewer));

	// ビューアオブジェクトを初期化する。

	viewer->m_hwnd = hwnd;
	viewer->m_mutex = VideoInfoMutexPtr(new VideoInfoMutex(hwnd));
	viewer->m_fileMapping = VideoInfoFileMappingPtr(new VideoInfoFileMapping(hwnd, m_dialog));

	// リストボックスに項目を追加する。

	TCHAR text[MAX_PATH] = {};
	::GetWindowText(hwnd, text, MAX_PATH);

	HWND list = ::GetDlgItem(m_dialog, IDC_VIEWER_LIST);

	int c = (int)::SendMessage(list, LB_GETCOUNT, 0, 0);
	int index = (int)::SendMessage(list, LB_INSERTSTRING, (WPARAM)c, (LPARAM)text);
	::SendMessage(list, LB_SETITEMDATA, (WPARAM)index, (LPARAM)hwnd);
	::SendMessage(list, LB_SETCURSEL, (WPARAM)index, 0);
}

void CInProcessApp::onViewerDestroyed(HWND hwnd)
{
	MY_TRACE(_T("CInProcessApp::onViewerDestroyed(0x%08p)\n"), hwnd);

	// ビューアオブジェクトをコンテナから削除する。

	m_viewers.erase(hwnd);

	// リストボックスから項目を削除する。

	HWND list = ::GetDlgItem(m_dialog, IDC_VIEWER_LIST);

	int c = (int)::SendMessage(list, LB_GETCOUNT, 0, 0);
	for (int i = c - 1; i >= 0; i--)
	{
		if (hwnd == (HWND)::SendMessage(list, LB_GETITEMDATA, (WPARAM)i, 0))
			::SendMessage(list, LB_DELETESTRING, (WPARAM)i, 0);
	}
}

void CInProcessApp::onViewerChanged(HWND hwnd)
{
	MY_TRACE(_T("CInProcessApp::onViewerChanged(0x%08p)\n"), hwnd);

	// リストボックスの項目を更新する。

	TCHAR text[MAX_PATH] = {};
	::GetWindowText(hwnd, text, MAX_PATH);

	HWND list = ::GetDlgItem(m_dialog, IDC_VIEWER_LIST);

	int c = (int)::SendMessage(list, LB_GETCOUNT, 0, 0);
	for (int i = c - 1; i >= 0; i--)
	{
		if (hwnd == (HWND)::SendMessage(list, LB_GETITEMDATA, (WPARAM)i, 0))
		{
			int curSel = ::SendMessage(list, LB_GETCURSEL, 0, 0);
			::SendMessage(list, LB_DELETESTRING, (WPARAM)i, 0);
			::SendMessage(list, LB_INSERTSTRING, (WPARAM)i, (LPARAM)text);
			::SendMessage(list, LB_SETITEMDATA, (WPARAM)i, (LPARAM)hwnd);
			::SendMessage(list, LB_SETCURSEL, (WPARAM)curSel, 0);
		}
	}
}

HWND CInProcessApp::getCurrentViewer(HWND hdlg)
{
	HWND list = ::GetDlgItem(hdlg, IDC_VIEWER_LIST);
	int index = (int)::SendMessage(list, LB_GETCURSEL, 0, 0);
	return (HWND)::SendMessage(list, LB_GETITEMDATA, (WPARAM)index, 0);
}

LRESULT CALLBACK CInProcessApp::wndProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_MANY_VIEW_VIEWER_CREATED)
	{
		MY_TRACE(_T("CInProcessApp::wndProc(WM_MANY_VIEW_VIEWER_CREATED, 0x%08X, 0x%08X)\n"), wParam, lParam);

		theApp.onViewerCreated((HWND)wParam);
		HWND parent = ::GetParent(hdlg);
		::PostMessage(parent, WM_FILTER_WRITE_FRAME_INFO, wParam, lParam);
	}
	else if (message == WM_MANY_VIEW_VIEWER_DESTROYED)
	{
		MY_TRACE(_T("CInProcessApp::wndProc(WM_MANY_VIEW_VIEWER_DESTROYED, 0x%08X, 0x%08X)\n"), wParam, lParam);

		theApp.onViewerDestroyed((HWND)wParam);
	}
	else if (message == WM_MANY_VIEW_VIEWER_CHANGED)
	{
		MY_TRACE(_T("CInProcessApp::wndProc(WM_MANY_VIEW_VIEWER_CHANGED, 0x%08X, 0x%08X)\n"), wParam, lParam);

		theApp.onViewerChanged((HWND)wParam);
	}
	else if (message == WM_MANY_VIEW_WRITE_FRAME_INFO)
	{
		MY_TRACE(_T("CInProcessApp::wndProc(WM_MANY_VIEW_WRITE_FRAME_INFO, 0x%08X, 0x%08X)\n"), wParam, lParam);

		HWND parent = ::GetParent(hdlg);
		::PostMessage(parent, WM_FILTER_WRITE_FRAME_INFO, wParam, lParam);
	}
	else if (message == WM_MANY_VIEW_WRITE_VIDEO_INFO)
	{
		MY_TRACE(_T("CInProcessApp::wndProc(WM_MANY_VIEW_WRITE_VIDEO_INFO, 0x%08X, 0x%08X)\n"), wParam, lParam);

		HWND parent = ::GetParent(hdlg);
		::PostMessage(parent, WM_FILTER_WRITE_VIDEO_INFO, wParam, lParam);
	}
	else if (message == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case IDC_CREATE_VIEWER:
			{
				MY_TRACE(_T("CInProcessApp::wndProc(WM_COMMAND, IDC_CREATE_VIEWER)\n"));

				theApp.postMessage(WM_MANY_VIEW_CREATE_VIEWER, 0, 0);

				break;
			}
		case IDC_DESTROY_VIEWER:
			{
				MY_TRACE(_T("CInProcessApp::wndProc(WM_COMMAND, IDC_DESTROY_VIEWER)\n"));

				HWND hwnd = getCurrentViewer(hdlg);
				theApp.postMessage(WM_MANY_VIEW_DESTROY_VIEWER, (WPARAM)hwnd, 0);

				break;
			}
		case IDC_EDIT_VIEWER:
			{
				MY_TRACE(_T("CInProcessApp::wndProc(WM_COMMAND, IDC_EDIT_VIEWER)\n"));

				HWND hwnd = getCurrentViewer(hdlg);
				::SetForegroundWindow(hwnd);
				theApp.postMessage(WM_MANY_VIEW_EDIT_VIEWER, (WPARAM)hwnd, (LPARAM)hdlg);

				break;
			}
		}
	}
	else if (message == WM_CREATE)
	{
		MY_TRACE(_T("CInProcessApp::wndProc(WM_CREATE)\n"));

		HWND list = ::CreateWindowEx(
			WS_EX_CLIENTEDGE,
			_T("LISTBOX"),
			_T("ViewerList"),
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
			0, 0, 100, 100, hdlg,
			(HMENU)IDC_VIEWER_LIST, theApp.m_instance, 0);

		HWND create = ::CreateWindowEx(
			0,
			_T("BUTTON"),
			_T("Create"),
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			0, 0, 100, 100, hdlg,
			(HMENU)IDC_CREATE_VIEWER, theApp.m_instance, 0);

		HWND edit = ::CreateWindowEx(
			0,
			_T("BUTTON"),
			_T("Edit"),
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			0, 0, 100, 100, hdlg,
			(HMENU)IDC_EDIT_VIEWER, theApp.m_instance, 0);

		HWND destroy = ::CreateWindowEx(
			0,
			_T("BUTTON"),
			_T("Destroy"),
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			0, 0, 100, 100, hdlg,
			(HMENU)IDC_DESTROY_VIEWER, theApp.m_instance, 0);

		MY_TRACE_HEX(list);
		MY_TRACE_HEX(create);
		MY_TRACE_HEX(edit);
		MY_TRACE_HEX(destroy);

		theApp.m_font = ::CreateFont(
			-16, 0, 0, 0,
			FW_BOLD, FALSE, FALSE, FALSE,
			0, 0, 0, 0, 0, _T("Yu Gothic UI"));

		MY_TRACE_HEX(theApp.m_font);

		::SendMessage(list, WM_SETFONT, (WPARAM)theApp.m_font, 0);
		::SendMessage(create, WM_SETFONT, (WPARAM)theApp.m_font, 0);
		::SendMessage(edit, WM_SETFONT, (WPARAM)theApp.m_font, 0);
		::SendMessage(destroy, WM_SETFONT, (WPARAM)theApp.m_font, 0);
	}
	else if (message == WM_DESTROY)
	{
		MY_TRACE(_T("CInProcessApp::wndProc(WM_DESTROY)\n"));

		::DeleteObject(theApp.m_font);
	}

	return ::DefWindowProc(hdlg, message, wParam, lParam);
}
