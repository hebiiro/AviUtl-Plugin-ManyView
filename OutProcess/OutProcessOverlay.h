#pragma once

class COutProcessViewer;

class COutProcessOverlay : public CWnd
{
public:

	class CPart
	{
	public:

		COutProcessOverlay* m_owner;
		int m_id;
		_bstr_t m_text;
		_bstr_t m_tooltipText;
		RectF m_rect;

		CPart(COutProcessOverlay* owner, int id, LPCWSTR text, LPCWSTR tooltipText)
			: m_owner(owner)
			, m_id(id)
			, m_text(text)
			, m_tooltipText(tooltipText)
			, m_rect(0.0f, 0.0f, 0.0f, 0.0f)
		{
		}

		virtual ~CPart()
		{
		}

		virtual void paint(Graphics& g, const RectF& rect, BOOL isActive) = 0;
		virtual void preCalcLayout(Graphics& g, REAL y, RectF* rect) = 0;
		virtual void calcLayout(Graphics& g, const RectF& rect) = 0;
		virtual int addItem(LPCWSTR text) = 0;
		virtual _variant_t getValue(int index) = 0;
		virtual void setValue(int index, const _variant_t& value) = 0;

		virtual void onTimer(UINT_PTR timerId)
		{
		}

		virtual BOOL onHitTest(CPoint point)
		{
			return FALSE;
		}

		virtual BOOL onLButtonDown(CPoint point)
		{
			return FALSE;
		}

		virtual void onKeyDown(DWORD keyCode)
		{
		}

		virtual BOOL onFocus(BOOL blur, BOOL reverse)
		{
			return TRUE;
		}

		virtual void onInvalidate()
		{
		}
	};

	class CSelectPart : public CPart
	{
	public:

		class CItem
		{
		public:

			_bstr_t m_text;
			GraphicsPath m_path;
			SizeF m_textSize;
			RectF m_textBorderRect;

			CItem(LPCWSTR text)
				: m_text(text)
				, m_textSize(0.0f, 0.0f)
				, m_textBorderRect(0.0f, 0.0f, 0.0f, 0.0f)
			{
			}
		};

		typedef std::shared_ptr<CItem> CItemPtr;
		typedef std::vector<CItemPtr> CItems;

	public:

		CItems m_items;
		int m_current;
		GraphicsPath m_path;
		SizeF m_textSize;
		RectF m_textBorderRect;
		ULONGLONG m_animationStartTime;
		RectF m_animationStartRect;
		RectF m_animationCurrentRect;
		RectF m_animationEndRect;

		CSelectPart(COutProcessOverlay* owner, int id, LPCWSTR text, LPCWSTR tooltipText)
			: CPart(owner, id, text, tooltipText)
			, m_current(-1)
			, m_textSize(0.0f, 0.0f)
			, m_textBorderRect(0.0f, 0.0f, 0.0f, 0.0f)
			, m_animationStartTime(0)
			, m_animationStartRect(0.0f, 0.0f, 0.0f, 0.0f)
			, m_animationCurrentRect(0.0f, 0.0f, 0.0f, 0.0f)
			, m_animationEndRect(0.0f, 0.0f, 0.0f, 0.0f)
		{
		}

		virtual ~CSelectPart()
		{
		}

		void init(int current)
		{
			m_current = current;
			if (m_current >= 0 && m_current < (int)m_items.size())
				m_animationEndRect = m_items[m_current]->m_textBorderRect;
			else
				m_animationEndRect = m_rect;
			m_animationStartRect = m_animationEndRect;
			m_animationCurrentRect = m_animationEndRect;
		}

		void setCurrent(int current)
		{
			m_current = current;
			m_animationStartRect = m_animationCurrentRect;
			if (m_current >= 0 && m_current < (int)m_items.size())
				m_animationEndRect = m_items[m_current]->m_textBorderRect;
			else
				m_animationEndRect = m_rect;
			startAnimationTimer();
			m_owner->InvalidateRect(CMyRect(m_rect), FALSE);
			m_owner->uploadValue(this);
			m_owner->SetActiveWindow();
			m_owner->SetFocus();
		}

