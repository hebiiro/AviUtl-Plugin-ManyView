#pragma once

#include "resource.h"
#include "OutProcessDialog.h"

class COutProcessApp : public CWinApp
{
public:

	HWND m_mainProcessWindow;
	int m_vram_w;
	int m_vram_h;
	int m_vram_size;
	COutProcessDialog m_dialog;
	COutProcessToolTip m_tooltip;

public:

	COutProcessApp();
	virtual ~COutProcessApp();

	HWND getAviUtlWindow();
	HWND getFilterWindow();
	HWND getFilterDialog();

	void postMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		::PostMessage(m_mainProcessWindow, message, wParam, lParam);
	}

	void sendMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		::SendMessage(m_mainProcessWindow, message, wParam, lParam);
	}

	LONG getWindowLong(int index)
	{
		return ::GetWindowLong(m_mainProcessWindow, index);
	}

public:

	virtual BOOL InitInstance();
	afx_msg void OnManyViewShowViewers(WPARAM wParam, LPARAM lParam);
	afx_msg void OnManyViewCreateViewer(WPARAM wParam, LPARAM lParam);
	afx_msg void OnManyViewDestroyViewer(WPARAM wParam, LPARAM lParam);
	afx_msg void OnManyViewEditViewer(WPARAM wParam, LPARAM lParam);
	afx_msg void OnManyViewReadFrameInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnManyViewReadVideoInfo(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

extern COutProcessApp theApp;
