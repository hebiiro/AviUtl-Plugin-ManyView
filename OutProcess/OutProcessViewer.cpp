#include "pch.h"
#include "OutProcess.h"
#include "OutProcessViewer.h"

const COutProcessViewer::Label COutProcessViewer::m_windowModeLabel[]
{
	{ MANY_VIEW_WINDOW_MODE_RESTORE,		L"restore" },
	{ MANY_VIEW_WINDOW_MODE_MINIMIZE,		L"minimize" },
	{ MANY_VIEW_WINDOW_MODE_MAXIMIZE,		L"maximize" },
	{ MANY_VIEW_WINDOW_MODE_FULLSCREEN,		L"fullscreen" },
};

const COutProcessViewer::Label COutProcessViewer::m_stretchModeLabel[]
{
	{ MANY_VIEW_STRETCH_MODE_NONE,			L"none" },
	{ MANY_VIEW_STRETCH_MODE_FIT,			L"fit" },
	{ MANY_VIEW_STRETCH_MODE_CROP,			L"crop" },
	{ MANY_VIEW_STRETCH_MODE_FULL,			L"full" },
};

const COutProcessViewer::Label COutProcessViewer::m_sizeModeLabel[]
{
	{ MANY_VIEW_SIZE_MODE_MANUAL,			L"manual" },
	{ MANY_VIEW_SIZE_MODE_AUTO,				L"auto" },
};

const COutProcessViewer::Label COutProcessViewer::m_frameModeLabel[]
{
	{ MANY_VIEW_FRAME_MODE_RELATIVE,		L"relative" },
	{ MANY_VIEW_FRAME_MODE_ABSOLUTE,		L"absolute" },
};

const COutProcessViewer::Label COutProcessViewer::m_updateModeLabel[]
{
	{ MANY_VIEW_UPDATE_MODE_MANUAL,			L"manual" },
	{ MANY_VIEW_UPDATE_MODE_AUTO,			L"auto" },
};

const COutProcessViewer::Label COutProcessViewer::m_alphaChannelModeLabel[]
{
	{ MANY_VIEW_ALPHA_CHANNEL_MODE_NONE,	L"none" },
	{ MANY_VIEW_ALPHA_CHANNEL_MODE_USE,		L"use" },
	{ MANY_VIEW_ALPHA_CHANNEL_MODE_AUTO,	L"auto" },
};

int COutProcessViewer::m_showGuideDuration = 750;
int COutProcessViewer::m_hideGuideDuration = 250;

_bstr_t COutProcessViewer::m_fontFamilyName = L"Yu Gothic UI";
REAL COutProcessViewer::m_fontHeight = 16.0f;
int COutProcessViewer::m_fontStyle = FontStyleBold;

Color COutProcessViewer::m_color(0x80, 0xff, 0x99, 0xff);
Color COutProcessViewer::m_outlineColor(0x80, 0x00, 0xff, 0x00);
REAL COutProcessViewer::m_outlineWidth = 3.0f;

Color COutProcessViewer::m_fontColor(0xff, 0xff, 0xff, 0xff);
Color COutProcessViewer::m_fontOutlineColor(0xff, 0x00, 0x00, 0x00);
REAL COutProcessViewer::m_fontOutlineWidth = 3.0f;

int COutProcessViewer::m_thickFrameWidth = 24;
int COutProcessViewer::m_captionHeight = 32;
int COutProcessViewer::m_seekBarSize = 64;
int COutProcessViewer::m_seekBarWidth = 16;
int COutProcessViewer::m_seekBarGridCount = 8;
int COutProcessViewer::m_seekBarGridLength = 16;

int COutProcessViewer::m_patternSize = 32;
Color COutProcessViewer::m_backgroundColor(0xff, 0x99, 0x99, 0x99);
Color COutProcessViewer::m_backgroundSubColor(0xff, 0x66, 0x66, 0x66);

