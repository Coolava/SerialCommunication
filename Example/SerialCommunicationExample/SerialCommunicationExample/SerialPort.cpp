#include "pch.h"
#include "SerialPort.h"
#include <assert.h>


	//extern HWND g_hViewWnd;//gethandletest

//*****************************************************************************//
// Constructor
CSerialPort::CSerialPort()
{
	total = 0 ;
	m_hComm = NULL;
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;
	m_ov.hEvent = NULL;
	m_hWriteEvent = NULL;
	m_hRecvEvent = NULL;
	m_hShutdownEvent = NULL;
	//m_szWriteBuffer = NULL;
	m_bThreadAlive = FALSE;
	mPort_Opened = FALSE;
}

//*****************************************************************************//
// Delete dynamic memory
CSerialPort::~CSerialPort()
{
	do
	{
		SetEvent(m_hShutdownEvent);
	} while (m_bThreadAlive);
	TRACE("Thread ended\n");
	//delete [] m_szWriteBuffer;
}

//*****************************************************************************//
// Initialize the port. This can be port 1 to 4.
BOOL CSerialPort::InitPort(
	std::wstring portString,	      // portnumber (1..4)
	UINT  baud,	          // baudrate
	char  parity,	      // parity 
	UINT  databits,	      // databits 
	UINT  stopbits,	      // stopbits 
	DWORD dwCommEvents    // EV_RXCHAR, EV_CTS etc
)
{

	if (m_bThreadAlive) 
	{
		do
		{
			SetEvent(m_hShutdownEvent);
		} while (m_bThreadAlive);
		TRACE("Thread ended\n");
	}

	if (m_ov.hEvent != NULL) ResetEvent(m_ov.hEvent);  
	m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hWriteEvent != NULL) ResetEvent(m_hWriteEvent);
	m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hRecvEvent != NULL) ResetEvent(m_hRecvEvent);
	m_hRecvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	
	if (m_hShutdownEvent != NULL) ResetEvent(m_hShutdownEvent);
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//Initialize the event objects
	m_hEventArray[0] = m_hShutdownEvent; // highest priority
	m_hEventArray[1] = m_ov.hEvent;
	m_hEventArray[2] = m_hWriteEvent;

	InitializeCriticalSection(&m_csCommunicationSync) ;
	
	//if (m_szWriteBuffer != NULL) delete [] m_szWriteBuffer;
	//m_szWriteBuffer = new char[writebuffersize];

	m_dwCommEvents = dwCommEvents;
	
	BOOL bResult = FALSE;
	//char *szPort = new char[50];
	//char *szBaud = new char[50];
	CString strport;
	CString szBaud;
	
	//now it critical
	//EnterCriticalSection(&m_csCommunicationSync) ;

	//if the port is already opened: close it
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}
	//sprintf_s(szPort, 50, "COM%d", portnr); // prepare port strings
	//"\\\\.\\COM10",     
	strport.Format(_T("\\\\.\\%s"), portString.c_str());
	szBaud.Format(_T("baud=%d parity=%c data=%d stop=%d"), baud, parity, databits, stopbits);
	////////////////////////////////////////////////////
	//get a handle to the port
	m_hComm = CreateFile(strport,//(LPCWSTR)szPort,		                // communication port string (COMX)				
					     GENERIC_READ | GENERIC_WRITE,	// read/write types
					      FILE_SHARE_WRITE,			                    // comm devices must be opened with exclusive access
					     NULL,                          // no security attributes
					     OPEN_EXISTING,               	// comm devices must use OPEN_EXISTING
						 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	// Async I/O
					     0);	                        // template must be 0 for comm devices		
	if (m_hComm == INVALID_HANDLE_VALUE)                // port not found
	{
		//delete [] szPort;
		//delete [] szBaud;
		return FALSE;
	}

	//set the timeout values
	m_CommTimeouts.ReadIntervalTimeout = 0xffffffff ;    //1000; 
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 0 ;      //1000;
	m_CommTimeouts.ReadTotalTimeoutConstant = 0 ;        //1000;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 2*baud ;//1000;
	m_CommTimeouts.WriteTotalTimeoutConstant = 0 ;       //1000;

	//Configure
	if(SetCommTimeouts(m_hComm, &m_CommTimeouts))
	{
		if(SetCommMask(m_hComm, dwCommEvents))
		{
			if(GetCommState(m_hComm, &m_dcb))
			{
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE ;
				if(BuildCommDCB(szBaud, &m_dcb))
				{
					if(SetCommState(m_hComm, &m_dcb))
						;//normal operation.. ocntinue
					else ProcessErrorMessage("SetCommState()") ;
				}
				else ProcessErrorMessage("BuildCommDCB()") ;
			}
			else ProcessErrorMessage("GetCommState()") ;
		}
		else ProcessErrorMessage("SetCommMask()") ;
	}
	else ProcessErrorMessage("SetCommTimeOuts()") ;
	//Flush the port
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);// flush the port
	
	//release critical section
	//LeaveCriticalSection(&m_csCommunicationSync) ;
	TRACE("Initialisation for communicationport %s completed.\nUse Startmonitor to communicate.\n",portString);

	mPort_Opened = TRUE;//ClosePort()
	return TRUE;
}


