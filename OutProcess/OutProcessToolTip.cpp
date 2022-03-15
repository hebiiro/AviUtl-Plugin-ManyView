#include "pch.h"
#include "OutProcess.h"
#include "OutProcessToolTip.h"
#include "OutProcessViewer.h"

static void getTextRect(CRect& rcText, const POINT& point, int w, int h)
{
	rcText.left = point.x - w / 2;
	rcText.top = point.y;
	rcText.right = rcText.left + w;
	rcText.bottom = rcText.top + h;
}

static void clampRect(CRect& rcText, const POINT& point)
{
	CRect rcMonitor; GetMonitorRect(point, rcMonitor);
	CPoint offset(0, 0);
	if (rcText.left < rcMonitor.left) offset.x = rcMonitor.left - rcText.left;
	if (rcText.right > rcMonitor.right) offset.x = rcMonitor.right - rcText.right;
	if (rcText.top < rcMonitor.top) offset.y = rcMonitor.top - rcText.top;
	if (rcText.bottom > rcMonitor.bottom) offset.y = rcMonitor.bottom - rcText.bottom;
	rcText.OffsetRect(offset);
}

BYTE COutProcessToolTip::m_alpha = 224;
int COutProcessToolTip::m_showDuration = 10000;
int COutProcessToolTip::m_hideDuration = 250;

_bstr_t COutProcessToolTip::m_fontFamilyName = L"Yu Gothic UI";
int COutProcessToolTip::m_fontHeight = 16;

Color COutProcessToolTip::m_fontColor(0xff, 0xff, 0xff, 0xff);
Color COutProcessToolTip::m_fontOutlineColor(0xff, 0x00, 0x00, 0x00);
int COutProcessToolTip::m_fontOutlineWidth = 2;

Color COutProcessToolTip::m_backgroundColor(0xff, 0x66, 0x66, 0xff);
Color COutProcessToolTip::m_backgroundOutlineColor(0xff, 0x99, 0x99, 0xff);
int COutProcessToolTip::m_backgroundOutlineWidth = 2;

BEGIN_MESSAGE_MAP(COutProcessToolTip, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

COutProcessToolTip::COutProcessToolTip()
{
	MY_TRACE(_T("COutProcessToolTip::COutProcessToolTip()\n"));

	m_text = L"";
	m_showStartTime = 0;
	m_hideStartTime = 0;
}

COutProcessToolTip::~COutProcessToolTip()
{
	MY_TRACE(_T("COutProcessToolTip::~COutProcessToolTip()\n"));
}

void COutProcessToolTip::loadStaticSettings(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("COutProcessToolTip::loadStaticSettings()\n"));

	GetPrivateProfileInt(element, L"alpha", m_alpha);
	GetPrivateProfileInt(element, L"showDuration", m_showDuration);
	GetPrivateProfileInt(element, L"hideDuration", m_hideDuration);

	GetPrivateProfileBSTR(element, L"fontFamilyName", m_fontFamilyName);
	GetPrivateProfileInt(element, L"fontHeight", m_fontHeight);

	GetPrivateProfileColor(element, L"fontColor", m_fontColor);
	GetPrivateProfileColor(element, L"fontOutlineColor", m_fontOutlineColor);
	GetPrivateProfileInt(element, L"fontOutlineWidth", m_fontOutlineWidth);

	GetPrivateProfileColor(element, L"backgroundColor", m_backgroundColor);
	GetPrivateProfileColor(element, L"backgroundOutlineColor", m_backgroundOutlineColor);
	GetPrivateProfileInt(element, L"backgroundOutlineWidth", m_backgroundOutlineWidth);
}

void COutProcessToolTip::show(LPCTSTR text, const POINT& point)
{
//	MY_TRACE(_T("COutProcessToolTip::show()\n"));

	if (m_showDuration <= 0)
	{
		if (IsWindowVisible())
			hide();

		return;
	}

	m_text = text;

//	MY_TRACE_WSTR((BSTR)m_text);

	{
		// recalc layout

		CClientDC dc(this);
		Graphics g(dc);

		Gdiplus::Font font(m_fontFamilyName, (REAL)m_fontHeight, FontStyleBold, UnitWorld);
		StringFormat stringFormat;
		stringFormat.SetAlignment(StringAlignmentCenter);
		stringFormat.SetLineAlignment(StringAlignmentCenter);

		RectF rect;
		g.MeasureString(
			m_text, -1,
			&font,
			PointF((REAL)point.x, (REAL)point.y),
			&stringFormat,
			&rect);

		int margin = min(4, m_fontHeight / 2);

		CRect rcText; getTextRect(rcText, point, (int)rect.Width + margin, (int)rect.Height + margin);
		clampRect(rcText, point);
		SetWindowPos(&CWnd::wndTopMost,
			rcText.left, rcText.top,
			rcText.Width(), rcText.Height(), SWP_NOACTIVATE | SWP_NOREDRAW);
//		MoveWindow(&rcText, FALSE);
	}

	startMouseLeaveTimer();
	stopHideTimer();
	startShowTimer();
	Invalidate(FALSE);
	SetLayeredWindowAttributes(0, m_alpha, LWA_ALPHA);
	ShowWindow(SW_SHOWNA);
	UpdateWindow();
}

void COutProcessToolTip::hide()
{
//	MY_TRACE(_T("COutProcessToolTip::hide()\n"));

	stopMouseLeaveTimer();
	stopShowTimer();
	stopHideTimer();
	ShowWindow(SW_HIDE);
}

