#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include <QPen>
#include <vector>
#include <cmath>
#include <QElapsedTimer>
#include "Datastruct.h"
#include "TargetQueue.h"
#pragma execution_character_set("utf-8")
class PointCloundWidget :public QWidget {

public:
	PointCloundWidget(QWidget *parent = nullptr):QWidget(parent){
        setPointCloud();
    }

    void setPointCloud() {
        pointClound.push_back({ 3, 5,    0 ,1 });
        pointClound.push_back({ 1, 2,    1 ,1});
        pointClound.push_back({ -3,8,    2 ,1 });
        pointClound.push_back({ -0.5, 0.2, 3,1 });
        pointClound.push_back({ 0.3, 2,    4,1});
        pointClound.push_back({ 0.5, 1,    5,1 });
        pointClound.push_back({ 2, 4,    6,1 });

        pointClound.push_back({ 3, 5,    0 ,0 });
        pointClound.push_back({ 1, 2,    1 ,0 });
        pointClound.push_back({ -3,8,    2 ,0 });
        pointClound.push_back({ -0.5, 0.2, 3,0 });
        pointClound.push_back({ 0.3, 2,    4,0 });
        pointClound.push_back({ 0.5, 1,    5,0 });
        pointClound.push_back({ 2, 4,    6,0 });
	}
protected:
    void paintEvent(QPaintEvent* event) override {

        QElapsedTimer timer;
        timer.start();

        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int width = this->width();
        int height = this->height();
        int axisMargin = 20; // ��������߽�ľ���
        int xMin = -5; // ��������Сֵ
        int xMax = 5;  // ���������ֵ
        int yMax = 8; // ���������ֵ
        int yMin = 0;
        // ����������
        QPen axisPen(Qt::black);
        painter.setPen(axisPen);

        // ��������
        painter.drawLine(axisMargin, height - axisMargin, width - axisMargin, height - axisMargin);
        painter.drawText(width - axisMargin + 5, height - axisMargin + 5, "X");

        // ��������
        painter.drawLine(axisMargin, height - axisMargin, axisMargin, axisMargin);
        painter.drawText(axisMargin - 10, axisMargin - 10, "Y");

        // ���ƺ�����̶ȺͿ̶�ֵ
        for (int i = xMin; i <= xMax; ++i) {
            int x = mapToPixel(i, xMin, xMax, axisMargin, width - axisMargin);
            painter.drawLine(x, height - axisMargin - 5, x, height - axisMargin + 5); // �̶���
            
            if(i>=0) //�÷Ǹ�ֵ����̶ȣ���ÿ�һЩ
                painter.drawText(x - 3, height - axisMargin + 20, QString::number(i)); // �̶�ֵ
            else
                painter.drawText(x - 7, height - axisMargin + 20, QString::number(i)); // �̶�ֵ
        }

        // ����������̶ȺͿ̶�ֵ
        int numYTicks = yMax;
        int yStep = (height - 2 * axisMargin) / numYTicks;
        for (int i = 0; i <= numYTicks; ++i) {
            int y = height - axisMargin - i * yStep;
            painter.drawLine(axisMargin , y, axisMargin + 5, y); // �̶���
            int yValue =i; // ����������̶�ֵ
            if (i== yMax+1)//�÷Ǹ�ֵ����̶ȣ���ÿ�һЩ
                painter.drawText(axisMargin - 17, y + 5, QString::number(yValue)); // �̶�ֵ
            else
                painter.drawText(axisMargin -13, y + 5, QString::number(yValue)); // �̶�ֵ
        }


        qDebug() << "���������֮ǰ�ĵ���������" << pointClound.size();
        qDebug() << "���������ǰ��target������" << lastTurnTargetNum;
        //lock.lock();

        //�������е����ݵ㲢����
        int colorDeep = 1;
        QVector<DataPoint> vector;
        queueLock.lock();
        for (auto& queue : pointCloundTargetQueue) {
            colorDeep = 1;
            //vector = queue.toVector();//תΪvector��Ȼ������ƣ��������øճ��ֵ�Ԫ�ػ�����ǰ�� ���Ȱ�Ԫ����ջ�ٳ��������α������죬��
            //for (auto it = vector.begin(); it != vector.end(); ++it) {
            //    const DataPoint& point = *it;
            for (const DataPoint& point : queue) {
                if (point.pointOrTarget == -1) {
                    colorDeep++;
                    continue;
                }
                float x = mapToPixel(point.x, xMin, xMax, axisMargin, width - axisMargin);

                if (x < 0)//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                    x -= 4;

                float y = height - mapToPixel(point.y, yMin, yMax, axisMargin, height - axisMargin);

                if (y > 5)//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                    y += 4;
                else
                    y -= 4;
                //���ﶨ��6�ֵ�;���
                QColor pointColor;
                float lightness = minLightness;
                if (queueSize > 1)
                    lightness = maxLightness - (maxLightness - minLightness) * ((colorDeep-1) / float(queueSize - 1));
                pointColor = colorArray[point.category].lighter(lightness);
                colorDeep++;
                QPen pen;
                pen.setColor(pointColor);
                pen.setWidth(4);
                painter.setPen(pen);
                painter.drawEllipse(QPoint(x, y), 14, 14);
                qDebug() << "����1�ɹ������ǿյ�" << point.category<<"����ɫ��ǳ�ȣ�" << lightness << pointColor;
            }
        }
        queueLock.unlock();
        qDebug() << "Ŀ��������";
        //if (qPointer != nullptr)
        //    qPointer->dequeueAll();

                //�������еĵ��Ʋ�����
        for (const DataPoint& point : pointClound) {
            /*if (point.pointOrTarget == 0)
                qDebug() << "��ͼ��ʱ���е��Ƶĵ����";
            else if (point.pointOrTarget == 1)
                qDebug() << "��ͼ��ʱ����Ŀ��ĵĵ����";*/
            qDebug() << "��ͼ��ʱ���е��Ƶĵ����";
            float x = mapToPixel(point.x, xMin, xMax, axisMargin, width - axisMargin);

            if (x < 0)//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                x -= 4;

            float y = height - mapToPixel(point.y, yMin, yMax, axisMargin, height - axisMargin);

            if (y > 5)//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                y += 4;
            else
                y -= 4;

            QColor pointColor;//���ﶨ��6�ֵ�;���
            pointColor = colorArray[point.category];
            QPen pen;
            pen.setColor(pointColor);
            pen.setWidth(2);
            painter.setPen(pen);
            if (point.pointOrTarget == 0)
                painter.drawEllipse(QPoint(x, y), 1, 1);
            else
                painter.drawEllipse(QPoint(x, y), 14, 14);

        }
        qDebug() << "���ƻ������";





        paintDone = true;
        qint64 elapsedTime = timer.elapsed();
        qDebug() << "paintglTook " << elapsedTime << "milliseconds";

    }

