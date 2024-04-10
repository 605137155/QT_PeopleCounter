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
		
		//读写间隔
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
				memset(&m_osRead, 0, sizeof(OVERLAPPED));//注意每次读取串口时都要初始化OVERLAPPED
				m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				ClearCommError(hSerial, &dwErrorFlags, &ComStat);
				if (!ComStat.cbInQue)
				{
					qDebug() << "缓冲区没有字节";
					continue; //读缓冲区没有字节
				}
				dwBytesRead = (dwBytesRead > (DWORD)ComStat.cbInQue) ? (DWORD)ComStat.cbInQue : dwBytesRead;//防止读取字节数超过数组最大值 
				

				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				if (!ReadFile(hSerial, Buffer, dwBytesRead, &dwBytesRead, &m_osRead)) {
					//cerr << "error of reading from serial Port" << endl;
					qDebug() << "something wrong happended";
					if (GetLastError() == ERROR_IO_PENDING)
					{
						GetOverlappedResult(hSerial, &m_osRead, &dwBytesRead, TRUE); // GetOverlappedResult函数的最后一个参数设为TRUE，函数会一直
						//等待，直到读操作完成或由于错误而返回。
						continue;
					}
				}
				else {
					int temp = 0;//0:01；
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
						//第一阶段
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
							continue; //进入下一阶段
						}
						else if (state == 0 && Buffer[i] != 0x01) {
							//重新开始
							lenToCheck = 0;
							continue;
						}

						//----------
						//第二阶段
						if (state == 1 && lenToCheck == 5 && Buffer[i] == 0x0a) {
							//qDebug() << "second stage_!";
							//如果type第一个字节为0a，则继续
							HeadToCheck[lenToCheck] = Buffer[i];
							lenToCheck++;
							continue;
						}
						else if (state == 1 && lenToCheck == 6 && Buffer[i] == 0x04) {
							//如果type第一个字节为04，则继续

							HeadToCheck[lenToCheck] = Buffer[i];
							lenToCheck++;
							continue;
						}
						else if (state == 1 && lenToCheck == 7) {
							state = 2; //进入第三阶段
							//计算数据部分字节长度
							lenData = (static_cast<uint16_t>(static_cast<unsigned char>(HeadToCheck[3])) << 8) | static_cast<unsigned char>(HeadToCheck[4]);
							
							//校验    静态转换  将一个类型的值强制转换为另一个（底层的二进制不变，如果转换的类型不同，可能会发生截断，如int转short int，而short int 转为int是安全的。 （编译时候检查）   ）
							//动态转换则是 针对指针或者引用，在运行的时候检查对象的实际类型，确保类型转换是安全的，如果失败，则会返回空指针nullptr（对于引用会抛出异常，这个时候时候用try{}catch{const std::bad_cast& e}即可）。
							uint8_t headCksum = static_cast<uint8_t>(Buffer[i]);
							
							for (int k = 0; k < lenToCheck; k++) {
								//acc ^= static_cast<uint8_t>(HeadToCheck[k]);
								acc ^= HeadToCheck[k];
							}
							uint8_t result = (~acc) & ((1 << (1 * 8)) - 1);
							qDebug() << "期望长度：" << headCksum << "vs 实际长度" << result;
							if (headCksum != result)
							{
								qDebug() << "出现不符合的长度！" << "原来的十六进制如下";
								for (int k = 0; k < lenToCheck; k++)
									qDebug() << hex << (HeadToCheck[k] & 0xFF);
							}
							acc = 0;//让acc为0，用于后续数据部分的字段校验
							continue;
						}
						else if (state == 1) {
							//重新来过
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
						//先取得targetNum
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
							//解析len
							memcpy(&numTarget, lenTarget, sizeof(numTarget));
							state = 3; //进入第三阶段
							//qDebug() << "entry the third stage: num_of target:"<< numTarget;
							continue;
						}//每个target有16个字节，所以数据部分的总字节数为16*target+前面的数量的4个字节
						else if (state == 2 && (4 + numTarget * 16) != lenData) {
							//如果数据长度对不上，则重新来过
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
						//解析data数据
						if (state == 3 && numCords < 15 && numTarget>0) {
							acc ^= Buffer[i];
							aTarget[numCords] = Buffer[i];
							numCords++;
							continue;
						}
						else if (state == 3 && numCords == 15) {
							acc ^= Buffer[i];
							aTarget[numCords] = Buffer[i];
							//收集到完整一个目标的数据，开始解析x y 和index

							float x;
							float y;
							int32_t index;
							memcpy(&x, &aTarget[0], sizeof(x));
							memcpy(&y, &aTarget[4], sizeof(y));
							memcpy(&index, &aTarget[12], sizeof(index));
							qDebug() << "xyindex" << x << y << index << count << frames;
							points.push_back({ x, y, index,1 });
							//重置numCords，并让numTarget减去1
							count++;
							numCords = 0;
							numTarget--;
							continue;
						}
						else if (state == 3 && numTarget == 0) {
							//检验，这里暂不实现
							uint8_t dataCksum = static_cast<uint8_t>(Buffer[i]);
							uint8_t result = (~acc) & ((1 << (1 * 8)) - 1);
							qDebug() << "期望长度：" << dataCksum << "vs 实际异或长度" << result;
							if (dataCksum != result) {
								qDebug() << "出现问题，数据部分异或出错";
							}
							//全部重置
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