		void startAnimationTimer()
		{
			MY_TRACE(_T("CSelectPart::startAnimationTimer()\n"));

			m_animationStartTime = ::GetTickCount64();
			m_owner->SetTimer((UINT_PTR)this, 10, 0);
		}

		void stopAnimationTimer()
		{
			MY_TRACE(_T("CSelectPart::stopAnimationTimer()\n"));

			m_animationStartTime = 0;
			m_owner->KillTimer((UINT_PTR)this);
		}

		void resetAnimationRect()
		{
			if (m_current >= 0 && m_current < (int)m_items.size())
			{
				m_animationStartRect = m_items[m_current]->m_textBorderRect;
				m_animationCurrentRect = m_items[m_current]->m_textBorderRect;
				m_animationEndRect = m_items[m_current]->m_textBorderRect;
			}
			else
			{
				m_animationStartRect = m_rect;
				m_animationCurrentRect = m_rect;
				m_animationEndRect = m_rect;
			}
		}

		template<typename T>
		static T lerp(T a, T b, double factor)
		{
			return (T)(a * (1 - factor) + b * factor);
		}

		static void morphRect(RectF* current, const RectF* start, const RectF* end, double factor)
		{
			current->X = lerp(start->X, end->X, factor);
			current->Y = lerp(start->Y, end->Y, factor);
			current->Width = lerp(start->Width, end->Width, factor);
			current->Height = lerp(start->Height, end->Height, factor);
		}

		virtual void paint(Graphics& g, const RectF& rect, BOOL isActive)
		{
			if (isActive && (m_id == m_owner->m_focusedId || m_id == m_owner->m_hoveredId))
			{
				// 背景を塗りつぶす。
				g.FillRectangle(m_owner->m_hotBrush.get(), rect);
			}

			{
				// アイテム群の背景を描画する。

				auto front = m_items.front();
				auto back = m_items.back();

				RectF backRect;
				backRect.X = front->m_textBorderRect.X;
				backRect.Y = front->m_textBorderRect.Y;
				backRect.Width = back->m_textBorderRect.X + back->m_textBorderRect.Width - front->m_textBorderRect.X;
				backRect.Height = back->m_textBorderRect.Y + back->m_textBorderRect.Height - front->m_textBorderRect.Y;

				drawRoundRect(g, m_owner->m_selectPen.get(), m_owner->m_selectBrush.get(), backRect);

				// 選択アイテムの矩形を塗りつぶす。
				drawRoundRect(g, 0, m_owner->m_selectHotBrush.get(), m_animationCurrentRect);
			}

			if (isActive && m_id == m_owner->m_focusedId)
			{
				g.DrawPath(m_owner->m_hotFontPen.get(), &m_path);
				g.FillPath(m_owner->m_hotFontBrush.get(), &m_path);
			}
			else
			{
				g.DrawPath(m_owner->m_fontPen.get(), &m_path);
				g.FillPath(m_owner->m_fontBrush.get(), &m_path);
			}

			for (size_t i = 0; i < m_items.size(); i++)
			{
				auto item = m_items[i];

				if (i == m_current)
				{
					g.DrawPath(m_owner->m_selectHotFontPen.get(), &item->m_path);
					g.FillPath(m_owner->m_selectHotFontBrush.get(), &item->m_path);
				}
				else
				{
					g.DrawPath(m_owner->m_selectFontPen.get(), &item->m_path);
					g.FillPath(m_owner->m_selectFontBrush.get(), &item->m_path);
				}
			}

			// 選択アイテムの矩形の縁を描く。
			drawRoundRect(g, m_owner->m_selectHotPen.get(), 0, m_animationCurrentRect);
		}

