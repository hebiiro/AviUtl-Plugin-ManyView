#include "pch.h"
#include "OutProcess.h"
#include "OutProcessViewer.h"

BEGIN_MESSAGE_MAP(COutProcessViewer, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_TIMER()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_NCRBUTTONDBLCLK()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCMOUSEHOVER()
	ON_WM_NCMOUSELEAVE()
END_MESSAGE_MAP()

BOOL COutProcessViewer::PreCreateWindow(CREATESTRUCT& cs)
{
	MY_TRACE(_T("COutProcessViewer::PreCreateWindow()\n"));

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_NOCLOSE | CS_DROPSHADOW;
	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = OUT_PROCESS_WND_EXTRA_MAX_SIZE;
	wc.hInstance = AfxGetInstanceHandle();
	wc.hCursor = ::LoadCursor(0, IDC_CROSS);
	wc.lpszClassName = _T("OutProcessViewer");
	AfxRegisterClass(&wc);
	cs.lpszName = _T("新規プレビュー");
	cs.lpszClass = _T("OutProcessViewer");
	cs.style = WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.dwExStyle = WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY;
//	cs.dwExStyle = WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY | WS_EX_NOACTIVATE;
//	cs.hwndParent = AfxGetMainWnd()->GetSafeHwnd();

	return CWnd::PreCreateWindow(cs);
}

int COutProcessViewer::OnCreate(LPCREATESTRUCT cs)
{
	MY_TRACE(_T("COutProcessViewer::OnCreate()\n"));

	if (CWnd::OnCreate(cs) == -1)
		return -1;

	if (!initProxy())
		return -1;

	HWND hwnd = GetSafeHwnd();
	HWND dialog = theApp.m_mainProcessWindow;
	m_video.mutex = VideoInfoMutexPtr(new VideoInfoMutex(hwnd));
	m_video.fileMapping = VideoInfoFileMappingPtr(new VideoInfoFileMapping(hwnd, dialog));

	m_overlay.Create(this);

	return 0;
}

void COutProcessViewer::OnDestroy()
{
	MY_TRACE(_T("COutProcessViewer::OnDestroy()\n"));

	stopShowGuideTimer();
	stopHideGuideTimer();

	m_overlay.DestroyWindow();

	termProxy();

	CWnd::OnDestroy();
}

void COutProcessViewer::OnPaint()
{
	CPaintDC dc(this);

	if (!m_offscreenDC.isValid())
		return;

	CRect rc; GetClientRect(&rc);

	{
		CRect rc2 = rc;
		rc2.OffsetRect(-rc2.left, -rc2.top);
		paint(m_offscreenDC, rc2);
	}

	dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &m_offscreenDC, 0, 0, SRCCOPY);
}

void COutProcessViewer::OnMove(int x, int y)
{
	CWnd::OnMove(x, y);

	Invalidate(FALSE);
}

void COutProcessViewer::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	recalcLayout();
	m_offscreenDC.update(this);
}

void COutProcessViewer::OnGetMinMaxInfo(MINMAXINFO* mmi)
{
	CWnd::OnGetMinMaxInfo(mmi);
}

void COutProcessViewer::OnTimer(UINT_PTR timerId)
{
	switch (timerId)
	{
	case TIMER_ID_SHOW_GUIDE:
		{
			stopShowGuideTimer();
			startHideGuideTimer();

			break;
		}
	case TIMER_ID_HIDE_GUIDE:
		{
			ULONGLONG now = ::GetTickCount64();
			ULONGLONG elapsedTime = now - m_hideGuideStartTime;

			if (elapsedTime > m_hideGuideDuration)
				stopHideGuideTimer();

			Invalidate(FALSE);

			break;
		}
	case TIMER_ID_RBUTTON:
		{
			stopRButtonTimer();

			m_overlay.show(this);

			break;
		}
	}

	CWnd::OnTimer(timerId);
}

