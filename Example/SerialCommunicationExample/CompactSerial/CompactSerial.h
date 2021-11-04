#pragma once
#include <Windows.h>
#include <atlstr.h>
#include <string>
#include <atomic>
class CompactSerial
{
public:
	CompactSerial();
	virtual ~CompactSerial();

	bool InitPort(std::wstring portString,
					size_t	baudrate,
					char	parity,
					size_t	databits,
					size_t	stopbits);
	void ClosePort();


	DWORD Read(BYTE* buffer, size_t size_buffer);
	DWORD Write(BYTE* buffer, size_t length);

	bool isOpened() { return port_opened_; }

private:

	std::atomic_bool port_opened_ = false;
	size_t numPort_ = 0;
	HANDLE handle_comm_ = INVALID_HANDLE_VALUE;
	COMMTIMEOUTS	commtimeout_ = COMMTIMEOUTS();
	DCB				dcb_ = DCB();

	/*Overlapped는 Serial 핸들과 계속 연결되어있기 때문에 지역변수로 선언하면 안됨*/
	OVERLAPPED		overlapped_ = OVERLAPPED();
};

