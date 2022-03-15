#include "pch.h"
#include "OutProcess.h"
#include "OutProcessViewer.h"

class Painter
{
public:

	COutProcessViewer& viewer;
	Graphics& g;

	FontFamilyPtr fontFamily;
	StringFormat stringFormat;

	SolidBrushPtr brush;
	PenPtr pen;
	SolidBrushPtr fontBrush;
	PenPtr fontPen;

	Painter(COutProcessViewer& viewer, Graphics& g, BYTE alpha)
		: viewer(viewer)
		, g(g)
	{
		fontFamily = Gdiplus::FontFamilyPtr(new Gdiplus::FontFamily(COutProcessViewer::m_fontFamilyName));

		stringFormat.SetAlignment(StringAlignmentCenter);
		stringFormat.SetLineAlignment(StringAlignmentCenter);

		brush = createBrush(MyColor(alpha, COutProcessViewer::m_color));
		pen = createPen(MyColor(alpha, COutProcessViewer::m_outlineColor), COutProcessViewer::m_outlineWidth);

		fontBrush = createBrush(MyColor(alpha, COutProcessViewer::m_fontColor));
		fontPen = createPen(MyColor(alpha, COutProcessViewer::m_fontOutlineColor), COutProcessViewer::m_fontOutlineWidth);

		setLineJoin(fontPen);
	}

	void drawParts(const CRect& rc)
	{
		GraphicsPath path;

		for (auto it = viewer.m_parts.begin(); it != viewer.m_parts.end(); it++)
		{
			if (it->first == viewer.HT_SEEK_BAR)
				continue;

			if (it->first == viewer.m_hitTest)
				g.FillRectangle(brush.get(), it->second->rc);

			path.AddRectangle(it->second->rc);
		}

		g.DrawPath(pen.get(), &path);
	}

	void drawTexts(const CRect& rc)
	{
		{
			// Draw Header.

			WCHAR windowText[MAX_PATH];
			::GetWindowTextW(viewer.GetSafeHwnd(), windowText, MAX_PATH);
			CRect windowRect;
			::GetWindowRect(viewer.GetSafeHwnd(), &windowRect);

			WCHAR text[MAX_PATH];
			::StringCbPrintfW(text, sizeof(text), L"%s, %d, %d, %dx%d",
				windowText,
				windowRect.left, windowRect.top,
				windowRect.Width(), windowRect.Height());

			switch (viewer.m_windowMode)
			{
			default:
			case MANY_VIEW_WINDOW_MODE_RESTORE:    ::StringCbCatW(text, sizeof(text), L" | Restore"); break;
			case MANY_VIEW_WINDOW_MODE_MINIMIZE:   ::StringCbCatW(text, sizeof(text), L" | Minimize"); break;
			case MANY_VIEW_WINDOW_MODE_MAXIMIZE:   ::StringCbCatW(text, sizeof(text), L" | Maximize"); break;
			case MANY_VIEW_WINDOW_MODE_FULLSCREEN: ::StringCbCatW(text, sizeof(text), L" | Fullscreen"); break;
			}

			switch (viewer.m_stretchMode)
			{
			default:
			case MANY_VIEW_STRETCH_MODE_NONE: break;
			case MANY_VIEW_STRETCH_MODE_FIT:  ::StringCbCatW(text, sizeof(text), L" | Fit"); break;
			case MANY_VIEW_STRETCH_MODE_CROP: ::StringCbCatW(text, sizeof(text), L" | Crop"); break;
			case MANY_VIEW_STRETCH_MODE_FULL: ::StringCbCatW(text, sizeof(text), L" | Full"); break;
			}

			Rect rect(rc.left, rc.top, rc.Width(), viewer.m_thickFrameWidth);

			drawString(text, (REAL)viewer.m_fontHeight, rect);
		}

		{
			// Draw Footer.

			const CSize imageSize = viewer.paint_getImageSize();

			WCHAR text[MAX_PATH];
			::StringCbPrintfW(text, sizeof(text), L"%d, %d, %dx%d / %dx%d (%0.2f%%), %d/%d",
				viewer.m_image.x,viewer. m_image.y,
				(int)(imageSize.cx * viewer.m_image.scale), (int)(imageSize.cy * viewer.m_image.scale),
				imageSize.cx, imageSize.cy, viewer.m_image.scale * 100, viewer.m_video.number + 1, viewer.m_video.maxNumber);

			switch (viewer.m_frameMode)
			{
			case MANY_VIEW_FRAME_MODE_RELATIVE:
				{
					WCHAR text2[MAX_PATH];
					::StringCbPrintfW(text2, sizeof(text2), L"(%+d)", viewer.m_image.frameOffset);
					::StringCbCatW(text, sizeof(text), text2);

					break;
				}
			default:
			case MANY_VIEW_FRAME_MODE_ABSOLUTE:
				{
					break;
				}
			}

			Rect rect(rc.left, rc.bottom - viewer.m_thickFrameWidth, rc.Width(), viewer.m_thickFrameWidth);

			drawString(text, (REAL)viewer.m_fontHeight, rect);
		}
	}