COutProcessViewer::COutProcessViewer()
	: m_overlay(this)
{
	m_hitTest = HTNOWHERE;
	m_dragStartPoint.x = 0;
	m_dragStartPoint.y = 0;
	m_dragStartImagePoint.x = 0;
	m_dragStartImagePoint.y = 0;
	m_dragStartFrameOffset = 0;
	m_dragStartFrameNumber = 0;
	m_showGuideStartPoint.x = 0;
	m_showGuideStartPoint.y = 0;
	m_showGuideStartTime = 0;
	m_hideGuideStartTime = 0;
	m_rbuttonStartTime = 0;
	m_image.x = 0;
	m_image.y = 0;
	m_image.w = 0;
	m_image.h = 0;
	m_image.scale = 1.0;
	m_image.frameOffset = 0;
	m_image.frameNumber = 0;
	m_video.number = 0;
	m_video.maxNumber = 0;
	m_windowMode = MANY_VIEW_WINDOW_MODE_RESTORE;
	m_stretchMode = MANY_VIEW_STRETCH_MODE_NONE;
	m_sizeMode = MANY_VIEW_SIZE_MODE_AUTO;
	m_frameMode = MANY_VIEW_FRAME_MODE_RELATIVE;
	m_updateMode = MANY_VIEW_UPDATE_MODE_AUTO;
	m_alphaChannelMode = MANY_VIEW_ALPHA_CHANNEL_MODE_NONE;
}

COutProcessViewer::~COutProcessViewer()
{
	DestroyWindow();
}

BOOL COutProcessViewer::initProxy()
{
	MY_TRACE(_T("COutProcessViewer::initProxy()\n"));

	return m_proxy.CreateThread(0, 0, NULL);
}

BOOL COutProcessViewer::termProxy()
{
	MY_TRACE(_T("COutProcessViewer::termProxy()\n"));

	m_proxy.PostThreadMessage(WM_QUIT, 0, 0);

	if (::WaitForSingleObject(m_proxy.m_hThread, 5000) != WAIT_OBJECT_0)
	{
		MY_TRACE(_T("COutProcessViewer::termProxy() failed.\n"));

		return FALSE;
	}

	MY_TRACE(_T("COutProcessViewer::termProxy() 3\n"));

	return TRUE;
}

void COutProcessViewer::loadStaticSettings(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("COutProcessViewer::loadStaticSettings()\n"));

	GetPrivateProfileInt(element, L"showGuideDuration", m_showGuideDuration);
	GetPrivateProfileInt(element, L"hideGuideDuration", m_hideGuideDuration);

	GetPrivateProfileBSTR(element, L"fontFamilyName", m_fontFamilyName);
	GetPrivateProfileFloat(element, L"fontHeight", m_fontHeight);
	GetPrivateProfileInt(element, L"fontStyle", m_fontStyle);

	GetPrivateProfileColor(element, L"color", m_color);
	GetPrivateProfileColor(element, L"outlineColor", m_outlineColor);
	GetPrivateProfileFloat(element, L"outlineWidth", m_outlineWidth);

	GetPrivateProfileColor(element, L"fontColor", m_fontColor);
	GetPrivateProfileColor(element, L"fontOutlineColor", m_fontOutlineColor);
	GetPrivateProfileFloat(element, L"fontOutlineWidth", m_fontOutlineWidth);

	GetPrivateProfileInt(element, L"thickFrameWidth", m_thickFrameWidth);
	GetPrivateProfileInt(element, L"captionHeight", m_captionHeight);
	GetPrivateProfileInt(element, L"seekBarSize", m_seekBarSize);
	GetPrivateProfileInt(element, L"seekBarWidth", m_seekBarWidth);
	GetPrivateProfileInt(element, L"seekBarGridCount", m_seekBarGridCount);
	GetPrivateProfileInt(element, L"seekBarGridLength", m_seekBarGridLength);

	GetPrivateProfileInt(element, L"patternSize", m_patternSize);
	GetPrivateProfileColor(element, L"backgroundColor", m_backgroundColor);
	GetPrivateProfileColor(element, L"backgroundSubColor", m_backgroundSubColor);
}

