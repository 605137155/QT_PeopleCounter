#pragma once
#ifndef PURECPPTHREAD_H
#define PURECPPTHREAD_H
#include <iostream>
#include <QThread>
#include "Datastruct.h"
#include <Windows.h>
#include <qDebug>
#include <cstdint>
#include <QVector>
#include <algorithm>
#include <thread>
#include <cstdint>
#include <atomic>
using namespace std;
class PureCppThread : public QThread {
    Q_OBJECT
public:
    PureCppThread(QObject* parent = nullptr) {
        const char* portname = new char[4];
        portname = "COM6";
        hSerial = CreateFile(L"COM6", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (hSerial == INVALID_HANDLE_VALUE) {
            std::cerr << "ERROR opening serial port" << std::endl;
        }
        dcbSerialParams = { 0 };
        dcbSerialParams.DCBlength = sizeof(DCB);
        GetCommState(hSerial, &dcbSerialParams);
        dcbSerialParams.BaudRate = 1382400;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
		SetupComm(hSerial, 1024, 1024);
		
		//��д���
		COMMTIMEOUTS TimeOuts;
		TimeOuts.ReadIntervalTimeout = 1000;
		TimeOuts.ReadTotalTimeoutMultiplier = 500;
		TimeOuts.ReadTotalTimeoutConstant = 5000;
		TimeOuts.WriteTotalTimeoutMultiplier = 500;
		TimeOuts.WriteTotalTimeoutConstant = 2000;
		SetCommTimeouts(hSerial, &TimeOuts);
		PurgeComm(hSerial, PURGE_TXCLEAR|PURGE_RXCLEAR);
		

    }
	~PureCppThread() {
		HANDLE hCom = *(HANDLE*)hSerial;
		CloseHandle(hCom);
	}


	void ReadSerialData(HANDLE hSerial) {
		
	}


	void run() override {
		if (!SetCommState(hSerial, &dcbSerialParams))
		{
			//cerr << "Error setting serial port state" << endl;
			CloseHandle(hSerial);
			//return 1;
		}

		char Buffer[256];
		DWORD dwBytesRead = 256;
		DWORD dwErrorFlags;
		COMSTAT ComStat;
		int count = 0;
		int frames = 0;
		QVector<DataPoint> points;
		OVERLAPPED m_osRead;
		memset(&m_osRead, 0, sizeof(OVERLAPPED));
		m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		ClearCommError(hSerial, &dwErrorFlags, &ComStat);
		while(!isInterruptionRequested()) {
			//qDebug() << "KAISHICHULU";
			
			if(!isInterruptionRequested())
			{
				memset(&m_osRead, 0, sizeof(OVERLAPPED));//ע��ÿ�ζ�ȡ����ʱ��Ҫ��ʼ��OVERLAPPED
				m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				ClearCommError(hSerial, &dwErrorFlags, &ComStat);
				if (!ComStat.cbInQue)
				{
					qDebug() << "������û���ֽ�";
					continue; //��������û���ֽ�
				}
				dwBytesRead = (dwBytesRead > (DWORD)ComStat.cbInQue) ? (DWORD)ComStat.cbInQue : dwBytesRead;//��ֹ��ȡ�ֽ��������������ֵ 
				

				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				if (!ReadFile(hSerial, Buffer, dwBytesRead, &dwBytesRead, &m_osRead)) {
					//cerr << "error of reading from serial Port" << endl;
					qDebug() << "something wrong happended";
					if (GetLastError() == ERROR_IO_PENDING)
					{
						GetOverlappedResult(hSerial, &m_osRead, &dwBytesRead, TRUE); // GetOverlappedResult���������һ��������ΪTRUE��������һֱ
						//�ȴ���ֱ����������ɻ����ڴ�������ء�
						continue;
					}
				}
				else {
					int temp = 0;//0:01��
					int state = 0;//
					uint16_t lenData = 0;
					char HeadToCheck[7]{};
					int lenToCheck = 0;
					int lenToDecodeTheTargetNum = 0;
					char lenTarget[4];
					int32_t numTarget = 0;
					int type;
					char aTarget[16];
					int numCords = 0;
					bool headChecked = false;
					uint8_t acc = 0;
					for (DWORD i = 0; i < dwBytesRead; ++i) {
						uint8_t a = (Buffer[i] & 0xFF);
						qDebug() << hex << (Buffer[i] & 0xFF);
						//qDebug() << "raw" << Buffer[i];
						//��һ�׶�
						/*if (Buffer[i] == 0x10) {
							qDebug() << "there is 10...";
						}*/
						//qDebug() << "id:" << state<< hex << (Buffer[i] & 0xFF)<< "lenticheck"<< lenToCheck;
						if (state == 0 && Buffer[i] == 0x01 && lenToCheck == 0)
						{
							//qDebug() << "there is 01, go to check the type ,if it is 0a08, continue, else restar";
							HeadToCheck[lenToCheck] = Buffer[i];
							lenToCheck++;
							continue;
						}
						else if (state == 0 && HeadToCheck[0] == 0x01 && lenToCheck < 4) {
							HeadToCheck[lenToCheck] = Buffer[i];
							lenToCheck++;
							continue;
						}
						else if (state == 0 && lenToCheck == 4 && HeadToCheck[0] == 0x01) {
							HeadToCheck[lenToCheck] = Buffer[i];
							lenToCheck++;
							state = 1;
							continue; //������һ�׶�
						}
						else if (state == 0 && Buffer[i] != 0x01) {
							//���¿�ʼ
							lenToCheck = 0;
							continue;
						}

						//----------
						//�ڶ��׶�
						if (state == 1 && lenToCheck == 5 && Buffer[i] == 0x0a) {
							//qDebug() << "second stage_!";
							//���type��һ���ֽ�Ϊ0a�������
							HeadToCheck[lenToCheck] = Buffer[i];
							lenToCheck++;
							continue;
						}
						else if (state == 1 && lenToCheck == 6 && Buffer[i] == 0x04) {
							//���type��һ���ֽ�Ϊ04�������

							HeadToCheck[lenToCheck] = Buffer[i];
							lenToCheck++;
							continue;
						}
						else if (state == 1 && lenToCheck == 7) {
							state = 2; //��������׶�
							//�������ݲ����ֽڳ���
							lenData = (static_cast<uint16_t>(static_cast<unsigned char>(HeadToCheck[3])) << 8) | static_cast<unsigned char>(HeadToCheck[4]);
							
							//У��    ��̬ת��  ��һ�����͵�ֵǿ��ת��Ϊ��һ�����ײ�Ķ����Ʋ��䣬���ת�������Ͳ�ͬ�����ܻᷢ���ضϣ���intתshort int����short int תΪint�ǰ�ȫ�ġ� ������ʱ���飩   ��
							//��̬ת������ ���ָ��������ã������е�ʱ��������ʵ�����ͣ�ȷ������ת���ǰ�ȫ�ģ����ʧ�ܣ���᷵�ؿ�ָ��nullptr���������û��׳��쳣�����ʱ��ʱ����try{}catch{const std::bad_cast& e}���ɣ���
							uint8_t headCksum = static_cast<uint8_t>(Buffer[i]);
							
							for (int k = 0; k < lenToCheck; k++) {
								//acc ^= static_cast<uint8_t>(HeadToCheck[k]);
								acc ^= HeadToCheck[k];
							}
							uint8_t result = (~acc) & ((1 << (1 * 8)) - 1);
							qDebug() << "�������ȣ�" << headCksum << "vs ʵ�ʳ���" << result;
							if (headCksum != result)
							{
								qDebug() << "���ֲ����ϵĳ��ȣ�" << "ԭ����ʮ����������";
								for (int k = 0; k < lenToCheck; k++)
									qDebug() << hex << (HeadToCheck[k] & 0xFF);
							}
							acc = 0;//��accΪ0�����ں������ݲ��ֵ��ֶ�У��
							continue;
						}
						else if (state == 1) {
							//��������
							//qDebug() << "chongxinlaiguo";
							state = 0 + 0;//
							lenData = 0;
							lenToCheck = 0;
							headChecked = false;
							state = 0;
							HeadToCheck[0] = 0x02;
							acc = 0;
							//qDebug() << "id::" << state <<"lentocheck"<< lenToCheck;
							continue;
						}

						//----------
						//��ȡ��targetNum
						if (state == 2 && lenToDecodeTheTargetNum < 3) {
							//qDebug() << "the"<< lenToDecodeTheTargetNum <<"buffer[i]" << (Buffer[i] & 0xFF);
							acc ^= Buffer[i];
							lenTarget[lenToDecodeTheTargetNum] = Buffer[i];
							lenToDecodeTheTargetNum++;
							continue;
						}
						else if (state == 2 && lenToDecodeTheTargetNum == 3) {
							acc ^= Buffer[i];
							lenTarget[lenToDecodeTheTargetNum] = Buffer[i];
							//����len
							memcpy(&numTarget, lenTarget, sizeof(numTarget));
							state = 3; //��������׶�
							//qDebug() << "entry the third stage: num_of target:"<< numTarget;
							continue;
						}//ÿ��target��16���ֽڣ��������ݲ��ֵ����ֽ���Ϊ16*target+ǰ���������4���ֽ�
						else if (state == 2 && (4 + numTarget * 16) != lenData) {
							//������ݳ��ȶԲ��ϣ�����������
							//qDebug() << "shujuchangduduibushang chognixnlaiguo ";
							state = 0;
							numTarget = 0;
							lenData = 0;
							lenToCheck = 0;
							headChecked = false;
							lenToDecodeTheTargetNum = 0;
							HeadToCheck[0] = 0x02;
							acc = 0;
							continue;
						}

						//----------
						//����data����
						if (state == 3 && numCords < 15 && numTarget>0) {
							acc ^= Buffer[i];
							aTarget[numCords] = Buffer[i];
							numCords++;
							continue;
						}
						else if (state == 3 && numCords == 15) {
							acc ^= Buffer[i];
							aTarget[numCords] = Buffer[i];
							//�ռ�������һ��Ŀ������ݣ���ʼ����x y ��index

							float x;
							float y;
							int32_t index;
							memcpy(&x, &aTarget[0], sizeof(x));
							memcpy(&y, &aTarget[4], sizeof(y));
							memcpy(&index, &aTarget[12], sizeof(index));
							qDebug() << "xyindex" << x << y << index << count << frames;
							points.push_back({ x, y, index,1 });
							//����numCords������numTarget��ȥ1
							count++;
							numCords = 0;
							numTarget--;
							continue;
						}
						else if (state == 3 && numTarget == 0) {
							//���飬�����ݲ�ʵ��
							uint8_t dataCksum = static_cast<uint8_t>(Buffer[i]);
							uint8_t result = (~acc) & ((1 << (1 * 8)) - 1);
							qDebug() << "�������ȣ�" << dataCksum << "vs ʵ����򳤶�" << result;
							if (dataCksum != result) {
								qDebug() << "�������⣬���ݲ���������";
							}
							//ȫ������
							state = 0;
							numCords = 0;
							lenData = 0;
							lenToCheck = 0;
							headChecked = false;
							lenToDecodeTheTargetNum = 0;
							HeadToCheck[0] = 0x02;
							frames++;
							emit targetReceived(points);
							points.clear();
							acc = 0;
							continue;

						}








						std::cout << std::hex << static_cast<int>(Buffer[i] & 0xFF) << "";
					}
				}
			}

		}


	}

	void threadStop(){

	}



signals:
    //void clusterReceived(const  QVector<DataPoint> dataPoints);
    void targetReceived(const  QVector<DataPoint> dataPoints_);

private:
    DCB dcbSerialParams;
	HANDLE hSerial;
	
	QVector<DataPoint> points;
};




#endif // PURECPPTHREAD_H
