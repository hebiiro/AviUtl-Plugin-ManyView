#include "pch.h"
#include "OutProcess.h"
#include "OutProcessDialog.h"
#include "../Common/MyTracer2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COutProcessApp theApp;

BEGIN_MESSAGE_MAP(COutProcessApp, CWinApp)
	ON_REGISTERED_THREAD_MESSAGE(WM_MANY_VIEW_SHOW_VIEWERS, OnManyViewShowViewers)
	ON_REGISTERED_THREAD_MESSAGE(WM_MANY_VIEW_CREATE_VIEWER, OnManyViewCreateViewer)
	ON_REGISTERED_THREAD_MESSAGE(WM_MANY_VIEW_DESTROY_VIEWER, OnManyViewDestroyViewer)
	ON_REGISTERED_THREAD_MESSAGE(WM_MANY_VIEW_EDIT_VIEWER, OnManyViewEditViewer)
	ON_REGISTERED_THREAD_MESSAGE(WM_MANY_VIEW_READ_FRAME_INFO, OnManyViewReadFrameInfo)
	ON_REGISTERED_THREAD_MESSAGE(WM_MANY_VIEW_READ_VIDEO_INFO, OnManyViewReadVideoInfo)
END_MESSAGE_MAP()

COutProcessApp::COutProcessApp()
{
	_tsetlocale(LC_ALL, _T(""));

	trace_init(0, 0);

	m_mainProcessWindow = 0;
	m_vram_w = 0;
	m_vram_h = 0;
	m_vram_size = 0;
}

COutProcessApp::~COutProcessApp()
{
	trace_term();
}

HWND COutProcessApp::getAviUtlWindow()
{
	return ::GetWindow(getFilterWindow(), GW_OWNER);
}

HWND COutProcessApp::getFilterWindow()
{
	return ::GetParent(getFilterDialog());
}

HWND COutProcessApp::getFilterDialog()
{
	return m_mainProcessWindow;
}

BOOL COutProcessApp::InitInstance()
{
	CWinApp::InitInstance();

	{
		m_mainProcessWindow = (HWND)_tcstoul(::GetCommandLine(), 0, 0);

		MY_TRACE_HWND(m_mainProcessWindow);
		HWND parent = ::GetParent(m_mainProcessWindow);
		MY_TRACE_HWND(parent);

#ifdef MANY_VIEW_CHECK_MAIN_PROCESS
		if (!::IsWindow(m_mainProcessWindow))
			return FALSE;
#endif
	}

	if (!AfxOleInit())
	{
		AfxMessageBox(_T("AfxOleInit() failed."));

		return FALSE;
	}

	{
		m_vram_w = ::GetWindowLong(m_mainProcessWindow, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_W);
		m_vram_h = ::GetWindowLong(m_mainProcessWindow, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_H);
		m_vram_size = ::GetWindowLong(m_mainProcessWindow, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_SIZE);

		MY_TRACE_NUM(m_vram_w);
		MY_TRACE_NUM(m_vram_h);
		MY_TRACE_NUM(m_vram_size);
	}

	if (0)
	{
		INITCOMMONCONTROLSEX icc = { sizeof(icc) };
		icc.dwICC = ICC_WIN95_CLASSES;
		::InitCommonControlsEx(&icc);
	}

	GdiplusStartupInput gdiSI;
	GdiplusStartupOutput gdiSO;
	ULONG_PTR gdiToken = 0;
	ULONG_PTR gdiHookToken = 0;

	gdiSI.SuppressBackgroundThread = TRUE;
	GdiplusStartup(&gdiToken, &gdiSI, &gdiSO);
	gdiSO.NotificationHook(&gdiHookToken);

	m_pMainWnd = &m_dialog;
	if (!m_dialog.Create(IDD_OUT_PROCESS))
	{
		AfxMessageBox(_T("dialog.Create(IDD_OUT_PROCESS) failed."));
	}
	else
	{
		m_tooltip.Create(&m_dialog);
		m_dialog.loadSystemSettings();
		m_dialog.loadSettings();

#ifndef MANY_VIEW_CHECK_MAIN_PROCESS
		m_dialog.ShowWindow(SW_SHOW);
//		m_dialog.createViewer();
#endif

		m_dialog.RunModalLoop(MLF_NOKICKIDLE);

		m_tooltip.DestroyWindow();
		m_dialog.saveSettings();
		m_dialog.DestroyWindow();
	}

	gdiSO.NotificationUnhook(gdiHookToken);
	GdiplusShutdown(gdiToken);

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}

void COutProcessApp::OnManyViewShowViewers(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessApp::OnManyViewShowViewers(0x%08X, 0x%08X)\n"), wParam, lParam);

	m_dialog.showViewers((int)wParam);
}

void COutProcessApp::OnManyViewCreateViewer(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessApp::OnManyViewCreateViewer(0x%08X, 0x%08X)\n"), wParam, lParam);

	m_dialog.createViewer();
}

void COutProcessApp::OnManyViewDestroyViewer(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessApp::OnManyViewDestroyViewer(0x%08X, 0x%08X)\n"), wParam, lParam);

	m_dialog.destroyViewer((HWND)wParam);
}

void COutProcessApp::OnManyViewEditViewer(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessApp::OnManyViewEditViewer(0x%08X, 0x%08X)\n"), wParam, lParam);

	m_dialog.editViewer((HWND)wParam, (HWND)lParam);
}

void COutProcessApp::OnManyViewReadFrameInfo(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessApp::OnManyViewReadFrameInfo(0x%08X, 0x%08X)\n"), wParam, lParam);

	m_dialog.readFrameInfo((HWND)wParam);
}

void COutProcessApp::OnManyViewReadVideoInfo(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessApp::OnManyViewReadVideoInfo(0x%08X, 0x%08X)\n"), wParam, lParam);

	m_dialog.readVideoInfo((HWND)wParam);
}
