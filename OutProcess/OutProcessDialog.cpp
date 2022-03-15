#include "pch.h"
#include "afxdialogex.h"
#include "OutProcess.h"
#include "OutProcessDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(COutProcessDialog, CDialogEx)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

COutProcessDialog::COutProcessDialog(CWnd* parent)
	: CDialogEx(IDD_OUT_PROCESS, parent)
{
	MY_TRACE(_T("COutProcessDialog::COutProcessDialog()\n"));
}

COutProcessDialog::~COutProcessDialog()
{
	MY_TRACE(_T("COutProcessDialog::~COutProcessDialog()\n"));
}

void COutProcessDialog::loadSystemSettings()
{
	MY_TRACE(_T("COutProcessDialog::loadSystemSettings()\n"));

	LPCWSTR filePath = m_fileUpdateChecker->getFilePath();

	MY_TRACE_WSTR(filePath);

	try
	{
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		if (document->load(filePath) == VARIANT_FALSE)
		{
			MY_TRACE(_T("%ws file loading failed\n"), filePath);

			return;
		}

		MSXML2::IXMLDOMElementPtr element = document->documentElement;

		{
			MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"viewer");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

				COutProcessViewer::loadStaticSettings(element);
			}
		}

		{
			MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"tooltip");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

				COutProcessToolTip::loadStaticSettings(element);
			}
		}

		{
			MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"overlay");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

				COutProcessOverlay::loadStaticSettings(element);
			}
		}
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
	}
}

void COutProcessDialog::loadSettings()
{
	MY_TRACE(_T("COutProcessDialog::loadSettings()\n"));

	WCHAR filePath[MAX_PATH] = {};
	::GetModuleFileNameW(AfxGetInstanceHandle(), filePath, MAX_PATH);
	::PathRemoveExtensionW(filePath);
	::StringCbCatW(filePath, sizeof(filePath), L"Settings.xml");

	MY_TRACE_WSTR(filePath);

	try
	{
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		if (document->load(filePath) == VARIANT_FALSE)
		{
			MY_TRACE(_T("%ws file loading failed\n"), filePath);

			return;
		}

		MSXML2::IXMLDOMElementPtr element = document->documentElement;

		{
			MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"viewer");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

				auto viewer = createViewer();

				viewer->loadSettings(element);
			}
		}
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
	}
}

void COutProcessDialog::saveSettings()
{
	MY_TRACE(_T("COutProcessDialog::saveSettings()\n"));

	WCHAR filePath[MAX_PATH] = {};
	::GetModuleFileNameW(AfxGetInstanceHandle(), filePath, MAX_PATH);
	::PathRemoveExtensionW(filePath);
	::StringCbCatW(filePath, sizeof(filePath), L"Settings.xml");

	MY_TRACE_WSTR(filePath);

	try
	{
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		MSXML2::IXMLDOMElementPtr element = appendElement(document, document, L"ManyViewSettings");

		for (auto it = m_viewers.begin(); it != m_viewers.end(); it++)
		{
			auto viewer = it->second;

			viewer->saveSettings(appendElement(document, element, L"viewer"));
		}

		saveXMLDocument(document, filePath);
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
	}
}

/*
	すべてのビューアを表示/非表示にする。
*/
BOOL COutProcessDialog::showViewers(int show)
{
	MY_TRACE(_T("COutProcessDialog::showViewers(%d)\n"), show);

	// すべてのビューアに対して ShowWindow() を呼び出す。

	for (auto it = m_viewers.begin(); it != m_viewers.end(); it++)
	{
		auto viewer = it->second;

		viewer->ShowWindow(show);
	}

	return TRUE;
}

/*
	新規ビューアを作成する。
*/
COutProcessViewerPtr COutProcessDialog::createViewer()
{
	MY_TRACE(_T("COutProcessDialog::createViewer()\n"));

	// ビューアオブジェクトを作成する。

	COutProcessViewerPtr viewer(new COutProcessViewer());

#ifdef MANY_VIEW_CHECK_MAIN_PROCESS
//	HWND parent = 0;
	HWND parent = theApp.getAviUtlWindow();
//	HWND parent = theApp.getFilterWindow();
//	HWND parent = theApp.getFilterDialog();
#else
	HWND parent = GetSafeHwnd();
#endif

	// ビューアウィンドウを作成する。

	if (!viewer->CreateEx(0, 0, 0, 0, 0, 0, 640, 360, parent, 0, 0))
	{
		MY_TRACE(_T("COutProcessViewer::CreateEx() failed.\n"));

		return 0;
	}

	// ビューアオブジェクトをコンテナに追加する。

	m_viewers.insert(COutProcessViewerContainer
		::value_type(viewer->GetSafeHwnd(), viewer));

	// 親プロセスに通知する。

	theApp.postMessage(WM_MANY_VIEW_VIEWER_CREATED, (WPARAM)viewer->GetSafeHwnd(), 0);

	return viewer;
}

