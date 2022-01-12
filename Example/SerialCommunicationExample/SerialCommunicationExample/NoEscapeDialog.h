#pragma once
#include <afxdialogex.h>
#include <vector>
/*No ESC, No Enter*/
class NoEscapeDialog :
    public CDialogEx
{
public:
    /*When calling constructor automatically register VK_ESCAPE, VK_RETURN*/
    NoEscapeDialog(UINT nIDTemplate, CWnd* pParent = NULL);

    virtual BOOL PreTranslateMessage(MSG* pMsg);

    /*If you want another keycode use this function. It will be initialize your new vector.*/
    void registerKeyCode(const std::vector<DWORD> &codeList);

    /*This virtual function called when PostNcDestroy, ALT + F4.*/
    virtual void prepareExit();
private:
    /*Keycode vector*/
    std::vector<DWORD> codeList_;

    /*Check is keycode registered.*/
    bool checkKeyCode(DWORD keyInput);
public:
    virtual void PostNcDestroy();
};

