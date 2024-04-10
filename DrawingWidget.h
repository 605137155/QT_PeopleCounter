#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include <QPen>
#include <vector>
#include <cmath>
#include <QRect>
#include <QMouseEvent>
#include "Datastruct.h"
#include <QDebug>
#include "TargetQueue.h"
#include <qcolor.h>
#pragma execution_character_set("utf-8")
class DrawingWidget : public QWidget{
    Q_OBJECT
public:
    DrawingWidget(QWidget* parent = nullptr) :QWidget(parent) {
        setPointCloud();
    }

    void setPointCloud() {
        /*pointClound.push_back({ 1, 2,1 ,1 });
        pointClound.push_back({ -3, 8,1, 1 });*/
        /*pointClound.push_back({ -0.5, 0.2,3 });
        pointClound.push_back({ 0.3, 2,4 ,1 });
        pointClound.push_back({ 0.5, 1,5 });*/
    }
protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int width = this->width();
        int height = this->height();
        //int axisMargin = 20; // 坐标轴离边界的距离
        //int xMin = -5; // 横坐标最小值
        //int xMax = 5;  // 横坐标最大值
        //int yMax = 10; // 纵坐标最大值
        //int yMin = 0;
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

            if (i >= 0) //让非负值对齐刻度，会好看一些
                painter.drawText(x - 3, height - axisMargin + 20, QString::number(i)); // 刻度值
            else
                painter.drawText(x - 7, height - axisMargin + 20, QString::number(i)); // 刻度值
        }

        // 绘制纵坐标刻度和刻度值
        int numYTicks = yMax;
        int yStep = (height - 2 * axisMargin) / numYTicks;
        for (int i = 0; i <= numYTicks; ++i) {
            int y = height - axisMargin - i * yStep;
            painter.drawLine(axisMargin, y, axisMargin + 5, y); // 刻度线
            int yValue = i; // 计算纵坐标刻度值
            if (i == yMax&&i>=10)//让非负值对齐刻度，会好看一些
                painter.drawText(axisMargin - 17, y + 5, QString::number(yValue)); // 刻度值
            else
                painter.drawText(axisMargin - 13, y + 5, QString::number(yValue)); // 刻度值
        }



        qDebug() << "运行到里了10";
        //遍历所有的数据点并绘制
        int colorDeep = 1;
        bool changed = false;
        QVector<DataPoint> vector;
        queueLock.lock();
        for (auto& queue : allQueues) {
            colorDeep = 1;
            //vector.clear();
            //vector = queue.toVector();//转为vector，然后反向绘制，这样能让刚出现的元素绘制在前面 【比把元素入栈再出来的两次遍历更快，】
            //for (auto it = vector.begin(); it != vector.end();++it) {
            //    const DataPoint& point = *it;
            for(const DataPoint& point:queue){
                if (point.pointOrTarget == -1) {
                    colorDeep++;
                    qDebug() << point.x<<point.y<<point.category<<point.pointOrTarget;
                    continue;
                }
                qDebug() << point.x << point.y << point.category << point.pointOrTarget;
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
                float lightness = minLightness; //100是没有任何改变
                if (queueSize > 1)
                    lightness = maxLightness - (maxLightness - minLightness) * ((colorDeep - 1) / float(queueSize - 1));
                pointColor = colorArray[point.category].lighter(lightness);
                colorDeep++;

                QPen pen;
                pen.setColor(pointColor);
                pen.setWidth(4);
                painter.setPen(pen);
                painter.drawEllipse(QPoint(x, y), 14, 14);
                qDebug() << "窗口2成功画到非空点类型"<<point.category<<"其颜色的浅度："<<lightness << pointColor;
                changed = true;
            }
        }
        queueLock.unlock();
        
        //if (!changed)

        //if (qPointer!=nullptr)
        //    qPointer->dequeueAll();

        QPen pen;
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setColor(Qt::black);
        pen.setWidth(2);
        painter.setPen(pen);
        //绘制绘制框框，自适应窗口大小
        qDebug() << "矩形框重新映射前的点" << startPoint_Rect.x() << startPoint_Rect.y() << endPoint_Rect.x() << endPoint_Rect.y();
        float rect_x = mapToPixel(double(startPoint_Rect.x()), xMin, xMax, axisMargin, width - axisMargin);
        float rect_y = height - mapToPixel(double(startPoint_Rect.y()), yMin, yMax, axisMargin, height - axisMargin);
        float width_rect = mapToPixel(endPoint_Rect.x(), xMin, xMax, axisMargin, width - axisMargin)- rect_x;
        float height_rect = height-mapToPixel(endPoint_Rect.y(), yMin, yMax, axisMargin, height - axisMargin) - rect_y;
        rect = QRect(QPoint(rect_x, rect_y),QSize(width_rect, height_rect));
        painter.drawRect(rect);

        float line_x1 = mapToPixel(line.x1(), xMin, xMax, axisMargin, width - axisMargin);
        float line_y1 = height - mapToPixel(line.y1(), yMin, yMax, axisMargin, height - axisMargin);
        float line_x2 = mapToPixel(line.x2(), xMin, xMax, axisMargin, width - axisMargin);
        float line_y2 = height - mapToPixel(line.y2(), yMin, yMax, axisMargin, height - axisMargin);
        //qDebug() << "线条重新映射后的像素"<< line_x1<< line_y1<< line_x2<< line_y2;

        //QPoint p1();
        painter.drawLine(QLine(QPoint(line_x1, line_y1), QPoint(line_x2, line_y2)));
        //painter.drawLine(line);

        //坐标轴上的几个点对应的像素，用于图像自适应窗口大小
        x1 = mapToPixel(xMin, xMin, xMax, axisMargin, width - axisMargin);
        x2 = mapToPixel(0.0, xMin, xMax, axisMargin, width - axisMargin);
        x3 = mapToPixel(xMax, xMin, xMax, axisMargin, width - axisMargin);
        y1 = mapToPixel(0.0, yMin, yMax, axisMargin, height - axisMargin);
        y2 = mapToPixel(yMax, yMin, yMax, axisMargin, height - axisMargin);
        qDebug() << x1 << x2 << x3 << y1 << y2<<width<<height;






    }

    //将坐标映射为像素坐标
    int mapToPixel(float value, float minValue, float maxValue, float minPixel, float maxPixel) {
        return static_cast<float>((value - minValue) * (maxPixel - minPixel) / (maxValue - minValue)) + minPixel;
    }
    //将像素坐标映射为坐标
    QPointF PixelToFloatCord(int pixelX, int pixelY, int x1, int y1, int x2, int y2, int x3) {
        //pexelY为从上到下增长，而y1到y2为从下到上增长，得转为相同的表示方式才能运算  y1：20  y2 439   与实际pixel相反,
        // 开发时候重点注意一下 
        float xScale = float(x3-x1)/10.0f;
        float yScale = float(y2-y1)/ float(yMax);
        //qDebug() << "这里的y2-y1是！！！"<< yScale;
        float x = static_cast<float>((float(pixelX)-float(x2))/xScale);
        //qDebug() << "这里的(pixelY- axisMargin是！！！" << float(pixelY - axisMargin);
        float y = static_cast<float>((float(y2-y1) - float(pixelY- axisMargin)) / yScale); //不+0.5会让y取值范围处于-5到5而不是0到10
        //qDebug() << "y:" << y;
        return QPointF(x, y);
    }

    void mousePressEvent(QMouseEvent* event) override {
        //记录矩形的起始点
        if (event->button() == Qt::LeftButton) {
            startPoint = event->pos();
            //qDebug() << "鼠标点击的开始像素起始点" << startPoint.x() << startPoint.y();
            drawing = true;
        }
        else
        {
            startPoint = event->pos();
            drawing_right = true;
        }


    }  

    void mouseMoveEvent(QMouseEvent* event) override {

        int width = event->x() - startPoint.x();
        int height = event->y() - startPoint.y();
        if (event->buttons() & Qt::LeftButton) {
            if (drawing) {
                //矩形的宽度和高度
                int width_ = event->x() - startPoint.x();
                int height_ = event->y() - startPoint.y();
                //qDebug() << "移动了：" << width_ << height_;
                rect = QRect(startPoint, QSize(width_, height_));
                //框框左上角顶点
                startPoint_Rect = PixelToFloatCord(rect.left(), rect.top(), x1, y1, x2, y2, x3);
                //框框右下角顶点
                endPoint_Rect = PixelToFloatCord(rect.right(), rect.bottom(), x1, y1, x2, y2, x3);
                qDebug() << "startPoint_Rect:" << startPoint_Rect.x() << startPoint_Rect.y();
                qDebug() << "endPoint_Rect:" << endPoint_Rect.x() << endPoint_Rect.y();
                update();
            }
        }
        if (event->buttons() & Qt::RightButton) {
            line = QLine(startPoint, QPoint(event->x(), event->y()));
            //线的开始和结束坐标
            QPointF line_Start = PixelToFloatCord(line.x1(), line.y1(), x1, y1, x2, y2, x3);
            QPointF line_End = PixelToFloatCord(line.x2(), line.y2(), x1, y1, x2, y2, x3);
            qDebug() << "线条："<<line.x1()<<line.y1()<<line.x2()<<line.y2();
            qDebug() << "对应坐标:"<< line_Start.x()<<line_Start.y()<<line_End.x()<<line_End.y();
            line = QLineF(line_Start,line_End);
            update();
        }
        
        
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        drawing = false;
        if (event->button() == Qt::LeftButton){
        //qDebug() << "释放鼠标时候绘制方框开始点:" << startPoint_Rect.x() << startPoint_Rect.y();
        //qDebug() << "释放鼠标时候方框终点:" << endPoint_Rect.x() << endPoint_Rect.y();
        

        //head.toHex();
        //qDebug() << "head:" << head;
        float start_x = startPoint_Rect.x();
        float start_y = startPoint_Rect.y();
        float end_x = endPoint_Rect.x();
        float end_y = endPoint_Rect.y();

        QByteArray byteArray_x1;
        byteArray_x1.resize(sizeof(start_x));
        std::memcpy(byteArray_x1.data(), &start_x, sizeof(start_x));
        byteArray_x1 = byteArray_x1.toHex().toUpper();
        qDebug() << "16进制的x1" << byteArray_x1;

        QByteArray byteArray_y1;
        byteArray_y1.resize(sizeof(start_y));
        std::memcpy(byteArray_y1.data(), &start_y, sizeof(start_y));
        byteArray_y1 = byteArray_y1.toHex().toUpper();
        qDebug() << "16进制的y1" << byteArray_y1;

        QByteArray byteArray_x2;
        byteArray_x2.resize(sizeof(end_x));
        std::memcpy(byteArray_x2.data(), &end_x, sizeof(end_x));
        byteArray_x2 = byteArray_x2.toHex().toUpper();
        qDebug() << "16进制的x2" << byteArray_x2;

        QByteArray byteArray_y2;
        byteArray_y2.resize(sizeof(end_y));
        std::memcpy(byteArray_y2.data(), &end_y, sizeof(end_y));
        byteArray_y2 = byteArray_y2.toHex().toUpper();
        qDebug() << "16进制的y2" << byteArray_y2;
        
        QByteArray rectFrame = QByteArray("55F914000000") + byteArray_x1 + byteArray_y1 + byteArray_x2 + byteArray_y2 + QByteArray("55");
        qDebug() << rectFrame;
        emit sendFrame(rectFrame);
        }
        if (event->button() == Qt::RightButton) {
            qDebug() << "释放鼠标时候线条终点：" << line.x1() << line.y1() << line.x2() << line.y2();
            //emit writeLine(line);
            float start_x = line.x1();
            float start_y = line.y1();
            float end_x = line.x2();
            float end_y = line.y2();

            QByteArray byteArray_x1;
            byteArray_x1.resize(sizeof(start_x));
            std::memcpy(byteArray_x1.data(), &start_x, sizeof(start_x));
            byteArray_x1 = byteArray_x1.toHex().toUpper();
            qDebug() << "16进制的x1" << byteArray_x1;

            QByteArray byteArray_y1;
            byteArray_y1.resize(sizeof(start_y));
            std::memcpy(byteArray_y1.data(), &start_y, sizeof(start_y));
            byteArray_y1 = byteArray_y1.toHex().toUpper();
            qDebug() << "16进制的y1" << byteArray_y1;

            QByteArray byteArray_x2;
            byteArray_x2.resize(sizeof(end_x));
            std::memcpy(byteArray_x2.data(), &end_x, sizeof(end_x));
            byteArray_x2 = byteArray_x2.toHex().toUpper();
            qDebug() << "16进制的x2" << byteArray_x2;

            QByteArray byteArray_y2;
            byteArray_y2.resize(sizeof(end_y));
            std::memcpy(byteArray_y2.data(), &end_y, sizeof(end_y));
            byteArray_y2 = byteArray_y2.toHex().toUpper();
            qDebug() << "16进制的y2" << byteArray_y2;

            QByteArray lineFrame = QByteArray("55F814000000") + byteArray_x1 + byteArray_y1 + byteArray_x2 + byteArray_y2 + QByteArray("55");
            qDebug() << lineFrame;
            emit sendFrame(lineFrame);
        }
    }

