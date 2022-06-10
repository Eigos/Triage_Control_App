#include "serialthread.h"

void SerialThread::run(){

    auto elapsedTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();

    running = true;

    while(running){
        auto currentTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
        try{
            if(currentTime - elapsedTime > 1000){
                elapsedTime = currentTime;
            }

            char data[255] {'\0'};

            serial->readSerialPort(data,254);

            if(data[0] != '\0'){
                emit newData(data);
            }
        }
        catch(std::exception &e){
        }

        this->msleep(5);
    }

}

bool SerialThread::isRunning(){
    return running;
}

bool SerialThread::isConnected(){
    return serial->isConnected();
}

void SerialThread::Stop(){
    running = false;
}

void SerialThread::Resume()
{
    running = true;
}

void newData(std::string data){
    return;
}

SerialThread::SerialThread(const char *portName, const uint32_t &BAUD)
{
    serial = new SerialPort(portName, BAUD);
}

SerialThread::~SerialThread()
{
    if(serial != nullptr){
        delete serial;
    }
}

void SerialThread::WriteToSerial(const std::string &data)
{
    serial->writeSerialPort(data.c_str(), data.size() - 1);

}
