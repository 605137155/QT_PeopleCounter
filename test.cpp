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
			int temp = 0;//0:01��
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
					HeadToCheck[lenToCheck + 1] = Buffer[i];
					lenToCheck++;
					continue;
				}
				else if (state == 1 && lenToCheck == 6 && Buffer[i] == 0x04) {
					//���type��һ���ֽ�Ϊ04�������

					HeadToCheck[lenToCheck + 1] = Buffer[i];
					lenToCheck++;
					continue;
				}
				else if (state == 1 && lenToCheck == 7) {
					state = 2; //��������׶�
					//�������ݲ����ֽڳ���
					lenData = (static_cast<uint16_t>(static_cast<unsigned char>(HeadToCheck[3])) << 8) | static_cast<unsigned char>(HeadToCheck[4]);
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
					//qDebug() << "id::" << state <<"lentocheck"<< lenToCheck;
					continue;
				}

				//----------
				//��ȡ��targetNum
				if (state == 2 && lenToDecodeTheTargetNum < 3) {
					//qDebug() << "the"<< lenToDecodeTheTargetNum <<"buffer[i]" << (Buffer[i] & 0xFF);
					lenTarget[lenToDecodeTheTargetNum] = Buffer[i];
					lenToDecodeTheTargetNum++;
					continue;
				}
				else if (state == 2 && lenToDecodeTheTargetNum == 3) {

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
					continue;
				}

				//----------
				//����data����
				if (state == 3 && numCords < 15 && numTarget>0) {
					aTarget[numCords] = Buffer[i];
					numCords++;
					continue;
				}
				else if (state == 3 && numCords == 15) {
					aTarget[numCords] = Buffer[i];
					//�ռ�������һ��Ŀ������ݣ���ʼ����x y ��index

					float x;
					float y;
					int32_t index;
					memcpy(&x, &aTarget[0], sizeof(x));
					memcpy(&y, &aTarget[4], sizeof(y));
					memcpy(&index, &aTarget[12], sizeof(index));
					qDebug() << "xyindex" << x << y << index << count << frames;
					//����numCords������numTarget��ȥ1
					count++;
					numCords = 0;
					numTarget--;
					continue;
				}
				else if (state == 3 && numTarget == 0) {
					//���飬�����ݲ�ʵ��

					//ȫ������
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

	cout << "��ʼ";
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