void COutProcessViewer::loadSettings(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("COutProcessViewer::loadSettings()\n"));

	_bstr_t windowName;
	int x = 0, y = 0, w = 0, h = 0;

	::GetPrivateProfileString(element, L"windowName", windowName);

	::GetPrivateProfileLabel(element, L"windowMode", m_windowMode, m_windowModeLabel);
	::GetPrivateProfileLabel(element, L"stretchMode", m_stretchMode, m_stretchModeLabel);
	::GetPrivateProfileLabel(element, L"sizeMode", m_sizeMode, m_sizeModeLabel);
	::GetPrivateProfileLabel(element, L"frameMode", m_frameMode, m_frameModeLabel);
	::GetPrivateProfileLabel(element, L"updateMode", m_updateMode, m_updateModeLabel);
	::GetPrivateProfileLabel(element, L"alphaChannelMode", m_alphaChannelMode, m_alphaChannelModeLabel);

	MY_TRACE_NUM(m_stretchMode);

	{
		MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"window");
		int c = nodeList->length;
		for (int i = 0; i < c; i++)
		{
			MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

			::GetPrivateProfileInt(element, L"x", x);
			::GetPrivateProfileInt(element, L"y", y);
			::GetPrivateProfileInt(element, L"w", w);
			::GetPrivateProfileInt(element, L"h", h);
		}
	}

	{
		MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"image");
		int c = nodeList->length;
		for (int i = 0; i < c; i++)
		{
			MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

			::GetPrivateProfileInt(element, L"x", m_image.x);
			::GetPrivateProfileInt(element, L"y", m_image.y);
			::GetPrivateProfileInt(element, L"w", m_image.w);
			::GetPrivateProfileInt(element, L"h", m_image.h);
			::GetPrivateProfileFloat(element, L"scale", m_image.scale);
			::GetPrivateProfileInt(element, L"frameOffset", m_image.frameOffset);
			::GetPrivateProfileInt(element, L"frameNumber", m_image.frameNumber);
		}
	}

	WINDOWPLACEMENT wp = { sizeof(wp) };
	::GetWindowPlacement(GetSafeHwnd(), &wp);
	wp.rcNormalPosition.left = x;
	wp.rcNormalPosition.top = y;
	wp.rcNormalPosition.right = x + w;
	wp.rcNormalPosition.bottom = y + h;
	::SetWindowPlacement(GetSafeHwnd(), &wp);

	setWindowMode(m_windowMode);
	setStretchMode(m_stretchMode);
	setSizeMode(m_sizeMode);
	setFrameMode(m_frameMode);
	setUpdateMode(m_updateMode);
	setAlphaChannelMode(m_alphaChannelMode);

	setWindowName(windowName);
}

void COutProcessViewer::saveSettings(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("COutProcessViewer::saveSettings()\n"));

	WCHAR windowName[MAX_PATH] = {};
	::GetWindowTextW(GetSafeHwnd(), windowName, MAX_PATH);
	::SetPrivateProfileString(element, L"windowName", windowName);

	::SetPrivateProfileLabel(element, L"windowMode", m_windowMode, m_windowModeLabel);
	::SetPrivateProfileLabel(element, L"stretchMode", m_stretchMode, m_stretchModeLabel);
	::SetPrivateProfileLabel(element, L"sizeMode", m_sizeMode, m_sizeModeLabel);
	::SetPrivateProfileLabel(element, L"frameMode", m_frameMode, m_frameModeLabel);
	::SetPrivateProfileLabel(element, L"updateMode", m_updateMode, m_updateModeLabel);
	::SetPrivateProfileLabel(element, L"alphaChannelMode", m_alphaChannelMode, m_alphaChannelModeLabel);

	{
		WINDOWPLACEMENT wp = { sizeof(wp) };
		::GetWindowPlacement(GetSafeHwnd(), &wp);
		MyRect rc(wp.rcNormalPosition);

		MSXML2::IXMLDOMElementPtr element2 = appendElement(element, L"window");

		::SetPrivateProfileInt(element2, L"x", rc.X);
		::SetPrivateProfileInt(element2, L"y", rc.Y);
		::SetPrivateProfileInt(element2, L"w", rc.Width);
		::SetPrivateProfileInt(element2, L"h", rc.Height);
	}

	{
		MSXML2::IXMLDOMElementPtr element2 = appendElement(element, L"image");

		::SetPrivateProfileInt(element2, L"x", m_image.x);
		::SetPrivateProfileInt(element2, L"y", m_image.y);
		::SetPrivateProfileInt(element2, L"w", m_image.w);
		::SetPrivateProfileInt(element2, L"h", m_image.h);
		::SetPrivateProfileFloat(element2, L"scale", m_image.scale);
		::SetPrivateProfileInt(element, L"frameOffset", m_image.frameOffset);
		::SetPrivateProfileInt(element, L"frameNumber", m_image.frameNumber);
	}
}

