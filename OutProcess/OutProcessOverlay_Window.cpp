#include "pch.h"
#include "OutProcess.h"
#include "OutProcessOverlay.h"
#include "OutProcessViewer.h"

BEGIN_MESSAGE_MAP(COutProcessOverlay, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_NCRBUTTONDBLCLK()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CTLCOLOR()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

BOOL COutProcessOverlay::Create(CWnd* parent)
{
	return CWnd::CreateEx(0, 0, 0, 0, CRect(0, 0, 0, 0), parent, 0, 0);
}

BOOL COutProcessOverlay::PreCreateWindow(CREATESTRUCT& cs)
{
	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_NOCLOSE;
	wc.hCursor = ::LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc = AfxWndProc;
	wc.hInstance = AfxGetInstanceHandle();
	wc.lpszClassName = _T("OutProcessOverlay");
	AfxRegisterClass(&wc);
	cs.lpszName = _T("OutProcessOverlay");
	cs.lpszClass = _T("OutProcessOverlay");
	cs.style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.dwExStyle = WS_EX_TOOLWINDOW | WS_EX_LAYERED;
//	cs.hwndParent = 0;

	return CWnd::PreCreateWindow(cs);
}

BOOL COutProcessOverlay::PreTranslateMessage(MSG* msg)
{
	switch (msg->message)
	{
	case WM_KEYDOWN:
		{
			switch (msg->wParam)
			{
			case VK_ESCAPE:
				{
					hide();

					return TRUE;
				}
			case VK_TAB:
				{
					if (::GetKeyState(VK_SHIFT) < 0)
					{
						auto it = m_parts.find(m_focusedId);
						if (it != m_parts.end())
						{
							if (!it->second->onFocus(TRUE, TRUE))
								return TRUE;

							invalidate(it->second.get());

							if (it == m_parts.begin())
								it = m_parts.end();
							it--;
						}
						else
						{
							it = m_parts.begin();
						}

						if (it != m_parts.end())
						{
							if (!it->second->onFocus(FALSE, TRUE))
								return TRUE;

							m_focusedId = it->second->m_id;
							invalidate(it->second.get());
						}
					}
					else
					{
						auto it = m_parts.find(m_focusedId);
						if (it != m_parts.end())
						{
							if (!it->second->onFocus(TRUE, FALSE))
								return TRUE;

							invalidate(it->second.get());

							it++;
							if (it == m_parts.end())
								it = m_parts.begin();
						}
						else
						{
							it = m_parts.begin();
						}

						if (it != m_parts.end())
						{
							if (!it->second->onFocus(FALSE, FALSE))
								return TRUE;

							m_focusedId = it->second->m_id;
							invalidate(it->second.get());
						}
					}

					return TRUE;
				}
			}

			break;
		}
	}

	return CWnd::PreTranslateMessage(msg);
}

int COutProcessOverlay::OnCreate(LPCREATESTRUCT cs)
{
	if (CWnd::OnCreate(cs) == -1)
		return -1;

	CEditPart* windowName = new CEditPart(this, ID_WINDOW_NAME, L"Window Name",
		L"***ビューアウィンドウの名前を指定します***\n"
		L"複数のビューアを識別しやすいように自由に名前を付けることが出来ます\n"
		L"エディットボックスがフォーカスを失った時点で変更が反映されます\n"
		L"もしくはエンターキーを押すと即座に変更が反映されます",
		32);
	m_parts.insert(CParts::value_type(windowName->m_id, CPartPtr(windowName)));
	windowName->addItem(L"");

	CSelectPart* windowMode = new CSelectPart(this, ID_WINDOW_MODE, L"Window Mode",
		L"***ビューアウィンドウの表示モードを指定します***\n"
		L"Restore | ビューアウィンドウを通常表示にします\n"
		L"Minimize | ビューアウィンドウを非表示にします\n"
		L"Maximize | ビューアウィンドウを最大化します\n"
		L"Fullscreen | ビューアウィンドウを擬似フルスクリーン化します");
	m_parts.insert(CParts::value_type(windowMode->m_id, CPartPtr(windowMode)));
	windowMode->addItem(L"Restore");
	windowMode->addItem(L"Minimize");
	windowMode->addItem(L"Maximize");
	windowMode->addItem(L"Fullscreen");

	CSelectPart* stretchMode = new CSelectPart(this, ID_STRETCH_MODE, L"Stretch Mode",
		L"***画像の伸縮モードを指定します***\n"
		L"None | 手動で伸縮します\n"
		L"Fit | ウィンドウサイズに合わせて伸縮表示します\n"
		L"Crop | 画像がウィンドウ全域に表示されるように画像を切り出しながら伸縮表示します\n"
		L"Full | アスペクト比を無視してウィンドウ全域に伸縮表示します");
	m_parts.insert(CParts::value_type(stretchMode->m_id, CPartPtr(stretchMode)));
	stretchMode->addItem(L"None");
	stretchMode->addItem(L"Fit");
	stretchMode->addItem(L"Crop");
	stretchMode->addItem(L"Full");

	CSelectPart* sizeMode = new CSelectPart(this, ID_SIZE_MODE, L"Size Mode",
		L"***表示サイズモードを指定します***\n"
		L"Manual | 表示サイズを手動で指定します\n"
		L"Auto | 画像サイズを表示サイズにします");
	m_parts.insert(CParts::value_type(sizeMode->m_id, CPartPtr(sizeMode)));
	sizeMode->addItem(L"Manual");
	sizeMode->addItem(L"Auto");

	CSelectPart* updateMode = new CSelectPart(this, ID_UPDATE_MODE, L"Update Mode",
		L"***更新モードを指定します***\n"
		L"Manual | 手動で画像を更新します。左ダブルクリックしない限り画像は更新されません\n"
		L"Auto | ホストアプリでの変更に反応して自動的に画像を更新します");
	m_parts.insert(CParts::value_type(updateMode->m_id, CPartPtr(updateMode)));
	updateMode->addItem(L"Manual");
	updateMode->addItem(L"Auto");

	CSelectPart* frameMode = new CSelectPart(this, ID_FRAME_MODE, L"Frame Mode",
		L"***フレーム番号モードを指定します***\n"
		L"Relative | 画像を取得するフレームの番号を相対値で指定します\n"
		L"Absolute | 画像を取得するフレームの番号を絶対値で指定します");
	m_parts.insert(CParts::value_type(frameMode->m_id, CPartPtr(frameMode)));
	frameMode->addItem(L"Relative");
	frameMode->addItem(L"Absolute");

	CSelectPart* alphaChannelMode = new CSelectPart(this, ID_ALPHA_CHANNEL_MODE, L"Alpha Channel Mode",
		L"***透過モードを指定します***\n"
		L"None | アルファチャンネルを使用しません\n"
		L"Use | アルファチャンネルを使用します\n"
		L"Auto | シーンオプションの「アルファチャンネルあり」に連動します");
	m_parts.insert(CParts::value_type(alphaChannelMode->m_id, CPartPtr(alphaChannelMode)));
	alphaChannelMode->addItem(L"None");
	alphaChannelMode->addItem(L"Use");
	alphaChannelMode->addItem(L"Auto");

	CEditPart* windowNumerics = new CEditPart(this, ID_WINDOW_NUMERICS, L"Window Numerics\n(x, y, w, h)",
		L"ビューアウィンドウの位置やサイズを数値で指定します",
		3);
	m_parts.insert(CParts::value_type(windowNumerics->m_id, CPartPtr(windowNumerics)));
	windowNumerics->addItem(L"");
	windowNumerics->addItem(L"");
	windowNumerics->addItem(L"");
	windowNumerics->addItem(L"");

	CEditPart* imageNumerics = new CEditPart(this, ID_IMAGE_NUMERICS, L"Image Numerics\n(x, y, w, h, scale)",
		L"画像の表示位置や表示サイズを数値で指定します\n"
		L"Size ModeがAutoの場合はwとhは指定しても無視されます",
		3);
	m_parts.insert(CParts::value_type(imageNumerics->m_id, CPartPtr(imageNumerics)));
	imageNumerics->addItem(L"");
	imageNumerics->addItem(L"");
	imageNumerics->addItem(L"");
	imageNumerics->addItem(L"");
	imageNumerics->addItem(L"");

	CEditPart* frame = new CEditPart(this, ID_FRAME, L"Frame\n(offset, number, max number)",
		L"表示するフレーム番号を指定します\n"
		L"Frame ModeがRelativeのときはoffsetが使用されます\n"
		L"Frame ModeがAbsoluteのときはnumberが使用されます\n"
		L"max numberは参考用に表示しているだけなので設定しても意味はないです",
		3);
	m_parts.insert(CParts::value_type(frame->m_id, CPartPtr(frame)));
	frame->addItem(L"");
	frame->addItem(L"");
	frame->addItem(L"");

	calcLayout();

	return 0;
}

void COutProcessOverlay::OnDestroy()
{
	m_parts.clear();

	CWnd::OnDestroy();
}

void COutProcessOverlay::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; GetClientRect(&rc);

	int width = rc.Width();
	int height = rc.Height();

	BOOL isActive = GetActiveWindow() == this;

	Bitmap offScreenBitmap(width, height);
	Graphics g(&offScreenBitmap);

	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.TranslateTransform(-0.5f, -0.5f);

	MyRectF rect(rc);
	rect.Inflate(-m_outlineWidth / 2, -m_outlineWidth / 2);

	g.FillRectangle(m_brush.get(), rect);
	g.DrawRectangle(m_pen.get(), rect);

	for (auto it = m_parts.begin(); it != m_parts.end(); it++)
	{
		if (dc.RectVisible(CMyRect(it->second->m_rect)))
			it->second->paint(g, it->second->m_rect, isActive);
	}

	Graphics(dc).DrawImage(&offScreenBitmap, 0, 0);
}

