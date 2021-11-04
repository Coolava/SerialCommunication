#include "pch.h"
#include "SerialController.h"
#include <array>

SerialController::SerialController()
{
	readInterval_ = 100;
}

SerialController::~SerialController()
{
}


bool SerialController::open(std::wstring portString, size_t baudrate, char parity, size_t databits, size_t stopbits)
{
	bool rslt = serial_.InitPort(portString, baudrate, parity, databits, stopbits, EV_RXCHAR);

	if (rslt == false)
	{
		return rslt;
	}
	serial_.StartMonitoring();
	return rslt;
}

void SerialController::close()
{
	serial_.StopMonitoring();
	serial_.ClosePort();
}

DWORD SerialController::write(BYTE* buffer, size_t length)
{
	return serial_.WriteToPort(buffer, length);
}

void SerialController::setReadInterval(int milliseconds)
{
	readInterval_ = milliseconds;
}

std::vector<std::wstring> SerialController::getPortList()
{
	std::vector<std::wstring> portVector;
	HKEY hKey;

	RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), &hKey);

	TCHAR szData[20], szName[100];
	DWORD index = 0, dwSize = 100, dwSize2 = 20, dwType = REG_SZ;

	memset(szData, 0x00, sizeof(szData));
	memset(szName, 0x00, sizeof(szName));


	while (ERROR_SUCCESS == RegEnumValue(hKey, index, szName, &dwSize, NULL, NULL, NULL, NULL))
	{
		index++;

		RegQueryValueEx(hKey, szName, NULL, &dwType, (LPBYTE)szData, &dwSize2);
		portVector.emplace_back(szData);

		memset(szData, 0x00, sizeof(szData));
		memset(szName, 0x00, sizeof(szName));
		dwSize = 100;
		dwSize2 = 20;
	}

	RegCloseKey(hKey);

	return portVector;
}