public slots:

    void pointCloundReady(TargetQueue *targetQueue) {
        //qDebug() << "运行到里了5";
        allQueues = targetQueue->getQueues();
        qPointer = targetQueue;
        //qDebug() << "运行到里了7";
        update();
        //qDebug() << "运行到里了111";
    }

    void clearWindow() {
        startPoint_Rect = QPointF();
        endPoint_Rect = QPointF();
        rect = QRect();
        line = QLine();
        update();
        //qDebug() << "清楚！！！！";

    }

    void updataQueueSize(int qSize) {
        queueLock.lock();
        queueSize = qSize;
        queueLock.unlock();
        qDebug() << "此时windows2队列的长度为" << queueSize;
    }

signals:
    void sendFrame(QByteArray rectFrame);

private:
    QVector<DataPoint> pointClound;
    QVector<QQueue<DataPoint>> allQueues;
    TargetQueue* qPointer = nullptr;
    int pointRadius = 3;
    QPoint startPoint;
    QRect rect;
    QLineF line;
    QPointF startPoint_Rect;
    QPointF endPoint_Rect;
    float w1;
    float h1;
    bool drawing = false;
    bool drawing_right = false;
    int axisMargin = 20; // 坐标轴离边界的距离
    int xMin = -5; // 横坐标最小值
    int xMax = 5;  // 横坐标最大值
    int yMax = 8; // 纵坐标最大值
    int yMin = 0;
    QMutex queueLock;
    //窗口随时改变的像素值，用于窗口绘制图像的自适应大小
    float x1; //-5，10 的像素值
    float y1;
    float x2; //5，0
    float y2;
    float x3; //0,0
    int lenColor = 6;
    QColor colorArray[6] = { Qt::red,Qt::blue,Qt::green,Qt::GlobalColor::magenta,Qt::yellow,Qt::gray};
    int queueSize = 13;
    float minLightness = 100;
    float maxLightness = 200.0;


};