void COutProcessOverlay::OnTimer(UINT_PTR timerId)
{
	for (auto it = m_parts.begin(); it != m_parts.end(); it++)
	{
		it->second->onTimer(timerId);
	}

	CWnd::OnTimer(timerId);
}

LRESULT COutProcessOverlay::OnNcHitTest(CPoint point)
{
	CPoint clientPoint(point);
	ScreenToClient(&clientPoint);

	for (auto it = m_parts.begin(); it != m_parts.end(); it++)
	{
		auto part = it->second;

		if (part->onHitTest(clientPoint))
		{
			if (m_hoveredId != part->m_id)
			{
				setHover(part->m_id);

				theApp.m_tooltip.show(part->m_tooltipText, point);
			}
			else
			{
				theApp.m_tooltip.updatePosition(point);
			}

			return HTCLIENT;
		}
	}

	setHover(0);

	theApp.m_tooltip.show(
		_T("***設定ウィンドウです***\n")
		_T("左ドラッグでウィンドウを移動します\n")
		_T("右クリックでウィンドウを閉じます"), point);

	return HTCAPTION;
}

void COutProcessOverlay::OnNcLButtonDblClk(UINT hitTest, CPoint point)
{
	CWnd::OnNcLButtonDblClk(hitTest, point);
}

