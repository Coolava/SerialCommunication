
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
#include <sstream>
#include <timeapi.h>

#pragma comment(lib,"Winmm.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSerialCommunicationExampleDlg dialog



CSerialCommunicationExampleDlg::CSerialCommunicationExampleDlg(CWnd* pParent /*=nullptr*/)
	: NoEscapeDialog(IDD_SERIALCOMMUNICATIONEXAMPLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	timeBeginPeriod(1);
}

CSerialCommunicationExampleDlg::~CSerialCommunicationExampleDlg()
{
	timeEndPeriod(1);
}

void CSerialCommunicationExampleDlg::DoDataExchange(CDataExchange* pDX)
{
	NoEscapeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, listReceived_);
	DDX_Control(pDX, IDC_LIST2, listSend_);
	DDX_Control(pDX, IDC_COMBO1, comboPort);
	DDX_Control(pDX, IDC_COMBO2, comboBaud);
}

BEGIN_MESSAGE_MAP(CSerialCommunicationExampleDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CSerialCommunicationExampleDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON5, &CSerialCommunicationExampleDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CSerialCommunicationExampleDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG, &CSerialCommunicationExampleDlg::OnBnClickedButtonConfig)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST1, &CSerialCommunicationExampleDlg::OnNMSetfocusList1)
	ON_NOTIFY(NM_KILLFOCUS, IDC_LIST1, &CSerialCommunicationExampleDlg::OnNMKillfocusList1)
END_MESSAGE_MAP()


// CSerialCommunicationExampleDlg message handlers

BOOL CSerialCommunicationExampleDlg::OnInitDialog()
{
	NoEscapeDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	auto portList = serialController_.getPortList();

	for (auto item : portList)
	{
		comboPort.AddString(item.c_str());
	}
	comboPort.SetCurSel(2);

	comboBaud.AddString(_T("19200"));
	comboBaud.AddString(_T("115200"));
	comboBaud.SetCurSel(1);

	listReceived_.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	listSend_.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	listReceived_.InsertColumn(0, _T("Time"), 0, 50);
	
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

	if (serialController_.isOpened() == true)
	{
		updateThread.stop();
		serialController_.close();
		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("Connect"));
	}
	else
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

		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("Disconnect"));
	}
}

void CSerialCommunicationExampleDlg::updater()
{
	
	bool opened = serialController_.isOpened();

	if (opened)
	{
		auto& dataQueue = serialController_.getDataQueue();

		while (dataQueue.size() > 0)
		{
			auto dataVector = dataQueue.dequeue();

			updateToList(dataVector);
		}

	}

}


void CSerialCommunicationExampleDlg::updateToList(const CSerialPort::Buffer& data)
{
	listReceived_.SetRedraw(FALSE);

	auto dataLength = data.data.size();
	auto columnCount = static_cast<size_t>(listReceived_.GetHeaderCtrl()->GetItemCount());

	for (auto i = columnCount-1; i < dataLength; i++)
	{
		listReceived_.InsertColumn(i+1, std::to_wstring(i).c_str(), 0, 50);
	}

	int itemCount = listReceived_.GetItemCount();

	listReceived_.InsertItem(itemCount, CString(data.timeStamp.c_str()));
	for (auto i = 0; i < dataLength; i++)
	{
		CString str;
		str.Format(_T("0x%X"), data.data[i]);
		listReceived_.SetItemText(itemCount, i + 1, str);
	}

	listReceived_.SetRedraw(TRUE);

	if(scroll_ == true)
		listReceived_.EnsureVisible(listReceived_.GetItemCount() - 1, FALSE);
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


void CSerialCommunicationExampleDlg::OnBnClickedButton5()
{
	listReceived_.DeleteAllItems();
}


void CSerialCommunicationExampleDlg::OnBnClickedButton6()
{
	listSend_.DeleteAllItems();
}


void CSerialCommunicationExampleDlg::OnBnClickedButtonConfig()
{
	// TODO: Add your control notification handler code here
}


void CSerialCommunicationExampleDlg::OnNMSetfocusList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	scroll_ = false;
	*pResult = 0;
}


void CSerialCommunicationExampleDlg::OnNMKillfocusList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	scroll_ = true;
	*pResult = 0;
}