//*****************************************************************************//
// start comm watching
BOOL CSerialPort::StartMonitoring()
{
	//if (!(m_Thread = AfxBeginThread(CommThread, this)))
	//	return FALSE;
	
	//if (!(m_ThreadWrite = AfxBeginThread(WriteChar, this,THREAD_PRIORITY_ABOVE_NORMAL)))
	//	return FALSE;


	if (!(m_ThreadRecv = AfxBeginThread(ReceiveChar, this,THREAD_PRIORITY_NORMAL)))
		return FALSE;

	m_bThreadAlive = true;
	//DWORD dwpClass = GetPriorityClass(GetCurrentProcess());
	//int pri = m_ThreadWrite->GetThreadPriority();

	TRACE("Thread started\n");
	return TRUE;	
}

//*****************************************************************************//
// Restart the comm thread
BOOL CSerialPort::RestartMonitoring()
{
	TRACE("Thread resumed\n");
	//m_Thread->ResumeThread();
	m_ThreadRecv->ResumeThread();
	return TRUE;	
}

//*****************************************************************************//
// Suspend the comm thread
BOOL CSerialPort::StopMonitoring()
{
	//TRACE("Thread suspended\n");
	//m_Thread->SuspendThread(); 
	//m_ThreadRecv->SuspendThread();
	return TRUE;	
}

//*****************************************************************************//
// If there is a error, give the right message
void CSerialPort::ProcessErrorMessage(char* ErrorText)
{
	char *Temp = new char[200];
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	sprintf_s(Temp, 200, "WARNING:  %s Failed with the following error: \n%s\nPort: %d\n", (char*)ErrorText, lpMsgBuf, m_nPortNr); 
	
	LocalFree(lpMsgBuf);
	delete[] Temp;
}

//WriteChar 스레드용

UINT CSerialPort::WriteChar(LPVOID pParam)
{
	CSerialPort* port = (CSerialPort* ) pParam;
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;
	DWORD BytesSent = 0;
	ResetEvent(port->m_hWriteEvent);
	// Gain ownership of the critical section
	
	//EnterCriticalSection(&port->m_csCommunicationSync);

	while (!WaitForSingleObject(port->m_hWriteEvent, INFINITE) )
	{
		ResetEvent(port->m_hWriteEvent);
		if (bWrite)
		{

			PurgeComm(port->m_hComm, PURGE_TXCLEAR | 
								 PURGE_TXABORT);

			port->m_ov.Offset = 0;// Initailize variables min
			port->m_ov.OffsetHigh = 0;

			EnterCriticalSection(&port->m_csCommunicationSync) ;
			bResult = WriteFile(port->m_hComm,		    // Handle to COMM Port					
							  port->m_szWriteBuffer,	// Pointer to message buffer in calling finction				
							  port->length,	                // Length of message to send
							  &BytesSent,	            // Where to store the number of bytes sent							
							 &port->m_ov);           // Overlapped structure
			LeaveCriticalSection(&port->m_csCommunicationSync) ;
		
		}
	}
	return 0;
}