LRESULT COutProcessViewer::OnNcHitTest(CPoint point)
{
	if (GetCapture() == this)
		return HTCLIENT;

	int ht = hitTest(point);

	if (m_hitTest != ht)
	{
		m_hitTest = ht;
		Invalidate(FALSE);

		switch (ht)
		{
		case HTLEFT:			theApp.m_tooltip.show(_T("左辺\nここを左ドラッグするとビューアの左辺の位置を変更できます"), point); break;
		case HTRIGHT:			theApp.m_tooltip.show(_T("右辺\nここを左ドラッグするとビューアの右辺の位置を変更できます"), point); break;
		case HTTOP:				theApp.m_tooltip.show(_T("上辺\nここを左ドラッグするとビューアの上辺の位置を変更できます"), point); break;
		case HTBOTTOM:			theApp.m_tooltip.show(_T("下辺\nここを左ドラッグするとビューアの下辺の位置を変更できます"), point); break;
		case HTTOPLEFT:			theApp.m_tooltip.show(_T("左上角\nここを左ドラッグするとビューアの左上角の位置を変更できます"), point); break;
		case HTTOPRIGHT:		theApp.m_tooltip.show(_T("右上角\nここを左ドラッグするとビューアの右上角の位置を変更できます"), point); break;
		case HTBOTTOMLEFT:		theApp.m_tooltip.show(_T("左下角\nここを左ドラッグするとビューアの左下角の位置を変更できます"), point); break;
		case HTBOTTOMRIGHT:		theApp.m_tooltip.show(_T("右下角\nここを左ドラッグするとビューアの右下角の位置を変更できます"), point); break;
		case HTCAPTION:			theApp.m_tooltip.show(_T("タイトルバー\nここを左ドラッグするとビューアの位置を変更できます"), point); break;
		case HT_DRAWING_AREA:	theApp.m_tooltip.show(_T("描画領域\nここを左ドラッグすると描画位置を変更できます\nマウスホイールすると描画倍率を変更できます\n右クリックすると設定ウィンドウを開けます"), point); break;
		case HT_SEEK_BAR:		theApp.m_tooltip.show(_T("シークバー\nここを左ドラッグすると描画するフレーム番号を変更できます"), point); break;
		}
	}
	else
	{
		theApp.m_tooltip.updatePosition(point);
	}

	if (m_showGuideStartPoint != point)
		startShowGuideTimer(point);

	switch (ht)
	{
	case HT_DRAWING_AREA:
	case HT_SEEK_BAR:
		return HTCLIENT;
	}

	return ht;
}

void COutProcessViewer::OnNcLButtonDblClk(UINT hitTest, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnNcLButtonDblClk()\n"));

	requestVideoInfoEx();

	CWnd::OnNcLButtonDblClk(hitTest, point);
}

void COutProcessViewer::OnLButtonDblClk(UINT flags, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnLButtonDblClk()\n"));

	requestVideoInfoEx();

	CWnd::OnLButtonDblClk(flags, point);
}

void COutProcessViewer::OnLButtonDown(UINT flags, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnLButtonDown()\n"));

	switch (m_hitTest)
	{
	case HT_DRAWING_AREA:
		{
			MY_TRACE(_T("HT_DRAWING_AREA\n"));

			if (m_stretchMode != MANY_VIEW_STRETCH_MODE_NONE)
				break;

			theApp.m_tooltip.hide();

			stopShowGuideTimer();
			stopHideGuideTimer();

			m_dragStartPoint = point;
			m_dragStartImagePoint.x = m_image.x;
			m_dragStartImagePoint.y = m_image.y;
			SetCapture();

			break;
		}
	case HT_SEEK_BAR:
		{
			MY_TRACE(_T("HT_SEEK_BAR\n"));

			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
			theApp.m_tooltip.hide();

			stopShowGuideTimer();
			stopHideGuideTimer();

			m_dragStartPoint = point;
			m_dragStartFrameOffset = m_image.frameOffset;
			m_dragStartFrameNumber = m_image.frameNumber;
			SetCapture();

			break;
		}
	}

	CWnd::OnLButtonDown(flags, point);
}

void COutProcessViewer::OnLButtonUp(UINT flags, CPoint point)
{
	if (GetCapture() == this)
	{
		MY_TRACE(_T("COutProcessViewer::OnLButtonUp()\n"));

		switch (m_hitTest)
		{
		case HT_DRAWING_AREA:
		case HT_SEEK_BAR:
			{
				ReleaseCapture();

				break;
			}
		}
	}

	CWnd::OnLButtonUp(flags, point);
}

