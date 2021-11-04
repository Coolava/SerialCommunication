
// SerialCommunicationExampleDlg.h : header file
//

#pragma once


// CSerialCommunicationExampleDlg dialog
class CSerialCommunicationExampleDlg : public CDialogEx
{
// Construction
public:
	CSerialCommunicationExampleDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERIALCOMMUNICATIONEXAMPLE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
