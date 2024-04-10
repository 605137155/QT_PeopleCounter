#include "UartThread.h"
#include <QDebug>
#include <QList>
#include <qserialportinfo>
#include <iomanip>
#include <QtEndian>
#include<QDateTime>
#include <QRandomGenerator>
#include <QEventLoop>
#include <QTimer>
#include <unordered_set>

#pragma execution_character_set("utf-8")

UartThread::UartThread(QObject* parent)
    : QThread(parent)
{
    intactFrameBuffer = "";
    // 配置串口参数
    serialPort = new QSerialPort(this);
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
    for (int i = 0; i < list.size(); i++) {
        QString pname = list[i].portName();
        //qDebug() << pname;
        if (pname != "COM1"){
            serialPort->setPortName(pname); // 替换为你的串口名
            break;
        }     
    }
    //serialPort->setBaudRate(115200);
    serialPort->setBaudRate(1382400);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    //serialPort->time
    serialPort->setReadBufferSize(1000);
    connect(serialPort, &QSerialPort::readyRead, this, &UartThread::readData);
    count_points = 0;
    count_clusters = 0;
    targetQueue = new TargetQueue();
    //targetQueue = std::make_shared<TargetQueue>;


}

UartThread::~UartThread()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    delete targetQueue;
}

void UartThread::run()
{
    if (!serialPort->open(QIODevice::ReadWrite)) {
        //emit dataReceived("Failed to open the serial port.");
        return;
    }

    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
    for (int i = 0; i < list.size(); i++) {
        QString pname = list[i].portName();
        //qDebug() << pname;
        if (pname != "COM1") {
            serialPort->setPortName(pname); // 替换为你的串口名
            break;
        }
    }

    while (!isInterruptionRequested()) {
        if (serialPort->waitForReadyRead(100)) { // 等待数据可用，超时时间可根据需要调整
            //QByteArray data = serialPort->readAll();
            //qDebug() << "first time data:"<<data;
            //emit dataReceived(data);
            //QThread::msleep(1000);
            //onReadyRead();

            QDateTime currenttime = QDateTime::currentDateTime();
            QString string_time = currenttime.toString("HH:mm:ss.z");

            qDebug() << "caijishujuzhong"<< string_time;
        }
    }
    serialPort->close();

}


void UartThread::setPortName(QString pname) {
    serialPort->setPortName(pname);
}

void UartThread::onReadyRead() {

    //QTimer::singleShot(10, this, &UartThread::readData);
}

void UartThread::updateQueue(int length){
    targetQueue->updateQueueLength(length);
}


void UartThread::readData() {
    //QThread::msleep(100);
    QByteArray data = serialPort->readAll();
    //qDebug() << "data _ byte:" << data;
    QByteArray hexdata = data.toHex().toUpper();
    //qDebug() << "本次读取的hexData:" << hexdata;
    //qDebug() << "本次读取的hexData长度:" << hexdata.size()/2<<"字节";
    QByteArray frameHead = hexdata.mid(0, 2);
    //qDebug() << "本次帧头frameHead:" << frameHead;
    QByteArray type = hexdata.mid(10, 4);
    //qDebug() << "本次类型type:" << type;
    if (frameHead=="01" && type == "0A08" || type == "0A04" || type == "0A0A") {
        //点云
        QByteArray type_last;
        QVector<DataPoint> dataPoints;
        //如果该批次数据还有帧数据在后面，则找出它并打印它
        while(intactFrameBuffer!="") {
            dataPoints.clear();
            QVector<DataPoint>().swap(dataPoints);
            processData(intactFrameBuffer, dataPoints, type_last);
            //qDebug() << "下次循环要处理的数据:"<< intactFrameBuffer;
            //qDebug() << "type of this frame after analysis："<<type_last;   
        }    
        intactFrameBuffer = hexdata;
    }
    else if(type!="") {
        intactFrameBuffer += hexdata;
    }
}