void COutProcessViewer::readFrameInfo()
{
	MY_TRACE(_T("COutProcessViewer::readFrameInfo()\n"));

#if 0
	// readVideoInfo() で取得できるのでここでは取得しない。
	theApp.getWindowLong(IN_PROCESS_WND_EXTRA_FRAME_NUMBER);
	theApp.getWindowLong(IN_PROCESS_WND_EXTRA_FRAME_MAX_NUMBER);
#endif

	if (m_updateMode == MANY_VIEW_UPDATE_MODE_AUTO)
		requestVideoInfoEx();
}

void COutProcessViewer::readVideoInfo()
{
	MY_TRACE(_T("COutProcessViewer::readVideoInfo()\n"));

	Synchronizer sync(*m_video.mutex);

	MSG msg = {};
	while (::PeekMessage(
		&msg,
		GetSafeHwnd(),
		WM_MANY_VIEW_READ_VIDEO_INFO,
		WM_MANY_VIEW_READ_VIDEO_INFO,
		PM_REMOVE))
	{
		MY_TRACE(_T("WM_MANY_VIEW_READ_VIDEO_INFO thinned out.\n"));
	}

	VideoInfoFileMapping::Buffer buffer(m_video.fileMapping.get());
	VideoInfo* info = buffer.getInfo();
	ColorRGB* src1 = buffer.getSrc1();
	ColorRGB* src2 = buffer.getSrc2();

	MY_TRACE_NUM(info->frame.number);
	MY_TRACE_NUM(info->frame.maxNumber);
	MY_TRACE_NUM(info->frame.w);
	MY_TRACE_NUM(info->frame.h);
	MY_TRACE_NUM(info->useAlpha);

	m_video.number = info->frame.number;
	m_video.maxNumber = info->frame.maxNumber;

	MY_TRACE_NUM(m_video.number);
	MY_TRACE_NUM(m_video.maxNumber);

	if (m_video.bits.IsNull() ||
		m_video.bits.GetWidth() != info->frame.w ||
		m_video.bits.GetHeight() != info->frame.h)
	{
		m_video.bits.Destroy();
//		BOOL result = m_video.bits.Create(info->frame.w, -info->frame.h, 32);
		BOOL result = m_video.bits.Create(info->frame.w, info->frame.h, 32);
		MY_TRACE_NUM(result);
		if (!result)
			return;
		m_video.bits.SetHasAlphaChannel(true);
	}

	BYTE* bits = (BYTE*)m_video.bits.GetBits();
	int bits_pitch = m_video.bits.GetPitch();
	int bits_width = m_video.bits.GetWidth();
	int bits_height = m_video.bits.GetHeight();
	int bits_bpp = m_video.bits.GetBPP();
	int bits_count = bits_width * bits_height;
	int memorySize = bits_count * 4;
	MY_TRACE_HEX(bits);
	MY_TRACE_NUM(bits_pitch);
	MY_TRACE_NUM(bits_width);
	MY_TRACE_NUM(bits_height);
	MY_TRACE_NUM(bits_bpp);
	MY_TRACE_NUM(memorySize);

//	ColorRGBA* pixel = (ColorRGBA*)bits;
	ColorRGBA* pixel = (ColorRGBA*)(bits - (memorySize + bits_pitch));

	DWORD start = timeGetTime();

	if (info->useAlpha)
	{
		for (int i = 0; i < bits_count; i++)
		{
			ColorRGB color1(src1[i]);
			ColorRGB color2(src2[i]);
			ColorRGBA color;

			BYTE ra = (BYTE)(255 - color2.r + color1.r);
			BYTE ga = (BYTE)(255 - color2.g + color1.g);
			BYTE ba = (BYTE)(255 - color2.b + color1.b);
			color.a = (BYTE)((ra + ga + ba) / 3);
			color.r = ra ? (BYTE)(color1.r * 255 / ra) : color1.r;
			color.g = ga ? (BYTE)(color1.g * 255 / ga) : color1.g;
			color.b = ba ? (BYTE)(color1.b * 255 / ba) : color1.b;

			pixel[i].a = color.a;
			pixel[i].r = ((color.r * color.a) + 128) >> 8;
			pixel[i].g = ((color.g * color.a) + 128) >> 8;
			pixel[i].b = ((color.b * color.a) + 128) >> 8;
		}
	}
	else
	{
		for (int i = 0; i < bits_count; i++)
		{
			ColorRGB color1(src1[i]);

			pixel[i].a = 255;
			pixel[i].r = color1.r;
			pixel[i].g = color1.g;
			pixel[i].b = color1.b;
		}
	}

	DWORD end = timeGetTime();

	MY_TRACE(_T("create image == %dms.\n"), end - start);

	Invalidate(FALSE);
}