		virtual void preCalcLayout(Graphics& g, REAL y, RectF* rect)
		{
			REAL margin = m_owner->m_fontHeight / 2;
			REAL padding = 0.0f;//m_owner->m_fontHeight / 8;

			m_textSize = calcTextSize(g, m_text, m_owner->m_font.get());
			m_textSize.Width += padding * 2 + margin * 2;
			m_textSize.Height += padding * 2 + margin * 2;

			rect->X = 0.0f;
			rect->Y = y;
			rect->Width = m_textSize.Width;
			rect->Height = m_textSize.Height;

			rect->Width += margin;

			for (size_t i = 0; i < m_items.size(); i++)
			{
				auto item = m_items[i];

				item->m_textSize = calcTextSize(g, item->m_text, m_owner->m_font.get());
				item->m_textSize.Width += padding * 2 + item->m_textSize.Height;
				item->m_textSize.Height += padding * 2 + margin * 2;

				rect->Width += item->m_textSize.Width;
				rect->Height = max(rect->Height, item->m_textSize.Height);
			}

			rect->Width += margin;
		}

		virtual void calcLayout(Graphics& g, const RectF& rect)
		{
			REAL margin = m_owner->m_fontHeight / 2;
			REAL padding = 0.0f;//m_owner->m_fontHeight / 8;

			{
				StringFormat stringFormat;
//				stringFormat.SetAlignment(StringAlignmentCenter);
				stringFormat.SetLineAlignment(StringAlignmentCenter);

				RectF textRect;
				textRect.X = rect.X;
				textRect.Y = rect.Y;
				textRect.Width = m_textSize.Width;
				textRect.Height = m_textSize.Height;
				textRect.Inflate(-margin, -margin);
				m_textBorderRect = textRect;
				textRect.Inflate(-padding, -padding);

				m_owner->createPath(m_path, m_text, textRect, &stringFormat);
			}

			REAL x = rect.GetRight() - margin;

			for (auto it = m_items.rbegin(); it != m_items.rend(); it++)
			{
				auto item = *it;

				StringFormat stringFormat;
				stringFormat.SetAlignment(StringAlignmentCenter);
				stringFormat.SetLineAlignment(StringAlignmentCenter);

				RectF textRect;
				textRect.X = x - item->m_textSize.Width;
				textRect.Y = rect.Y + (rect.Height - item->m_textSize.Height) / 2;
				textRect.Width = item->m_textSize.Width;
				textRect.Height = item->m_textSize.Height;
				textRect.Inflate(0, -margin);
				item->m_textBorderRect = textRect;
				textRect.Inflate(-padding, -padding);

				m_owner->createPath(item->m_path, item->m_text, textRect, &stringFormat);

				x -= item->m_textSize.Width;
			}

			resetAnimationRect();
		}

		virtual int addItem(LPCWSTR text)
		{
			m_items.push_back(CItemPtr(new CItem(text)));
			return (int)m_items.size();
		}

		virtual _variant_t getValue(int index)
		{
			return m_current;
		}

		virtual void setValue(int index, const _variant_t& value)
		{
			m_current = value;
		}

		virtual void onTimer(UINT_PTR timerId)
		{
			if (timerId == (UINT_PTR)this)
			{
				ULONGLONG currentTime = ::GetTickCount64();
				ULONGLONG elapsedTime = currentTime - m_animationStartTime;

				if (elapsedTime < m_owner->m_animationDuration)
				{
					morphRect(
						&m_animationCurrentRect,
						&m_animationStartRect,
						&m_animationEndRect,
						(double)elapsedTime / m_owner->m_animationDuration);
				}
				else
				{
					m_animationCurrentRect = m_animationEndRect;

					stopAnimationTimer();
				}

				m_owner->InvalidateRect(CMyRect(m_rect), FALSE);
			}
		}

		virtual BOOL onHitTest(CPoint _point)
		{
			MyPointF point(_point);

			if (m_textBorderRect.Contains(point))
				return TRUE;

			for (size_t i = 0; i < m_items.size(); i++)
			{
				auto item = m_items[i];

				if (item->m_textBorderRect.Contains(point))
				{
					MY_TRACE_NUM(i);

					return TRUE;
				}
			}

			return FALSE;
		}

