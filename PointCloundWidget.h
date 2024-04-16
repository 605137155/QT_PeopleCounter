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
        //pointClound.push_back({ 3, 5,    0 ,1 });
        //pointClound.push_back({ 1, 2,    1 ,1});
        //pointClound.push_back({ -3,8,    2 ,1 });
        //pointClound.push_back({ -0.5, 0.2, 3,1 });
        //pointClound.push_back({ 0.3, 2,    4,1});
        //pointClound.push_back({ 0.5, 1,    5,1 });
        //pointClound.push_back({ 2, 4,    6,1 });

        //pointClound.push_back({ 3, 5,    0 ,0 });
        //pointClound.push_back({ 1, 2,    1 ,0 });
        //pointClound.push_back({ -3,8,    2 ,0 });
        //pointClound.push_back({ -0.5, 0.2, 3,0 });
        //pointClound.push_back({ 0.3, 2,    4,0 });
        //pointClound.push_back({ 0.5, 1,    5,0 });
        //pointClound.push_back({ 2, 4,    6,0 });
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

        //���ƹ켣
        painter.setRenderHint(QPainter::Antialiasing);
        for (const auto& categoryPoints : tragePointsByCategory) {
            if (categoryPoints.size() < 2) continue;
            //��category�ĵ����ɫ
            QColor pointColor;//���ﶨ��6�ֵ�;���
            pointColor = colorArray[categoryPoints[0].category];
            QPen pen;
            pen.setColor(pointColor);
            pen.setWidth(2);
            painter.setPen(pen);
            for (int i = 1; i < categoryPoints.size(); i++) {
                if (categoryPoints[i].pointOrTarget != 1 || categoryPoints[i - 1].pointOrTarget != 1) continue;
                float x1 = mapToPixel(categoryPoints[i].x, xMin, xMax, axisMargin, width - axisMargin);
                float y1 = height - mapToPixel(categoryPoints[i].y, yMin, yMax, axisMargin, height - axisMargin);
                if (x1 < 0) x1 -= 4;//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                if (y1 > 5) y1 += 4; else  y1 -= 4;//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ

                float x2 = mapToPixel(categoryPoints[i-1].x, xMin, xMax, axisMargin, width - axisMargin);
                float y2 = height - mapToPixel(categoryPoints[i-1].y, yMin, yMax, axisMargin, height - axisMargin);
                if (x2 < 0) x2 -= 4;//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                if (y2 > 5) y2 += 4; else  y2 -= 4;//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ

                if (categoryPoints[i].pointOrTarget == 1&& categoryPoints[i-1].pointOrTarget==1)
                    //painter.drawEllipse(QPoint(x, y), 1, 1);
                    painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
            }
        }
        qDebug() << "�켣�������";


        //����Ŀ��
        int colorDeep = 1;
        queueLock.lock();
        for (auto& queue : pointCloundTargetQueue) {
            colorDeep = 1;
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
            //qDebug() << "���Ƶ���������:" << point.category;
            pointColor = colorArray[point.category];
            QPen pen;
            pen.setColor(pointColor);
            pen.setWidth(2);
            painter.setPen(pen);
            if (point.pointOrTarget == 0)
                painter.drawEllipse(QPoint(x, y), 1, 1);
            else
            {
                painter.drawEllipse(QPoint(x, y), 14, 14);
                qDebug() << "��������ֵĵ�";
            }

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
        //qDebug()<<"paintDone :" << paintDone;
        if (paintDone) {
            if (pc.size() == 0) {
                //qDebug() << "��1�����������,�������Ŀյ���";
                clusterReady = true;
            }else 
            {
                    pointClound = pc;
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
        //qDebug() << "��ʱwindows1�ĳ���Ϊ" << queueSize;
    }

    void pointTargetReady(TargetQueue* targetQueue) {
        qPointer = targetQueue;

        lock.lock();
        pointCloundTargetQueue = targetQueue->getQueues();
        //��ÿ�����׵ķǿյ���뵽�켣�����
        queueLock.lock();
        tragePointsByCategory.resize(pointCloundTargetQueue.size());
        for (int i = 0; i < pointCloundTargetQueue.size(); ++i) {
            const auto& queue = pointCloundTargetQueue[i];
            if (!queue.isEmpty()) {
                const DataPoint& latestPoint = queue.last();
                tragePointsByCategory[latestPoint.category].append(latestPoint);
            }
        }

        queueLock.unlock();

        if (paintDone) {
            clusterReady = true;
        }
        paintDone = false;
        lastTurnTargetNum = targetQueue->getThisTurnNum();
        //qDebug() << "update()֮ǰ����λ��Ƶ�pointClound �� pointCloundTarget ��size�ֱ�Ϊ��" << pointClound.size() << targetQueue->getThisTurnNum();
        lock.unlock();
        update();
        

    }



    
private:
    QVector<DataPoint> pointClound;
    QVector<DataPoint> targetTracePoints = {};
    QVector<QVector<DataPoint>> tragePointsByCategory;
    //QImage tracePointsImage = QImage(this->size(), QImage::Format_ARGB32);
    QVector<QQueue<DataPoint>>  pointCloundTargetQueue;
    TargetQueue* qPointer= nullptr;
    int lastTurnTargetNum = 0;
    int pointRadius = 3;
    QMutex lock;
    bool paintDone = true;
    bool targetReady = false;
    bool clusterReady = false;
    int lenColor = 6;
    QColor colorArray[6] = { Qt::red,Qt::blue,Qt::darkYellow,Qt::darkMagenta,Qt::darkGreen,Qt::gray };
    int queueSize = 13;
    float minLightness = 100;
    float maxLightness = 200.0;
    QMutex queueLock;

};