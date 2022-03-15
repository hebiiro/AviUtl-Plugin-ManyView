#include "pch.h"
#include "InProcess.h"
#include "InProcessViewer.h"

CInProcessViewer::CInProcessViewer()
{
	m_hwnd = 0;
}

CInProcessViewer::~CInProcessViewer()
{
}

BOOL CInProcessViewer::writeVideoInfo(FILTER* fp, void* editp)
{
	MY_TRACE(_T("CInProcessViewer::writeVideoInfo()\n"));

	int frameNumber = (int)::GetWindowLong(m_hwnd, OUT_PROCESS_WND_EXTRA_FRAME_NUMBER);
//	int frameNumber = fp->exfunc->get_frame(editp);
	int frameMaxNumber = fp->exfunc->get_frame_n(editp);

	BOOL useAlpha = FALSE;

	int alphaChannelMode = (int)::GetWindowLong(m_hwnd, OUT_PROCESS_WND_EXTRA_ALPHA_CHANNEL_MODE);

	switch (alphaChannelMode)
	{
	case MANY_VIEW_ALPHA_CHANNEL_MODE_NONE: useAlpha = FALSE; break;
	case MANY_VIEW_ALPHA_CHANNEL_MODE_USE: useAlpha = TRUE; break;
	case MANY_VIEW_ALPHA_CHANNEL_MODE_AUTO:
		{
			int scene = theApp.m_memref.Exedit_SceneDisplaying();
			auls::SCENE_SETTING* ss = theApp.m_memref.Exedit_SceneSetting();

			if (ss[scene].flag & ss[scene].FLAG_ALPHA)
				useAlpha = TRUE;
			else
				useAlpha = FALSE;

			break;
		}
	}

	if (0)
	{
		// 画像サイズを取得する。
		int width = 0, height = 0;
		fp->exfunc->get_pixel_filtered(editp, frameNumber, NULL, &width, &height);

		int cacheNumber = 1;
		BOOL result = fp->exfunc->set_ycp_filtering_cache_size(fp, width, height, cacheNumber, 0);
		MY_TRACE_NUM(result);

		{
			Synchronizer sync(*m_mutex);
			VideoInfoFileMapping::Buffer buffer(m_fileMapping.get());
			VideoInfo* info = buffer.getInfo();
			ColorRGB* src1 = buffer.getSrc1();
			ColorRGB* src2 = buffer.getSrc2();

			info->frame.number = frameNumber;
			info->frame.maxNumber = frameMaxNumber;
			info->frame.w = width;
			info->frame.h = height;
			info->useAlpha = useAlpha;

			MY_TRACE_NUM(info->frame.number);
			MY_TRACE_NUM(info->frame.maxNumber);
			MY_TRACE_NUM(info->frame.w);
			MY_TRACE_NUM(info->frame.h);
			MY_TRACE_NUM(info->useAlpha);

			DWORD start = ::timeGetTime();

			int w = 0, h = 0;
			PIXEL_YC* yc = fp->exfunc->get_ycp_filtering_cache_ex(fp, editp, frameNumber, &w, &h);
			fp->exfunc->yc2rgb((PIXEL*)src1, yc, w * h);

			if (useAlpha)
			{
				theApp.m_do_draw_white = TRUE;
				int w = 0, h = 0;
				PIXEL_YC* yc = fp->exfunc->get_ycp_filtering_cache_ex(fp, editp, frameNumber, &w, &h);
				fp->exfunc->yc2rgb((PIXEL*)src2, yc, w * h);
				theApp.m_do_draw_white = FALSE;
			}

			DWORD end = ::timeGetTime();

			MY_TRACE(_T("CInProcessViewer::writeVideoInfo() => %dms.\n"), end - start);
		}
	}
	else
	{
		// 画像サイズを取得する。
		int width = 0, height = 0;
		fp->exfunc->get_pixel_filtered(editp, frameNumber, NULL, &width, &height);

		{
			Synchronizer sync(*m_mutex);
			VideoInfoFileMapping::Buffer buffer(m_fileMapping.get());
			VideoInfo* info = buffer.getInfo();
			ColorRGB* src1 = buffer.getSrc1();
			ColorRGB* src2 = buffer.getSrc2();

			info->frame.number = frameNumber;
			info->frame.maxNumber = frameMaxNumber;
			info->frame.w = width;
			info->frame.h = height;
			info->useAlpha = useAlpha;

			DWORD start = ::timeGetTime();

			fp->exfunc->get_pixel_filtered(editp, frameNumber, src1, NULL, NULL);

			if (useAlpha)
			{
				theApp.m_do_draw_white = TRUE;
				fp->exfunc->get_pixel_filtered(editp, frameNumber, src2, NULL, NULL);
				theApp.m_do_draw_white = FALSE;
			}

			DWORD end = ::timeGetTime();

			MY_TRACE(_T("CInProcessViewer::writeVideoInfo() => %dms.\n"), end - start);

			MY_TRACE_NUM(info->frame.number);
			MY_TRACE_NUM(info->frame.maxNumber);
			MY_TRACE_NUM(info->frame.w);
			MY_TRACE_NUM(info->frame.h);
			MY_TRACE_NUM(info->useAlpha);
		}
	}

	theApp.postMessage(WM_MANY_VIEW_READ_VIDEO_INFO, (WPARAM)m_hwnd, 0);

	return TRUE;
}
