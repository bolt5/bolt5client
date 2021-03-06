#include "client.h"

Client::Client() {
    printf("Client init\n");

#ifdef UDP_SPEC
    udp = new udp_client("localhost", 1230);
#endif
}

void Client::addToReadStream(void* data, uint32_t size) {
#ifdef CS16
    short* ptr = (short*)data;
#else
    float* ptr = (float*)data;
#endif
    inputStreamBuffer.insert(inputStreamBuffer.end(), ptr, ptr+size);
    decimationAndFiltr();
}

int tmp = 0;


void Client::decimationAndFiltr() {
    uint32_t inBuffSize = inputStreamBuffer.size();
    int i=0;
    float mul = 1024.0*8.0 - 1.0;
    
    for (; i < inBuffSize-BL*2; i+=decimationK*2) {
        float iC = 0.0;
        float qC = 0.0;
        
        for (int j=0; j<BL; j++) {
            iC += inputStreamBuffer[j*2 +     i] * B[j];
            qC += inputStreamBuffer[j*2 + 1 + i] * B[j];
        }
        
        if (start) {
            max = iC;
            start = false;
        }
        else {
            float fi = std::fabs(iC);
            float fq = std::fabs(qC);

            if (fi > max) {
                max = fi;
                sign = iC < 0 ? 1 : 0;
            }

            if (fq > max) {
                max = fq;
                sign = qC < 0 ? 1 : 0;
            }
        }

#ifdef UDP_SPEC
        outputStreamBufferUDP.push_back(iC);
        outputStreamBufferUDP.push_back(qC);
#endif

        iC *= mul/max;
        qC *= mul/max;

        
        outputStreamBuffer.push_back((int16_t)iC);
        outputStreamBuffer.push_back((int16_t)qC);
        
        if (outputStreamBuffer.size() >= FRAMELEN*2) {
#ifdef UDP_SPEC
            printf("UDP send\n");
            udp->send((char*)outputStreamBufferUDP.data(), 1024 * 8);
            outputStreamBufferUDP.clear();
#endif
            sendBatch(outputStreamBuffer.data(), FRAMELEN*4);

            outputStreamBuffer.erase(outputStreamBuffer.begin(), outputStreamBuffer.begin() + FRAMELEN*2);
        }
    }
    
    inputStreamBuffer.erase(inputStreamBuffer.begin(), inputStreamBuffer.begin() + i);
}

void Client::addToQueue(int16_t *capture_buff, unsigned long size) {
    memcpy(queue+queueCounter, capture_buff, size);
    queueCounter += size;
    
    if (queueCounter >= FRAMELEN) {
        sendBatch(queue, FRAMELEN);
        memcpy(queue, queue+queueCounter, queueCounter);
        queueCounter = 0;
    }
}

bool Client::connectToServer(std::string address, int port) {
    socHandle = socket(AF_INET , SOCK_STREAM , 0);

    if (socHandle == -1) {
       printf("Could not create socket\n");
    }

    if(inet_addr(address.c_str()) == -1) {
        if ((he = gethostbyname(address.c_str())) == NULL) {
            herror("gethostbyname");
            printf("Failed to resolve hostname\n");
            return false;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
        for(int i = 0; addr_list[i] != NULL; i++) {
            addr.sin_addr = *addr_list[i];
            break;
        }
    }
    else {
         addr.sin_addr.s_addr = inet_addr(address.c_str());
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (int err = connect(socHandle, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        //printf("connect failed. Error %d\n", err);
        close(socHandle);
        return false;
    }
    connected = true;

    send(socHandle, "RTLSDR", 7, 0);

    return true;
}

bool Client::reconnect() {
    printf("Reconnecting...\n");
    close(socHandle);
    socHandle = socket(AF_INET , SOCK_STREAM , 0);
    int i = 0;
    
    while(!connect(socHandle, (struct sockaddr *)&addr, sizeof(addr))) {
        i++;
        printf("%d\n",i);
    }
    
    connected = true;
    return true;
}

void Client::sendBatch(int16_t* _capture_buffer, unsigned long size_b) {
    memcpy(capture_buffer, _capture_buffer, size_b);
    this->size = size_b;

    if (th[thN] != NULL) {
        th[thN]->join();
        delete th[thN];
    }

    th[thN] = new std::thread(&Client::sender, this, thN);
    thN = (thN+1) % THREADS;
}

void Client::sender(int n) {
    memcpy(transmitBuff, "STRT", 4);
    unsigned char* ptr = transmitBuff + 4;
    uLong s1ps2 = 0;;

    for (int i=0; i<size; i+=2) {
        buff1[i/2] = capture_buffer[i];
        buff2[i/2] = capture_buffer[i + 1];
    }
    uLong tmpSize;
    uint32_t totalSize = 2 * sizeof(uint32_t) + 8;
    
    uint32_t* compressSize = (uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    tmpSize = sizeof(buff1);
    compress((Bytef*)(ptr), &tmpSize, buff1, size/2);
    *compressSize = tmpSize;
    totalSize += *compressSize;
    s1ps2 += *compressSize;

    ptr += *compressSize;
    compressSize = (uint32_t*)ptr;
    tmpSize = sizeof(buff2);
    ptr += sizeof(uint32_t);
    compress((Bytef*)ptr, &tmpSize, buff2, size/2);

    *compressSize = tmpSize;
    totalSize += *compressSize;
    s1ps2 += *compressSize;
    ptr += *compressSize;
    
    memcpy(ptr, &max, sizeof(max));
    ptr += sizeof(max);
    memcpy(ptr, &sign, sizeof(sign));
    ptr += sizeof(sign);
    totalSize += sizeof(max) + sizeof(sign);

    memcpy(ptr, "FNSH", 4);
    
    printf("SendStart %d %f\n", sign, max);
    long ret;
    
    //printf("!%d!\n", socHandle);
    ret = send(socHandle, transmitBuff, totalSize, 0);
  
    transmitData += ret;

    if (ret < 0) {
        printf("Error on transmiting!\n");
        connected = false;
        
        return;
    }

    if (ret < totalSize) {
        printf("Not all data transmited! %d\n", totalSize - ret);
        return;
    }
    //delete th[n];
}

