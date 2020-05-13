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

	/*Overlapped�� Serial �ڵ�� ��� ����Ǿ��ֱ� ������ ���������� �����ϸ� �ȵ�*/
	OVERLAPPED		overlapped_ = OVERLAPPED();
};

