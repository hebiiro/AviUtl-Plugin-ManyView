#pragma once

class CInProcessViewer
{
public:

	HWND m_hwnd;
	VideoInfoMutexPtr m_mutex;
	VideoInfoFileMappingPtr m_fileMapping;

	CInProcessViewer();
	virtual ~CInProcessViewer();

	BOOL writeVideoInfo(FILTER* fp, void* editp);
};
