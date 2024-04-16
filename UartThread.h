#ifndef UARTTHREAD_H
#define UARTTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
//#include<QtSerialPort/qserialport.h>
#include "Datastruct.h"
#include "TargetQueue.h"
class UartThread : public QThread {
    Q_OBJECT
public:
    UartThread(QObject* parent = nullptr);
    ~UartThread();

    void run() override;
    void setPortName(QString pname);
public slots:
    void writeFrame(QByteArray frame);
    void updateQueue(int length);
    void updateBaud(int baudrate);
signals:
    void clusterReceived(const  QVector<DataPoint> dataPoints);
    //void targetReceived(const  QVector<DataPoint> dataPoints_);
    void targetReceived(TargetQueue* targetQueue);
    void sendTargetNum(int num);

private:
    QSerialPort* serialPort;
    QByteArray intactFrameBuffer;
    QMutex lock;
    qint16 count_points;
    qint16 count_clusters;
    //std::shared_ptr<TargetQueue> targetQueue;
    TargetQueue * targetQueue;

    void processData(QByteArray& data, QVector<DataPoint>& dataPoints,QByteArray& type_last);
    uint32_t UartThread::xorCheck(QByteArray& data);
private slots:
    void readData();
    void onReadyRead();


};




#endif // UARTTHREAD_H
