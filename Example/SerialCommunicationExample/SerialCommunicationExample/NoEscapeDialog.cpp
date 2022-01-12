#include "pch.h"
#include "NoEscapeDialog.h"


NoEscapeDialog::NoEscapeDialog(UINT nIDTemplate, CWnd* pParent)
	: CDialogEx(nIDTemplate, pParent)
	, codeList_({VK_ESCAPE, VK_RETURN})
{
}

BOOL NoEscapeDialog::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) 
	{
		if (checkKeyCode(static_cast<DWORD>(pMsg->wParam)) == true)
		{
			return 0;
		}
	}
	else if (pMsg->message == WM_SYSKEYDOWN)
	{
		if (pMsg->wParam == VK_F4)
		{
			prepareExit();
			return 0;
		}
	}


	return CDialogEx::PreTranslateMessage(pMsg);
}

void NoEscapeDialog::registerKeyCode(const std::vector<DWORD>& codeList)
{
	codeList_ = codeList;
}

void NoEscapeDialog::prepareExit()
{
}

bool NoEscapeDialog::checkKeyCode(DWORD keyInput)
{
	auto rslt = std::find(codeList_.begin(), codeList_.end(), keyInput);
	return rslt!= codeList_.end() ? true : false;
}



void NoEscapeDialog::PostNcDestroy()
{
	prepareExit();
	CDialogEx::PostNcDestroy();
}