		virtual BOOL onLButtonDown(CPoint point)
		{
			for (size_t i = 0; i < m_items.size(); i++)
			{
				auto item = m_items[i];

				if (item->m_textBorderRect.Contains((REAL)point.x, (REAL)point.y))
				{
					MY_TRACE_NUM(i);

					setCurrent((int)i);
					m_owner->setFocus(m_id);

					return TRUE;
				}
			}

			return FALSE;
		}

		virtual void onKeyDown(DWORD keyCode)
		{
			switch (keyCode)
			{
			case VK_LEFT:
				{
					m_current--;
					if (m_current < 0)
						m_current = (int)m_items.size() - 1;
					setCurrent(m_current);

					break;
				}
			case VK_RIGHT:
				{
					m_current++;
					if (m_current < 0 || m_current == m_items.size())
						m_current = 0;
					setCurrent(m_current);

					break;
				}
			}
		}
	};

	class CEditPart : public CPart
	{
	public:

		class CItem : public CEdit
		{
		public:

			static const UINT WM_ALL_SEL = WM_APP + 2021;

			CEditPart* m_owner;
			SizeF m_textSize;
			RectF m_textBorderRect;

			CItem(CEditPart* owner, LPCWSTR text)
				: m_owner(owner)
				, m_textSize(0.0f, 0.0f)
				, m_textBorderRect(0.0f, 0.0f, 0.0f, 0.0f)
			{
				Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
					CRect(0, 0, 0, 0), m_owner->m_owner, 0);
			}

			virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
			{
				switch (message)
				{
				case WM_NCCALCSIZE:
					{
						int borderWidth = (int)m_owner->m_owner->m_editOutlineWidth;
						CEdit::WindowProc(message, wParam, lParam);
						CRect* rc = (CRect*)lParam;
						rc->InflateRect(-rc->Height() / 4, -borderWidth);
						return 0;
					}
				case WM_NCPAINT:
					{
						CWindowDC dc(this);
						CRect rcClient; GetClientRect(&rcClient);
						CRect rcWindow; GetWindowRect(&rcWindow);
						rcClient.OffsetRect(-rcWindow.left, -rcWindow.top);
						rcWindow.OffsetRect(-rcWindow.left, -rcWindow.top);

						if (0)
						{
							CRgn rgn; rgn.CreateRectRgnIndirect(&rcClient);
							dc.SelectClipRgn(&rgn, RGN_DIFF);
						}

						Graphics g(dc);
						MyRectF rect(rcWindow);

						g.SetSmoothingMode(SmoothingModeAntiAlias);
						g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
						g.TranslateTransform(-0.5f, -0.5f);

						if (m_owner->m_id == m_owner->m_owner->m_focusedId)
						{
							g.FillRectangle(m_owner->m_owner->m_hotBrush.get(), rect);
						}
						else
						{
							g.FillRectangle(m_owner->m_owner->m_brush.get(), rect);
						}

						REAL diameter = rect.Height / 2.0f;

						if (GetFocus() == this)
						{
							drawRoundRect(g,
								m_owner->m_owner->m_editHotPen.get(),
								m_owner->m_owner->m_editHotBrush.get(), rect, diameter);
						}
						else
						{
							drawRoundRect(g,
								m_owner->m_owner->m_editPen.get(),
								m_owner->m_owner->m_editBrush.get(), rect, diameter);
						}
						return TRUE;
					}
				case WM_NCHITTEST:
					{
						m_owner->m_owner->OnNcHitTest(CPoint(lParam));

						break;
					}
				case WM_SETFOCUS:
					{
						m_owner->m_owner->setFocus(m_owner->m_id);
						SetWindowPos(0, 0, 0, 0, 0,
							SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE |
							SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_FRAMECHANGED);

						SetSel(0, -1);

						break;
					}
				case WM_KILLFOCUS:
					{
						m_owner->m_owner->uploadValue(m_owner);
						SetWindowPos(0, 0, 0, 0, 0,
							SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE |
							SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_FRAMECHANGED);

						break;
					}
				case WM_CHAR:
					{
						if (wParam == VK_RETURN)
						{
							m_owner->m_owner->uploadValue(m_owner);

							return 0;
						}

						break;
					}
				case WM_ALL_SEL:
					{
						SetSel(0, -1);

						break;
					}
				}

				return CEdit::WindowProc(message, wParam, lParam);
			}
		};

