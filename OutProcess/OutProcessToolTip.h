#pragma once

class COutProcessToolTip : public CWnd
{
public:

	static const int TIMER_ID_SHOW = 2020;
	static const int TIMER_ID_HIDE = 2021;
	static const int TIMER_ID_MOUSE_LEAVE = 2022;

	static BYTE m_alpha;
	static int m_showDuration;
	static int m_hideDuration;

	static _bstr_t m_fontFamilyName;
	static int m_fontHeight;

	static Color m_fontColor;
	static Color m_fontOutlineColor;
	static int m_fontOutlineWidth;

	static Color m_backgroundColor;
	static Color m_backgroundOutlineColor;
	static int m_backgroundOutlineWidth;

public:

	_bstr_t m_text;
	ULONGLONG m_showStartTime;
	ULONGLONG m_hideStartTime;

public:

	COutProcessToolTip();
	virtual ~COutProcessToolTip();

	static void loadStaticSettings(const MSXML2::IXMLDOMElementPtr& element);

	void show(LPCTSTR text, const POINT& point);
	void hide();
	void updatePosition(const POINT& point);

	void startShowTimer();
	void stopShowTimer();

	void startHideTimer();
	void stopHideTimer();

	void startMouseLeaveTimer();
	void stopMouseLeaveTimer();

public:

	virtual BOOL Create(CWnd* parent);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT cs);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR timerId);
	DECLARE_MESSAGE_MAP()
};


