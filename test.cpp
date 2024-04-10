#include <Windows.h>
#include <iostream>
#include <qDebug>
#include <cstdint>
#include <thread>
using namespace std;

void ReadSerialData(HANDLE hSerial) {
	char Buffer[256];
	DWORD dwBytesRead;
	int count = 0;
	int frames = 0;
	for (int k = 0; k < 1000; k++) {
		//qDebug() << "KAISHICHULU";

		if (!ReadFile(hSerial, Buffer, sizeof(Buffer), &dwBytesRead, NULL)) {
			cerr << "error of reading from serial Port" << endl;
			qDebug() << "something wrong happended";
		}
		else {
			int temp = 0;//0:01；
			int state = 0;//
			uint16_t lenData = 0;
			char HeadToCheck[7];
			int lenToCheck = 0;
			int lenToDecodeTheTargetNum = 0;
			char lenTarget[4];
			int32_t numTarget = 0;
			int type;
			char aTarget[16];
			int numCords = 0;
			bool headChecked = false;

			for (DWORD i = 0; i < dwBytesRead; ++i) {
				uint8_t a = (Buffer[i] & 0xFF);
				//qDebug() << hex << (Buffer[i] & 0xFF);
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
					HeadToCheck[lenToCheck + 1] = Buffer[i];
					lenToCheck++;
					continue;
				}
				else if (state == 1 && lenToCheck == 6 && Buffer[i] == 0x04) {
					//如果type第一个字节为04，则继续

					HeadToCheck[lenToCheck + 1] = Buffer[i];
					lenToCheck++;
					continue;
				}
				else if (state == 1 && lenToCheck == 7) {
					state = 2; //进入第三阶段
					//计算数据部分字节长度
					lenData = (static_cast<uint16_t>(static_cast<unsigned char>(HeadToCheck[3])) << 8) | static_cast<unsigned char>(HeadToCheck[4]);
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
					//qDebug() << "id::" << state <<"lentocheck"<< lenToCheck;
					continue;
				}

				//----------
				//先取得targetNum
				if (state == 2 && lenToDecodeTheTargetNum < 3) {
					//qDebug() << "the"<< lenToDecodeTheTargetNum <<"buffer[i]" << (Buffer[i] & 0xFF);
					lenTarget[lenToDecodeTheTargetNum] = Buffer[i];
					lenToDecodeTheTargetNum++;
					continue;
				}
				else if (state == 2 && lenToDecodeTheTargetNum == 3) {

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
					continue;
				}

				//----------
				//解析data数据
				if (state == 3 && numCords < 15 && numTarget>0) {
					aTarget[numCords] = Buffer[i];
					numCords++;
					continue;
				}
				else if (state == 3 && numCords == 15) {
					aTarget[numCords] = Buffer[i];
					//收集到完整一个目标的数据，开始解析x y 和index

					float x;
					float y;
					int32_t index;
					memcpy(&x, &aTarget[0], sizeof(x));
					memcpy(&y, &aTarget[4], sizeof(y));
					memcpy(&index, &aTarget[12], sizeof(index));
					qDebug() << "xyindex" << x << y << index << count << frames;
					//重置numCords，并让numTarget减去1
					count++;
					numCords = 0;
					numTarget--;
					continue;
				}
				else if (state == 3 && numTarget == 0) {
					//检验，这里暂不实现

					//全部重置
					state = 0;
					numCords = 0;
					lenData = 0;
					lenToCheck = 0;
					headChecked = false;
					lenToDecodeTheTargetNum = 0;
					HeadToCheck[0] = 0x02;
					frames++;
					continue;

				}








				std::cout << std::hex << static_cast<int>(Buffer[i] & 0xFF) << "";
			}
		}

	}

}

int main_(int argc, char* argv[])
{

	cout << "开始";
	const char* portname = new char[4];
	portname = "COM6";
	HANDLE hSerial = CreateFile(L"COM6", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSerial == INVALID_HANDLE_VALUE) {
		std::cerr << "ERROR opening serial port" << std::endl;
		return 1;
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(DCB);
	GetCommState(hSerial, &dcbSerialParams);

	dcbSerialParams.BaudRate = 1382400;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams))
	{
		cerr << "Error setting serial port state" << endl;
		CloseHandle(hSerial);
		return 1;
	}


	std::thread serialReader(ReadSerialData, hSerial);

	serialReader.join();

	return 0;
}