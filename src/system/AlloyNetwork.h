/*
 * MachineID.h
 *
 *  Created on: Mar 17, 2019
 *      Author: blake
 */

#ifndef SRC_SYSTEM_ALLOYNETWORK_H_
#define SRC_SYSTEM_ALLOYNETWORK_H_
#include <string>
#include<vector>
namespace aly {
void SANITY_CHECK_MACHINEID();
std::string MakeMachineId();
std::string GetMachineName();
std::string GetCpuId();
struct NetworkInterface{
	std::string name;
	std::string mac;
	std::string ip;
	NetworkInterface(const std::string& name="",const std::string& mac="",const std::string& ip=""):name(name),mac(mac),ip(ip){

	}
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
	std::basic_ostream<C, R> & ss, const NetworkInterface& type) {
	ss<<"[INTERFACE="<<type.name<<" | MAC="<<type.mac<<" | IP="<<type.ip<<"]";
	return ss;
}

std::vector<NetworkInterface> GetNetworkAddresses();
}
#endif /* SRC_SYSTEM_ALLOYNETWORK_H_ */
