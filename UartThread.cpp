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
    // ���ô��ڲ���
    serialPort = new QSerialPort(this);
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
    for (int i = 0; i < list.size(); i++) {
        QString pname = list[i].portName();
        //qDebug() << pname;
        if (pname != "COM1"){
            serialPort->setPortName(pname); // �滻Ϊ��Ĵ�����
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
            serialPort->setPortName(pname); // �滻Ϊ��Ĵ�����
            break;
        }
    }

    while (!isInterruptionRequested()) {
        if (serialPort->waitForReadyRead(100)) { // �ȴ����ݿ��ã���ʱʱ��ɸ�����Ҫ����
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
    //qDebug() << "���ζ�ȡ��hexData:" << hexdata;
    //qDebug() << "���ζ�ȡ��hexData����:" << hexdata.size()/2<<"�ֽ�";
    QByteArray frameHead = hexdata.mid(0, 2);
    //qDebug() << "����֡ͷframeHead:" << frameHead;
    QByteArray type = hexdata.mid(10, 4);
    //qDebug() << "��������type:" << type;
    if (frameHead=="01" && type == "0A08" || type == "0A04" || type == "0A0A") {
        //����
        QByteArray type_last;
        QVector<DataPoint> dataPoints;
        //������������ݻ���֡�����ں��棬���ҳ�������ӡ��
        while(intactFrameBuffer!="") {
            dataPoints.clear();
            QVector<DataPoint>().swap(dataPoints);
            processData(intactFrameBuffer, dataPoints, type_last);
            //qDebug() << "�´�ѭ��Ҫ���������:"<< intactFrameBuffer;
            //qDebug() << "type of this frame after analysis��"<<type_last;   
        }    
        intactFrameBuffer = hexdata;
    }
    else if(type!="") {
        intactFrameBuffer += hexdata;
    }
}



void UartThread::processData(QByteArray& data, QVector<DataPoint>& dataPoints, QByteArray& type_last) {
    //����֡�����͸���type_last
    qDebug() << "����ѭ���ڲ�Ҫ�����֡" << data;
    type_last = data.mid(10, 4);

    //�ֽ�����dataLength����Ӧ��16�������ĸ���ΪdataLength*2
    quint16 dataLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(QByteArray::fromHex(data.mid(6, 4)).constData()));
    //qDebug() << "ѭ���ڲ����ݳ���datalength:" << dataLength << "   num_x16 :" << 2 * dataLength << "point_length_x16:" << 2 * dataLength - 8 << "num_points:" << (2 * dataLength - 8) / 32;

    if (type_last == "0A0A") {
        //qDebug() << "0A0A��֡";
        data = data.mid(2 * (dataLength + 9));
        return;
    }

    ////֡ͷУ��
    QByteArray Head_cksum = data.mid(14, 2);
    //qDebug() << "ѭ���ڲ�ͷ�����λ:" << Head_cksum;
    uint8_t headCksum = qFromBigEndian<uint8_t>(reinterpret_cast<const uchar*>(QByteArray::fromHex(Head_cksum).constData()));
    QByteArray&& headToCheck = QByteArray::fromHex(data.mid(0, 14));
    uint8_t actualSum = xorCheck(headToCheck);
    //qDebug() << "����sum:" << headCksum << "ʵ��sum" << actualSum;
    if (headCksum != actualSum) {
        qDebug() << "֡ͷУ�鷢������������֡";
        data = data.mid(2 * (dataLength + 9));
        return;
    }

    //ȡ��������
    qint32 target_num = qFromLittleEndian<qint32>(reinterpret_cast<const qint32*>(QByteArray::fromHex(data.mid(16, 8)).constData()));

    //qDebug() << "ѭ���ڲ�Ŀ�����target_num:" << target_num;
    QByteArray data_data = data.mid(24, 2 * dataLength - 8);
    //qDebug() << "ѭ���ڲ������ݲ���data��" << data_data;
    int type_point = 0;
    if (type_last == "0A04") {
        type_point = 1;
    }
    else if (type_last == "0A08")
        type_point = 0;
    else {

    }



    //����У��
    QByteArray data_cksum = data.mid(16, 2 * dataLength);
    //qDebug() << "ѭ���ڲ�������֡:" << data_cksum;
    QByteArray&& dataToCheck = QByteArray::fromHex(data_cksum);
    uint8_t dataCksum = qFromBigEndian<uint8_t>(reinterpret_cast<const uchar*>(QByteArray::fromHex(data.mid(24 + 2 * dataLength - 8, 8)).constData()));
    actualSum = xorCheck(dataToCheck);
    //qDebug() << "ѭ���ڲ����ݼ��֡:" << data.mid(24 + 2 * dataLength - 8, 8);
    //qDebug() << "����sum:" << dataCksum << "ʵ��sum" << actualSum;
    if (dataCksum != actualSum) {
        qDebug() << "����֡У�鷢������������֡";
        data = data.mid(2 * (dataLength + 9));
        return;
    }

    if (!type_point) //��
        for (int i = 0; i < target_num; i++) {
            float x_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(i * 32, 8)).constData())); //
            float y_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(8 + i * 32, 8)).constData())); //
            qint32 idx = qFromLittleEndian<qint32>(reinterpret_cast<const qint32*>(QByteArray::fromHex(data_data.mid(24 + i * 32, 8)).constData())); //
            //qDebug() << "the" << i << "th one:" << "x:" << x_point << ", original:" << data_data.mid(i * 32, 8) << " y:" << y_point << ", original:" << data_data.mid(8 + i * 32, 8) << " id:" << idx << "original" << data_data.mid(24 + i * 32, 8);
            count_points++;
            //qDebug() << "count_points" << count_points << "count_frames" << count_clusters;
            //���target��ͬ�Ĳ���
            dataPoints.push_back({ -x_point, y_point, idx, type_point }); //Ϊ�������Ǿ��񣬺�����ȡ��
            //�����������ֵ�֡���������֡������dataΪ��""������������ѭ��
            if (data_data.mid(i * 32, 8) == "") {
                data = "";
                qDebug() << "��������ֵ��пա�����ѭ��";
                return;
            }
        }
    else//Ŀ��
    {
        std::unordered_set<int> visited;
        for (int i = 0; i < target_num; i++) {
            float x_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(i * 32, 8)).constData())); //
            float y_point = qFromLittleEndian<float>(reinterpret_cast<const char*>(QByteArray::fromHex(data_data.mid(8 + i * 32, 8)).constData())); //
            qint32 idx = qFromLittleEndian<qint32>(reinterpret_cast<const qint32*>(QByteArray::fromHex(data_data.mid(24 + i * 32, 8)).constData())); //
            qDebug() << "the" << i << "th one:" << "x:" << x_point << ", original:" << data_data.mid(i * 32, 8) << " y:" << y_point << ", original:" << data_data.mid(8 + i * 32, 8) << " id:" << idx << "original" << data_data.mid(24 + i * 32, 8);
            count_points++;
            qDebug() << "count_points" << count_points << "count_frames" << count_clusters;
            //���target��ͬ�Ĳ���
            visited.insert(idx);
            targetQueue->addDataPoint({ -x_point, y_point, idx, type_point }); //Ϊ�������Ǿ��񣬺�����ȡ��
            targetQueue->updateEnqueueTimes();
            //�����������ֵ�֡���������֡������dataΪ��""������������ѭ��
            if (data_data.mid(i * 32, 8) == "") {
                data = "";
                qDebug() << "��������ֵ��пա�����ѭ��";
                return;
            }
        }


        //�ж�����Щ����û�вɼ���������Щ���ͼ���ӿյ㣬���������û�з����ı�
        for (int i = 0; i < 6; i++) {
            if (visited.find(i) == visited.end()) {
                //û����ӹ��������һ���յ�Ԫ�ص��ö����У�ע�⣬��ʱ��target_num=0��ʱ�����ӵĴ��벻Ҫ�г�ͻ����Ҫ��Targetqueue.h��Ӧ�ã������point.type���еģ�ֻ��pointOrTarget=-1
                targetQueue->addDataPoint({ 0, 0, i, -1 });
                qDebug() << "����˿յ�"<<i;
            }
        }
        
    }


    if (target_num > 0)
    {
        count_clusters++;
        if (type_point == 0)//��
        {
            //count_points++;
            QVector<DataPoint> firstPoints = dataPoints;
            emit clusterReceived(firstPoints);


            qDebug() << "count_points:" << count_points << "count_points_size:" << firstPoints.size() <<"���Ʊ����͵���һ������";
        }
        else//Ŀ��
        {
            qDebug() << "0A04��֡";
            qDebug() << "���е�����";
            //if (targetQueue->getEnqueueTimes() >= 3)
            {
                qDebug() << "���е�����2";
                emit targetReceived(targetQueue);
                emit sendTargetNum(target_num);
            }
            qDebug() << "count_clusters:" << count_clusters << "count_clusters_size:" << targetQueue->getTargetNum() << "Ŀ�걻���͵���������";
        }
    }
    else {
        qDebug() << "��֡z";
        QVector<DataPoint> nullPoints = QVector<DataPoint>();

        if (type_point==0) {
            count_points++;
            emit clusterReceived(nullPoints);
        }
        else
        {
            qDebug() << "0A04�Ŀ�֡z��������";
            count_clusters++;
            //�����pointOrTarget=-1����typeҲΪ-1����ʱΪ���еĶ��ж���ӿ�Ԫ��
            //targetQueue->addDataPoint({0.0, 0.0, -1 ,-1,});
            targetQueue->updateEnqueueTimes();
            //if (targetQueue->getEnqueueTimes() >= 3)
            emit targetReceived(targetQueue);
            emit sendTargetNum(target_num);
            qDebug() << "�յ㱻���͵�����";
        }
        
    }
    
    //��ʣ�µ�֡���ݸ���ԭ����data����������ѭ������ɼ�������һ������
    //���data���ֽ���+�������ֽ��� < data���ֽ�������ʣ�µĸ���data�����ں�������
    //qDebug() <<"ѭ���ڲ���������ݳ���datasupposed_size:"<< 2 * (dataLength + 9)<<"vs ѭ���ڲ�ʵ�����ݳ���actuall:"<< data.size();
    //if (2 * (dataLength + 9) < data.size())
    data = data.mid(2 * (dataLength + 9));
    //qDebug() << "����ѭ���ڲ�ʣ������lastdata" << data;
}


uint32_t UartThread::xorCheck(QByteArray& data){
    uint32_t acc = 0;
    for (int i = 0;i<data.size();++i) {
        acc ^= static_cast<uint8_t>(data[i]);
    }
    return (~acc) & ((1 << (1 * 8)) - 1);
}

void UartThread::writeFrame(QByteArray frame){

    qDebug() << "������" << frame;
    if (serialPort->isOpen()) {
        serialPort->write(frame);
    }
}

void UartThread::updateBaud(int baud) {
    qDebug() << "������:"<<baud ;
    serialPort->setBaudRate(baud);
}