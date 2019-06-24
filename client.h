#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h>
#include <vector>
#include <thread>
#include <fstream>
#include <math.h>

#include <zlib/zlib.h>
#include "fdacoefs.h"

#define deflateX2

#define FRAMELEN 131072

#define THREADS 1

class Client
{
public:
    bool connected = false;
    
    Client();
    bool connectToServer(std::string addr, int port);
    bool reconnect();
    void addToQueue(int16_t* capture_buff, unsigned long size);
    void sendBatch(int16_t* alsa_capture_buffer, unsigned long size);
    void compessBatch(int16_t* alsa_capture_buffer, unsigned long oldSize, unsigned long newSize);
    void addToReadStream(void* data, uint32_t size);
    void setBytePerSample(int i) {bytes_per_sample = i;}
    void setDecimationK(int i) {decimationK = i;}

    unsigned int transmitData = 0;
private:
#ifdef UDP_SPEC
    udp_client* udp;
#endif
    
    float min;
    float max;
    bool start = true;
    
    std::thread* th[THREADS];
    int thN = 0;

    int bytes_per_sample;
    int decimationK;

    struct hostent *he;
    struct in_addr **addr_list;

    int socHandle = -1;
    struct sockaddr_in addr;
    struct hostent* hostinfo;

#ifdef CS16
    std::vector<int16_t> inputStreamBuffer;
#else
    std::vector<float> inputStreamBuffer;
#endif
    std::vector<int16_t> outputStreamBuffer;
    
#ifdef UDP_SPEC
    std::vector<float> outputStreamBufferUDP;
#endif
    
    int16_t queue[FRAMELEN*2];
    int queueCounter = 0;
    
    unsigned char capture_buffer[FRAMELEN*4];
    unsigned long size;
    unsigned char buff1[FRAMELEN*4];
    unsigned char buff2[FRAMELEN*4];
    unsigned char transmitBuff[FRAMELEN*4 + 8];

    void decimationAndFiltr();
    void sender(int n);
    bool tryToConnect();
};

#endif // CLIENT_H
