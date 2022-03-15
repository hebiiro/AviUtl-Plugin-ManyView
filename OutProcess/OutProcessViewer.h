#pragma once

#include "OutProcessProxy.h"
#include "OutProcessToolTip.h"
#include "OutProcessOverlay.h"

class COutProcessViewer : public CWnd
{
public:

	struct Label
	{
		int value;
		LPCWSTR label;
	};

	static const int HT_DRAWING_AREA = 1000;
	static const int HT_SEEK_BAR     = 1001;

	static const int TIMER_ID_SHOW_GUIDE = 2020;
	static const int TIMER_ID_HIDE_GUIDE = 2021;
	static const int TIMER_ID_RBUTTON    = 2022;

	static const Label m_windowModeLabel[];
	static const Label m_stretchModeLabel[];
	static const Label m_sizeModeLabel[];
	static const Label m_frameModeLabel[];
	static const Label m_updateModeLabel[];
	static const Label m_alphaChannelModeLabel[];

public:

	class CPart
	{
	public:

		int ht;
		Rect rc;

		CPart(int ht, int x, int y, int w, int h)
			: ht(ht)
			, rc(x, y, w, h)
		{
		}
	};

	typedef std::shared_ptr<CPart> CPartPtr;
	typedef std::map<int, CPartPtr> CParts;

public:

	static int m_showGuideDuration;
	static int m_hideGuideDuration;

	static _bstr_t m_fontFamilyName;
	static REAL m_fontHeight;
	static int m_fontStyle;

	static Color m_color;
	static Color m_outlineColor;
	static REAL m_outlineWidth;

	static Color m_fontColor;
	static Color m_fontOutlineColor;
	static REAL m_fontOutlineWidth;

	static int m_thickFrameWidth;
	static int m_captionHeight;
	static int m_seekBarSize;
	static int m_seekBarWidth;
	static int m_seekBarGridCount;
	static int m_seekBarGridLength;

	static int m_patternSize;
	static Color m_backgroundColor;
	static Color m_backgroundSubColor;

public:

	COutProcessProxy m_proxy;

	int m_hitTest;
	CParts m_parts;
	CPoint m_dragStartPoint;
	CPoint m_dragStartImagePoint;
	int m_dragStartFrameOffset;
	int m_dragStartFrameNumber;
	CPoint m_showGuideStartPoint;
	ULONGLONG m_showGuideStartTime;
	ULONGLONG m_hideGuideStartTime;
	ULONGLONG m_rbuttonStartTime;
	COffscreenDC m_offscreenDC;
	CBrush m_patternBrush;
	struct {
		int x, y, w, h;
		double scale;
		int frameOffset, frameNumber;
	} m_image;
	struct {
		int number, maxNumber;
		CImage bits;
		VideoInfoMutexPtr mutex;
		VideoInfoFileMappingPtr fileMapping;
	} m_video;
	COutProcessOverlay m_overlay;
	int m_windowMode;
	int m_stretchMode;
	int m_sizeMode;
	int m_frameMode;
	int m_updateMode;
	int m_alphaChannelMode;

public:

	COutProcessViewer();
	virtual ~COutProcessViewer();

	BOOL initProxy();
	BOOL termProxy();

	static void loadStaticSettings(const MSXML2::IXMLDOMElementPtr& element);

	void loadSettings(const MSXML2::IXMLDOMElementPtr& element);
	void saveSettings(const MSXML2::IXMLDOMElementPtr& element);

	BOOL paint_isDragging(int ht);
	BYTE paint_calcAlpha();
	CSize paint_getImageSize();

	void paint(CDC& dc, const CRect& rc);
	void paint_drawImage(CDC& dc, const CRect& rc);
	void paint_drawImage_None(CDC& dc, const CRect& rc);
	void paint_drawImage_Fit(CDC& dc, const CRect& rc);
	void paint_drawImage_Crop(CDC& dc, const CRect& rc);
	void paint_drawImage_Full(CDC& dc, const CRect& rc);

	void readFrameInfo();
	void readVideoInfo();
	void insertPart(int ht, int x, int y, int w, int h);
	void recalcLayout();
	int hitTest(CPoint point);
	void showContextMenu(UINT hitTest, CPoint point);

	void startShowGuideTimer();
	void startShowGuideTimer(const POINT& point);
	void stopShowGuideTimer();

	void startHideGuideTimer();
	void stopHideGuideTimer();

	void startRButtonTimer();
	void stopRButtonTimer();

	void setWindowName(LPCTSTR windowName);
	void setWindowMode(int windowMode);
	void setStretchMode(int stretchMode);
	void setSizeMode(int sizeMode);
	void setFrameMode(int frameMode);
	void setUpdateMode(int updateMode);
	void setAlphaChannelMode(int alphaChannelMode);
	void setFrameOffset(int frameOffset);
	void setFrameNumber(int frameNumber);
	int getFrameNumber();
	void requestVideoInfoEx();

public:

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT cs);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* mmi);
	afx_msg void OnTimer(UINT_PTR timerId);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT hitTest, CPoint point);
	afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnNcRButtonDblClk(UINT hitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT hitTest, CPoint point);
	afx_msg void OnRButtonDblClk(UINT flags, CPoint point);
	afx_msg void OnRButtonDown(UINT flags, CPoint point);
	afx_msg void OnRButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint point);
	afx_msg void OnMouseHWheel(UINT flags, short delta, CPoint point);
	afx_msg void OnMouseHover(UINT flags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnNcMouseLeave();
	DECLARE_MESSAGE_MAP()
};