void COutProcessToolTip::updatePosition(const POINT& point)
{
//	MY_TRACE(_T("COutProcessToolTip::updatePosition()\n"));

	if (!IsWindowVisible())
		return;

	CRect rcWindow; GetWindowRect(&rcWindow);
	CRect rcText; getTextRect(rcText, point, rcWindow.Width(), rcWindow.Height());
	clampRect(rcText, point);
	SetWindowPos(&CWnd::wndTopMost,
		rcText.left, rcText.top,
		rcText.Width(), rcText.Height(), SWP_NOACTIVATE);
	UpdateWindow();
}

void COutProcessToolTip::startShowTimer()
{
	m_showStartTime = ::GetTickCount64();
	SetTimer(TIMER_ID_SHOW, m_showDuration, 0);
}

void COutProcessToolTip::stopShowTimer()
{
	m_showStartTime = 0;
	KillTimer(TIMER_ID_SHOW);
}

void COutProcessToolTip::startHideTimer()
{
	m_hideStartTime = ::GetTickCount64();
	SetTimer(TIMER_ID_HIDE, 10, 0);
}

void COutProcessToolTip::stopHideTimer()
{
	m_hideStartTime = 0;
	KillTimer(TIMER_ID_HIDE);
}

void COutProcessToolTip::startMouseLeaveTimer()
{
	m_hideStartTime = ::GetTickCount64();
	SetTimer(TIMER_ID_MOUSE_LEAVE, 100, 0);
}

void COutProcessToolTip::stopMouseLeaveTimer()
{
	m_hideStartTime = 0;
	KillTimer(TIMER_ID_MOUSE_LEAVE);
}

BOOL COutProcessToolTip::Create(CWnd* parent)
{
	MY_TRACE(_T("COutProcessToolTip::Create(0x%p)\n"), parent);

	return CWnd::CreateEx(0, 0, 0, 0, CRect(0, 0, 0, 0), parent, 0, 0);
}

BOOL COutProcessToolTip::PreCreateWindow(CREATESTRUCT& cs)
{
	MY_TRACE(_T("COutProcessToolTip::PreCreateWindow()\n"));

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_NOCLOSE | CS_DROPSHADOW;
	wc.hCursor = ::LoadCursor(0, IDC_CROSS);
	wc.lpfnWndProc = AfxWndProc;
	wc.hInstance = AfxGetInstanceHandle();
	wc.lpszClassName = _T("OutProcessToolTip");
	AfxRegisterClass(&wc);
	cs.lpszName = _T("OutProcessToolTip");
	cs.lpszClass = _T("OutProcessToolTip");
	cs.style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST;
	cs.hwndParent = 0;

	return CWnd::PreCreateWindow(cs);
}

int COutProcessToolTip::OnCreate(LPCREATESTRUCT cs)
{
	MY_TRACE(_T("COutProcessToolTip::OnCreate()\n"));

	if (CWnd::OnCreate(cs) == -1)
		return -1;

	return 0;
}

void COutProcessToolTip::OnDestroy()
{
	MY_TRACE(_T("COutProcessToolTip::OnDestroy()\n"));

	hide();

	CWnd::OnDestroy();
}

void COutProcessToolTip::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; GetClientRect(&rc);

	int width = rc.Width();
	int height = rc.Height();

	Bitmap offScreenBitmap(width, height);
	Graphics g(&offScreenBitmap);

	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.TranslateTransform(-0.5f, -0.5f);

	g.FillRectangle(&SolidBrush(m_backgroundColor), MyRect(rc));
	g.DrawRectangle(&Pen(m_backgroundOutlineColor, (REAL)m_backgroundOutlineWidth), MyRect(rc));

	{
		SolidBrush fontBrush(m_fontColor);
		Pen fontPen(m_fontOutlineColor, (REAL)m_fontOutlineWidth);
		fontPen.SetLineJoin(LineJoinRound);
		FontFamily fontFamily(m_fontFamilyName);
		StringFormat stringFormat;
		stringFormat.SetAlignment(StringAlignmentCenter);
		stringFormat.SetLineAlignment(StringAlignmentCenter);

		GraphicsPath path;
		path.AddString(m_text, -1, &fontFamily, FontStyleBold, (REAL)m_fontHeight, MyRect(rc), &stringFormat);
		g.DrawPath(&fontPen, &path);
		g.FillPath(&fontBrush, &path);
	}

	Graphics(dc).DrawImage(&offScreenBitmap, 0, 0);
}

void COutProcessToolTip::OnTimer(UINT_PTR timerId)
{
	switch (timerId)
	{
	case TIMER_ID_SHOW:
		{
			stopShowTimer();
			startHideTimer();

			break;
		}
	case TIMER_ID_HIDE:
		{
			ULONGLONG now = ::GetTickCount64();
			ULONGLONG elapsedTime = now - m_hideStartTime;

			if (elapsedTime > m_hideDuration)
			{
				stopMouseLeaveTimer();
				stopHideTimer();
				ShowWindow(SW_HIDE);
			}
			else
			{
				double alpha = max(0, 1.0 - (double)elapsedTime / m_hideDuration);
				SetLayeredWindowAttributes(0, (BYTE)(m_alpha * alpha), LWA_ALPHA);
//				Invalidate(FALSE);
			}

			break;
		}
	case TIMER_ID_MOUSE_LEAVE:
		{
			CPoint point; ::GetCursorPos(&point);
			HWND hwnd = ::WindowFromPoint(point);
			DWORD pid = 0;
			DWORD tid = ::GetWindowThreadProcessId(hwnd, &pid);

			if (pid != ::GetCurrentProcessId())
			{
				stopMouseLeaveTimer();
				stopShowTimer();
				startHideTimer();
			}

			break;
		}
	}

	CWnd::OnTimer(timerId);
}
