#ifndef __SOAPY_ENUM_H__
#define __SOAPY_ENUM_H__

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <SoapySDR/Device.hpp>

class SDRDevInfo;

class SoapyEnum
{
public:
    SoapyEnum();
    ~SoapyEnum();

    std::vector<SDRDevInfo*>* enumerateDevices(std::string _remoteAddr = "", bool _noInit = false);
    std::vector<std::string> getRemotes();
    std::vector<std::string> getFactories();
    void addRemote(std::string _remoteAddr);
    void removeRemote(std::string _remoteAddr);
    void reset();

private:
    bool m_soapyInitialized;
    bool m_hasRemote;
    std::vector<std::string> m_Factories;
    std::vector<std::string> m_Modules;
    std::vector<std::string> m_Remotes;
    std::map<std::string, std::vector<SDRDevInfo*> > m_Devs;
    std::mutex m_Muitex;
};

#endif // __SOAPY_ENUM_H__

