#include "SoapyEnum.h"
#include "SDRDevInfo.h"

#include <iostream>
#include <algorithm>

#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>

using namespace std;

SoapyEnum::SoapyEnum() : m_soapyInitialized(false), m_hasRemote(false)
{

}

SoapyEnum::~SoapyEnum()
{
    reset();
}

vector<SDRDevInfo*>* SoapyEnum::enumerateDevices(std::string _remoteAddr, bool _noInit)
{
    if(m_Devs.find(_remoteAddr) != m_Devs.end())
        return &m_Devs.at(_remoteAddr);

    if(_noInit) {
        return NULL;
    }


    if(!m_soapyInitialized) {
        cout << "SoapySDR init.." << endl;
        cout << "\tAPI Version: v" << SoapySDR::getAPIVersion() << endl;
        cout << "\tABI Version: v" << SoapySDR::getABIVersion() << endl;
        cout << "\tInstall root: " << SoapySDR::getRootPath() << endl << flush;

        cout << "\tLoading modules... " << endl << flush;

        vector<string> localMods = SoapySDR::listModules();
        for(string module : localMods) {
            cout << "Loading bundled SoapySDR module " << module << ".." << endl << flush;
            SoapySDR::loadModule(module);
        }

        m_Factories.clear();
        cout << "\tAvailable factories...";
        SoapySDR::FindFunctions factories = SoapySDR::Registry::listFindFunctions();

        for(SoapySDR::FindFunctions::const_iterator It = factories.begin(); It != factories.end(); It++)
        {
            if(It != factories.begin()) {
                cout << ", ";
            }
            cout << It->first;

            if(It->first == "remote") {
                m_hasRemote = true;
            }
            m_Factories.push_back(It->first);
        }
        if(factories.empty()) {
            cout << "--- No factories found! ---" << endl;
        }

        if ((factories.size() == 1) && factories.find("null") != factories.end()) {
            cout << "--- Just 'null' factory found. ---" << endl;
            cout << "--- No modules available. ---" << endl;
        }
        cout << endl;
        m_soapyInitialized = true;
    }

    m_Modules = SoapySDR::listModules();

    vector<SoapySDR::Kwargs> results;
    SoapySDR::Kwargs enumArgs;
    bool isRemote = false;

    if(_remoteAddr.length()) {
        cout << "--- Enumerating remote address: ---" << _remoteAddr << endl;
        enumArgs["driver"] = "remote";
        enumArgs["remote"] = _remoteAddr;
        isRemote = true;
        results = SoapySDR::Device::enumerate(enumArgs);
    } else {
        _remoteAddr = "local";
        results = SoapySDR::Device::enumerate();
    }


    auto dev_i = m_Devs.find(_remoteAddr);
    if(dev_i != m_Devs.end()) {
        while(dev_i->second.size()) {
            SDRDevInfo* devRemove = dev_i->second.back();
            dev_i->second.pop_back();
            delete devRemove;
        }
    }

    int idx = 0;
    for(auto& deviceArgs : results)
    {
        SDRDevInfo* dev = new SDRDevInfo();

        for(SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); it++) {
            std::cout << "  " << it->first << " = " << it->second << std::endl;
            if(it->first == "available") {
                dev->setAvailable((it->second == "Yes") ? true : false);
            }
            else if(it->first == "driver") {
                dev->setDriver(it->second);
            }
            else if(it->first == "label" || it->first == "device") {
                dev->setName(it->second);
            }
            else if(it->first == "manufacturer") {
                dev->setManufacturer(it->second);
            }
            else if(it->first == "product") {
                dev->setProduct(it->second);
            }
            else if(it->first == "serial") {
                dev->setSerial(it->second);
            }
            else if(it->first == "tuner") {
                dev->setTuner(it->second);
            }
        }

        if(deviceArgs.count("remote")) {
            isRemote = true;
        } else {
            isRemote = false;
        }
        dev->setRemote(isRemote);

        cout << "Make device " << idx << endl;
        SoapySDR::Device* device = SoapySDR::Device::make(deviceArgs);
        SoapySDR::Kwargs info = device->getHardwareInfo();
        for (SoapySDR::Kwargs::const_iterator it = info.begin(); it != info.end(); it++) {
            cout << "  " << it->first << "=" << it->second << endl;
            if(it->first == "hardware") {
                dev->setHardware(it->second);
            }
        }
        dev->setDeviceArgs(deviceArgs);
        SoapySDR::Device::unmake(device);

        m_Devs[_remoteAddr].push_back(dev);
        idx++;
    }

    return &m_Devs[_remoteAddr];
}

std::vector<std::string> SoapyEnum::getRemotes()
{
    return m_Remotes;
}

std::vector<std::string> SoapyEnum::getFactories()
{
    return m_Factories;
}

void SoapyEnum::addRemote(std::string _remoteAddr)
{
    std::vector<std::string>::iterator Iter = std::find(m_Remotes.begin(), m_Remotes.end(), _remoteAddr);

    if(Iter == m_Remotes.end())
        m_Remotes.push_back(_remoteAddr);
}

void SoapyEnum::reset()
{
    m_soapyInitialized = false;
    m_Factories.clear();
    m_Modules.clear();

    for(auto devs : m_Devs) {
        while(devs.second.size()) {
            SDRDevInfo* devRemove = devs.second.back();
            devs.second.pop_back();
            devRemove->setSoapyDevice(nullptr);
            delete devRemove;
        }
    }
    m_Devs.clear();
}