void COutProcessViewer::insertPart(int ht, int x, int y, int w, int h)
{
	m_parts.insert(CParts::value_type(ht, new CPart(ht, x, y, w, h)));
}

void COutProcessViewer::recalcLayout()
{
//	MY_TRACE(_T("COutProcessViewer::recalcLayout()\n"));

	if (m_patternBrush.GetSafeHandle())
		m_patternBrush.DeleteObject();

	if (m_patternSize > 1)
	{
		int half = m_patternSize / 2;

		COffscreenDC dc;
		dc.update(this, m_patternSize, m_patternSize);

		dc.FillSolidRect(0, 0, m_patternSize, m_patternSize, m_backgroundColor.ToCOLORREF());
		dc.FillSolidRect(0, half, half, half, m_backgroundSubColor.ToCOLORREF());
		dc.FillSolidRect(half, 0, half, half, m_backgroundSubColor.ToCOLORREF());

		m_patternBrush.CreatePatternBrush(&dc.m_bitmap);
	}

	m_parts.clear();

	CRect rc; GetClientRect(&rc);
	int width = rc.Width();
	int height = rc.Height();

	insertPart(HTTOPLEFT, rc.left, rc.top, m_thickFrameWidth * 2, m_thickFrameWidth * 2);
	insertPart(HTTOPRIGHT, rc.right - m_thickFrameWidth * 2, rc.top, m_thickFrameWidth * 2, m_thickFrameWidth * 2);
	insertPart(HTBOTTOMLEFT, rc.left, rc.bottom - m_thickFrameWidth * 2, m_thickFrameWidth * 2, m_thickFrameWidth * 2);
	insertPart(HTBOTTOMRIGHT, rc.right - m_thickFrameWidth * 2, rc.bottom - m_thickFrameWidth * 2, m_thickFrameWidth * 2, m_thickFrameWidth * 2);

	insertPart(HTLEFT, rc.left, rc.top + m_thickFrameWidth * 2, m_thickFrameWidth, height - m_thickFrameWidth * 4);
	insertPart(HTRIGHT, rc.right - m_thickFrameWidth, rc.top + m_thickFrameWidth * 2, m_thickFrameWidth, height - m_thickFrameWidth * 4);
	insertPart(HTTOP, rc.left + m_thickFrameWidth * 2, rc.top, width - m_thickFrameWidth * 4, m_thickFrameWidth);
	insertPart(HTBOTTOM, rc.left + m_thickFrameWidth * 2, rc.bottom - m_thickFrameWidth, width - m_thickFrameWidth * 4, m_thickFrameWidth);

	insertPart(HTCAPTION, rc.left + m_thickFrameWidth, rc.top + m_thickFrameWidth, width - m_thickFrameWidth * 2, m_captionHeight);

	insertPart(HT_SEEK_BAR,
		(rc.left + rc.right - m_seekBarSize) / 2,
		rc.bottom - m_thickFrameWidth - m_seekBarSize - m_seekBarGridLength,
		m_seekBarSize, m_seekBarSize);
}

