#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <TlHelp32.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

class ProcessGetter
{
public:
	const wchar_t* PortToProcess(const u_short& port);
private:
	void InitializeWinsock();
	const wchar_t* IDToProcess(u_short id);
};