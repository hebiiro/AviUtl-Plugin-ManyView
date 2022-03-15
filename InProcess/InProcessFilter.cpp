#include "pch.h"
#include "InProcess.h"

using func_proc_type = decltype(FILTER::func_proc);
func_proc_type true_exedit_func_proc = 0;
BOOL hook_exedit_func_proc(void *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("hook_exedit_func_proc() begin\n"));

	if (theApp.m_do_draw_white)
	{
		const PIXEL_YC white = { 4096, 0, 0 };

		for (int y = 0; y < fpip->h; y++)
		{
			for (int x = 0; x < fpip->w; x++)
			{
				fpip->ycp_edit[x + y * fpip->max_w] = white;
			}
		}
	}

	BOOL result = true_exedit_func_proc(fp, fpip);

	MY_TRACE(_T("hook_exedit_func_proc() end\n"));

	return result;
}

//---------------------------------------------------------------------
//		フィルタ構造体定義
//---------------------------------------------------------------------
FILTER_DLL g_filter =
{
	FILTER_FLAG_ALWAYS_ACTIVE |
//	FILTER_FLAG_MAIN_MESSAGE |
	FILTER_FLAG_WINDOW_THICKFRAME |
	FILTER_FLAG_WINDOW_SIZE |
	FILTER_FLAG_DISP_FILTER |
	FILTER_FLAG_EX_INFORMATION,
	WINDOW_W, WINDOW_H,
	theApp.m_name,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	func_proc,
	func_init,
	func_exit,
	NULL,
	func_WndProc,
	NULL, NULL,
	NULL,
	NULL,
	theApp.m_information,
	NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL,
};

//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void)
{
	return &g_filter;
}

//---------------------------------------------------------------------
//		初期化
//---------------------------------------------------------------------

BOOL func_init(FILTER *fp)
{
	MY_TRACE(_T("func_init()\n"));

	FILTER* exedit = auls::Exedit_GetFilter(fp);

	MY_TRACE_HEX(exedit);

	if (exedit)
	{
		true_exedit_func_proc = exedit->func_proc;

		DetourTransactionBegin();
		DetourUpdateThread(::GetCurrentThread());
		DetourAttach((PVOID*)&true_exedit_func_proc, hook_exedit_func_proc);
		if (DetourTransactionCommit() == NO_ERROR)
		{
			MY_TRACE(_T("hook succeeded.\n"));
		}
		else
		{
			MY_TRACE(_T("hook failed.\n"));
			return FALSE;
		}
	}

	return theApp.init(fp);
}

//---------------------------------------------------------------------
//		終了
//---------------------------------------------------------------------
BOOL func_exit(FILTER *fp)
{
	MY_TRACE(_T("func_exit()\n"));

	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());
	DetourDetach((PVOID*)&true_exedit_func_proc, hook_exedit_func_proc);
	if (DetourTransactionCommit() == NO_ERROR)
	{
		MY_TRACE(_T("unhook succeeded.\n"));
	}
	else
	{
		MY_TRACE(_T("unhook failed.\n"));
	}

	return theApp.exit(fp);
}

//---------------------------------------------------------------------
//		フィルタされた画像をバッファにコピー
//---------------------------------------------------------------------
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("func_proc() : %d\n"), ::GetTickCount());

	MY_TRACE_NUM(fp->exfunc->is_editing(fpip->editp));
	MY_TRACE_NUM(fp->exfunc->is_saving(fpip->editp));
	MY_TRACE_NUM(fp->exfunc->is_filter_active(fp));

	if (!::IsWindowVisible(fp->hwnd))
		return FALSE;

	theApp.writeFrameInfo(fp, fpip->editp);

	return TRUE;
}

//---------------------------------------------------------------------
//		WndProc
//---------------------------------------------------------------------
BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp)
{
	//	TRUEを返すと全体が再描画される

	switch (message)
	{
	case WM_FILTER_UPDATE:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_UPDATE)\n"));

			if (fp->exfunc->is_editing(editp) != TRUE) break; // 編集中でなければ終了

			break;
		}
	case WM_FILTER_CHANGE_EDIT:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_CHANGE_EDIT)\n"));

			if (fp->exfunc->is_editing(editp) != TRUE) break; // 編集中でなければ終了

			break;
		}
	case WM_FILTER_CHANGE_WINDOW:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_CHANGE_WINDOW)\n"));

			if (fp->exfunc->is_filter_window_disp(fp))
			{
				theApp.writeFrameInfo(fp, editp);
				theApp.showViewers(SW_SHOW);
			}
			else
			{
				theApp.showViewers(SW_HIDE);
			}

			break;
		}
	case WM_FILTER_WRITE_FRAME_INFO:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_WRITE_FRAME_INFO)\n"));

			theApp.writeFrameInfo(fp, editp, (HWND)wParam);

			break;
		}
	case WM_FILTER_WRITE_VIDEO_INFO:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_WRITE_VIDEO_INFO)\n"));

			theApp.writeVideoInfo(fp, editp, (HWND)wParam);

			break;
		}
	case WM_SIZE:
		{
			MY_TRACE(_T("func_WndProc(WM_SIZE)\n"));

			theApp.recalcLayout();

			break;
		}
	}

	return FALSE;
}

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			theApp.dllInit(instance);

			break;
		}
	case DLL_PROCESS_DETACH:
		{
			MY_TRACE(_T("DLL_PROCESS_DETACH\n"));

			break;
		}
	}

	return TRUE;
}