int COutProcessViewer::hitTest(CPoint point)
{
	ScreenToClient(&point);

	if (m_parts[HTTOPLEFT]->rc.Contains(point.x, point.y)) return HTTOPLEFT;
	if (m_parts[HTTOPRIGHT]->rc.Contains(point.x, point.y)) return HTTOPRIGHT;
	if (m_parts[HTBOTTOMLEFT]->rc.Contains(point.x, point.y)) return HTBOTTOMLEFT;
	if (m_parts[HTBOTTOMRIGHT]->rc.Contains(point.x, point.y)) return HTBOTTOMRIGHT;
	if (m_parts[HTLEFT]->rc.Contains(point.x, point.y)) return HTLEFT;
	if (m_parts[HTRIGHT]->rc.Contains(point.x, point.y)) return HTRIGHT;
	if (m_parts[HTTOP]->rc.Contains(point.x, point.y)) return HTTOP;
	if (m_parts[HTBOTTOM]->rc.Contains(point.x, point.y)) return HTBOTTOM;
	if (m_parts[HTCAPTION]->rc.Contains(point.x, point.y)) return HTCAPTION;
	if (m_parts[HT_SEEK_BAR]->rc.Contains(point.x, point.y)) return HT_SEEK_BAR;

	return HT_DRAWING_AREA;
}

struct EnumWindowsParam
{
	CMenu* menu;
	HWND owner;
};

inline BOOL isOwner(HWND hwnd, HWND owner)
{
	do
	{
		hwnd = ::GetWindow(hwnd, GW_OWNER);
		MY_TRACE_HWND(hwnd);

		if (hwnd == owner)
			return TRUE;
	}
	while (hwnd);

	return FALSE;
}

static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
{
	EnumWindowsParam* p = (EnumWindowsParam*)lParam;

	if (!::IsWindowVisible(hwnd))
		return TRUE;

	MY_TRACE_HWND(hwnd);

	if (!isOwner(hwnd, p->owner))
		return TRUE;

	TCHAR className[MAX_PATH] = {};
	::GetClassName(hwnd, className, MAX_PATH);
	if (::lstrcmp(className, _T("OutProcessViewer")) == 0)
		return TRUE;

	TCHAR windowText[MAX_PATH] = {};
	::GetWindowText(hwnd, windowText, MAX_PATH);
	p->menu->AppendMenu(MF_STRING, (UINT_PTR)hwnd, windowText);

	return TRUE;
}