//*****************************************************************************//
// Write a character.
void CSerialPort::WriteChar(CSerialPort* port)
{
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;
	DWORD BytesSent = 0;
	ResetEvent(port->m_hWriteEvent);

	if (bWrite)
	{
		port->m_ov.Offset = 0;// Initailize variables min
		port->m_ov.OffsetHigh = 0;
		// Clear buffer
		PurgeComm(port->m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR|
								 PURGE_TXABORT | PURGE_RXABORT);
		
		//실제 데이터를 보내기에 앞서 일단 데이터를 송신한다는 메세지를 한번 쏜다...
		//::PostMessage(port->m_pOwner->m_hWnd, WM_COMM_TXCHAR, (WPARAM)BytesSent, (LPARAM) 0 ) ;
		//실제 데이터를 보낸다. WriteFile
		bResult = WriteFile(port->m_hComm,		    // Handle to COMM Port					
						  port->m_szWriteBuffer,	// Pointer to message buffer in calling finction				
						  port->length,	                // Length of message to send
						  &BytesSent,	            // Where to store the number of bytes sent							
						  &port->m_ov);           // Overlapped structure

		
		//여기서부터 거의 모든 시간이 소요된다.
		// deal with any error codes
		if (!bResult)
		{
			DWORD dwError = GetLastError();
			switch (dwError)
			{
				case ERROR_IO_PENDING:
					{
						//continue to GetOverlappedResults()
						BytesSent = 0;
						bWrite = FALSE;
						break;
					}
				default:
					{
						// all other error codes
						port->ProcessErrorMessage("WriteFile()");
					}
			}
		} 
		else
		{

		}
	}// end if(bWrite)
	if (!bWrite)
	{
		bWrite = TRUE;
		bResult = GetOverlappedResult(port->m_hComm,	// Handle to COMM port  
									  &port->m_ov,      // Overlapped structure
									  &BytesSent,       // Stores number of bytes sent
									  TRUE);            // Wait flag
		
		//LeaveCriticalSection(&port->m_csCommunicationSync) ;
		
		//deal with the error code
		if (!bResult)  
		{
			port->ProcessErrorMessage("GetOverlappedResults() in WriteFile()");
		}	
	}

}

std::string TimepointToString(const std::chrono::system_clock::time_point& time)
{
	using namespace std::chrono;
	// get number of milliseconds for the current second
	// (remainder after division into seconds)
	auto us = duration_cast<microseconds>(time.time_since_epoch()) % 1000000;

	// convert to std::time_t in order to convert to std::tm (broken time)
	auto timer = system_clock::to_time_t(time);

	// convert to broken time
	std::tm bt;

	localtime_s(&bt, &timer);

	std::ostringstream oss;
	oss << std::put_time(&bt, "%T"); // HH:MM:SS
	oss << ',' << std::setfill('0') << std::setw(6) << us.count() << " ";

	return oss.str();
}