	void drawSeekBar(int ht)
	{
		auto it = viewer.m_parts.find(ht);

		if (it == viewer.m_parts.end())
			return;

		auto rc = it->second->rc;

		REAL ox = (REAL)rc.X + rc.Width / 2.0f;
		REAL oy = (REAL)rc.Y + rc.Height / 2.0f;

		if (viewer.m_seekBarGridCount > 0)
		{
			// 放射状のグリッドを描画する。

			REAL angle = 360.0f / viewer.m_seekBarGridCount;
			REAL size = viewer.m_seekBarSize / 2.0f - viewer.m_seekBarWidth;
			REAL length = viewer.m_seekBarSize / 2.0f + viewer.m_seekBarGridLength;

			Matrix matrix;
			g.GetTransform(&matrix);
			g.TranslateTransform(ox, oy);
			for (int i = 0; i < viewer.m_seekBarGridCount; i++)
			{
				g.DrawLine(this->pen.get(), 0.0f, -length, 0.0f, -size);
				g.RotateTransform(angle);
			}
			g.SetTransform(&matrix);
		}

		REAL number = (REAL)viewer.m_video.number + 1;
		REAL maxNumber = (REAL)viewer.m_video.maxNumber;
		REAL angle = maxNumber ? ((number - maxNumber) / maxNumber * 360) : 0.0f;
		REAL size = (REAL)viewer.m_seekBarSize;

		angle = max(360 + angle, 0);
		
//		MY_TRACE(_T("angle = %f\n"), angle);

		drawArc(g, this->pen.get(), this->brush.get(),
			ox - size / 2, oy - size / 2, size, size, (REAL)viewer.m_seekBarWidth, angle);

		{
			REAL size = (REAL)viewer.m_seekBarSize - viewer.m_seekBarWidth * 4;

			if (viewer.m_hitTest == ht)
			{
				g.FillEllipse(this->brush.get(), ox - size / 2, oy - size / 2, size, size);
				g.DrawEllipse(this->pen.get(), ox - size / 2, oy - size / 2, size, size);
			}
		}
	}

	void drawArc(Graphics& g, Pen* pen, Brush* brush, REAL ox, REAL oy, REAL w, REAL h, REAL width, REAL angle)
	{
		RectF rect(ox, oy, w, h);

		GraphicsPath path;

		path.StartFigure();
		path.AddArc(rect, 270, -angle);
		rect.Inflate(-width, -width);
		path.AddArc(rect, 270 - angle, angle);
		path.CloseFigure();

		if (brush) g.FillPath(brush, &path);
		if (pen) g.DrawPath(pen, &path);
	}

	void drawPolygon(BOOL fill, const Point* pts, int count)
	{
		if (fill)
			g.FillPolygon(this->brush.get(), pts, count);
		g.DrawPolygon(this->pen.get(), pts, count);
	}

	void drawRectangle(BOOL fill, int x, int y, int w, int h)
	{
		if (fill)
			g.FillRectangle(this->brush.get(), x, y, w, h);
		g.DrawRectangle(this->pen.get(), x, y, w, h);
	}

	void drawString(LPCWSTR text, REAL emSize, const Rect& rect)
	{
		GraphicsPath path;
		path.AddString(text, -1, this->fontFamily.get(),
			FontStyleBold, emSize, rect, &stringFormat);
		g.DrawPath(this->fontPen.get(), &path);
		g.FillPath(this->fontBrush.get(), &path);
	}
};

BOOL COutProcessViewer::paint_isDragging(int ht)
{
	if (GetCapture() != this)
		return FALSE;

	switch (ht)
	{
	case HTCAPTION:
	case HTLEFT:
	case HTRIGHT:
	case HTTOP:
	case HTTOPLEFT:
	case HTTOPRIGHT:
	case HTBOTTOM:
	case HTBOTTOMLEFT:
	case HTBOTTOMRIGHT:
	case HT_DRAWING_AREA:
	case HT_SEEK_BAR:
		return TRUE;
	}

	return FALSE;
}

BYTE COutProcessViewer::paint_calcAlpha()
{
	if (m_showGuideStartTime)
	{
		return 255;
	}
	else if (m_hideGuideStartTime)
	{
		ULONGLONG nowTime = ::GetTickCount64();
		ULONGLONG elapsedTime = nowTime - m_hideGuideStartTime;

		return (BYTE)max(0, 255 - (double)elapsedTime / m_hideGuideDuration * 255);
	}

	return 0;
}

CSize COutProcessViewer::paint_getImageSize()
{
	if (m_sizeMode == MANY_VIEW_SIZE_MODE_MANUAL)
	{
		return CSize(m_image.w, m_image.h);
	}
	else if (!m_video.bits.IsNull())
	{
		return CSize(m_video.bits.GetWidth(), m_video.bits.GetHeight());
	}

	return CSize(0, 0);
}

