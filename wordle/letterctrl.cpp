#include "stdafx.h"
#include "letterctrl.h"

static COLORREF SelectBackground(CLetterCtrl::EState state)
{
	switch (state)
	{
	case CLetterCtrl::Missing: return RGB(58, 58, 60);
	case CLetterCtrl::Exists: return RGB(181, 159, 59);
	case CLetterCtrl::Correct: return RGB(83, 141, 78);
	default: return RGB(18, 18, 19);
	}
}
static COLORREF SelectBorder(CLetterCtrl::EState state)
{
	switch (state)
	{
	case CLetterCtrl::Empty: return RGB(58, 58, 60);
	case CLetterCtrl::Input: return RGB(86, 87, 88);
	default: return SelectBackground(state);
	}
}

CLetterCtrl::CLetterCtrl()
	: CStatic()
{
	m_created = false;
	m_size = 40;
	m_animStage = Idle;
	m_state = Empty;
	m_startDelay = 0;
}

BEGIN_MESSAGE_MAP(CLetterCtrl, CStatic)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CLetterCtrl::BeginTransition(EState state, wchar_t ch, int startDelay)
{
	if (state == Empty || state == Input)
	{
		// No transition
		m_state = state;
		m_char = ch;
		RefreshInternalBuffer();
		Invalidate();
		UpdateWindow();
	}
	else if (m_animStage == Idle)
	{
		m_startDelay = startDelay;
		m_nextState = state;
		m_nextChar = ch;
		m_animStage = Collapsing;
		m_animProgress = 0;
		m_animMax = 10;
	}
}

bool CLetterCtrl::TickTimer()
{
	if (m_startDelay > 0)
	{
		--m_startDelay;
		return false;
	}

	if (m_animStage != Idle)
	{
		m_animProgress++;
		if (m_animProgress >= m_animMax)
		{
			m_animProgress = 0;
			if (m_animStage == Collapsing)
			{
				m_state = m_nextState;
				m_char = m_nextChar;
				m_animStage = Expanding;
				RefreshInternalBuffer();
			}
			else
			{
				m_animStage = Idle;
			}
		}
		Invalidate();
		UpdateWindow();
	}

	return m_animStage == Idle;
}

void CLetterCtrl::OnPaint()
{
	CRect updateRect, windowRect;
	if (GetUpdateRect(&updateRect, FALSE) == 0)
		return;

	PAINTSTRUCT paint;
	CDC *dc = BeginPaint(&paint);

	this->GetWindowRect(&windowRect);

	if (!m_created)
	{
		m_size = windowRect.Width();
		RefreshInternalBuffer(dc);
	}

	int squish = 0;
	if (m_animStage == Collapsing)
		squish = (m_animProgress * m_size) / m_animMax;
	else if (m_animStage == Expanding)
		squish = m_size - ((m_animProgress * m_size) / m_animMax);

	CBitmap *oldBitmap = m_bufferDC.SelectObject(&m_buffer);
	dc->FillSolidRect(&updateRect, RGB(0,0,0));
	dc->StretchBlt(0, squish / 2, m_size, m_size - squish, &m_bufferDC, 0, 0, m_size, m_size, SRCCOPY);
	m_bufferDC.SelectObject(oldBitmap);

	EndPaint(&paint);
}

void CLetterCtrl::RefreshInternalBuffer(CDC *baseDC)
{
	if (!m_created)
	{
		if (baseDC == NULL)
			return;

		OutputDebugString(L"Creating buffer\n");
		m_created = true;
		if (m_bufferDC.CreateCompatibleDC(baseDC) == 0)
			OutputDebugString(L"CreateCompatibleDC failed\n");
		if (m_buffer.CreateCompatibleBitmap(baseDC, m_size, m_size) == 0)
			OutputDebugString(L"CreateCompatibleBitmap failed\n");
	}

	CBitmap *oldBitmap = m_bufferDC.SelectObject(&m_buffer);

	CRect rect(0, 0, m_size, m_size);
	m_borderPen.CreatePen(PS_SOLID, 2, SelectBorder(m_state));
	m_innerBrush.CreateSolidBrush(SelectBackground(m_state));

	CPen *oldPen = m_bufferDC.SelectObject(&m_borderPen);
	CBrush *oldBrush = m_bufferDC.SelectObject(&m_innerBrush);
	m_bufferDC.Rectangle(rect);

	if (m_state != Empty)
	{
		wchar_t str[2];
		str[0] = m_char;
		str[1] = 0;

		m_bufferDC.SetTextColor(RGB(215, 228, 220));
		m_bufferDC.SetBkColor(SelectBackground(m_state));
		CFont *oldFont = m_bufferDC.SelectObject(this->GetFont());
		m_bufferDC.DrawTextW(str, 1, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		m_bufferDC.SelectObject(oldFont);
	}

	m_bufferDC.SelectObject(oldBrush);
	m_bufferDC.SelectObject(oldPen);

	m_borderPen.DeleteObject();
	m_innerBrush.DeleteObject();

	m_bufferDC.SelectObject(oldBitmap);
}
