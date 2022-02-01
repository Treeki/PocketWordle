#pragma once

class CLetterCtrl : public CStatic
{
public:
	CLetterCtrl();

	enum EState {
		Empty = 0,
		Input = 1,
		Missing = 2,
		Exists = 3,
		Correct = 4
	};

	enum EAnimStage {
		Idle,
		Collapsing,
		Expanding
	};

	void BeginTransition(EState state, wchar_t ch, int startDelay = 0);
	bool TickTimer();
	void RefreshInternalBuffer(CDC *baseDC = 0);

	void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	CPen m_borderPen;
	CBrush m_innerBrush;

	int m_size;
	bool m_created;
	CDC m_bufferDC;
	CBitmap m_buffer;

	EAnimStage m_animStage;
	int m_startDelay;
	int m_animProgress, m_animMax;
	EState m_state, m_nextState;
	wchar_t m_char, m_nextChar;
};
