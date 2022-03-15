#include "pch.h"
#include "OutProcess.h"
#include "OutProcessOverlay.h"
#include "OutProcessViewer.h"

BYTE COutProcessOverlay::m_alpha = 224;
int COutProcessOverlay::m_animationDuration = 250;

_bstr_t COutProcessOverlay::m_fontFamilyName = L"Yu Gothic UI";
REAL COutProcessOverlay::m_fontHeight = 16.0f;
int COutProcessOverlay::m_fontStyle = FontStyleBold;

Color COutProcessOverlay::m_color						(0xff, 0x01, 0x01, 0x01);
Color COutProcessOverlay::m_outlineColor					(0xff, 0x1f, 0x1f, 0x1f);
Color COutProcessOverlay::m_fontColor					(0xff, 0xff, 0xff, 0xff);
Color COutProcessOverlay::m_fontOutlineColor				(0xff, 0x00, 0x00, 0x00);
Color COutProcessOverlay::m_hotColor						(0xff, 0x29, 0x29, 0x29);
Color COutProcessOverlay::m_hotOutlineColor				(0xff, 0x1f, 0x1f, 0x1f);
Color COutProcessOverlay::m_hotFontColor					(0xff, 0xff, 0xff, 0xff);
Color COutProcessOverlay::m_hotFontOutlineColor			(0xff, 0x00, 0x00, 0x00);
REAL COutProcessOverlay::m_outlineWidth					= 5.0f;
REAL COutProcessOverlay::m_fontOutlineWidth				= 0.0f;

Color COutProcessOverlay::m_selectColor					(0xff, 0x66, 0x66, 0x66);
Color COutProcessOverlay::m_selectOutlineColor			(0xff, 0xcc, 0xcc, 0xcc);
Color COutProcessOverlay::m_selectFontColor				(0xff, 0xcc, 0xcc, 0xcc);
Color COutProcessOverlay::m_selectFontOutlineColor		(0xff, 0x00, 0x00, 0x00);
Color COutProcessOverlay::m_selectHotColor				(0xff, 0x00, 0x7c, 0xff);
Color COutProcessOverlay::m_selectHotOutlineColor		(0xff, 0xff, 0xff, 0xff);
Color COutProcessOverlay::m_selectHotFontColor			(0xff, 0xff, 0xff, 0xff);
Color COutProcessOverlay::m_selectHotFontOutlineColor	(0xff, 0x00, 0x00, 0x00);
REAL COutProcessOverlay::m_selectOutlineWidth			= 0.0f;
REAL COutProcessOverlay::m_selectFontOutlineWidth		= 0.0f;

Color COutProcessOverlay::m_editColor					(0xff, 0x66, 0x66, 0x66);
Color COutProcessOverlay::m_editOutlineColor				(0xff, 0x00, 0x00, 0x00);
Color COutProcessOverlay::m_editFontColor				(0xff, 0xff, 0xff, 0xff);
Color COutProcessOverlay::m_editHotColor					(0xff, 0xff, 0x7c, 0x00);
Color COutProcessOverlay::m_editHotOutlineColor			(0xff, 0x00, 0x00, 0x00);
Color COutProcessOverlay::m_editHotFontColor				(0xff, 0xff, 0xff, 0xff);
REAL COutProcessOverlay::m_editOutlineWidth				= 0.0f;

COutProcessOverlay::COutProcessOverlay(COutProcessViewer* viewer)
	: m_viewer(viewer)
{
	m_focusedId = 0;
	m_hoveredId = 0;
	m_editFontHeight = 0;
}

COutProcessOverlay::~COutProcessOverlay()
{
}