void COutProcessOverlay::OnLButtonDblClk(UINT flags, CPoint point)
{
	CWnd::OnLButtonDblClk(flags, point);
}

void COutProcessOverlay::OnLButtonDown(UINT flags, CPoint point)
{
	for (auto it = m_parts.begin(); it != m_parts.end(); it++)
	{
		if (it->second->onLButtonDown(point))
			return;
	}

	CWnd::OnLButtonDown(flags, point);
}

void COutProcessOverlay::OnMouseMove(UINT flags, CPoint point)
{
	CWnd::OnMouseMove(flags, point);
}

void COutProcessOverlay::OnNcRButtonDblClk(UINT hitTest, CPoint point)
{
	CWnd::OnNcRButtonDblClk(hitTest, point);
}

void COutProcessOverlay::OnNcRButtonDown(UINT hitTest, CPoint point)
{
	hide();
//	CWnd::OnNcRButtonDown(hitTest, point);
}

void COutProcessOverlay::OnRButtonDblClk(UINT flags, CPoint point)
{
	CWnd::OnRButtonDblClk(flags, point);
}

void COutProcessOverlay::OnRButtonDown(UINT flags, CPoint point)
{
	hide();
//	CWnd::OnRButtonDown(flags, point);
}

void COutProcessOverlay::OnKeyDown(UINT keyCode, UINT repeat, UINT flags)
{
	auto it = m_parts.find(m_focusedId);

	if (it != m_parts.end())
		it->second->onKeyDown(keyCode);

	CWnd::OnKeyDown(keyCode, repeat, flags);
}

HBRUSH COutProcessOverlay::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (nCtlColor)
	{
	case CTLCOLOR_EDIT:
		{
			if (pWnd->GetFocus() == pWnd)
			{
				pDC->SetBkMode(OPAQUE);
				pDC->SetBkColor(m_editHotColor.ToCOLORREF());
				pDC->SetTextColor(m_editHotFontColor.ToCOLORREF());
			}
			else
			{
				pDC->SetBkMode(OPAQUE);
				pDC->SetBkColor(m_editColor.ToCOLORREF());
				pDC->SetTextColor(m_editFontColor.ToCOLORREF());
			}

			break;
		}
	}

	return brush;
}

void COutProcessOverlay::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	Invalidate(FALSE);
}
