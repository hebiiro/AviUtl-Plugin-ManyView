#pragma once

#include "OutProcessViewer.h"

typedef std::shared_ptr<COutProcessViewer> COutProcessViewerPtr;
typedef std::map<HWND, COutProcessViewerPtr> COutProcessViewerContainer;

class COutProcessDialog : public CDialogEx
{
public:

	static const int TIMER_ID_CHECK_MAIN_PROCESS = 2021;
	static const int TIMER_ID_CHECK_SETTING_FILE = 2022;

public:

	COutProcessViewerContainer m_viewers;
	CFileUpdateCheckerPtr m_fileUpdateChecker;

public:

	COutProcessDialog(CWnd* pParent = nullptr);
	virtual ~COutProcessDialog();

	void loadSystemSettings();

	void loadSettings();
	void saveSettings();

	BOOL showViewers(int show);
	COutProcessViewerPtr createViewer();
	BOOL destroyViewer(HWND hwnd);
	BOOL editViewer(HWND hwnd, HWND origin);
	BOOL readFrameInfo(HWND hwnd);
	BOOL readVideoInfo(HWND hwnd);

	COutProcessViewerPtr getViewer(HWND hwnd)
	{
		auto it = m_viewers.find(hwnd);

		if (it == m_viewers.end())
			return 0;

		return it->second;
	}

protected:

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OUT_PROCESS };
#endif

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
};
