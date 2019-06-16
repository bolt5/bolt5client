#include "SoapyEnum.h"
#include "SDRDevInfo.h"
#include "client.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <iomanip>
#include <fstream>

Client client;
std::ofstream rawFile;

int g_exitRecvThread = 0;

int count = 0;

void recvThread(SoapySDR::Device* dev, SoapySDR::Stream* stream, std::vector<char>* recvBuf, int bytes_per_sample)
{
    dev->activateStream(stream);
    int numElems = recvBuf->size() / bytes_per_sample;
    void* Buffs[1];
    int flags;
    long long timeNs;
    int k = dev->getSampleRate(SOAPY_SDR_RX, 0)/125000.0;
    
    client.setDecimationK(k);

    Buffs[0] = (void*)recvBuf->data();
    std::cout << " k = " << k << std::endl;

    while(!g_exitRecvThread)
    {
        
        int n_stream_read = dev->readStream(stream, Buffs, numElems, flags, timeNs);
        if(n_stream_read < 0)
            std::cout << "Soapy read failed with code: " << n_stream_read << std::endl;
		else {
            std::cout << "Read " << n_stream_read << std::endl;
            client.addToReadStream(recvBuf->data(), n_stream_read*2);
		}

    }

    dev->deactivateStream(stream);
}


int main(int argc, char **argv)
{
    
    if (argc != 3 && argc != 4) {
        std::cout << "usage: ./Bolt5_Client host port [frequency]" << std::endl;
        return 0;
    }
    
    std::string addr = argv[1];
    int port = atoi(argv[2]);
    
    std::cout << "Trying to connect..." << std::endl;;
    if (!client.connectToServer(addr, port)) {
        std::cout << "unsuccessfull :(" << std::endl;
        return 0;
    }
    std::cout << "Successfull! :)" << std::endl;
    
    double freq = 868100000.0;
    
    if (argc == 4)
        freq = (double) atoi(argv[3]);
    
    SoapyEnum sdr_enum;
    std::string sdr_driver("rtlsdr");

    std::vector<SDRDevInfo*>* sdrDevices = sdr_enum.enumerateDevices();

    if(sdrDevices != 0) {

        std::vector<std::string> factories = sdr_enum.getFactories();

        std::cout << factories.size() << " devices" << std::endl;
        
        if(std::find(factories.begin(), factories.end(), sdr_driver) != factories.end())
        {
            std::cout << "Found " << sdr_driver << " factory " << std::endl;

            auto result = std::find_if(sdrDevices->begin(), sdrDevices->end(), [sdr_driver](SDRDevInfo* dev_i) { return dev_i->getDriver() == sdr_driver;});
            if(result == sdrDevices->end())
                std::cout << "Device " << sdr_driver << " not found " << std::endl;
            else
                std::cout << "Device " << sdr_driver << " found! " << std::endl;

            // Make RTL Device

            SDRDevInfo* devInfo = (*result);
            SoapySDR::Device* soapyDev = devInfo->getSoapyDevice();
            
            if(soapyDev)
            {
                SoapySDR::RangeList freqRange = soapyDev->getFrequencyRange(SOAPY_SDR_RX, 0);
                SoapySDR::RangeList sampleRateRange = soapyDev->getSampleRateRange(SOAPY_SDR_RX, 0);

                double sampleRate = soapyDev->getSampleRate(SOAPY_SDR_RX, 0);

				//std::cout << freqRange
				
                std::cout << "Got center frequency: " << freq / 1000000.0 << " MHz" << std::endl;
                std::cout << "Got sample rate: " << sampleRate / 1000.0 << " KHz" << std::endl;

                std::vector<std::string> formats = soapyDev->getStreamFormats(SOAPY_SDR_RX, 0);
                std::cout << "Supported formats: ";
                for(auto fmt : formats)
                    std::cout << fmt << ", ";
                std::cout << std::endl;

                int bytes_per_sample = 4;
                std::string fmt;
#ifdef CS16
                if(std::find(formats.begin(), formats.end(), "CS16") != formats.end()) {
                    fmt = "CS16";
                    bytes_per_sample = sizeof(short)*2;
                }
                else
#endif
                if(std::find(formats.begin(), formats.end(), "CF32") != formats.end()) {
                    fmt = "CF32";
                    bytes_per_sample = sizeof(float)*2;
                }
                else {
                    fmt = formats.back();
                    bytes_per_sample = sizeof(float)*2;
                }

                client.setBytePerSample(bytes_per_sample);
                
                SoapySDR::Kwargs args = devInfo->getDeviceArgs();
                SoapySDR::Stream* pStream = soapyDev->setupStream(SOAPY_SDR_RX, fmt, std::vector<size_t>(), args);
                if(!pStream) {
                    std::cout << "Error setup stream !!! Exit.. " << std::endl;
                    exit(EXIT_FAILURE);
                }

                std::cout << "Format: " << fmt << std::endl;
                
                // Set new frequancy and samplerate
				std::cout << "Max frec.: " << freqRange.back().maximum() << std::endl;
				std::cout << "Min frec.: " << freqRange.front().minimum() << std::endl;

				std::cout << "Max sample frec.: " << sampleRateRange.back().maximum() << std::endl;
				std::cout << "Min sample frec.: " << sampleRateRange.front().minimum() << std::endl;

				//freq = 868000000.0;
                sampleRate = 1000000.0;
                
                soapyDev->setFrequency(SOAPY_SDR_RX, 0, freq);
                soapyDev->setSampleRate(SOAPY_SDR_RX, 0, sampleRate);
                soapyDev->setGain(SOAPY_SDR_RX, 0, 15.0);
                soapyDev->setGainMode(SOAPY_SDR_RX, 0, true);
                soapyDev->setDCOffsetMode(SOAPY_SDR_RX, 0, false);
                soapyDev->writeSetting("digital_agc", "true");
                soapyDev->writeSetting("direct_samp", "0");
                soapyDev->writeSetting("offset_tune", "true");
                
                std::cout << "Gain = " << soapyDev->getGain(SOAPY_SDR_RX, 0) << std::endl;
    
                std::cout << "Setting new center freq: " << soapyDev->getFrequency(SOAPY_SDR_RX, 0) / 1000000.0 << "MHz" << " new samplerate: " << soapyDev-> getSampleRate (SOAPY_SDR_RX, 0) / 1000 << "KHz" << std::endl;

                int streamMTU = soapyDev->getStreamMTU(pStream);

				std::cout << "streamMTU = " << soapyDev->getStreamMTU(pStream) << std::endl;
				
				std::vector<char> recvBuf(streamMTU * bytes_per_sample);

                std::cout << "Starting receiver thread.... " << std::endl;

                std::thread t(recvThread, soapyDev, pStream, &recvBuf, bytes_per_sample);

                std::cout << "Press ENTER to exit receiving thread" << std::endl;

                int c;
                do
                {
                    c = getchar();
                } while (c != '\n' && c != EOF);


                g_exitRecvThread = 1;
                if(t.joinable())
                    t.join();


                soapyDev->closeStream(pStream);
            }
        }
    }
    return 0;
}
