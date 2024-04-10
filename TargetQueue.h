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
	int numPoints; //表示存在的点的数量（不包括不符合规则的点）
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
		if (point.category >= 0 && point.category < numTypes) {//这里为有空点和实点，空点为只有一部分的点存在的时候为另外没有采集到的点分配的空点
			//qDebug() << "队的长度::"<< queues[point.category].size();
			DataPoint p = queues[point.category].dequeue();
			if (p.category != -1 && p.pointOrTarget != -1)
				numPoints--;
			queues[point.category].enqueue(point);
			if (point.category != -1) {
				numPoints++;
			}
			else if (point.category == -1) //类别=0的时候，此时雷达线程采集的点数为0，为所有队列添加空元素
			{
				qDebug() << "出现第7种及以上的点,or category=-1的空点，此时空点已经通过解析的时候加入了，然后用于后续控件中的颜色控制";
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
				//int category = queue.at(0).category;//其实这里随意，不需要
				while (queue.size() < queueLength)
					queue.enqueue({ 0,0,-1,-1 });
			}
		}
		else {
			queueLength = newLength;
			for (auto& queue : queues) {
				//int category = queue.at(0).category;//其实这里随意，不需要
				while (queue.size() > queueLength)
					queue.dequeue();
				qDebug() << "变小后队的实际长度:" << queue.size();

			}
		}
		
		lock.unlock();
		qDebug() << "此时的长度为"<<queueLength;
	}
};