//스레드용 Receivechar
UINT CSerialPort::ReceiveChar(LPVOID pParam)
{
	CSerialPort* port = (CSerialPort* ) pParam;

	BOOL  bRead = TRUE; 
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	COMSTAT comstat;
	DWORD CommEvent = 0;
	unsigned char RXBuff[4096];
	while(1)
	{
		if( !port->m_bThreadAlive )
			return 0;

		WaitCommEvent(port->m_hComm,&CommEvent, &port->m_ov );

		if (CommEvent & EV_RXCHAR)
		{
			CommEvent = 0;
			for (;;) 
			{ 		
				// Gain ownership of the comm port critical section.
				// This process guarantees no other part of this program 
				// is using the port object.
		
				//EnterCriticalSection(&port->m_csCommunicationSync) ;

				// ClearCommError() will update the COMSTAT structure and
				// clear any other errors.
				bResult = ClearCommError(port->m_hComm, &dwError, &comstat);
				//LeaveCriticalSection(&port->m_csCommunicationSync);

				// start forever loop.  I use this type of loop because I
				// do not know at runtime how many loops this will have to
				// run. My solution is to start a forever loop and to
				// break out of it when I have processed all of the
				// data available.  Be careful with this approach and
				// be sure your loop will exit.
				// My reasons for this are not as clear in this sample 
				// as it is in my production code, but I have found this 
				// solutiion to be the most efficient way to do this.

				if (comstat.cbInQue == 0)
				{
					// break out when all bytes have been read
					break;
				}

				//EnterCriticalSection(&port->m_csCommunicationSync) ;

				if (bRead)
				{

					bResult = ReadFile(port->m_hComm, // Handle to COMM port 
									   RXBuff,	      // RX Buffer Pointer
									   1024,             // Read one byte(1:한개씩, 2:두개마다 한개씩)
									   &BytesRead,    // Stores number of bytes read
									   &port->m_ov);  // pointer to the m_ov structure
					//deal with the error code
					if (!bResult)  
					{ 
						switch (dwError = GetLastError()) 
						{ 
							case ERROR_IO_PENDING: 	
								{ 
									// asynchronous i/o is still in progress 
									// Proceed on to GetOverlappedResults();
									bRead = FALSE;
									break;
								}
							default:
								{
									// Another error has occured.  Process this error.
									port->ProcessErrorMessage("ReadFile()");
									break;
								} 
						}
					}
					else
					{
						// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
						bRead = TRUE;
					}
				}//close if(bRead)


				if (!bRead)
				{
					bRead = TRUE;
					bResult = GetOverlappedResult(port->m_hComm, // Handle to COMM port 
												  &port->m_ov,   // Overlapped structure
												  &BytesRead,    // Stores number of bytes read
												  TRUE); 
					if (!bResult)  
					{
						port->ProcessErrorMessage("GetOverlappedResults() in ReadFile()");
					}	
				}

				if(BytesRead > 0)
				{
					std::string timeStamp = TimepointToString(std::chrono::system_clock::now());
					Buffer buffer{
						timeStamp, std::vector<BYTE>(&RXBuff[0], &RXBuff[0] + BytesRead)
					};
					port->dataQueue.enqueue(buffer);

					BytesRead = 0;
				}
			}
		}
		Sleep(1);
	}
	return 0;
}

//*****************************************************************************//
// Write a string to the port
DWORD CSerialPort::WriteToPort(BYTE* buffer, size_t length)
{	
	
	DWORD BytesSent = 0;

	PurgeComm(m_hComm, PURGE_TXCLEAR | 
					   PURGE_TXABORT);

	m_ov.Offset = 0;// Initailize variables min
	m_ov.OffsetHigh = 0;
			
	WriteFile(m_hComm,		    // Handle to COMM Port					
				buffer,	// Pointer to message buffer in calling finction				
				length,	                // Length of message to send
				&BytesSent,	            // Where to store the number of bytes sent							
				&m_ov);           // Overlapped structure

	return BytesSent;
}


//*****************************************************************************//
// Return the device control block
DCB CSerialPort::GetDCB()
{
	return m_dcb;
}


//*****************************************************************************//
// Return the communication event masks
DWORD CSerialPort::GetCommEvents()
{
	return m_dwCommEvents;
}



void CSerialPort::ClosePort()//이것만 내가 추가한것..
{
	if(m_bThreadAlive != NULL)
	{
		m_bThreadAlive = FALSE;
		SetEvent(m_hShutdownEvent) ;
		m_Thread = NULL ;
	}

	if(m_hComm)
	{
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR |
								 PURGE_RXABORT | PURGE_TXABORT);
		CloseHandle(m_hComm) ;
		m_hComm = INVALID_HANDLE_VALUE ;
	}

	Sleep(500) ;
	total = 0 ;
	m_hComm = NULL;
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;
	m_ov.hEvent = NULL;
	m_hWriteEvent = NULL;
	m_hRecvEvent = NULL;
	m_hShutdownEvent = NULL;
	//m_szWriteBuffer = NULL;
	m_bThreadAlive = FALSE;

	mPort_Opened = FALSE;
}