void COutProcessViewer::OnNcRButtonDblClk(UINT hitTest, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnNcRButtonDblClk()\n"));

	stopRButtonTimer();

	CWnd::OnNcRButtonDblClk(hitTest, point);
}

void COutProcessViewer::OnNcRButtonDown(UINT hitTest, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnNcRButtonDblClk()\n"));

//	startRButtonTimer();
	showContextMenu(hitTest, point);

	CWnd::OnNcRButtonDown(hitTest, point);
}

void COutProcessViewer::OnRButtonDblClk(UINT flags, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnRButtonDblClk()\n"));

	stopRButtonTimer();

	CWnd::OnRButtonDblClk(flags, point);
}

void COutProcessViewer::OnRButtonDown(UINT flags, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnRButtonDown()\n"));

//	startRButtonTimer();
	m_overlay.show(this);

	CWnd::OnRButtonDown(flags, point);
}

void COutProcessViewer::OnRButtonUp(UINT flags, CPoint point)
{
	CWnd::OnRButtonUp(flags, point);
}

void COutProcessViewer::OnMouseMove(UINT flags, CPoint point)
{
	if (GetCapture() == this)
	{
		MY_TRACE(_T("COutProcessViewer::OnMouseMove()\n"));

		switch (m_hitTest)
		{
		case HT_DRAWING_AREA:
			{
				MY_TRACE(_T("HT_DRAWING_AREA\n"));

				CPoint offset = point - m_dragStartPoint;
				m_image.x = m_dragStartImagePoint.x + offset.x;
				m_image.y = m_dragStartImagePoint.y + offset.y;

				Invalidate(FALSE);

				break;
			}
		case HT_SEEK_BAR:
			{
				MY_TRACE(_T("HT_SEEK_BAR\n"));

				CPoint offset = point - m_dragStartPoint;
				int frameOffset = m_dragStartFrameOffset + offset.x - offset.y * 10;
				int frameNumber = m_dragStartFrameNumber + offset.x - offset.y * 10;

				if (m_frameMode == MANY_VIEW_FRAME_MODE_ABSOLUTE)
					setFrameNumber(frameNumber);
				else
					setFrameOffset(frameOffset);

				if (m_updateMode == MANY_VIEW_UPDATE_MODE_MANUAL)
					Invalidate(FALSE);

				break;
			}
		}
	}

	{
		TRACKMOUSEEVENT tme = { sizeof(tme) };
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = GetSafeHwnd();
		::TrackMouseEvent(&tme);
	}

	CWnd::OnMouseMove(flags, point);
}