    int mapToPixel(float value, float minValue, float maxValue, float minPixel, float maxPixel) {
        return static_cast<float>((value - minValue) * (maxPixel - minPixel) / (maxValue - minValue)) + minPixel;
    }
public slots:
    void pointCloundReady(const QVector<DataPoint> pc) {
        lock.lock();
        pointClound = QVector<DataPoint>();
        qDebug()<<"paintDone :" << paintDone;
        if (paintDone) {
            
            if (pc.size() == 0) {
                qDebug() << "��1�����������,�������Ŀյ���";
                clusterReady = true;
            }else 
            {
                    pointClound = pc;
                    //lock.unlock();
                    //qDebug() << "��2�����������_ " << pointClound.size()<<"��ʱĿ�������:"<< pointCloundTarget.size()<<"pc��size:"<<pc.size();
                    //clusterReady = true;
            }
            }
        paintDone = false;
        qDebug() << "update()֮ǰ����λ��Ƶ�pointClound �� pointCloundTarget ��size�ֱ�Ϊ��" << pointClound.size() << lastTurnTargetNum;
        lock.unlock();
        update();
        }
        
    void updataQueueSize(int qSize) {
        queueLock.lock();
        queueSize = qSize;
        queueLock.unlock();
        qDebug() << "��ʱwindows1�ĳ���Ϊ" << queueSize;
    }

    void pointTargetReady(TargetQueue* targetQueue) {
        qPointer = targetQueue;
        qDebug() << "���е�����6";
        lock.lock();
        pointCloundTargetQueue = targetQueue->getQueues();
        qDebug() << "���е�����8";
        //pointCloundTarget = QVector<DataPoint>();
        if (paintDone) {
            clusterReady = true;
        }
        paintDone = false;
        //qDebug() << "���е�����1222";
        lastTurnTargetNum = targetQueue->getThisTurnNum();
        qDebug() << "���е�����1222";
        qDebug() << "update()֮ǰ����λ��Ƶ�pointClound �� pointCloundTarget ��size�ֱ�Ϊ��" << pointClound.size() << targetQueue->getThisTurnNum();
        lock.unlock();
        update();
        

    }



    
private:
    QVector<DataPoint> pointClound;
    QVector<QQueue<DataPoint>>  pointCloundTargetQueue;
    TargetQueue* qPointer= nullptr;
    int lastTurnTargetNum = 0;
    int pointRadius = 3;
    QMutex lock;
    bool paintDone = true;
    bool targetReady = false;
    bool clusterReady = false;
    int lenColor = 6;
    QColor colorArray[6] = { Qt::red,Qt::blue,Qt::green,Qt::GlobalColor::magenta,Qt::yellow,Qt::gray };
    int queueSize = 13;
    float minLightness = 100;
    float maxLightness = 200.0;
    QMutex queueLock;

};