		typedef std::shared_ptr<CItem> CItemPtr;
		typedef std::vector<CItemPtr> CItems;

	public:

		CItems m_items;
		GraphicsPath m_path;
		int m_width;
		SizeF m_textSize;
		RectF m_textBorderRect;

		CEditPart(COutProcessOverlay* owner, int id, LPCWSTR text, LPCWSTR tooltipText, int width)
			: CPart(owner, id, text, tooltipText)
			, m_width(width)
			, m_textSize(0.0f, 0.0f)
			, m_textBorderRect(0.0f, 0.0f, 0.0f, 0.0f)
		{
		}

		virtual ~CEditPart()
		{
		}

		virtual void paint(Graphics& g, const RectF& rect, BOOL isActive)
		{
			if (isActive && (m_id == m_owner->m_focusedId || m_id == m_owner->m_hoveredId))
			{
				// 背景を塗りつぶす。
				g.FillRectangle(m_owner->m_hotBrush.get(), rect);
			}

			if (isActive && m_id == m_owner->m_focusedId)
			{
				g.DrawPath(m_owner->m_hotFontPen.get(), &m_path);
				g.FillPath(m_owner->m_hotFontBrush.get(), &m_path);
			}
			else
			{
				g.DrawPath(m_owner->m_fontPen.get(), &m_path);
				g.FillPath(m_owner->m_fontBrush.get(), &m_path);
			}
		}

		virtual void preCalcLayout(Graphics& g, REAL y, RectF* rect)
		{
			REAL space = 2.0f;
			REAL margin = m_owner->m_fontHeight / 2;
			REAL padding = 0.0f;//m_owner->m_fontHeight / 8;
			REAL borderWidth = m_owner->m_editOutlineWidth;

			m_textSize = calcTextSize(g, m_text, m_owner->m_font.get());
			m_textSize.Width += padding * 2 + margin * 2;
			m_textSize.Height += padding * 2 + margin * 2;

			rect->X = 0.0f;
			rect->Y = y;
			rect->Width = m_textSize.Width;
			rect->Height = m_textSize.Height;

			for (size_t i = 0; i < m_items.size(); i++)
			{
				auto item = m_items[i];

				REAL w = (REAL)(m_owner->m_editFontHeight * (m_width + 1) / 2);
				REAL h = (REAL)(m_owner->m_editFontHeight + borderWidth * 2);

				item->m_textSize.Width = w + padding * 2 + space * 2 + h;
				item->m_textSize.Height = h + padding * 2 + margin * 2;

				rect->Width += item->m_textSize.Width;
				rect->Height = max(rect->Height, item->m_textSize.Height);
			}
		}