void COutProcessViewer::showContextMenu(UINT hitTest, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::showContextMenu(%d, %d, %d)\n"), hitTest, point.x, point.y);

	CMenu menu; menu.CreatePopupMenu();

	HWND parent = ::GetParent(theApp.m_mainProcessWindow);
	MY_TRACE_HWND(parent);
	HWND owner = ::GetWindow(parent, GW_OWNER);
	MY_TRACE_HWND(owner);

	menu.AppendMenu(MF_STRING, (UINT_PTR)owner, _T("AviUtl"));

	EnumWindowsParam param = { &menu, owner, };
	::EnumWindows(enumWindowsProc, (LPARAM)&param);

	for (auto it = theApp.m_dialog.m_viewers.begin(); it != theApp.m_dialog.m_viewers.end(); it++)
	{
		UINT flags = MF_STRING;
		if (it == theApp.m_dialog.m_viewers.begin())
			flags |= MF_MENUBREAK;

		auto viewer = it->second;
		CString windowText; viewer->GetWindowText(windowText);
		menu.AppendMenu(flags, (UINT_PTR)viewer->GetSafeHwnd(), windowText);
	}

	UINT id = (UINT)menu.TrackPopupMenu(
		TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, this);

	if (!id)
		return;

	HWND hwnd = (HWND)id;

	::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void COutProcessViewer::startShowGuideTimer()
{
	CPoint point; ::GetCursorPos(&point);
	startShowGuideTimer(point);
}

void COutProcessViewer::startShowGuideTimer(const POINT& point)
{
//	if (!m_showGuideStartTime)
		Invalidate(FALSE);

	m_showGuideStartPoint = point;
	m_showGuideStartTime = ::GetTickCount64();
	SetTimer(TIMER_ID_SHOW_GUIDE, m_showGuideDuration, 0);
}

void COutProcessViewer::stopShowGuideTimer()
{
	m_showGuideStartTime = 0;
	KillTimer(TIMER_ID_SHOW_GUIDE);
}

void COutProcessViewer::startHideGuideTimer()
{
	m_hideGuideStartTime = ::GetTickCount64();
	SetTimer(TIMER_ID_HIDE_GUIDE, 10, 0);
}

void COutProcessViewer::stopHideGuideTimer()
{
	m_hideGuideStartTime = 0;
	KillTimer(TIMER_ID_HIDE_GUIDE);
}

void COutProcessViewer::startRButtonTimer()
{
	UINT doubleClickTime = ::GetDoubleClickTime();
	MY_TRACE_NUM(doubleClickTime);
	m_rbuttonStartTime = ::GetTickCount64();
	SetTimer(TIMER_ID_RBUTTON, doubleClickTime, 0);
}

void COutProcessViewer::stopRButtonTimer()
{
	m_rbuttonStartTime = 0;
	KillTimer(TIMER_ID_RBUTTON);
}

void COutProcessViewer::setWindowName(LPCTSTR windowName)
{
	SetWindowText(windowName);
	Invalidate(FALSE);

	// 親プロセスに通知する。

	theApp.postMessage(WM_MANY_VIEW_VIEWER_CHANGED, (WPARAM)GetSafeHwnd(), 0);
}

void COutProcessViewer::setWindowMode(int windowMode)
{
	MY_TRACE(_T("COutProcessViewer::setWindowMode(%d)\n"), windowMode);

	m_windowMode = windowMode;
	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_WINDOW_MODE, m_windowMode);

	// ウィンドウが表示されているディスプレイ情報を取得する
	HMONITOR monitor = ::MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX monitor_info = {};
	monitor_info.cbSize = sizeof(monitor_info);
	::GetMonitorInfo(monitor, &monitor_info);

	switch (m_windowMode)
	{
	default:
	case MANY_VIEW_WINDOW_MODE_RESTORE:
		{
			ShowWindow(SW_RESTORE);
			break;
		}
	case MANY_VIEW_WINDOW_MODE_MINIMIZE:
		{
			ShowWindow(SW_HIDE);
//			ShowWindow(SW_MINIMIZE);
			break;
		}
	case MANY_VIEW_WINDOW_MODE_MAXIMIZE:
		{
			ShowWindow(SW_MAXIMIZE);
//			ModifyStyle(0, WS_MAXIMIZE);
			MoveWindow(&monitor_info.rcWork);
			break;
		}
	case MANY_VIEW_WINDOW_MODE_FULLSCREEN:
		{
			ShowWindow(SW_MAXIMIZE);
//			ModifyStyle(0, WS_MAXIMIZE);
			MoveWindow(&monitor_info.rcMonitor);
			break;
		}
	}
}

