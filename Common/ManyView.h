#pragma once

//--------------------------------------------------------------------

const UINT WM_MANY_VIEW_SHOW_VIEWERS = ::RegisterWindowMessage(_T("WM_MANY_VIEW_SHOW_VIEWERS"));

const UINT WM_MANY_VIEW_CREATE_VIEWER = ::RegisterWindowMessage(_T("WM_MANY_VIEW_CREATE_VIEWER"));
const UINT WM_MANY_VIEW_DESTROY_VIEWER = ::RegisterWindowMessage(_T("WM_MANY_VIEW_DESTROY_VIEWER"));
const UINT WM_MANY_VIEW_EDIT_VIEWER = ::RegisterWindowMessage(_T("WM_MANY_VIEW_EDIT_VIEWER"));

const UINT WM_MANY_VIEW_VIEWER_CREATED = ::RegisterWindowMessage(_T("WM_MANY_VIEW_VIEWER_CREATED"));
const UINT WM_MANY_VIEW_VIEWER_DESTROYED = ::RegisterWindowMessage(_T("WM_MANY_VIEW_VIEWER_DESTROYED"));
const UINT WM_MANY_VIEW_VIEWER_CHANGED = ::RegisterWindowMessage(_T("WM_MANY_VIEW_VIEWER_CHANGED"));

const UINT WM_MANY_VIEW_READ_FRAME_INFO = ::RegisterWindowMessage(_T("WM_MANY_VIEW_READ_FRAME_INFO"));
const UINT WM_MANY_VIEW_WRITE_FRAME_INFO = ::RegisterWindowMessage(_T("WM_MANY_VIEW_WRITE_FRAME_INFO"));

const UINT WM_MANY_VIEW_READ_VIDEO_INFO = ::RegisterWindowMessage(_T("WM_MANY_VIEW_READ_VIDEO_INFO"));
const UINT WM_MANY_VIEW_WRITE_VIDEO_INFO = ::RegisterWindowMessage(_T("WM_MANY_VIEW_WRITE_VIDEO_INFO"));

const int IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_W		= sizeof(LONG) * 0;
const int IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_H		= sizeof(LONG) * 1;
const int IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_SIZE	= sizeof(LONG) * 2;
const int IN_PROCESS_WND_EXTRA_FRAME_NUMBER		= sizeof(LONG) * 3;
const int IN_PROCESS_WND_EXTRA_FRAME_MAX_NUMBER	= sizeof(LONG) * 4;
const int IN_PROCESS_WND_EXTRA_MAX_SIZE			= sizeof(LONG) * 5;

const int OUT_PROCESS_WND_EXTRA_FRAME_NUMBER			= sizeof(LONG) * 0;
const int OUT_PROCESS_WND_EXTRA_WINDOW_MODE			= sizeof(LONG) * 1;
const int OUT_PROCESS_WND_EXTRA_STRETCH_MODE			= sizeof(LONG) * 2;
const int OUT_PROCESS_WND_EXTRA_SIZE_MODE				= sizeof(LONG) * 3;
const int OUT_PROCESS_WND_EXTRA_UPDATE_MODE			= sizeof(LONG) * 4;
const int OUT_PROCESS_WND_EXTRA_FRAME_MODE			= sizeof(LONG) * 5;
const int OUT_PROCESS_WND_EXTRA_ALPHA_CHANNEL_MODE	= sizeof(LONG) * 6;
const int OUT_PROCESS_WND_EXTRA_MAX_SIZE				= sizeof(LONG) * 7;

const int MANY_VIEW_WINDOW_MODE_RESTORE		= 0;
const int MANY_VIEW_WINDOW_MODE_MINIMIZE		= 1;
const int MANY_VIEW_WINDOW_MODE_MAXIMIZE		= 2;
const int MANY_VIEW_WINDOW_MODE_FULLSCREEN	= 3;

