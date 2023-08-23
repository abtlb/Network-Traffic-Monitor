#pragma once

#include "resource.h"
#include "Include/pcap.h"
#include "Include/pcap/funcattrs.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <bitset>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

struct IPAddress;
class PacketManager
{
public:
	void setup();
private:
	int devNum;
	pcap_if_t* get_devices_list();
	void display_devices_options();
	IPAddress* get_address(sockaddr* addr);
	pcap_if_t* select_interface(pcap_if_t* alldevs);
	pcap_t* open_adapter(pcap_if_t* dev);
	void compile_and_set_filter(pcap_t* adhandle, long netmask);
	void free_devices_list();
};

void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data);
void PrintMessage(double in, double out, wchar_t* process);