void COutProcessViewer::setStretchMode(int stretchMode)
{
	MY_TRACE(_T("COutProcessViewer::setStretchMode(%d)\n"), stretchMode);

	m_stretchMode = stretchMode;
	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_STRETCH_MODE, m_stretchMode);

	Invalidate(FALSE);
}

void COutProcessViewer::setSizeMode(int sizeMode)
{
	m_sizeMode = sizeMode;
	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_SIZE_MODE, m_sizeMode);

	Invalidate(FALSE);
}

void COutProcessViewer::setUpdateMode(int updateMode)
{
	m_updateMode = updateMode;
	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_UPDATE_MODE, m_updateMode);

	Invalidate(FALSE);
}

void COutProcessViewer::setFrameMode(int frameMode)
{
	m_frameMode = frameMode;
	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_FRAME_MODE, m_frameMode);

	requestVideoInfoEx();
}

void COutProcessViewer::setAlphaChannelMode(int alphaChannelMode)
{
	m_alphaChannelMode = alphaChannelMode;
	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_ALPHA_CHANNEL_MODE, m_alphaChannelMode);

	requestVideoInfoEx();
}

void COutProcessViewer::setFrameOffset(int frameOffset)
{
	MY_TRACE(_T("COutProcessViewer::setFrameOffset(%d)\n"), frameOffset);

	if (m_image.frameOffset != frameOffset)
	{
		m_image.frameOffset = frameOffset;

		requestVideoInfoEx();
	}
}

void COutProcessViewer::setFrameNumber(int frameNumber)
{
	MY_TRACE(_T("COutProcessViewer::setFrameNumber(%d)\n"), frameNumber);

	frameNumber = max(frameNumber, 0);
	frameNumber = min(frameNumber, m_video.maxNumber - 1);

	if (m_image.frameNumber != frameNumber)
	{
		m_image.frameNumber = frameNumber;

		requestVideoInfoEx();
	}
}

int COutProcessViewer::getFrameNumber()
{
	MY_TRACE(_T("COutProcessViewer::getFrameNumber()\n"));

	if (m_frameMode == MANY_VIEW_FRAME_MODE_ABSOLUTE)
	{
		return m_image.frameNumber;
	}
	else
	{
		return m_image.frameOffset +
			theApp.getWindowLong(IN_PROCESS_WND_EXTRA_FRAME_NUMBER);
	}
}
#if 0
void COutProcessViewer::requestVideoInfo(BOOL force)
{
	MY_TRACE(_T("COutProcessViewer::requestVideoInfo(%d)\n"), force);

	if (!force)
	{
		if (m_updateMode == MANY_VIEW_UPDATE_MODE_MANUAL)
			return;
	}

	int frameNumber = getFrameNumber();

	if (!force)
	{
		if (m_video.number == frameNumber)
			return;
	}

	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_FRAME_NUMBER, frameNumber);
	m_proxy.PostThreadMessage(WM_MANY_VIEW_WRITE_VIDEO_INFO, (WPARAM)GetSafeHwnd(), 0);

	MY_TRACE(_T("WM_MANY_VIEW_WRITE_VIDEO_INFO posted.\n"));
}
#endif
void COutProcessViewer::requestVideoInfoEx()
{
	MY_TRACE(_T("COutProcessViewer::requestVideoInfoEx()\n"));

	::SetWindowLong(GetSafeHwnd(), OUT_PROCESS_WND_EXTRA_FRAME_NUMBER, getFrameNumber());
	m_proxy.PostThreadMessage(WM_MANY_VIEW_WRITE_VIDEO_INFO, (WPARAM)GetSafeHwnd(), 0);

	MY_TRACE(_T("WM_MANY_VIEW_WRITE_VIDEO_INFO posted.\n"));
}