		virtual void calcLayout(Graphics& g, const RectF& rect)
		{
			REAL space = 2.0f;
			REAL margin = m_owner->m_fontHeight / 2;
			REAL padding = 0.0f;//m_owner->m_fontHeight / 8;
			REAL borderWidth = m_owner->m_editOutlineWidth;

			{
				StringFormat stringFormat;
//				stringFormat.SetAlignment(StringAlignmentCenter);
				stringFormat.SetLineAlignment(StringAlignmentCenter);

				RectF textRect;
				textRect.X = rect.X;
				textRect.Y = rect.Y;
				textRect.Width = m_textSize.Width;
				textRect.Height = m_textSize.Height;
				textRect.Inflate(-margin, -margin);
				m_textBorderRect = textRect;
				textRect.Inflate(-padding, -padding);

				m_owner->createPath(m_path, m_text, textRect, &stringFormat);
			}

			REAL x = rect.GetRight() - margin;

			for (auto it = m_items.rbegin(); it != m_items.rend(); it++)
			{
				auto item = *it;

				StringFormat stringFormat;
				stringFormat.SetAlignment(StringAlignmentCenter);
				stringFormat.SetLineAlignment(StringAlignmentCenter);

				RectF textRect;
				textRect.X = x - item->m_textSize.Width;
				textRect.Y = rect.Y + (rect.Height - item->m_textSize.Height) / 2;
				textRect.Width = item->m_textSize.Width;
				textRect.Height = item->m_textSize.Height;
				textRect.Inflate(-space, -margin);
				item->m_textBorderRect = textRect;
				textRect.Inflate(-padding, -padding);

				item->MoveWindow(&CMyRect(item->m_textBorderRect));
				item->SetFont(&m_owner->m_editFont);

				x -= item->m_textSize.Width;
			}
		}

		virtual int addItem(LPCWSTR text)
		{
			m_items.push_back(CItemPtr(new CItem(this, text)));
			return (int)m_items.size();
		}

		virtual _variant_t getValue(int index)
		{
			if (index < 0 || index >= (int)m_items.size())
				return L"";

			WCHAR text[MAX_PATH] = {};
			::GetWindowTextW(m_items[index]->GetSafeHwnd(), text, MAX_PATH);
			return text;
		}

		virtual void setValue(int index, const _variant_t& value)
		{
			if (index >= 0 && index < (int)m_items.size())
				::SetWindowTextW(m_items[index]->GetSafeHwnd(), (_bstr_t)value);
		}

		virtual BOOL onHitTest(CPoint _point)
		{
			MyPointF point(_point);

			if (m_textBorderRect.Contains(point))
				return TRUE;

			for (size_t i = 0; i < m_items.size(); i++)
			{
				auto item = m_items[i];

				CRect rc; item->GetWindowRect(&rc);
				m_owner->ScreenToClient(&rc);
				if (rc.PtInRect(_point))
					return TRUE;
			}

			return FALSE;
		}

		virtual BOOL onFocus(BOOL blur, BOOL reverse)
		{
			size_t c = m_items.size();

			if (c == 0)
				return TRUE;

			if (blur)
			{
				if (reverse)
				{
					for (size_t i = 1; i < c; i++)
					{
						if (m_items[i]->GetFocus() == m_items[i].get())
						{
							m_items[i - 1]->SetFocus();
							return FALSE;
						}
					}
				}
				else
				{
					for (size_t i = 0; i < c - 1; i++)
					{
						if (m_items[i]->GetFocus() == m_items[i].get())
						{
							m_items[i + 1]->SetFocus();
							return FALSE;
						}
					}
				}

				m_owner->SetFocus();
			}
			else
			{
				if (reverse)
				{
					m_items[c - 1]->SetFocus();
				}
				else
				{
					m_items[0]->SetFocus();
				}
			}

			return TRUE;
		}

