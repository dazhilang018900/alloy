#include <system/AlloyFileUtil.h>
#include <common/AlloyCommon.h>
#include "MachineID.h"
#include <sstream>
namespace aly {

void SANITY_CHECK_MACHINEID() {
	std::cout << aly::MakeMachineId() << std::endl;
}

namespace detail {

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
std::string getNetwork() {
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
std::string getCPU() {
	int cpuinfo[4] = {0, 0, 0, 0};
	__cpuid(cpuinfo, 0);
	std::stringstream ss;
	ss<<cpuinfo[0]<<cpuinfo[1]<<cpuinfo[2]<<cpuinfo[3];
	return ss.str();
}
std::string getMachine() {
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
#ifdef DARWIN
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/if_types.h>
#else
#include <linux/if.h>
#include <linux/sockios.h>
#endif
std::string getMachine() {
	static struct utsname u;
	if (uname(&u) < 0) {
		return "unknown";
	}
	return std::string((char*) u.nodename);
}
std::string getNetwork() {
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
			for(int i=0;i<6;i++) {
				mac<<n2hexstr(LLADDR(sdl))<<((i<5)?":":"");
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
std::string getCPU() {
	const NXArchInfo* info = NXGetLocalArchInfo();
	return MakeString()<< (unsigned short)info->cputype<< (unsigned short)info->cpusubtype;
}
#else
std::string getCPU() {
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
}
std::string MakeMachineId() {
	using namespace aly::detail;
	std::stringstream ss;
	ss << getMachine() << "|" << getCPU() << "|" << getNetwork();
	std::string str = ss.str();
	std::vector<char> data(str.begin(), str.end());
	return HashCode(data, HashMethod::SHA1);
}

}