void UartThread::processData(QByteArray& data, QVector<DataPoint>& dataPoints, QByteArray& type_last) {
    //将该帧的类型赋给type_last
    qDebug() << "本次循环内部要处理的帧" << data;
    type_last = data.mid(10, 4);

    //字节数量dataLength，对应的16进制数的个数为dataLength*2
    quint16 dataLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(QByteArray::fromHex(data.mid(6, 4)).constData()));
    //qDebug() << "循环内部数据长度datalength:" << dataLength << "   num_x16 :" << 2 * dataLength << "point_length_x16:" << 2 * dataLength - 8 << "num_points:" << (2 * dataLength - 8) / 32;

    if (type_last == "0A0A") {
        //qDebug() << "0A0A的帧";
        data = data.mid(2 * (dataLength + 9));
        return;
    }

    ////帧头校验
    QByteArray Head_cksum = data.mid(14, 2);
    //qDebug() << "循环内部头部检测位:" << Head_cksum;
    uint8_t headCksum = qFromBigEndian<uint8_t>(reinterpret_cast<const uchar*>(QByteArray::fromHex(Head_cksum).constData()));
    QByteArray&& headToCheck = QByteArray::fromHex(data.mid(0, 14));
    uint8_t actualSum = xorCheck(headToCheck);
    //qDebug() << "期望sum:" << headCksum << "实际sum" << actualSum;
    if (headCksum != actualSum) {
        qDebug() << "帧头校验发生错误，舍弃该帧";
        data = data.mid(2 * (dataLength + 9));
        return;
    }

    //取出点数量
    qint32 target_num = qFromLittleEndian<qint32>(reinterpret_cast<const qint32*>(QByteArray::fromHex(data.mid(16, 8)).constData()));

    //qDebug() << "循环内部目标个数target_num:" << target_num;
    QByteArray data_data = data.mid(24, 2 * dataLength - 8);
    //qDebug() << "循环内部的数据部分data：" << data_data;
    int type_point = 0;
    if (type_last == "0A04") {
        type_point = 1;
    }
    else if (type_last == "0A08")
        type_point = 0;
    else {

    }



    //数据校验
    QByteArray data_cksum = data.mid(16, 2 * dataLength);
    //qDebug() << "循环内部的数据帧:" << data_cksum;
    QByteArray&& dataToCheck = QByteArray::fromHex(data_cksum);
    uint8_t dataCksum = qFromBigEndian<uint8_t>(reinterpret_cast<const uchar*>(QByteArray::fromHex(data.mid(24 + 2 * dataLength - 8, 8)).constData()));
    actualSum = xorCheck(dataToCheck);
    //qDebug() << "循环内部数据检测帧:" << data.mid(24 + 2 * dataLength - 8, 8);
    //qDebug() << "期望sum:" << dataCksum << "实际sum" << actualSum;
    if (dataCksum != actualSum) {
        qDebug() << "数据帧校验发生错误，舍弃该帧";
        data = data.mid(2 * (dataLength + 9));
        return;
    }

    if (!type_point) //点
        for (int i = 0; i < target_num; i++) {
            float x_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(i * 32, 8)).constData())); //
            float y_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(8 + i * 32, 8)).constData())); //
            qint32 idx = qFromLittleEndian<qint32>(reinterpret_cast<const qint32*>(QByteArray::fromHex(data_data.mid(24 + i * 32, 8)).constData())); //
            //qDebug() << "the" << i << "th one:" << "x:" << x_point << ", original:" << data_data.mid(i * 32, 8) << " y:" << y_point << ", original:" << data_data.mid(8 + i * 32, 8) << " id:" << idx << "original" << data_data.mid(24 + i * 32, 8);
            count_points++;
            //qDebug() << "count_points" << count_points << "count_frames" << count_clusters;
            //点和target不同的操作
            dataPoints.push_back({ -x_point, y_point, idx, type_point }); //为看起来是镜像，横坐标取反
            //如果发现有奇怪的帧，则放弃该帧，并令data为空""，避免陷入死循环
            if (data_data.mid(i * 32, 8) == "") {
                data = "";
                qDebug() << "遇到了奇怪的有空“”的循环";
                return;
            }
        }
    else//目标
    {
        std::unordered_set<int> visited;
        for (int i = 0; i < target_num; i++) {
            float x_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(i * 32, 8)).constData())); //
            float y_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(8 + i * 32, 8)).constData())); //
            qint32 idx = qFromLittleEndian<qint32>(reinterpret_cast<const qint32*>(QByteArray::fromHex(data_data.mid(24 + i * 32, 8)).constData())); //
            qDebug() << "the" << i << "th one:" << "x:" << x_point << ", original:" << data_data.mid(i * 32, 8) << " y:" << y_point << ", original:" << data_data.mid(8 + i * 32, 8) << " id:" << idx << "original" << data_data.mid(24 + i * 32, 8);
            count_points++;
            qDebug() << "count_points" << count_points << "count_frames" << count_clusters;
            //点和target不同的操作
            visited.insert(idx);
            targetQueue->addDataPoint({ -x_point, y_point, idx, type_point }); //为看起来是镜像，横坐标取反
            targetQueue->updateEnqueueTimes();
            //如果发现有奇怪的帧，则放弃该帧，并令data为空""，避免陷入死循环
            if (data_data.mid(i * 32, 8) == "") {
                data = "";
                qDebug() << "遇到了奇怪的有空“”的循环";
                return;
            }
        }


        //判断有哪些类型没有采集到，将这些类型加添加空点，避免该类型没有发生改变
        for (int i = 0; i < 6; i++) {
            if (visited.find(i) == visited.end()) {
                //没有添加过，则添加一个空的元素到该队列中，注意，此时与target_num=0的时候的添加的代码不要有冲突，需要与Targetqueue.h对应好，这里的point.type是有的，只是pointOrTarget=-1
                targetQueue->addDataPoint({ 0, 0, i, -1 });
                qDebug() << "添加了空点"<<i;
            }
        }
        
    }


    if (target_num > 0)
    {
        count_clusters++;
        if (type_point == 0)//点
        {
            //count_points++;
            QVector<DataPoint> firstPoints = dataPoints;
            emit clusterReceived(firstPoints);


            qDebug() << "count_points:" << count_points << "count_points_size:" << firstPoints.size() <<"点云被发送到第一个窗口";
        }
        else//目标
        {
            qDebug() << "0A04的帧";
            qDebug() << "运行到里了";
            //if (targetQueue->getEnqueueTimes() >= 3)
            {
                qDebug() << "运行到里了2";
                emit targetReceived(targetQueue);
                emit sendTargetNum(target_num);
            }
            qDebug() << "count_clusters:" << count_clusters << "count_clusters_size:" << targetQueue->getTargetNum() << "目标被发送到两个窗口";
        }
    }
    else {
        qDebug() << "空帧z";
        QVector<DataPoint> nullPoints = QVector<DataPoint>();

        if (type_point==0) {
            count_points++;
            emit clusterReceived(nullPoints);
        }
        else
        {
            qDebug() << "0A04的空帧z被发送了";
            count_clusters++;
            //这里的pointOrTarget=-1，且type也为-1，此时为所有的队列都添加空元素
            //targetQueue->addDataPoint({0.0, 0.0, -1 ,-1,});
            targetQueue->updateEnqueueTimes();
            //if (targetQueue->getEnqueueTimes() >= 3)
            emit targetReceived(targetQueue);
            emit sendTargetNum(target_num);
            qDebug() << "空点被发送到窗口";
        }
        
    }
    
    //把剩下的帧数据赋给原来的data变量，用于循环输出采集到的这一批数据
    //如果data的字节数+其他的字节数 < data的字节数，则将剩下的赋给data，用于后续处理
    //qDebug() <<"循环内部理想的数据长度datasupposed_size:"<< 2 * (dataLength + 9)<<"vs 循环内部实际数据长度actuall:"<< data.size();
    //if (2 * (dataLength + 9) < data.size())
    data = data.mid(2 * (dataLength + 9));
    //qDebug() << "本次循环内部剩余数据lastdata" << data;
}


uint32_t UartThread::xorCheck(QByteArray& data){
    uint32_t acc = 0;
    for (int i = 0;i<data.size();++i) {
        acc ^= static_cast<uint8_t>(data[i]);
    }
    return (~acc) & ((1 << (1 * 8)) - 1);
}

void UartThread::writeFrame(QByteArray frame){

    qDebug() << "被触发" << frame;
    if (serialPort->isOpen()) {
        serialPort->write(frame);
    }
}

void UartThread::updateBaud(int baud) {
    qDebug() << "波特率:"<<baud ;
    serialPort->setBaudRate(baud);
}