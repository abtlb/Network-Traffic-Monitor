#include "Include/pcap.h"
#include "Include/pcap/funcattrs.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <bitset>
#include <string>
#include <unordered_map>
#include "Network.h"
#include "ProcessGetter.h"

#pragma comment(lib, "Ws2_32.lib")

struct IPAddress
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
};

struct IPHeader
{
	u_char ver_ihlen;//version and ip header length
	u_char tos;//Type of Service
	u_short len;//Length of packet
	u_short identifer;//Packet identification
	u_short flags_foffset;//flags and fragment offset
	u_char ttl;//time to live
	u_char protocol;//higher layer protocol
	u_short checksum;//for error checking
	IPAddress srcAddr;
	IPAddress dstAddr;
	u_int op_pad;
};

struct TCPHeader
{
	u_short srcPort;
	u_short dstPort;
	//there are more fields but i need just those for now
};

bool compare_addr(IPAddress* addr1, IPAddress* addr2);

IPAddress* ip;

void PacketManager::setup()
{
	pcap_if_t* alldevs = get_devices_list();//points to the first device, other devices follow consecutively
	//display_devices_options();
	pcap_if_t* dev = select_interface(alldevs);
	long ipAddressVal = ((struct sockaddr_in*)(dev->addresses->addr))->sin_addr.S_un.S_addr;
	ip = (IPAddress*)(&ipAddressVal);
	pcap_t* adhandle = open_adapter(dev);
	long netmask;
	if (dev->addresses == NULL)
	{
		netmask = 0xffffff;
	}
	else
	{
		netmask = ((struct sockaddr_in*)(dev->addresses->netmask))->sin_addr.S_un.S_addr;
	}
	compile_and_set_filter(adhandle, netmask);
	std::cout << "Listening on " << dev->description << "...\n";
	pcap_freealldevs(alldevs);
	pcap_loop(adhandle, 0, packet_handler, NULL);

}

void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data)
{
	static int inAcc = 0;//incoming packets accumelator
	static int outAcc = 0;//outgoing packets accumelator
	static int lastChecked = clock();
	static int checkingInterval = 1000;//calculate throughput every 1000 ms
	static std::unordered_map<std::wstring, int> map;//process -> consumption
	static int maxConsumption = 0;
	static std::wstring maxProcess;
	static ProcessGetter pg;

	IPHeader* ih = (IPHeader*)(pkt_data + 14); //ethernet frame length is 14
	IPAddress* destAddr = &(ih->dstAddr);
	int ihLen = (ih->ver_ihlen & 0xf) * 4;
	TCPHeader* th = (TCPHeader*)((ihLen + (u_char*)(ih)));//* 4 because fields contains length in words
	u_short port;
	if (compare_addr(destAddr, ip))//incoming packet
	{
		inAcc += header->len;
		port = ntohs(th->dstPort);
	}
	else
	{
		outAcc += header->len;
		port = ntohs(th->srcPort);
	}
	std::wstring process = pg.PortToProcess(port);
	map[process] = map[process] + header->len;
	if (map[process] > maxConsumption)
	{
		maxConsumption = map[process];
		maxProcess = process;
	}


	int currTime = clock();
	int timeInterval = currTime - lastChecked;
	if (timeInterval >= checkingInterval)
	{
		double inRate = inAcc / (double)(timeInterval);
		double outRate = outAcc / (double)(timeInterval);
		PrintMessage(inRate, outRate, process);
		map.clear();
		maxConsumption = 0;
		inAcc = 0;
		outAcc = 0;
		lastChecked = currTime;
	}
}

pcap_if_t* PacketManager::get_devices_list()
{
	pcap_if_t* alldevs;
	char err[PCAP_ERRBUF_SIZE];
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, err))
	{
		std::cout << "Error occured: " << err << std::endl;
		return nullptr;
	}
	return alldevs;
}

void PacketManager::display_devices_options()
{
	pcap_if_t* device = PacketManager::get_devices_list();
	devNum = 1;
	while (device != NULL)
	{
		std::cout << devNum << ": \n";
		std::cout << "\tDevice Name: " << device->name << std::endl;
		std::cout << "\tDescription: " << device->description << std::endl;
		std::cout << "\tIs Loopback: " << ((device->flags & PCAP_IF_LOOPBACK) ? "Yes" : "No") << std::endl;
		if (device->addresses != NULL)
		{
			IPAddress* ipAddress = get_address(device->addresses->addr);
			IPAddress* netmask = get_address(device->addresses->netmask);
			IPAddress* broadAddress = get_address(device->addresses->broadaddr);

			std::cout << "\t IP Address: " << (int)ipAddress->byte1 << "." << (int)ipAddress->byte2 << "."
				<< (int)ipAddress->byte3 << "." << (int)ipAddress->byte4 << std::endl;
			std::cout << "\t Subnet mask: " << (int)netmask->byte1 << "." << (int)netmask->byte2 << "."
				<< (int)netmask->byte3 << "." << (int)netmask->byte4 << std::endl;
			std::cout << "\t Broadcast Address: " << (int)broadAddress->byte1 << "." << (int)broadAddress->byte2 << "."
				<< (int)broadAddress->byte3 << "." << (int)broadAddress->byte4 << std::endl;
		}
		device = device->next;
		devNum++;
	}
}

IPAddress* PacketManager::get_address(sockaddr* addr)
{
	long val = ((struct sockaddr_in*)(addr))->sin_addr.S_un.S_addr;
	IPAddress* ipAddress = (IPAddress*)(&val);
	return ipAddress;
}

pcap_if_t* PacketManager::select_interface(pcap_if_t* alldevs)
{
	pcap_if_t* dev = alldevs;
	while (dev != NULL)
	{
		if (dev->addresses != NULL)
		{
			IPAddress* ip = get_address(dev->addresses->addr);
			if ((int)ip->byte1 == 192)//found the interface
			{
				break;
			}
		}
		dev = dev->next;
	}
	if (dev == NULL)//didn't find interface
	{
		throw new std::exception;
	}

	return dev;
}

pcap_t* PacketManager::open_adapter(pcap_if_t* dev)
{
	pcap_t* adhandle;
	char err[256];
	adhandle = pcap_open(dev->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, err);
	if (adhandle == NULL)
	{
		std::cout << "Couldn't open adapter.";
		free_devices_list();
		return nullptr;
	}
	return adhandle;
}

void PacketManager::compile_and_set_filter(pcap_t* adhandle, long netmask)
{
	//compile the filter
	bpf_program fcode;//holds the BPF program that specifies the filter to be applied to packets
	int success = pcap_compile(adhandle, &fcode, "ip && (tcp)", 1, netmask);
	if (success < 0)
	{
		std::cout << "Unable to compile the filter.";
		free_devices_list();
		return;
	}

	//set the filter
	success = pcap_setfilter(adhandle, &fcode);
	if (success < 0)
	{
		std::cerr << "Error setting the filter.";
		free_devices_list();
		return;
	}
}

void PacketManager::free_devices_list()
{
	auto alldevs = get_devices_list();
	pcap_freealldevs(alldevs);
}

bool compare_addr(IPAddress* addr1, IPAddress* addr2)
{
	return addr1->byte1 == addr2->byte1 && addr1->byte2 == addr2->byte2 && addr1->byte3 == addr2->byte3 && addr1->byte4 == addr2->byte4;
}