void COutProcessOverlay::loadStaticSettings(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("COutProcessOverlay::loadStaticSettings()\n"));
#if 1
	GetPrivateProfileInt(element, L"alpha", m_alpha);
	GetPrivateProfileInt(element, L"animationDuration", m_animationDuration);

	GetPrivateProfileBSTR(element, L"fontFamilyName", m_fontFamilyName);
	GetPrivateProfileFloat(element, L"fontHeight", m_fontHeight);
	GetPrivateProfileInt(element, L"fontStyle", m_fontStyle);

	GetPrivateProfileColor(element, L"color", m_color);
	GetPrivateProfileColor(element, L"outlineColor", m_outlineColor);
	GetPrivateProfileColor(element, L"fontColor", m_fontColor);
	GetPrivateProfileColor(element, L"fontOutlineColor", m_fontOutlineColor);
	GetPrivateProfileColor(element, L"hotColor", m_hotColor);
	GetPrivateProfileColor(element, L"hotOutlineColor", m_hotOutlineColor);
	GetPrivateProfileColor(element, L"hotFontColor", m_hotFontColor);
	GetPrivateProfileColor(element, L"hotFontOutlineColor", m_hotFontOutlineColor);
	GetPrivateProfileFloat(element, L"outlineWidth", m_outlineWidth);
	GetPrivateProfileFloat(element, L"fontOutlineWidth", m_fontOutlineWidth);

	GetPrivateProfileColor(element, L"selectColor", m_selectColor);
	GetPrivateProfileColor(element, L"selectOutlineColor", m_selectOutlineColor);
	GetPrivateProfileColor(element, L"selectFontColor", m_selectFontColor);
	GetPrivateProfileColor(element, L"selectFontOutlineColor", m_selectFontOutlineColor);
	GetPrivateProfileColor(element, L"selectHotColor", m_selectHotColor);
	GetPrivateProfileColor(element, L"selectHotOutlineColor", m_selectHotOutlineColor);
	GetPrivateProfileColor(element, L"selectHotFontColor", m_selectHotFontColor);
	GetPrivateProfileColor(element, L"selectHotFontOutlineColor", m_selectHotFontOutlineColor);
	GetPrivateProfileFloat(element, L"selectOutlineWidth", m_selectOutlineWidth);
	GetPrivateProfileFloat(element, L"selectFontOutlineWidth", m_selectFontOutlineWidth);

	GetPrivateProfileColor(element, L"editColor", m_editColor);
	GetPrivateProfileColor(element, L"editOutlineColor", m_editOutlineColor);
	GetPrivateProfileColor(element, L"editFontColor", m_editFontColor);
	GetPrivateProfileColor(element, L"editHotColor", m_editHotColor);
	GetPrivateProfileColor(element, L"editHotOutlineColor", m_editHotOutlineColor);
	GetPrivateProfileColor(element, L"editHotFontColor", m_editHotFontColor);
	GetPrivateProfileFloat(element, L"editOutlineWidth", m_editOutlineWidth);
#endif
}

void COutProcessOverlay::calcLayout()
{
	MY_TRACE(_T("COutProcessOverlay::calcLayout()\n"));

	SetLayeredWindowAttributes(0, m_alpha, LWA_ALPHA);

	m_fontFamily = Gdiplus::FontFamilyPtr(new Gdiplus::FontFamily(m_fontFamilyName));
	m_font = Gdiplus::FontPtr(new Gdiplus::Font(m_fontFamilyName, m_fontHeight, m_fontStyle, UnitWorld));

	m_brush = createBrush(m_color);
	m_pen = createPen(m_outlineColor, m_outlineWidth);
	m_fontBrush = createBrush(m_fontColor);
	m_fontPen = createPen(m_fontOutlineColor, m_fontOutlineWidth);
	m_hotBrush = createBrush(m_hotColor);
	m_hotPen = createPen(m_hotOutlineColor, m_fontOutlineWidth);
	m_hotFontBrush = createBrush(m_hotFontColor);
	m_hotFontPen = createPen(m_hotFontOutlineColor, m_fontOutlineWidth);

	m_selectBrush = createBrush(m_selectColor);
	m_selectPen = createPen(m_selectOutlineColor, m_selectOutlineWidth);
	m_selectFontBrush = createBrush(m_selectFontColor);
	m_selectFontPen = createPen(m_selectFontOutlineColor, m_selectFontOutlineWidth);
	m_selectHotBrush = createBrush(m_selectHotColor);
	m_selectHotPen = createPen(m_selectHotOutlineColor, m_selectOutlineWidth);
	m_selectHotFontBrush = createBrush(m_selectHotFontColor);
	m_selectHotFontPen = createPen(m_selectHotFontOutlineColor, m_selectOutlineWidth);

	m_editBrush = createBrush(m_editColor);
	m_editPen = createPen(m_editOutlineColor, m_editOutlineWidth);
	m_editHotBrush = createBrush(m_editHotColor);
	m_editHotPen = createPen(m_editHotOutlineColor, m_editOutlineWidth);

	setLineJoin(m_fontPen);
	setLineJoin(m_hotFontPen);
	setLineJoin(m_selectPen);
	setLineJoin(m_selectFontPen);
	setLineJoin(m_selectHotPen);
	setLineJoin(m_selectHotFontPen);
	setLineJoin(m_editPen);
	setLineJoin(m_editHotPen);

	if (m_editFont.GetSafeHandle())
		m_editFont.DeleteObject();

	int weight = 0;
	BYTE italic = 0;
	BYTE underline = 0;
	BYTE strikeOut = 0;

	if (m_fontStyle & FontStyleBold) weight = FW_BOLD;
	if (m_fontStyle & FontStyleItalic) italic = 1;
	if (m_fontStyle & FontStyleUnderline) underline = 1;
	if (m_fontStyle & FontStyleStrikeout) strikeOut = 1;

	m_editFont.CreateFont(
		(int)-m_fontHeight, 0, 0, 0,
		weight, italic, underline, strikeOut,
		DEFAULT_CHARSET, 0, 0, 0, 0, m_fontFamilyName);

	CClientDC dc(this);
	Graphics g(dc);

	{
		CFont* oldFont = dc.SelectObject(&m_editFont);
		CSize size = dc.GetTextExtent(_T("fy"));
		m_editFontHeight = size.cy;
		dc.SelectObject(oldFont);
	}

	REAL maxWidth = 0.0f;
	REAL maxHeight = 0.0f;
	REAL y = 0.0f;

	{
		// ここで CPart::m_rect を仮決定する。

		for (auto it = m_parts.begin(); it != m_parts.end(); it++)
		{
			auto part = it->second;

			part->preCalcLayout(g, y, &part->m_rect);
			maxWidth = max(maxWidth, part->m_rect.Width);
			y += part->m_rect.Height;
		}

		maxHeight = y;
	}

	maxWidth += m_outlineWidth * 2;
	maxHeight += m_outlineWidth * 2;

	SetWindowPos(0, 0, 0, (int)maxWidth, (int)maxHeight, SWP_NOZORDER | SWP_NOMOVE);

	{
		// ここで描画位置や子ウィンドウの位置を決定する。

		CRect rc; GetClientRect(&rc);
		MyRectF rect(rc);
		rect.Inflate(-m_outlineWidth, -m_outlineWidth);

		for (auto it = m_parts.begin(); it != m_parts.end(); it++)
		{
			auto part = it->second;

			part->m_rect.X += rect.X;
			part->m_rect.Y += rect.Y;
			part->m_rect.Width = rect.Width;

			it->second->calcLayout(g, part->m_rect);
		}
	}
}

