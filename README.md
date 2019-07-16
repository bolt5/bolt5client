# bolt5 could client
Client for the connection to Bolt5 server.
## Dependencies
- SoapySDR - https://github.com/pothosware/SoapySDR/wiki
- SoapyRTLSDR (for RTL-SDR) - https://github.com/pothosware/SoapyRTLSDR
## Install
```
git cline https://github.com/bolt5/bolt5client.git
cd bolt5client
mkdir build
cd build
cmake ..
make
```
## LimeSDR
For usage LimeSDR you need to comment line "#define RTLSDR" in main.cpp

## Run
./bolt5client addres pirt [freq]



