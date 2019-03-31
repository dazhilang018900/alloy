#include <system/AlloyFileUtil.h>
#include <common/AlloyCommon.h>
#include <sstream>
#include <map>
#include "AlloyNetwork.h"
namespace aly {

void SANITY_CHECK_MACHINEID() {
	std::cout << aly::MakeMachineId() << std::endl;
	std::cout << aly::GetMachineName() << std::endl;
	for (auto pr : GetNetworkAddresses()) {
		std::cout <<pr<<std::endl;
	}
}
template<typename I> std::string n2hexstr(I w,
		size_t hex_len = sizeof(I) << 1) {
	static const char* digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');
	for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		rc[i] = digits[(w >> j) & 0x0f];
	return rc;
}
#ifdef ALY_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <atlstr.h>
std::string GetNetworkInterfaces() {
	IP_ADAPTER_INFO AdapterInfo[32];
	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus != ERROR_SUCCESS)return;
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	std::stringstream ss;
	std::vector<std::string> addresses;
	do {
		std::stringstream mac;
		for(int i=0;i<6;i++) {
			mac<<n2hexstr(pAdapterInfo->Address[i])<<((i<5)?":":"");
		}
		addresses.push_back(mac.str());
	}while((pAdapterInfo=pAdapterInfo->Next)!=0);
	std::sort(addresses.begin(),addresses.end());
	for(auto s:addresses) {
		ss<<"["<<s<<"]";
	}
	return ss.str();
}
std::string GetCpuId() {
	int cpuinfo[4] = {0, 0, 0, 0};
	__cpuid(cpuinfo, 0);
	std::stringstream ss;
	ss<<n2hexstr(cpuinfo[0])<<n2hexstr(cpuinfo[1])<<n2hexstr(cpuinfo[2])<<n2hexstr(cpuinfo[3]);
	return ss.str();
}
std::string GetMachineName() {
	const DWORD size = 1024;
	char computerName[size];
	GetComputerName(computerName, &size);
	return std::string(computerName);
}
#else

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef DARWIN
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/if_types.h>
#else
#include <linux/if.h>
#include <linux/sockios.h>
#endif

