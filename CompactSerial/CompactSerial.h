#pragma once
#include <Windows.h>
#include <atlstr.h>
class CompactSerial
{
public:
	CompactSerial();
	virtual ~CompactSerial();

	bool InitPort(	size_t	numPort,
					size_t	baudrate,
					char	parity,
					size_t	databits,
					size_t	stopbits,
					size_t	nBufferSize);
	void ClosePort();


	DWORD Read(BYTE* buffer, size_t size_buffer);
	DWORD Write(BYTE* buffer, size_t length);


private:

	bool port_opened_ = false;
	size_t numPort_ = 0;
	HANDLE handle_comm_ = INVALID_HANDLE_VALUE;
	COMMTIMEOUTS	commtimeout_ = COMMTIMEOUTS();
	DCB				dcb_ = DCB();

	/*Overlapped는 Serial 핸들과 계속 연결되어있기 때문에 지역변수로 선언하면 안됨*/
	OVERLAPPED		overlapped_ = OVERLAPPED();
};

