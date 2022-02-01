#include "stdafx.h"
#include "wordle.h"
#include "wordleDlg.h"
#include "words.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CWordleDlg::CWordleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWordleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWordleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWordleDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CWordleDlg message handlers

BOOL CWordleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRect baseRect;
	GetWindowRect(baseRect);
	int pad = 5;
	int maxWidth = (baseRect.Width() - pad * (WORD_SIZE + 1)) / WORD_SIZE;
	int maxHeight = (baseRect.Height() - pad * (ATTEMPTS + 1)) / ATTEMPTS;
	int size = min(maxWidth, maxHeight);
	int padsize = pad + size;

	m_letterFont.CreateFont(32, 0, 0, 0, FW_DONTCARE, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

	for (int y = 0; y < ATTEMPTS; y++) {
		for (int x = 0; x < WORD_SIZE; x++) {
			CRect rect(x * padsize + pad, y * padsize + pad, x * padsize + padsize, y * padsize + padsize);
			m_letters[y][x].Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTER, rect, this, 1800 + y * 10 + x);
			m_letters[y][x].SetFont(&m_letterFont);
		}
	}

	m_timerId = 0;
	InitGame();
	
	return TRUE;
}

void CWordleDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar >= 'A' && nChar <= 'Z')
	{
		InputLetter(nChar);
	}
	else if (nChar == VK_BACK || nChar == VK_DELETE)
	{
		RemoveLastLetter();
	}
	else if (nChar == VK_RETURN)
	{
		CommitGuess();
	}
}

BOOL CWordleDlg::PreTranslateMessage(MSG *pMsg)
{
	// Allow us to receive the Enter key
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		CommitGuess();
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

HBRUSH CWordleDlg::OnCtlColor(CDC *pDc, CWnd *pWnd, UINT nCtlColor)
{
	return (HBRUSH) GetStockObject(BLACK_BRUSH);
}

void CWordleDlg::InitGame()
{
	m_attemptIndex = 0;
	m_letterIndex = 0;
	for (int i = 0; i < WORD_SIZE; i++)
		m_currentInput[i] = 0;
	m_wonGame = false;
	
	int maxWords = CountWords();
	srand(GetTickCount());
	for (;;)
	{
		int index = rand();
		if (index < maxWords)
		{
			SelectWord(index, m_word);
			break;
		}
	}
}

void CWordleDlg::InputLetter(wchar_t letter)
{
	if (m_timerId != 0 || m_attemptIndex >= ATTEMPTS || m_wonGame)
		return;

	if (m_letterIndex < WORD_SIZE)
	{
		m_currentInput[m_letterIndex] = letter;
		m_letters[m_attemptIndex][m_letterIndex].BeginTransition(CLetterCtrl::Input, letter);
		++m_letterIndex;
	}
}

void CWordleDlg::RemoveLastLetter()
{
	if (m_timerId != 0 || m_attemptIndex >= ATTEMPTS || m_wonGame)
		return;

	if (m_letterIndex > 0)
	{
		--m_letterIndex;
		m_currentInput[m_letterIndex] = 0;
		m_letters[m_attemptIndex][m_letterIndex].BeginTransition(CLetterCtrl::Empty, 0);
	}
}

void CWordleDlg::CommitGuess()
{
	if (m_timerId != 0 || m_attemptIndex >= ATTEMPTS || m_wonGame)
		return;

	if (m_letterIndex < WORD_SIZE)
	{
		MessageBox(L"Not enough letters", L"Wordle", MB_OK);
		return;
	}
	if (!CheckWord(m_currentInput))
	{
		MessageBox(L"Not in word list", L"Wordle", MB_OK);
		return;
	}

	// Judge this guess
	CLetterCtrl::EState states[WORD_SIZE];
	bool checked[WORD_SIZE];
	for (int i = 0; i < WORD_SIZE; i++)
	{
		checked[i] = false;
		states[i] = CLetterCtrl::Missing;
	}

	// Mark correct characters
	for (int i = 0; i < WORD_SIZE; i++)
	{
		if (!checked[i] && m_currentInput[i] == m_word[i])
		{
			checked[i] = true;
			states[i] = CLetterCtrl::Correct;
		}
	}

	// Mark other characters that appear somewhere in the word
	for (int i = 0; i < WORD_SIZE; i++)
	{
		if (states[i] == CLetterCtrl::Correct)
			continue;

		for (int j = 0; j < WORD_SIZE; j++)
		{
			if (!checked[j] && m_currentInput[j] == m_word[i])
			{
				checked[j] = true;
				states[j] = CLetterCtrl::Exists;
			}
		}
	}

	for (int i = 0; i < WORD_SIZE; i++)
	{
		m_letters[m_attemptIndex][i].BeginTransition(states[i], m_currentInput[i]);
		m_currentInput[i] = 0;
	}

	// All correct?
	m_wonGame = true;
	for (int i = 0; i < WORD_SIZE; i++)
	{
		if (states[i] != CLetterCtrl::Correct)
			m_wonGame = false;
	}

	++m_attemptIndex;
	m_letterIndex = 0;

	if (m_timerId == 0)
	{
		m_timerId = this->SetTimer(1, 25, NULL);
	}
}

void CWordleDlg::AnimationCompleted()
{
	if (m_wonGame)
	{
		MessageBox(L"Nice", L"Wordle", MB_OK);
	}
	else if (m_attemptIndex >= ATTEMPTS)
	{
		wchar_t word[WORD_SIZE + 1];
		for (int i = 0; i < WORD_SIZE; i++)
			word[i] = m_word[i];
		word[WORD_SIZE] = 0;
		MessageBox(word, L"Wordle", MB_OK);
	}
}

void CWordleDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_timerId == nIDEvent)
	{
		bool allDone = true;

		for (int i = 0; i < ATTEMPTS; i++)
		{
			for (int j = 0; j < WORD_SIZE; j++)
			{
				allDone = allDone && m_letters[i][j].TickTimer();
			}
		}

		if (allDone)
		{
			KillTimer(m_timerId);
			m_timerId = 0;
			AnimationCompleted();
		}
	}
}