void COutProcessViewer::paint(CDC& dc, const CRect& rc)
{
	int saveDC = dc.SaveDC();

//	dc.SetStretchBltMode(COLORONCOLOR);
	dc.SetStretchBltMode(HALFTONE);

	if (m_patternBrush.GetSafeHandle())
		dc.FillRect(rc, &m_patternBrush);
	else
		dc.FillSolidRect(rc, m_backgroundColor.ToCOLORREF());

	if (!m_video.bits.IsNull())
		paint_drawImage(dc, rc);

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
//	g.SetCompositingMode(CompositingModeSourceOver);
//	g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
//	g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.TranslateTransform(-0.5f, -0.5f);

	if (paint_isDragging(m_hitTest))
	{
		Painter painter(*this, g, 255);

		painter.drawTexts(rc);
		painter.drawSeekBar(HT_SEEK_BAR);
	}
	else
	{
//		BYTE alpha = 255;
		BYTE alpha = paint_calcAlpha();

		if (alpha)
		{
			Painter painter(*this, g, alpha);

			painter.drawParts(rc);
			painter.drawTexts(rc);
			painter.drawSeekBar(HT_SEEK_BAR);
		}
	}

	dc.RestoreDC(saveDC);
}

void COutProcessViewer::paint_drawImage(CDC& dc, const CRect& rc)
{
	switch (m_stretchMode)
	{
	default:
	case MANY_VIEW_STRETCH_MODE_NONE: paint_drawImage_None(dc, rc); break;
	case MANY_VIEW_STRETCH_MODE_FIT:  paint_drawImage_Fit(dc, rc); break;
	case MANY_VIEW_STRETCH_MODE_CROP: paint_drawImage_Crop(dc, rc); break;
	case MANY_VIEW_STRETCH_MODE_FULL: paint_drawImage_Full(dc, rc); break;
	}
}

void COutProcessViewer::paint_drawImage_None(CDC& dc, const CRect& rc)
{
	const CSize imageSize = paint_getImageSize();
	const int bits_w = m_video.bits.GetWidth();
	const int bits_h = m_video.bits.GetHeight();
	const int draw_w = (int)(imageSize.cx * m_image.scale);
	const int draw_h = (int)(imageSize.cy * m_image.scale);

	struct { int x, y, w, h; } dst;
	struct { int x, y, w, h; } src;

	src.x = 0;
	src.y = 0;
	src.w = bits_w;
	src.h = bits_h;

	dst.x = rc.left + m_image.x;
	dst.y = rc.top + m_image.y;
	dst.w = draw_w;
	dst.h = draw_h;

	m_video.bits.AlphaBlend(dc,
		dst.x, dst.y, dst.w, dst.h,
		src.x, src.y, src.w, src.h);
}

void COutProcessViewer::paint_drawImage_Fit(CDC& dc, const CRect& rc)
{
	const int bits_w = m_video.bits.GetWidth();
	const int bits_h = m_video.bits.GetHeight();
	const int rc_w = rc.Width();
	const int rc_h = rc.Height();

	struct { int x, y, w, h; } dst;
	struct { int x, y, w, h; } src;

	src.x = 0;
	src.y = 0;
	src.w = bits_w;
	src.h = bits_h;

	dst.w = rc_w;
	dst.h = rc_w * bits_h / bits_w;

	if (dst.h > rc_h)
	{
		dst.w = rc_h * bits_w / bits_h;
		dst.h = rc_h;
	}

	dst.x = rc.left + (rc_w - dst.w) / 2;
	dst.y = rc.top + (rc_h - dst.h) / 2;

	m_video.bits.AlphaBlend(dc,
		dst.x, dst.y, dst.w, dst.h,
		src.x, src.y, src.w, src.h);
}

void COutProcessViewer::paint_drawImage_Crop(CDC& dc, const CRect& rc)
{
	const int bits_w = m_video.bits.GetWidth();
	const int bits_h = m_video.bits.GetHeight();
	const int rc_w = rc.Width();
	const int rc_h = rc.Height();

	struct { int x, y, w, h; } dst;
	struct { int x, y, w, h; } src;

	src.x = 0;
	src.y = 0;
	src.w = bits_w;
	src.h = bits_h;

	dst.w = rc_w;
	dst.h = rc_w * bits_h / bits_w;

	if (dst.h < rc_h)
	{
		dst.w = rc_h * bits_w / bits_h;
		dst.h = rc_h;
	}

	dst.x = rc.left + (rc_w - dst.w) / 2;
	dst.y = rc.top + (rc_h - dst.h) / 2;

	m_video.bits.AlphaBlend(dc,
		dst.x, dst.y, dst.w, dst.h,
		src.x, src.y, src.w, src.h);
}

void COutProcessViewer::paint_drawImage_Full(CDC& dc, const CRect& rc)
{
	const int bits_w = m_video.bits.GetWidth();
	const int bits_h = m_video.bits.GetHeight();

	struct { int x, y, w, h; } dst;
	struct { int x, y, w, h; } src;

	src.x = 0;
	src.y = 0;
	src.w = bits_w;
	src.h = bits_h;

	dst.x = rc.left;
	dst.y = rc.top;
	dst.w = rc.Width();
	dst.h = rc.Height();

	m_video.bits.AlphaBlend(dc,
		dst.x, dst.y, dst.w, dst.h,
		src.x, src.y, src.w, src.h);
}
