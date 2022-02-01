#pragma once
#include "letterctrl.h"

class CWordleDlg : public CDialog
{
// Construction
public:
	CWordleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WORDLE_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	enum { ATTEMPTS = 6, WORD_SIZE = 5, LETTER_COUNT = 26 };

	UINT_PTR m_timerId;

	CFont m_letterFont;
	CLetterCtrl m_letters[ATTEMPTS][WORD_SIZE];
	int m_attemptIndex;
	int m_letterIndex;
	wchar_t m_currentInput[WORD_SIZE];
	wchar_t m_word[WORD_SIZE];
	CButton m_buttons[LETTER_COUNT];
	bool m_wonGame;

	void InitGame();
	void InputLetter(wchar_t letter);
	void RemoveLastLetter();
	void CommitGuess();
	void AnimationCompleted();

	BOOL PreTranslateMessage(MSG *pMsg);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC *pDc, CWnd *pWnd, UINT nCtlColor);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
};
