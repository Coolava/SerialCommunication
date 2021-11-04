
// SerialCommunicationExampleDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "SerialCommunicationExample.h"
#include "SerialCommunicationExampleDlg.h"
#include "afxdialogex.h"
#include <chrono>
#include <vector>
#include <array>
#include <string>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSerialCommunicationExampleDlg dialog



CSerialCommunicationExampleDlg::CSerialCommunicationExampleDlg(CWnd* pParent /*=nullptr*/)
	: NoEscapeDialog(IDD_SERIALCOMMUNICATIONEXAMPLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSerialCommunicationExampleDlg::DoDataExchange(CDataExchange* pDX)
{
	NoEscapeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, listControl_);
	DDX_Control(pDX, IDC_COMBO1, comboPort);
	DDX_Control(pDX, IDC_COMBO2, comboBaud);
}

BEGIN_MESSAGE_MAP(CSerialCommunicationExampleDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CSerialCommunicationExampleDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSerialCommunicationExampleDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CSerialCommunicationExampleDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CSerialCommunicationExampleDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CSerialCommunicationExampleDlg message handlers

BOOL CSerialCommunicationExampleDlg::OnInitDialog()
{
	NoEscapeDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	listControl_.InsertColumn(0, _T("Index"),0, 100);
	listControl_.InsertColumn(1, _T("Data(dec)"), 0, 100);
	listControl_.InsertColumn(2, _T("Data(hex)"),0, 100);

	auto portList = serialController_.getPortList();

	for (auto item : portList)
	{
		comboPort.AddString(item.c_str());
	}
	comboPort.SetCurSel(1);

	comboBaud.AddString(_T("19200"));
	comboBaud.AddString(_T("115200"));
	comboBaud.SetCurSel(0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSerialCommunicationExampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		NoEscapeDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSerialCommunicationExampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSerialCommunicationExampleDlg::OnBnClickedButton1()
{
	CString strPort, strBaud;
	comboPort.GetWindowText(strPort);
	comboBaud.GetWindowText(strBaud);

	long  baud = _tcstol(strBaud.GetBuffer(), nullptr, 10);


	bool rslt = serialController_.open(strPort.GetBuffer(), baud);

	if (rslt == false)
	{
		AfxMessageBox(_T("Open Fail"));
	}
	
	updateThread.setInterval(100);
	updateThread.start(std::bind(&CSerialCommunicationExampleDlg::updater, this));

}

void CSerialCommunicationExampleDlg::updater()
{
	
	bool opened = serialController_.isOpened();

	if (opened)
	{
		auto& dataQueue = serialController_.getDataQueue();

		if (dataQueue.size() > 0)
		{
			auto dataVector = dataQueue.dequeue();

			updateToList(dataVector);
		}

	}

}

void CSerialCommunicationExampleDlg::updateToList(const std::vector<unsigned char>& dataVector)
{
	listControl_.SetRedraw(FALSE);

	listControl_.DeleteAllItems();
	listControl_.SetItemCount(dataVector.size());

	int count = 0;
	for (auto& item : dataVector)
	{
		listControl_.InsertItem(count, std::to_wstring(count).c_str());

		listControl_.SetItemText(count, 1, std::to_wstring(item).c_str());
		CString str;
		str.Format(_T("0x%X"), item);
		listControl_.SetItemText(count, 2, str);
		count++;
	}
	listControl_.SetRedraw(TRUE);
}


void CSerialCommunicationExampleDlg::sender()
{

	std::array<BYTE, 1024> arr;

	static int start = 0;

	int i = start;
	for (auto& item : arr)
	{
		item = i++;
	}

	serialController_.write(arr.data(), arr.size());
	start++;
}

void CSerialCommunicationExampleDlg::prepareExit()
{
	serialController_.close();
	updateThread.stop();
	sendThread.stop();
}

void CSerialCommunicationExampleDlg::OnBnClickedButton2()
{
	serialController_.close();
	updateThread.stop();
}


void CSerialCommunicationExampleDlg::OnBnClickedButton3()
{
	sendThread.setInterval(100);
	sendThread.start(std::bind(&CSerialCommunicationExampleDlg::sender, this));
}


void CSerialCommunicationExampleDlg::OnBnClickedButton4()
{
	sendThread.stop();
}
