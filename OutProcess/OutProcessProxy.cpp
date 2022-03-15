#include "pch.h"
#include "OutProcess.h"
#include "OutProcessProxy.h"

BEGIN_MESSAGE_MAP(COutProcessProxy, CWinThread)
	ON_THREAD_MESSAGE(WM_PROXY_CLOSE, OnProxyClose)
	ON_REGISTERED_THREAD_MESSAGE(WM_MANY_VIEW_WRITE_VIDEO_INFO, OnManyViewWriteVideoInfo)
END_MESSAGE_MAP()

COutProcessProxy::COutProcessProxy()
{
	m_bAutoDelete = false;
}

COutProcessProxy::~COutProcessProxy()
{
}

BOOL COutProcessProxy::InitInstance()
{
	return TRUE;
}

int COutProcessProxy::ExitInstance()
{
	return CWinThread::ExitInstance();
}

void COutProcessProxy::OnProxyClose(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessProxy::OnProxyClose(0x%08X, 0x%08X)\n"), wParam, lParam);

	::PostQuitMessage(0);
}

void COutProcessProxy::OnManyViewWriteVideoInfo(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessProxy::OnManyViewWriteVideoInfo(0x%08X, 0x%08X)\n"), wParam, lParam);

	MSG msg = {};
	while (::PeekMessage(
		&msg,
		(HWND)-1,
		WM_MANY_VIEW_WRITE_VIDEO_INFO,
		WM_MANY_VIEW_WRITE_VIDEO_INFO,
		PM_REMOVE))
	{
		MY_TRACE(_T("WM_MANY_VIEW_WRITE_VIDEO_INFO thinned out.\n"));
	}

	theApp.sendMessage(WM_MANY_VIEW_WRITE_VIDEO_INFO, wParam, lParam);
}