const int MANY_VIEW_STRETCH_MODE_NONE		= 0;
const int MANY_VIEW_STRETCH_MODE_FIT			= 1;
const int MANY_VIEW_STRETCH_MODE_CROP		= 2;
const int MANY_VIEW_STRETCH_MODE_FULL		= 3;

const int MANY_VIEW_SIZE_MODE_MANUAL			= 0;
const int MANY_VIEW_SIZE_MODE_AUTO			= 1;

const int MANY_VIEW_FRAME_MODE_RELATIVE		= 0;
const int MANY_VIEW_FRAME_MODE_ABSOLUTE		= 1;

const int MANY_VIEW_UPDATE_MODE_MANUAL		= 0;
const int MANY_VIEW_UPDATE_MODE_AUTO			= 1;

const int MANY_VIEW_ALPHA_CHANNEL_MODE_NONE	= 0;
const int MANY_VIEW_ALPHA_CHANNEL_MODE_USE	= 1;
const int MANY_VIEW_ALPHA_CHANNEL_MODE_AUTO	= 2;

//--------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

struct ColorRGB { BYTE b, g, r; };
struct ColorRGBA { BYTE b, g, r, a; };

#pragma pack(pop)

struct VideoInfo
{
	struct { int w, h, number, maxNumber; } frame;
	BOOL useAlpha;
};

//--------------------------------------------------------------------

class VideoInfoFileMapping
{
public:

	int m_vram_w;
	int m_vram_h;
	int m_vram_size;
	FileMappingPtr m_fileMapping;

public:

	VideoInfoFileMapping(HWND hwnd, HWND dialog)
	{
		TCHAR name[MAX_PATH] = {};
		::StringCbPrintf(name, sizeof(name), _T("ManyView.VideoInfo.0x%08p"), hwnd);

		m_vram_w = ::GetWindowLong(dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_W);
		m_vram_h = ::GetWindowLong(dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_H);
		m_vram_size = ::GetWindowLong(dialog, IN_PROCESS_WND_EXTRA_VIDEO_BUFFER_SIZE);

		MY_TRACE_NUM(m_vram_w);
		MY_TRACE_NUM(m_vram_h);
		MY_TRACE_NUM(m_vram_size);

		m_fileMapping = FileMappingPtr(new FileMapping(sizeof(VideoInfo) + m_vram_size * 2, name));
	}

	FileMapping* get()
	{
		return m_fileMapping.get();
	}

public:

	class Buffer
	{
	public:

		VideoInfoFileMapping* m_fileMapping;
		BYTE* m_buffer;

		Buffer(VideoInfoFileMapping* fileMapping)
		{
			m_fileMapping = fileMapping;
			m_buffer = m_fileMapping->get()->allocBuffer();
		}

		~Buffer()
		{
			m_fileMapping->get()->freeBuffer(m_buffer), m_buffer = 0;
		}

		BYTE* get()
		{
			return m_buffer;
		}

		VideoInfo* getInfo()
		{
			return (VideoInfo*)get();
		}

		ColorRGB* getSrc1()
		{
			return (ColorRGB*)(get() + sizeof(VideoInfo));
		}

		ColorRGB* getSrc2()
		{
			return (ColorRGB*)(get() + sizeof(VideoInfo) + m_fileMapping->m_vram_size);
		}
	};
};

typedef std::shared_ptr<VideoInfoFileMapping> VideoInfoFileMappingPtr;

class VideoInfoMutex : public Mutex
{
public:

	VideoInfoMutex(HWND hwnd)
	{
		TCHAR name[MAX_PATH] = {};
		::StringCbPrintf(name, sizeof(name), _T("ManyView.VideoInfoMutex.0x%08p"), hwnd);
		Mutex::init(0, FALSE, name);
	}
};

typedef std::shared_ptr<VideoInfoMutex> VideoInfoMutexPtr;

//--------------------------------------------------------------------