BOOL COutProcessViewer::OnMouseWheel(UINT flags, short delta, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnMouseWheel()\n"));

	if (delta == 0)
		return CWnd::OnMouseWheel(flags, delta, point);

	switch (m_stretchMode)
	{
	case MANY_VIEW_STRETCH_MODE_FIT:
	case MANY_VIEW_STRETCH_MODE_CROP:
	case MANY_VIEW_STRETCH_MODE_FULL:
		return CWnd::OnMouseWheel(flags, delta, point);
	}

	double factor;
	double add;

	if (m_image.scale > 1.0)
	{
		factor = 2.0;
	}
	else if (m_image.scale < 1.0)
	{
		factor = 20.0;
	}
	else
	{
		if (delta > 0)	factor = 2.0;
		else			factor = 20.0;
	}

	if (delta > 0)
	{
		add = 1.0;
	}
	else
	{
		add = -1.0;
	}

	CRect windowRect; GetWindowRect(&windowRect);
	CSize windowSize = windowRect.Size();
	auto prev_scale = m_image.scale;
	CPoint prev_center;
	prev_center.x = (int)((windowSize.cx / 2 - m_image.x) / prev_scale);
	prev_center.y = (int)((windowSize.cy / 2 - m_image.y) / prev_scale);
/*
	CPoint prev;
	prev.x = (int)((windowSize.cx / 2 + m_image.x) * m_image.scale);
	prev.y = (int)((windowSize.cy / 2 + m_image.y) * m_image.scale);
*/

	auto temp = m_image.scale;
	temp *= factor;
	temp = floor(temp);
	temp += add;
	temp /= factor;
	temp  = max(temp, 0.05);
	m_image.scale = temp;

	m_image.x = (int)(windowSize.cx / 2 - prev_center.x * m_image.scale);
	m_image.y = (int)(windowSize.cy / 2 - prev_center.y * m_image.scale);

/*
	m_image.x += (int)(windowSize.cx / 2 * (prev_scale - m_image.scale));
	m_image.y += (int)(windowSize.cy / 2 * (prev_scale - m_image.scale));
*/
/*
	m_image.x += (int)((windowSize.cx / 2 - m_image.x) * (prev_scale - m_image.scale));
	m_image.y += (int)((windowSize.cy / 2 - m_image.y) * (prev_scale - m_image.scale));
*/
/*
	CPoint current;
	current.x = (int)((windowSize.cx / 2 + m_image.x) * m_image.scale);
	current.y = (int)((windowSize.cy / 2 + m_image.y) * m_image.scale);

	m_image.x -= current.x - prev.x;
	m_image.y -= current.y - prev.y;
*/
/*
画像サイズ		100
ウィンドウサイズ	100
拡大率			1.00	描画位置	-50
拡大率			2.00	描画位置	-150	-100

画像サイズ		100
ウィンドウサイズ	100
拡大率			1.00	描画位置	0
拡大率			2.00	描画位置	-50		-50

画像サイズ		100
ウィンドウサイズ	100
拡大率			1.00	描画位置	50
拡大率			2.00	描画位置	50		+0

ウィンドウサイズ / 2 - 描画位置
50 - -50 = 100
50 -   0 = 50
50 -  50 = 0

画像サイズ		100
ウィンドウサイズ	100
拡大率			0.50	描画位置	0		中心		100
拡大率			0.50	描画位置	50		中心		0
拡大率			0.50	描画位置	-50		中心		200
拡大率			1.00	描画位置	0		中心		50
拡大率			1.00	描画位置	50		中心		0
拡大率			1.00	描画位置	-50		中心		100
拡大率			2.00	描画位置	0		中心		25
拡大率			2.00	描画位置	50		中心		0
拡大率			2.00	描画位置	-50		中心		50

拡大率			0.50	描画位置	0		中心		100
↓
拡大率			2.00	描画位置	-150

	prev_center = (windowSize / 2 - m_image) / prev_scale
	center = (windowSize / 2 - m_image) / m_image.scale
	m_image = 

	100 = (50 - 0) / 0.5
	-150 = windowSize / 2 - prev_center * m_image.scale
	m_image = m_image.scale - windowSize / 2

	prev_center = (windowSize / 2 - m_image) / prev_scale
	m_image = 
*/
	startShowGuideTimer(point);
//	Invalidate(FALSE);

	return CWnd::OnMouseWheel(flags, delta, point);
}

void COutProcessViewer::OnMouseHWheel(UINT flags, short delta, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnMouseHWheel()\n"));

	CWnd::OnMouseHWheel(flags, delta, point);
}

void COutProcessViewer::OnMouseHover(UINT flags, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnMouseHover()\n"));

	CWnd::OnMouseHover(flags, point);
}

void COutProcessViewer::OnMouseLeave()
{
	MY_TRACE(_T("COutProcessViewer::OnMouseLeave()\n"));

	m_hitTest = HTNOWHERE;
	Invalidate(FALSE);

	CWnd::OnMouseLeave();
}

void COutProcessViewer::OnNcMouseMove(UINT hitTest, CPoint point)
{
	{
		TRACKMOUSEEVENT tme = { sizeof(tme) };
		tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
		tme.hwndTrack = GetSafeHwnd();
		::TrackMouseEvent(&tme);
	}

	CWnd::OnNcMouseMove(hitTest, point);
}

void COutProcessViewer::OnNcMouseHover(UINT flags, CPoint point)
{
	MY_TRACE(_T("COutProcessViewer::OnNcMouseHover()\n"));

	CWnd::OnNcMouseHover(flags, point);
}

void COutProcessViewer::OnNcMouseLeave()
{
	MY_TRACE(_T("COutProcessViewer::OnNcMouseLeave()\n"));

	m_hitTest = HTNOWHERE;
	Invalidate(FALSE);

	CWnd::OnNcMouseLeave();
}
