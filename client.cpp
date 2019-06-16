#include "client.h"

Client::Client() {
    printf("Client init\n");
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
    float mul = 1024.0*16.0 - 1.0;
    
    for (; i < inBuffSize-BL*2; i+=decimationK*2) {
        float iC = 0.0;
        float qC = 0.0;
        
        for (int j=0; j<BL; j++) {
            iC += inputStreamBuffer[j*2 +     i] * B[j];
            qC += inputStreamBuffer[j*2 + 1 + i] * B[j];
        }
        
        if (start) {
            min = max = iC;
            start = false;
        }
        else {
            if (iC < min || qC < min)
                min = std::min(iC, qC);
            
            if (iC > max || qC > max)
                max = std::max(iC, qC);
        }
        
        iC *= mul/max;
        qC *= mul/max;
        
        outputStreamBuffer.push_back((int16_t)iC);
        outputStreamBuffer.push_back((int16_t)qC);
        
        
        if (outputStreamBuffer.size() >= FRAMELEN*2) {
            sendBatch(outputStreamBuffer.data(), FRAMELEN*4);
            outputStreamBuffer.erase(outputStreamBuffer.begin(), outputStreamBuffer.begin() + FRAMELEN*2);
        }
    }
    
    inputStreamBuffer.erase(inputStreamBuffer.begin(), inputStreamBuffer.begin() + i);
}

void Client::addToQueue(int16_t *capture_buff, unsigned long size) {
    memcpy(queue+queueCounter, capture_buff, size);
    queueCounter+=size;
    
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

    if (connect(socHandle, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        //perror("connect failed. Error");
        return false;
    }
    return true;
}

void Client::sendBatch(int16_t* alsa_capture_buffer, unsigned long size_b) {
    memcpy(capture_buffer, alsa_capture_buffer, size_b);
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
    printf("2\n");
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
    
    memcpy(ptr, &min, sizeof(min));
    ptr += sizeof(min);
    memcpy(ptr, &max, sizeof(max));
    ptr += sizeof(max);
    totalSize += sizeof(max) + sizeof(min);

    
    //printf("Size = %u\n", s1ps2);
    memcpy(ptr, "FNSH", 4);
    
    //qDebug() << "size " << totalSize;
    //unsigned long ret = soc->write((char*)transmitBuff, totalSize);
    printf("SendStart\n");
    long ret = send(socHandle, transmitBuff, totalSize, 0);
  
    transmitData += ret;

    if (ret < 0) {
        printf("Error on transmiting! %d\n", ret);
        printf("Reconnecting...\n", ret);
        //reconnect();
        return;
    }

    if (ret < totalSize) {
        printf("Not all data transmited! %d\n", totalSize - ret);
        return;
    }
    //delete th[n];
}

