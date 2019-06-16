#include <SDRDevInfo.h>
#include <SoapySDR/Device.hpp>

SDRDevInfo::SDRDevInfo() : m_Index(0), m_Available(false),
                        m_Remote(false), m_soapyDevice(nullptr)
{

}

SDRDevInfo::~SDRDevInfo()
{
    if(m_soapyDevice != nullptr) {
        SoapySDR::Device::unmake(m_soapyDevice);
    }
}

std::string SDRDevInfo::getDeviceId() {
    std::string deviceId;

    deviceId.append(getName());
    deviceId.append(" :: ");
    deviceId.append(getSerial());

    return deviceId;
}

int SDRDevInfo::getIndex() const {
    return m_Index;
}

void SDRDevInfo::setIndex(const int index) {
    m_Index = index;
}

bool SDRDevInfo::isAvailable() const {
    return m_Available;
}

void SDRDevInfo::setAvailable(bool available) {
    m_Available = available;
}

const std::string& SDRDevInfo::getName() const {
    return m_Name;
}

void SDRDevInfo::setName(const std::string& name) {
    m_Name = name;
}

const std::string& SDRDevInfo::getSerial() const {
    return m_Serial;
}

void SDRDevInfo::setSerial(const std::string& serial) {
    m_Serial = serial;
}

const std::string& SDRDevInfo::getTuner() const {
    return m_Tuner;
}

void SDRDevInfo::setTuner(const std::string& tuner) {
    m_Tuner = tuner;
}

const std::string& SDRDevInfo::getManufacturer() const {
    return m_Manufacturer;
}

void SDRDevInfo::setManufacturer(const std::string& manufacturer) {
    m_Manufacturer = manufacturer;
}

const std::string& SDRDevInfo::getProduct() const {
    return m_Product;
}

void SDRDevInfo::setProduct(const std::string& product) {
    m_Product = product;
}

const std::string& SDRDevInfo::getDriver() const {
    return m_Driver;
}

void SDRDevInfo::setDriver(const std::string& driver) {
    m_Driver = driver;
}

void SDRDevInfo::setHardware(const std::string& hardware) {
    m_Hardware = hardware;
}

bool SDRDevInfo::isRemote() const {
    return m_Remote;
}

void SDRDevInfo::setRemote(bool remote) {
    m_Remote = remote;
}

void SDRDevInfo::setDeviceArgs(SoapySDR::Kwargs deviceArgs) {
    m_deviceArgs = deviceArgs;
}

SoapySDR::Kwargs SDRDevInfo::getDeviceArgs() {
    return m_deviceArgs;
}

void SDRDevInfo::setStreamArgs(SoapySDR::Kwargs streamsArgs) {
    m_streamArgs = streamsArgs;
}

SoapySDR::Kwargs SDRDevInfo::getStreamArgs() {
    return m_streamArgs;
}

void SDRDevInfo::setSoapyDevice(SoapySDR::Device* dev)
{
    if(m_soapyDevice) {
        SoapySDR::Device::unmake(m_soapyDevice);
    }
    m_soapyDevice = dev;
}

SoapySDR::Device* SDRDevInfo::getSoapyDevice()
{
    if(m_soapyDevice == nullptr) {
           m_soapyDevice = SoapySDR::Device::make(m_deviceArgs);
    }
    return m_soapyDevice;
}



