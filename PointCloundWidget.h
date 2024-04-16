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
        int axisMargin = 20; // 坐标轴离边界的距离
        int xMin = -5; // 横坐标最小值
        int xMax = 5;  // 横坐标最大值
        int yMax = 8; // 纵坐标最大值
        int yMin = 0;
        // 绘制坐标轴
        QPen axisPen(Qt::black);
        painter.setPen(axisPen);

        // 横坐标轴
        painter.drawLine(axisMargin, height - axisMargin, width - axisMargin, height - axisMargin);
        painter.drawText(width - axisMargin + 5, height - axisMargin + 5, "X");

        // 纵坐标轴
        painter.drawLine(axisMargin, height - axisMargin, axisMargin, axisMargin);
        painter.drawText(axisMargin - 10, axisMargin - 10, "Y");

        // 绘制横坐标刻度和刻度值
        for (int i = xMin; i <= xMax; ++i) {
            int x = mapToPixel(i, xMin, xMax, axisMargin, width - axisMargin);
            painter.drawLine(x, height - axisMargin - 5, x, height - axisMargin + 5); // 刻度线
            
            if(i>=0) //让非负值对齐刻度，会好看一些
                painter.drawText(x - 3, height - axisMargin + 20, QString::number(i)); // 刻度值
            else
                painter.drawText(x - 7, height - axisMargin + 20, QString::number(i)); // 刻度值
        }

        // 绘制纵坐标刻度和刻度值
        int numYTicks = yMax;
        int yStep = (height - 2 * axisMargin) / numYTicks;
        for (int i = 0; i <= numYTicks; ++i) {
            int y = height - axisMargin - i * yStep;
            painter.drawLine(axisMargin , y, axisMargin + 5, y); // 刻度线
            int yValue =i; // 计算纵坐标刻度值
            if (i== yMax+1)//让非负值对齐刻度，会好看一些
                painter.drawText(axisMargin - 17, y + 5, QString::number(yValue)); // 刻度值
            else
                painter.drawText(axisMargin -13, y + 5, QString::number(yValue)); // 刻度值
        }


        qDebug() << "绘制完点云之前的点云数量：" << pointClound.size();
        qDebug() << "绘制完点云前的target数量：" << lastTurnTargetNum;
        //lock.lock();

        //遍历所有的数据点并绘制

        //绘制轨迹
        painter.setRenderHint(QPainter::Antialiasing);
        for (const auto& categoryPoints : tragePointsByCategory) {
            if (categoryPoints.size() < 2) continue;
            //该category的点的颜色
            QColor pointColor;//这里定义6种点和聚类
            pointColor = colorArray[categoryPoints[0].category];
            QPen pen;
            pen.setColor(pointColor);
            pen.setWidth(2);
            painter.setPen(pen);
            for (int i = 1; i < categoryPoints.size(); i++) {
                if (categoryPoints[i].pointOrTarget != 1 || categoryPoints[i - 1].pointOrTarget != 1) continue;
                float x1 = mapToPixel(categoryPoints[i].x, xMin, xMax, axisMargin, width - axisMargin);
                float y1 = height - mapToPixel(categoryPoints[i].y, yMin, yMax, axisMargin, height - axisMargin);
                if (x1 < 0) x1 -= 4;//调整映射后的偏差，点的视觉效果对准刻度一些
                if (y1 > 5) y1 += 4; else  y1 -= 4;//调整映射后的偏差，点的视觉效果对准刻度一些

                float x2 = mapToPixel(categoryPoints[i-1].x, xMin, xMax, axisMargin, width - axisMargin);
                float y2 = height - mapToPixel(categoryPoints[i-1].y, yMin, yMax, axisMargin, height - axisMargin);
                if (x2 < 0) x2 -= 4;//调整映射后的偏差，点的视觉效果对准刻度一些
                if (y2 > 5) y2 += 4; else  y2 -= 4;//调整映射后的偏差，点的视觉效果对准刻度一些

                if (categoryPoints[i].pointOrTarget == 1&& categoryPoints[i-1].pointOrTarget==1)
                    //painter.drawEllipse(QPoint(x, y), 1, 1);
                    painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
            }
        }
        qDebug() << "轨迹绘制完毕";


        //绘制目标
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
                if (x < 0)//调整映射后的偏差，点的视觉效果对准刻度一些
                    x -= 4;
                float y = height - mapToPixel(point.y, yMin, yMax, axisMargin, height - axisMargin);
                if (y > 5)//调整映射后的偏差，点的视觉效果对准刻度一些
                    y += 4;
                else
                    y -= 4;
                //这里定义6种点和聚类
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
                qDebug() << "窗口1成功画到非空点" << point.category<<"其颜色的浅度：" << lightness << pointColor;
            }
        }
        queueLock.unlock();
        qDebug() << "目标绘制完毕";
        //if (qPointer != nullptr)
        //    qPointer->dequeueAll();

        //遍历所有的点云并绘制
        for (const DataPoint& point : pointClound) {
            qDebug() << "画图的时候有点云的点出现";
            float x = mapToPixel(point.x, xMin, xMax, axisMargin, width - axisMargin);

            if (x < 0)//调整映射后的偏差，点的视觉效果对准刻度一些
                x -= 4;

            float y = height - mapToPixel(point.y, yMin, yMax, axisMargin, height - axisMargin);

            if (y > 5)//调整映射后的偏差，点的视觉效果对准刻度一些
                y += 4;
            else
                y -= 4;

            QColor pointColor;//这里定义6种点和聚类
            //qDebug() << "点云的种类索引:" << point.category;
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
                qDebug() << "出现了奇怪的点";
            }

        }
        qDebug() << "点云绘制完毕";




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
                //qDebug() << "第1种情况被触发,传过来的空点云";
                clusterReady = true;
            }else 
            {
                    pointClound = pc;
            }
        }
        paintDone = false;
        qDebug() << "update()之前的这次绘制的pointClound 和 pointCloundTarget 的size分别为：" << pointClound.size() << lastTurnTargetNum;
        lock.unlock();
        update();
        }
        
    void updataQueueSize(int qSize) {
        queueLock.lock();
        queueSize = qSize;
        queueLock.unlock();
        //qDebug() << "此时windows1的长度为" << queueSize;
    }

    void pointTargetReady(TargetQueue* targetQueue) {
        qPointer = targetQueue;

        lock.lock();
        pointCloundTargetQueue = targetQueue->getQueues();
        //将每个队首的非空点加入到轨迹点池中
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
        //qDebug() << "update()之前的这次绘制的pointClound 和 pointCloundTarget 的size分别为：" << pointClound.size() << targetQueue->getThisTurnNum();
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