void COutProcessOverlay::invalidate(CPart* part)
{
	part->onInvalidate();
	InvalidateRect(CMyRect(part->m_rect), FALSE);
}

void COutProcessOverlay::setFocus(int partId)
{
	if (m_focusedId == partId)
		return;

	auto it = m_parts.find(m_focusedId);
	if (it != m_parts.end())
		invalidate(it->second.get());

	m_focusedId = partId;

	it = m_parts.find(m_focusedId);
	if (it != m_parts.end())
		invalidate(it->second.get());
}

void COutProcessOverlay::setHover(int partId)
{
	if (m_hoveredId == partId)
		return;

	auto it = m_parts.find(m_hoveredId);
	if (it != m_parts.end())
		invalidate(it->second.get());

	m_hoveredId = partId;

	it = m_parts.find(m_hoveredId);
	if (it != m_parts.end())
		invalidate(it->second.get());
}

void COutProcessOverlay::show(CWnd* origin)
{
	downloadValue();

	CenterWindow(origin);
	ShowWindow(SW_SHOW);

	auto it = m_parts.begin();
	if (it != m_parts.end())
	{
		m_focusedId = it->second->m_id;
		it->second->onFocus(FALSE, FALSE);
	}
	else
	{
		m_focusedId = 0;
	}
}

void COutProcessOverlay::hide()
{
	ShowWindow(SW_HIDE);
}

void COutProcessOverlay::downloadValue()
{
	MY_TRACE(_T("COutProcessOverlay::downloadValue()\n"));

	WINDOWPLACEMENT wp = { sizeof(wp) };
	m_viewer->GetWindowPlacement(&wp);
	CRect rc(wp.rcNormalPosition);
	CString windowText; m_viewer->GetWindowText(windowText);

	MY_TRACE_STR((LPCTSTR)windowText);

	CSelectPart* select;
	CEditPart* edit;

	edit = static_cast<CEditPart*>(m_parts.find(ID_WINDOW_NAME)->second.get());
	edit->m_items[0]->SetWindowText(windowText);

	select = static_cast<CSelectPart*>(m_parts.find(ID_WINDOW_MODE)->second.get());
	select->init(m_viewer->m_windowMode);

	select = static_cast<CSelectPart*>(m_parts.find(ID_STRETCH_MODE)->second.get());
	select->init(m_viewer->m_stretchMode);

	select = static_cast<CSelectPart*>(m_parts.find(ID_SIZE_MODE)->second.get());
	select->init(m_viewer->m_sizeMode);

	select = static_cast<CSelectPart*>(m_parts.find(ID_UPDATE_MODE)->second.get());
	select->init(m_viewer->m_updateMode);

	select = static_cast<CSelectPart*>(m_parts.find(ID_FRAME_MODE)->second.get());
	select->init(m_viewer->m_frameMode);

	select = static_cast<CSelectPart*>(m_parts.find(ID_ALPHA_CHANNEL_MODE)->second.get());
	select->init(m_viewer->m_alphaChannelMode);

	edit = static_cast<CEditPart*>(m_parts.find(ID_WINDOW_NUMERICS)->second.get());
	edit->m_items[0]->SetWindowText(FormatText(rc.left));
	edit->m_items[1]->SetWindowText(FormatText(rc.top));
	edit->m_items[2]->SetWindowText(FormatText(rc.Width()));
	edit->m_items[3]->SetWindowText(FormatText(rc.Height()));

	edit = static_cast<CEditPart*>(m_parts.find(ID_IMAGE_NUMERICS)->second.get());
	edit->m_items[0]->SetWindowText(FormatText(m_viewer->m_image.x));
	edit->m_items[1]->SetWindowText(FormatText(m_viewer->m_image.y));
	edit->m_items[2]->SetWindowText(FormatText(m_viewer->m_image.w));
	edit->m_items[3]->SetWindowText(FormatText(m_viewer->m_image.h));
	edit->m_items[4]->SetWindowText(FormatText(m_viewer->m_image.scale));

	edit = static_cast<CEditPart*>(m_parts.find(ID_FRAME)->second.get());
	edit->m_items[0]->SetWindowText(FormatText(m_viewer->m_image.frameOffset));
	edit->m_items[1]->SetWindowText(FormatText(m_viewer->m_image.frameNumber));
	edit->m_items[2]->SetWindowText(FormatText(m_viewer->m_video.maxNumber));
}

