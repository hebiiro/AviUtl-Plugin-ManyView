#pragma once

class COutProcessProxy : public CWinThread
{
public:

	static const UINT WM_PROXY_CLOSE = WM_APP + 1000;

	COutProcessProxy();
	virtual ~COutProcessProxy();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:

	afx_msg void OnProxyClose(WPARAM wParam, LPARAM lParam);
	afx_msg void OnManyViewWriteVideoInfo(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