/*
	指定されたビューアを削除する。
*/
BOOL COutProcessDialog::destroyViewer(HWND hwnd)
{
	MY_TRACE(_T("COutProcessDialog::destroyViewer(0x%08p)\n"), hwnd);

	// コンテナから削除する。

	m_viewers.erase(hwnd);

	// 親プロセスに通知する。

	theApp.postMessage(WM_MANY_VIEW_VIEWER_DESTROYED, (WPARAM)hwnd, 0);

	return TRUE;
}

/*
	ビューア編集用のオーバーレイウィンドウを表示する。
*/
BOOL COutProcessDialog::editViewer(HWND hwnd, HWND origin)
{
	MY_TRACE(_T("COutProcessDialog::editViewer(0x%08p, 0x%08p)\n"), hwnd, origin);

	auto viewer = getViewer(hwnd);
	if (!viewer)
		return FALSE;

	viewer->m_overlay.show(CWnd::FromHandle(origin));

	return TRUE;
}

BOOL COutProcessDialog::readFrameInfo(HWND hwnd)
{
	MY_TRACE(_T("COutProcessDialog::readFrameInfo(0x%08p)\n"), hwnd);

	auto viewer = getViewer(hwnd);
	if (!viewer)
		return FALSE;

	viewer->readFrameInfo();

	return TRUE;
}

BOOL COutProcessDialog::readVideoInfo(HWND hwnd)
{
	MY_TRACE(_T("COutProcessDialog::readVideoInfo(0x%08p)\n"), hwnd);

	auto viewer = getViewer(hwnd);
	if (!viewer)
		return FALSE;

	viewer->readVideoInfo();

	return TRUE;
}

void COutProcessDialog::DoDataExchange(CDataExchange* pDX)
{
	MY_TRACE(_T("COutProcessDialog::DoDataExchange()\n"));

	CDialogEx::DoDataExchange(pDX);
}

BOOL COutProcessDialog::OnInitDialog()
{
	MY_TRACE(_T("COutProcessDialog::OnInitDialog()\n"));

	CDialogEx::OnInitDialog();

	return TRUE;
}

int COutProcessDialog::OnCreate(LPCREATESTRUCT cs)
{
	MY_TRACE(_T("COutProcessDialog::OnCreate()\n"));

	if (CDialogEx::OnCreate(cs) == -1)
		return -1;

	WCHAR filePath[MAX_PATH] = {};
	::GetModuleFileNameW(AfxGetInstanceHandle(), filePath, MAX_PATH);
	::PathRemoveExtensionW(filePath);
	::StringCbCatW(filePath, sizeof(filePath), L"SystemSettings.xml");

	MY_TRACE_WSTR(filePath);

	m_fileUpdateChecker = CFileUpdateCheckerPtr(new CFileUpdateChecker(filePath));

	SetTimer(TIMER_ID_CHECK_SETTING_FILE, 1000, 0);

#ifdef MANY_VIEW_CHECK_MAIN_PROCESS
	SetTimer(TIMER_ID_CHECK_MAIN_PROCESS, 1000, 0);
#endif

	return 0;
}

void COutProcessDialog::OnDestroy()
{
	MY_TRACE(_T("COutProcessDialog::OnDestroy()\n"));

	KillTimer(TIMER_ID_CHECK_MAIN_PROCESS);

	m_viewers.clear();

	CDialogEx::OnDestroy();
}

void COutProcessDialog::OnTimer(UINT_PTR timerId)
{
	if (timerId == TIMER_ID_CHECK_MAIN_PROCESS)
	{
		if (!::IsWindow(theApp.m_mainProcessWindow))
		{
			KillTimer(TIMER_ID_CHECK_MAIN_PROCESS);
			PostQuitMessage(0);
		}
	}
	else if (timerId == TIMER_ID_CHECK_SETTING_FILE)
	{
		if (m_fileUpdateChecker->isFileUpdated())
		{
			loadSystemSettings();

			for (auto it = m_viewers.begin(); it != m_viewers.end(); it++)
			{
				auto viewer = it->second;

				viewer->m_overlay.calcLayout();
				viewer->recalcLayout();
				viewer->Invalidate(FALSE);
			}
		}
	}

	CDialogEx::OnTimer(timerId);
}