void COutProcessOverlay::uploadValue(CPart* part)
{
	MY_TRACE(_T("COutProcessOverlay::uploadValue(0x%p)\n"), part);

	switch (part->m_id)
	{
	case ID_WINDOW_NAME:
		{
			CEditPart* edit = static_cast<CEditPart*>(part);
			CString text; edit->m_items[0]->GetWindowText(text);
			m_viewer->setWindowName(text);
			break;
		}
	case ID_WINDOW_MODE:		m_viewer->setWindowMode(part->getValue(-1)); break;
	case ID_STRETCH_MODE:		m_viewer->setStretchMode(part->getValue(-1)); break;
	case ID_SIZE_MODE:			m_viewer->setSizeMode(part->getValue(-1)); break;
	case ID_UPDATE_MODE:		m_viewer->setUpdateMode(part->getValue(-1)); break;
	case ID_FRAME_MODE:			m_viewer->setFrameMode(part->getValue(-1)); break;
	case ID_ALPHA_CHANNEL_MODE:	m_viewer->setAlphaChannelMode(part->getValue(-1)); break;
	case ID_WINDOW_NUMERICS:
		{
			CEditPart* edit = static_cast<CEditPart*>(part);
			CString text;
			edit->m_items[0]->GetWindowText(text); int x = _tcstol(text, 0, 0);
			edit->m_items[1]->GetWindowText(text); int y = _tcstol(text, 0, 0);
			edit->m_items[2]->GetWindowText(text); int w = _tcstol(text, 0, 0);
			edit->m_items[3]->GetWindowText(text); int h = _tcstol(text, 0, 0);
			WINDOWPLACEMENT wp = { sizeof(wp) };
			m_viewer->GetWindowPlacement(&wp);
			wp.rcNormalPosition.left = x;
			wp.rcNormalPosition.top = y;
			wp.rcNormalPosition.right = x + w;
			wp.rcNormalPosition.bottom = y + h;
			m_viewer->SetWindowPlacement(&wp);
			break;
		}
	case ID_IMAGE_NUMERICS:
		{
			CEditPart* edit = static_cast<CEditPart*>(part);
			CString text;
			edit->m_items[0]->GetWindowText(text); m_viewer->m_image.x = _tcstol(text, 0, 0);
			edit->m_items[1]->GetWindowText(text); m_viewer->m_image.y = _tcstol(text, 0, 0);
			edit->m_items[2]->GetWindowText(text); m_viewer->m_image.w = _tcstol(text, 0, 0);
			edit->m_items[3]->GetWindowText(text); m_viewer->m_image.h = _tcstol(text, 0, 0);
			edit->m_items[4]->GetWindowText(text); m_viewer->m_image.scale = _tcstod(text, 0);
			m_viewer->Invalidate(FALSE);
			break;
		}
	case ID_FRAME:
		{
			CEditPart* edit = static_cast<CEditPart*>(part);
			CString text;
			edit->m_items[0]->GetWindowText(text); m_viewer->setFrameOffset(_tcstol(text, 0, 0));
			edit->m_items[1]->GetWindowText(text); m_viewer->setFrameNumber(_tcstol(text, 0, 0));
			m_viewer->Invalidate(FALSE);
			break;
		}
	}
}
