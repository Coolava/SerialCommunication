
// SerialCommunicationExampleDlg.h : header file
//

#pragma once
#include "NoEscapeDialog.h"
#include "ThreadController.h"
#include "SerialController.h"

#include <iomanip>
// CSerialCommunicationExampleDlg dialog
class CSerialCommunicationExampleDlg : public NoEscapeDialog
{
// Construction
public:
	CSerialCommunicationExampleDlg(CWnd* pParent = nullptr);	// standard constructor
	~CSerialCommunicationExampleDlg();
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
public:
	afx_msg void OnBnClickedButton1();

private:
	SerialController serialController_;

	ThreadController updateThread;
	void updater();
	void updateToList(const CSerialPort::Buffer &data);

	CListCtrl listReceived_;
	CListCtrl listSend_;

	CComboBox comboPort;
	CComboBox comboBaud;


	ThreadController sendThread;
	void sender();

	virtual void prepareExit();

	bool scroll_ = true;
	std::string TimepointToString(const std::chrono::system_clock::time_point& p_tpTime);
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButtonConfig();
	afx_msg void OnNMSetfocusList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMKillfocusList1(NMHDR* pNMHDR, LRESULT* pResult);
};