		virtual void onInvalidate()
		{
			for (auto it = m_items.begin(); it != m_items.end(); it++)
			{
				auto item = *it;

				item->SetRedraw(FALSE);
				item->SetWindowPos(0, 0, 0, 0, 0,
					SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE |
					SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_FRAMECHANGED);
				item->SetRedraw(TRUE);
				item->Invalidate(FALSE);
				item->UpdateWindow();
			}
		}
	};

	typedef std::shared_ptr<CPart> CPartPtr;
	typedef std::map<int, CPartPtr> CParts;

	static const int ID_WINDOW_NAME			= 1;
	static const int ID_WINDOW_MODE			= 2;
	static const int ID_STRETCH_MODE		= 3;
	static const int ID_SIZE_MODE			= 4;
	static const int ID_UPDATE_MODE			= 5;
	static const int ID_FRAME_MODE			= 6;
	static const int ID_ALPHA_CHANNEL_MODE	= 7;
	static const int ID_WINDOW_NUMERICS		= 8;
	static const int ID_IMAGE_NUMERICS		= 9;
	static const int ID_FRAME				= 10;

	static BYTE m_alpha;
	static int m_animationDuration;

	static _bstr_t m_fontFamilyName;
	static REAL m_fontHeight;
	static int m_fontStyle;

	static Color m_color;
	static Color m_outlineColor;
	static Color m_fontColor;
	static Color m_fontOutlineColor;
	static Color m_hotColor;
	static Color m_hotOutlineColor;
	static Color m_hotFontColor;
	static Color m_hotFontOutlineColor;
	static REAL m_outlineWidth;
	static REAL m_fontOutlineWidth;

	static Color m_selectColor;
	static Color m_selectOutlineColor;
	static Color m_selectFontColor;
	static Color m_selectFontOutlineColor;
	static Color m_selectHotColor;
	static Color m_selectHotOutlineColor;
	static Color m_selectHotFontColor;
	static Color m_selectHotFontOutlineColor;
	static REAL m_selectOutlineWidth;
	static REAL m_selectFontOutlineWidth;

	static Color m_editColor;
	static Color m_editOutlineColor;
	static Color m_editFontColor;
	static Color m_editHotColor;
	static Color m_editHotOutlineColor;
	static Color m_editHotFontColor;
	static REAL m_editOutlineWidth;

public:

	Gdiplus::FontFamilyPtr m_fontFamily;
	Gdiplus::FontPtr m_font;

	Gdiplus::SolidBrushPtr m_brush;
	Gdiplus::PenPtr m_pen;
	Gdiplus::SolidBrushPtr m_fontBrush;
	Gdiplus::PenPtr m_fontPen;
	Gdiplus::SolidBrushPtr m_hotBrush;
	Gdiplus::PenPtr m_hotPen;
	Gdiplus::SolidBrushPtr m_hotFontBrush;
	Gdiplus::PenPtr m_hotFontPen;

	Gdiplus::SolidBrushPtr m_selectBrush;
	Gdiplus::PenPtr m_selectPen;
	Gdiplus::SolidBrushPtr m_selectFontBrush;
	Gdiplus::PenPtr m_selectFontPen;
	Gdiplus::SolidBrushPtr m_selectHotBrush;
	Gdiplus::PenPtr m_selectHotPen;
	Gdiplus::SolidBrushPtr m_selectHotFontBrush;
	Gdiplus::PenPtr m_selectHotFontPen;

	Gdiplus::SolidBrushPtr m_editBrush;
	Gdiplus::PenPtr m_editPen;
	Gdiplus::SolidBrushPtr m_editHotBrush;
	Gdiplus::PenPtr m_editHotPen;

	CFont m_editFont;
	int m_editFontHeight;

	COutProcessViewer* m_viewer;
	CParts m_parts;
	int m_focusedId;
	int m_hoveredId;

	COutProcessOverlay(COutProcessViewer* viewer);
	virtual ~COutProcessOverlay();

	void createPath(Gdiplus::GraphicsPath& path, LPCWSTR text,
		const RectF& textRect, const Gdiplus::StringFormat& stringFormat)
	{
		path.Reset();
		path.AddString(text, -1, m_fontFamily.get(),
			m_font->GetStyle(), m_fontHeight, textRect, &stringFormat);
	}

	static void loadStaticSettings(const MSXML2::IXMLDOMElementPtr& element);

	void calcLayout();
	void invalidate(CPart* part);
	void setFocus(int partId);
	void setHover(int partId);
	void show(CWnd* origin);
	void hide();
	void downloadValue();
	void uploadValue(CPart* part);

	virtual BOOL Create(CWnd* parent);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg int OnCreate(LPCREATESTRUCT cs);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcRButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	DECLARE_MESSAGE_MAP()
};
