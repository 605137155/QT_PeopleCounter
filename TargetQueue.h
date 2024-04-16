#pragma once
#include <QQueue>
#include <QDebug>
#include <QVector>
#include <QMutex>
#include "DataStruct.h"
#pragma execution_character_set("utf-8")
class TargetQueue
{

private:
	QVector<QQueue<DataPoint>> queues;
	int numTypes;
	long int enqueueTimes;
	QMutex lock;
	int numPoints; //��ʾ���ڵĵ�������������������Ϲ���ĵ㣩
	int queueLength;
public: 
	TargetQueue(int n=6):numTypes(n),enqueueTimes(0),numPoints(0), queueLength(13){
		queues.resize(numTypes);
		for (auto& queue : queues) {
			for (int i = 0; i < queueLength; i++)
				queue.append({ 0,0,0,-1 });
		}
	};

	~TargetQueue() {
		/*for (int i = 0; i < queues.size(); ++i) {
			QQueue<DataPoint>& queue = queues[i];
			while (!queue.isEmpty()) {
				DataPoint data = queue.dequeue();
			}
		}
		queues.clear();*/
	}

	void addDataPoint(const DataPoint point) {
		lock.lock();
		if (point.category >= 0 && point.category < numTypes) {//����Ϊ�пյ��ʵ�㣬�յ�Ϊֻ��һ���ֵĵ���ڵ�ʱ��Ϊ����û�вɼ����ĵ����Ŀյ�
			//qDebug() << "�ӵĳ���::"<< queues[point.category].size();
			DataPoint p = queues[point.category].dequeue();
			if (p.category != -1 && p.pointOrTarget != -1)
				numPoints--;
			queues[point.category].enqueue(point);
			if (point.category != -1) {
				numPoints++;
			}
			else if (point.category == -1) //���=0��ʱ�򣬴�ʱ�״��̲߳ɼ��ĵ���Ϊ0��Ϊ���ж�����ӿ�Ԫ��
			{
				qDebug() << "���ֵ�7�ּ����ϵĵ�,or category=-1�Ŀյ㣬��ʱ�յ��Ѿ�ͨ��������ʱ������ˣ�Ȼ�����ں����ؼ��е���ɫ����";
				/*for (auto& queue : queues) {
					queue.dequeue();
					queue.enqueue(point);
				}*/
			}
		}
		lock.unlock();
	}


	/*void dequeueAll() {
		for (auto& queue : queues) {
			if (!queue.isEmpty()) {
				DataPoint point = queue.dequeue();
				if (point.category!= -1)
					numPoints--;
			}
		}
	}*/

	void updateEnqueueTimes() {
		enqueueTimes++;
	}

	long int getEnqueueTimes() {
		return enqueueTimes;
	}

	const QVector<QQueue<DataPoint>>& getQueues() const {
		return queues;
	}

	int getTargetNum() {
		return numPoints;
	}

	int getThisTurnNum() {
		int num_last = 0;
		qDebug() << "daozheli";
		for (auto& queue : queues) {
			qDebug() << queue.size();
		}
		for (auto& queue : queues) {
			if (queue.last().category >= 0 && queue.last().category < numTypes)
				num_last++;
		}
		return num_last;
	}



	void updateQueueLength(int newLength) {
		lock.lock();
		if (newLength > queueLength) {
			queueLength = newLength;
			for (auto& queue : queues) {
				//int category = queue.at(0).category;//��ʵ�������⣬����Ҫ
				while (queue.size() < queueLength)
					queue.enqueue({ 0,0,-1,-1 });
			}
		}
		else {
			queueLength = newLength;
			for (auto& queue : queues) {
				//int category = queue.at(0).category;//��ʵ�������⣬����Ҫ
				while (queue.size() > queueLength)
					queue.dequeue();
				qDebug() << "��С��ӵ�ʵ�ʳ���:" << queue.size();

			}
		}
		
		lock.unlock();
		qDebug() << "��ʱ�ĳ���Ϊ"<<queueLength;
	}
};