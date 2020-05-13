#include "CompactSerial.h"

CompactSerial::CompactSerial()
{
	ZeroMemory(&commtimeout_, sizeof(COMMTIMEOUTS));
	ZeroMemory(&dcb_, sizeof(DCB));
	ZeroMemory(&overlapped_, sizeof(OVERLAPPED));

}

CompactSerial::~CompactSerial()
{
}

bool CompactSerial::InitPort(size_t numPort, size_t baudrate, char parity, size_t databits, size_t stopbits, size_t nBufferSize)
{
	bool result = false;

	numPort_ = numPort;

	CString strport;
	CString szBaud;
	strport.Format(_T("\\\\.\\COM%d"), numPort);
	szBaud.Format(_T("baud=%d parity=%c data=%d stop=%d"), baudrate, parity, databits, stopbits);

	handle_comm_ = CreateFile(strport,		                // communication port string (COMX)				
						GENERIC_READ | GENERIC_WRITE,	// read/write types
						FILE_SHARE_WRITE,				// comm devices must be opened with exclusive access
						NULL,							// no security attributes
						OPEN_EXISTING,               	// comm devices must use OPEN_EXISTING
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	// Async I/O
						0);								// template must be 0 for comm devices		

	if (handle_comm_ != INVALID_HANDLE_VALUE)                // port not found
	{
		commtimeout_.ReadIntervalTimeout = 0xffffffff;
		commtimeout_.ReadTotalTimeoutMultiplier = 0;
		commtimeout_.ReadTotalTimeoutConstant = 0;
		commtimeout_.WriteTotalTimeoutMultiplier = 2 * baudrate;
		commtimeout_.WriteTotalTimeoutConstant = 0;

		if (SetCommTimeouts(handle_comm_, &commtimeout_))
		{
			DWORD dwCommEvents = EV_RXCHAR;
			if (SetCommMask(handle_comm_, dwCommEvents))
			{
				if (GetCommState(handle_comm_, &dcb_))
				{
					dcb_.fRtsControl = RTS_CONTROL_ENABLE;
					if (BuildCommDCB(szBaud, &dcb_))
					{
						if (SetCommState(handle_comm_, &dcb_))
						{
							port_opened_ = true;
							result = true;

							overlapped_.Offset = 0;
							overlapped_.OffsetHigh = 0;
							overlapped_.hEvent = NULL;

							memset(&overlapped_, 0, sizeof(overlapped_));
							overlapped_.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
						}
					}
				}
			}
		}
	}
	return result;
}

void CompactSerial::ClosePort()
{
	if (handle_comm_ != INVALID_HANDLE_VALUE)
	{
		PurgeComm(handle_comm_, PURGE_RXCLEAR | PURGE_TXCLEAR |
			PURGE_RXABORT | PURGE_TXABORT);
		CloseHandle(handle_comm_);
		handle_comm_ = INVALID_HANDLE_VALUE;

	}

	port_opened_ = false;
}

DWORD CompactSerial::Read(BYTE* buffer, size_t size_buffer)
{
	DWORD num_read = 0;

	bool result = false;

	DWORD error = 0;
	COMSTAT comstat;
	DWORD comm_event = 0;
	if (port_opened_)
	{
		result = WaitCommEvent(handle_comm_, &comm_event, &overlapped_);

		if (comm_event & EV_RXCHAR)
		{
			comm_event = NULL;

			result = ClearCommError(handle_comm_, &error, &comstat);

			if (result)
			{

				result = ReadFile(handle_comm_, // Handle to COMM port 
					buffer,	      // RX Buffer Pointer
					size_buffer,  // Read one byte(1:한개씩, 2:두개마다 한개씩)
					&num_read,    // Stores number of bytes read
					&overlapped_);  // pointer to the m_ov structure
			}
		}
	}
	return num_read;
}

DWORD CompactSerial::Write(BYTE* buffer, size_t length)
{
	DWORD BytesSent = 0;
	if (port_opened_)
	{
		overlapped_.Offset = 0;
		overlapped_.OffsetHigh = 0;

		WriteFile(handle_comm_,		    // Handle to COMM Port					
			buffer,	// Pointer to message buffer in calling finction				
			length,	                // Length of message to send
			&BytesSent,	            // Where to store the number of bytes sent							
			(LPOVERLAPPED)&overlapped_);           // Overlapped structure

	}
	return BytesSent;
}

