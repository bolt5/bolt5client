#ifndef __SDR_DEV_INFO_H__
#define __SDR_DEV_INFO_H__

#include <string>
#include <vector>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>


class SDRDevInfo {
public:
    SDRDevInfo();
    ~SDRDevInfo();

    std::string getDeviceId();

    int getIndex() const;
    void setIndex(const int index);

    bool isAvailable() const;
    void setAvailable(bool available);

    const std::string& getName() const;
    void setName(const std::string& name);

    const std::string& getSerial() const;
    void setSerial(const std::string& serial);

    const std::string& getTuner() const;
    void setTuner(const std::string& tuner);

    const std::string& getManufacturer() const;
    void setManufacturer(const std::string& manufacturer);

    const std::string& getProduct() const;
    void setProduct(const std::string& product);

    const std::string& getDriver() const;
    void setDriver(const std::string& driver);

    const std::string& getHardware() const;
    void setHardware(const std::string& hardware);

    bool isRemote() const;
    void setRemote(bool remote);

    void setDeviceArgs(SoapySDR::Kwargs deviceArgs);
    SoapySDR::Kwargs getDeviceArgs();

    void setStreamArgs(SoapySDR::Kwargs streamArgs);
    SoapySDR::Kwargs getStreamArgs();

    void setSoapyDevice(SoapySDR::Device* dev);
    SoapySDR::Device* getSoapyDevice();


private:
    int m_Index;
    std::string m_Name;
    std::string m_Serial;
    std::string m_Product;
    std::string m_Manufacturer;
    std::string m_Tuner;
    std::string m_Driver;
    std::string m_Hardware;
    bool m_Available;
    bool m_Remote;
    SoapySDR::Kwargs m_deviceArgs;
    SoapySDR::Kwargs m_streamArgs;
    SoapySDR::Device* m_soapyDevice;

};

#endif // __SDR_DEV_INFO_H__