std::string GetMachineName() {
	static struct utsname u;
	if (uname(&u) < 0) {
		return "unknown";
	}
	return std::string((char*) u.nodename);
}
std::vector<NetworkInterface> GetNetworkAddresses() {
	std::vector<NetworkInterface> result;
	std::map<std::string,std::string> macAddresses;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0)
		return std::vector<NetworkInterface>();
	struct ifconf conf;
	char ifconfbuf[128 * sizeof(struct ifreq)];
	memset(ifconfbuf, 0, sizeof(ifconfbuf));
	conf.ifc_buf = ifconfbuf;
	conf.ifc_len = sizeof(ifconfbuf);
	if (ioctl(sock, SIOCGIFCONF, &conf)) {
		return std::vector<NetworkInterface>();
	}
	struct ifreq *ifr;
	std::vector<std::string> addresses;
	for (ifr = conf.ifc_req;
			(char *) ifr < (char *) conf.ifc_req + conf.ifc_len; ifr++) {

		if (ifr->ifr_addr.sa_data == (ifr + 1)->ifr_addr.sa_data)
			continue;
		if (ioctl(sock, SIOCGIFFLAGS, ifr))
			continue;
		if (ioctl(sock, SIOCGIFHWADDR, ifr) == 0) {
			std::stringstream mac;
			for (int i = 0; i < 6; i++) {
				mac << n2hexstr(ifr->ifr_addr.sa_data[i])
						<< ((i < 5) ? ":" : "");
			}
			macAddresses[std::string(ifr->ifr_name)]=mac.str();
		}
	}
	close(sock);
	struct ifaddrs *interfaces = NULL;
	struct ifaddrs *temp_addr = NULL;
	int success = 0;
	// retrieve the current interfaces - returns 0 on success
	success = getifaddrs(&interfaces);
	if (success == 0) {
		// Loop through linked list of interfaces
		temp_addr = interfaces;
		while (temp_addr != NULL) {
			if (temp_addr->ifa_addr->sa_family == AF_INET) {
				// Check if interface is en0 which is the wifi connection on the iPhone
				std::string ethName(temp_addr->ifa_name);
				std::string ipAddress = inet_ntoa(
						((struct sockaddr_in*) temp_addr->ifa_addr)->sin_addr);
				result.push_back({ethName,macAddresses[ethName],ipAddress});
			}
			temp_addr = temp_addr->ifa_next;
		}
	}
	// Free memory
	freeifaddrs(interfaces);

	/*
	 *
	 * void ListIpAddresses(IpAddresses& ipAddrs)
	 {
	 IP_ADAPTER_ADDRESSES* adapter_addresses(NULL);
	 IP_ADAPTER_ADDRESSES* adapter(NULL);

	 // Start with a 16 KB buffer and resize if needed -
	 // multiple attempts in case interfaces change while
	 // we are in the middle of querying them.
	 DWORD adapter_addresses_buffer_size = 16 * KB;
	 for (int attempts = 0; attempts != 3; ++attempts)
	 {
	 adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(adapter_addresses_buffer_size);
	 assert(adapter_addresses);

	 DWORD error = ::GetAdaptersAddresses(
	 AF_UNSPEC,
	 GAA_FLAG_SKIP_ANYCAST |
	 GAA_FLAG_SKIP_MULTICAST |
	 GAA_FLAG_SKIP_DNS_SERVER |
	 GAA_FLAG_SKIP_FRIENDLY_NAME,
	 NULL,
	 adapter_addresses,
	 &adapter_addresses_buffer_size);

	 if (ERROR_SUCCESS == error)
	 {
	 // We're done here, people!
	 break;
	 }
	 else if (ERROR_BUFFER_OVERFLOW == error)
	 {
	 // Try again with the new size
	 free(adapter_addresses);
	 adapter_addresses = NULL;

	 continue;
	 }
	 else
	 {
	 // Unexpected error code - log and throw
	 free(adapter_addresses);
	 adapter_addresses = NULL;

	 // @todo
	 LOG_AND_THROW_HERE();
	 }
	 }

	 // Iterate through all of the adapters
	 for (adapter = adapter_addresses; NULL != adapter; adapter = adapter->Next)
	 {
	 // Skip loopback adapters
	 if (IF_TYPE_SOFTWARE_LOOPBACK == adapter->IfType)
	 {
	 continue;
	 }

	 // Parse all IPv4 and IPv6 addresses
	 for (
	 IP_ADAPTER_UNICAST_ADDRESS* address = adapter->FirstUnicastAddress;
	 NULL != address;
	 address = address->Next)
	 {
	 auto family = address->Address.lpSockaddr->sa_family;
	 if (AF_INET == family)
	 {
	 // IPv4
	 SOCKADDR_IN* ipv4 = reinterpret_cast<SOCKADDR_IN*>(address->Address.lpSockaddr);

	 char str_buffer[INET_ADDRSTRLEN] = {0};
	 inet_ntop(AF_INET, &(ipv4->sin_addr), str_buffer, INET_ADDRSTRLEN);
	 ipAddrs.mIpv4.push_back(str_buffer);
	 }
	 else if (AF_INET6 == family)
	 {
	 // IPv6
	 SOCKADDR_IN6* ipv6 = reinterpret_cast<SOCKADDR_IN6*>(address->Address.lpSockaddr);

	 char str_buffer[INET6_ADDRSTRLEN] = {0};
	 inet_ntop(AF_INET6, &(ipv6->sin6_addr), str_buffer, INET6_ADDRSTRLEN);

	 std::string ipv6_str(str_buffer);

	 // Detect and skip non-external addresses
	 bool is_link_local(false);
	 bool is_special_use(false);

	 if (0 == ipv6_str.find("fe"))
	 {
	 char c = ipv6_str[2];
	 if (c == '8' || c == '9' || c == 'a' || c == 'b')
	 {
	 is_link_local = true;
	 }
	 }
	 else if (0 == ipv6_str.find("2001:0:"))
	 {
	 is_special_use = true;
	 }

	 if (! (is_link_local || is_special_use))
	 {
	 ipAddrs.mIpv6.push_back(ipv6_str);
	 }
	 }
	 else
	 {
	 // Skip all other types of addresses
	 continue;
	 }
	 }
	 }

	 // Cleanup
	 free(adapter_addresses);
	 adapter_addresses = NULL;

	 // Cheers!
	 }
	 */
	return result;
}
std::string GetNetworkInterfaces() {
	std::stringstream ss;
#ifdef DARWIN
	struct ifaddrs* ifaphead;
	if ( getifaddrs( &ifaphead ) != 0 )
	return;
	struct ifaddrs* ifap;
	std::vector<std::string> addresses;
	for ( ifap = ifaphead; ifap; ifap = ifap->ifa_next )
	{
		struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifap->ifa_addr;
		if ( sdl && ( sdl->sdl_family == AF_LINK ) && ( sdl->sdl_type == IFT_ETHER ))
		{
			std::stringstream mac;
			unsigned char* ptr=(unsigned char*)(LLADDR(sdl));
			for(int i=0;i<6;i++) {
				mac<<n2hexstr(ptr[i])<<((i<5)?":":"");
			}
			addresses.push_back(mac.str());
		}
	}
	std::sort(addresses.begin(),addresses.end());
	for(auto s:addresses) {
		ss<<"["<<s<<"]";
	}
	freeifaddrs( ifaphead );
#else
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0)
		return "";
	struct ifconf conf;
	char ifconfbuf[128 * sizeof(struct ifreq)];
	memset(ifconfbuf, 0, sizeof(ifconfbuf));
	conf.ifc_buf = ifconfbuf;
	conf.ifc_len = sizeof(ifconfbuf);
	if (ioctl(sock, SIOCGIFCONF, &conf)) {
		return "";
	}
	struct ifreq *ifr;
	std::vector<std::string> addresses;
	for (ifr = conf.ifc_req;
			(char *) ifr < (char *) conf.ifc_req + conf.ifc_len; ifr++) {

		if (ifr->ifr_addr.sa_data == (ifr + 1)->ifr_addr.sa_data)
			continue;
		if (ioctl(sock, SIOCGIFFLAGS, ifr))
			continue;
		if (ioctl(sock, SIOCGIFHWADDR, ifr) == 0) {
			std::stringstream mac;
			for (int i = 0; i < 6; i++) {
				mac << n2hexstr(ifr->ifr_addr.sa_data[i])
						<< ((i < 5) ? ":" : "");
			}
			addresses.push_back(mac.str());
		}
	}
	close(sock);
	std::sort(addresses.begin(), addresses.end());
	for (auto s : addresses) {
		ss << "[" << s << "]";
	};
#endif
	return ss.str();
}

#ifdef DARWIN
#include <mach-o/arch.h>
std::string GetCpuId() {
	const NXArchInfo* info = NXGetLocalArchInfo();
	return MakeString()<< n2hexstr(info->cputype)<< n2hexstr(info->cpusubtype);
}
#else
std::string GetCpuId() {
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
#ifdef __arm__
	eax = 0xFD;
	ebx = 0xC1;
	ecx = 0x72;
	edx = 0x1D;
	return;
#else
	asm volatile("cpuid" :
			"=a" (eax),
			"=b" (ebx),
			"=c" (ecx),
			"=d" (edx) : "0" (eax), "2" (ecx));
#endif
	return MakeString() << n2hexstr(eax) << n2hexstr(ebx) << n2hexstr(ecx)
			<< n2hexstr(edx);
}
#endif // !DARWIN
#endif

std::string MakeMachineId() {
	using namespace aly::detail;
	std::stringstream ss;
	ss << GetMachineName() << "|" << GetCpuId() << "|" << GetNetworkInterfaces();
	std::string str = ss.str();
	std::vector<char> data(str.begin(), str.end());
	return HashCode(data, HashMethod::SHA1);
}

}
