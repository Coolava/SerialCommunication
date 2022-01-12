#pragma once
#include "ThreadController.h"
#include "SafeQueue.h"
#include "SerialPort.h"
#include <vector>
#include <string>

class SerialController
{
public:
	SerialController();
	~SerialController();

	bool open(std::wstring portString,
				size_t	baudrate,
				char	parity = 'n',
				size_t	databits = 8,
				size_t	stopbits = 1);
	void close();

	bool isOpened() {
		return serial_.mPort_Opened;
	}

	DWORD write(BYTE* buffer, size_t length);

	void setReadInterval(int milliseconds);

	auto& getDataQueue() { return serial_.getDataQueue(); }

	std::vector<std::wstring> getPortList();
private:
	SafeQueue<std::vector<unsigned char>> dataQueue_;

	CSerialPort serial_;

	/*Milliseconds*/
	int readInterval_;
};

