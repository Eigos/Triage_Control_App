#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QtGlobal> // qInfo
#include <mutex>
#include <time.h>
#include "simpleserial.h"
#include <string>
#include <chrono>
#include <exception>



class SerialThread : public QThread
{
    Q_OBJECT
private:
    SerialPort* serial;

    bool running = false;

    void Process();

signals:
    void newData(std::string data);

public:

    SerialThread(const char *portName, const uint32_t& BAUD);

    ~SerialThread();

    void WriteToSerial(const std::string& data);

    void run();

    void Stop();

    void Resume();

    bool isRunning();

    bool isConnected();


};

#endif // SERIALTHREAD_H
