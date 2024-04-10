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
        //int axisMargin = 20; // ��������߽�ľ���
        //int xMin = -5; // ��������Сֵ
        //int xMax = 5;  // ���������ֵ
        //int yMax = 10; // ���������ֵ
        //int yMin = 0;
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

            if (i >= 0) //�÷Ǹ�ֵ����̶ȣ���ÿ�һЩ
                painter.drawText(x - 3, height - axisMargin + 20, QString::number(i)); // �̶�ֵ
            else
                painter.drawText(x - 7, height - axisMargin + 20, QString::number(i)); // �̶�ֵ
        }

        // ����������̶ȺͿ̶�ֵ
        int numYTicks = yMax;
        int yStep = (height - 2 * axisMargin) / numYTicks;
        for (int i = 0; i <= numYTicks; ++i) {
            int y = height - axisMargin - i * yStep;
            painter.drawLine(axisMargin, y, axisMargin + 5, y); // �̶���
            int yValue = i; // ����������̶�ֵ
            if (i == yMax&&i>=10)//�÷Ǹ�ֵ����̶ȣ���ÿ�һЩ
                painter.drawText(axisMargin - 17, y + 5, QString::number(yValue)); // �̶�ֵ
            else
                painter.drawText(axisMargin - 13, y + 5, QString::number(yValue)); // �̶�ֵ
        }



        qDebug() << "���е�����10";
        //�������е����ݵ㲢����
        int colorDeep = 1;
        bool changed = false;
        QVector<DataPoint> vector;
        queueLock.lock();
        for (auto& queue : allQueues) {
            colorDeep = 1;
            //vector.clear();
            //vector = queue.toVector();//תΪvector��Ȼ������ƣ��������øճ��ֵ�Ԫ�ػ�����ǰ�� ���Ȱ�Ԫ����ջ�ٳ��������α������죬��
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

                if (x < 0)//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                    x -= 4;

                float y = height - mapToPixel(point.y, yMin, yMax, axisMargin, height - axisMargin);

                if (y > 5)//����ӳ����ƫ�����Ӿ�Ч����׼�̶�һЩ
                    y += 4;
                else
                    y -= 4;

                //���ﶨ��6�ֵ�;���
                QColor pointColor;
                float lightness = minLightness; //100��û���κθı�
                if (queueSize > 1)
                    lightness = maxLightness - (maxLightness - minLightness) * ((colorDeep - 1) / float(queueSize - 1));
                pointColor = colorArray[point.category].lighter(lightness);
                colorDeep++;

                QPen pen;
                pen.setColor(pointColor);
                pen.setWidth(4);
                painter.setPen(pen);
                painter.drawEllipse(QPoint(x, y), 14, 14);
                qDebug() << "����2�ɹ������ǿյ�����"<<point.category<<"����ɫ��ǳ�ȣ�"<<lightness << pointColor;
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
        //���ƻ��ƿ������Ӧ���ڴ�С
        qDebug() << "���ο�����ӳ��ǰ�ĵ�" << startPoint_Rect.x() << startPoint_Rect.y() << endPoint_Rect.x() << endPoint_Rect.y();
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
        //qDebug() << "��������ӳ��������"<< line_x1<< line_y1<< line_x2<< line_y2;

        //QPoint p1();
        painter.drawLine(QLine(QPoint(line_x1, line_y1), QPoint(line_x2, line_y2)));
        //painter.drawLine(line);

        //�������ϵļ������Ӧ�����أ�����ͼ������Ӧ���ڴ�С
        x1 = mapToPixel(xMin, xMin, xMax, axisMargin, width - axisMargin);
        x2 = mapToPixel(0.0, xMin, xMax, axisMargin, width - axisMargin);
        x3 = mapToPixel(xMax, xMin, xMax, axisMargin, width - axisMargin);
        y1 = mapToPixel(0.0, yMin, yMax, axisMargin, height - axisMargin);
        y2 = mapToPixel(yMax, yMin, yMax, axisMargin, height - axisMargin);
        qDebug() << x1 << x2 << x3 << y1 << y2<<width<<height;






    }

    //������ӳ��Ϊ��������
    int mapToPixel(float value, float minValue, float maxValue, float minPixel, float maxPixel) {
        return static_cast<float>((value - minValue) * (maxPixel - minPixel) / (maxValue - minValue)) + minPixel;
    }
    //����������ӳ��Ϊ����
    QPointF PixelToFloatCord(int pixelX, int pixelY, int x1, int y1, int x2, int y2, int x3) {
        //pexelYΪ���ϵ�����������y1��y2Ϊ���µ�����������תΪ��ͬ�ı�ʾ��ʽ��������  y1��20  y2 439   ��ʵ��pixel�෴,
        // ����ʱ���ص�ע��һ�� 
        float xScale = float(x3-x1)/10.0f;
        float yScale = float(y2-y1)/ float(yMax);
        //qDebug() << "�����y2-y1�ǣ�����"<< yScale;
        float x = static_cast<float>((float(pixelX)-float(x2))/xScale);
        //qDebug() << "�����(pixelY- axisMargin�ǣ�����" << float(pixelY - axisMargin);
        float y = static_cast<float>((float(y2-y1) - float(pixelY- axisMargin)) / yScale); //��+0.5����yȡֵ��Χ����-5��5������0��10
        //qDebug() << "y:" << y;
        return QPointF(x, y);
    }

    void mousePressEvent(QMouseEvent* event) override {
        //��¼���ε���ʼ��
        if (event->button() == Qt::LeftButton) {
            startPoint = event->pos();
            //qDebug() << "������Ŀ�ʼ������ʼ��" << startPoint.x() << startPoint.y();
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
                //���εĿ�Ⱥ͸߶�
                int width_ = event->x() - startPoint.x();
                int height_ = event->y() - startPoint.y();
                //qDebug() << "�ƶ��ˣ�" << width_ << height_;
                rect = QRect(startPoint, QSize(width_, height_));
                //������ϽǶ���
                startPoint_Rect = PixelToFloatCord(rect.left(), rect.top(), x1, y1, x2, y2, x3);
                //������½Ƕ���
                endPoint_Rect = PixelToFloatCord(rect.right(), rect.bottom(), x1, y1, x2, y2, x3);
                qDebug() << "startPoint_Rect:" << startPoint_Rect.x() << startPoint_Rect.y();
                qDebug() << "endPoint_Rect:" << endPoint_Rect.x() << endPoint_Rect.y();
                update();
            }
        }
        if (event->buttons() & Qt::RightButton) {
            line = QLine(startPoint, QPoint(event->x(), event->y()));
            //�ߵĿ�ʼ�ͽ�������
            QPointF line_Start = PixelToFloatCord(line.x1(), line.y1(), x1, y1, x2, y2, x3);
            QPointF line_End = PixelToFloatCord(line.x2(), line.y2(), x1, y1, x2, y2, x3);
            qDebug() << "������"<<line.x1()<<line.y1()<<line.x2()<<line.y2();
            qDebug() << "��Ӧ����:"<< line_Start.x()<<line_Start.y()<<line_End.x()<<line_End.y();
            line = QLineF(line_Start,line_End);
            update();
        }
        
        
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        drawing = false;
        if (event->button() == Qt::LeftButton){
        //qDebug() << "�ͷ����ʱ����Ʒ���ʼ��:" << startPoint_Rect.x() << startPoint_Rect.y();
        //qDebug() << "�ͷ����ʱ�򷽿��յ�:" << endPoint_Rect.x() << endPoint_Rect.y();
        

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
        qDebug() << "16���Ƶ�x1" << byteArray_x1;

        QByteArray byteArray_y1;
        byteArray_y1.resize(sizeof(start_y));
        std::memcpy(byteArray_y1.data(), &start_y, sizeof(start_y));
        byteArray_y1 = byteArray_y1.toHex().toUpper();
        qDebug() << "16���Ƶ�y1" << byteArray_y1;

        QByteArray byteArray_x2;
        byteArray_x2.resize(sizeof(end_x));
        std::memcpy(byteArray_x2.data(), &end_x, sizeof(end_x));
        byteArray_x2 = byteArray_x2.toHex().toUpper();
        qDebug() << "16���Ƶ�x2" << byteArray_x2;

        QByteArray byteArray_y2;
        byteArray_y2.resize(sizeof(end_y));
        std::memcpy(byteArray_y2.data(), &end_y, sizeof(end_y));
        byteArray_y2 = byteArray_y2.toHex().toUpper();
        qDebug() << "16���Ƶ�y2" << byteArray_y2;
        
        QByteArray rectFrame = QByteArray("55F914000000") + byteArray_x1 + byteArray_y1 + byteArray_x2 + byteArray_y2 + QByteArray("55");
        qDebug() << rectFrame;
        emit sendFrame(rectFrame);
        }
        if (event->button() == Qt::RightButton) {
            qDebug() << "�ͷ����ʱ�������յ㣺" << line.x1() << line.y1() << line.x2() << line.y2();
            //emit writeLine(line);
            float start_x = line.x1();
            float start_y = line.y1();
            float end_x = line.x2();
            float end_y = line.y2();

            QByteArray byteArray_x1;
            byteArray_x1.resize(sizeof(start_x));
            std::memcpy(byteArray_x1.data(), &start_x, sizeof(start_x));
            byteArray_x1 = byteArray_x1.toHex().toUpper();
            qDebug() << "16���Ƶ�x1" << byteArray_x1;

            QByteArray byteArray_y1;
            byteArray_y1.resize(sizeof(start_y));
            std::memcpy(byteArray_y1.data(), &start_y, sizeof(start_y));
            byteArray_y1 = byteArray_y1.toHex().toUpper();
            qDebug() << "16���Ƶ�y1" << byteArray_y1;

            QByteArray byteArray_x2;
            byteArray_x2.resize(sizeof(end_x));
            std::memcpy(byteArray_x2.data(), &end_x, sizeof(end_x));
            byteArray_x2 = byteArray_x2.toHex().toUpper();
            qDebug() << "16���Ƶ�x2" << byteArray_x2;

            QByteArray byteArray_y2;
            byteArray_y2.resize(sizeof(end_y));
            std::memcpy(byteArray_y2.data(), &end_y, sizeof(end_y));
            byteArray_y2 = byteArray_y2.toHex().toUpper();
            qDebug() << "16���Ƶ�y2" << byteArray_y2;

            QByteArray lineFrame = QByteArray("55F814000000") + byteArray_x1 + byteArray_y1 + byteArray_x2 + byteArray_y2 + QByteArray("55");
            qDebug() << lineFrame;
            emit sendFrame(lineFrame);
        }
    }

public slots:

    void pointCloundReady(TargetQueue *targetQueue) {
        //qDebug() << "���е�����5";
        allQueues = targetQueue->getQueues();
        qPointer = targetQueue;
        //qDebug() << "���е�����7";
        update();
        //qDebug() << "���е�����111";
    }

    void clearWindow() {
        startPoint_Rect = QPointF();
        endPoint_Rect = QPointF();
        rect = QRect();
        line = QLine();
        update();
        //qDebug() << "�����������";

    }

    void updataQueueSize(int qSize) {
        queueLock.lock();
        queueSize = qSize;
        queueLock.unlock();
        qDebug() << "��ʱwindows2���еĳ���Ϊ" << queueSize;
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
    int axisMargin = 20; // ��������߽�ľ���
    int xMin = -5; // ��������Сֵ
    int xMax = 5;  // ���������ֵ
    int yMax = 8; // ���������ֵ
    int yMin = 0;
    QMutex queueLock;
    //������ʱ�ı������ֵ�����ڴ��ڻ���ͼ�������Ӧ��С
    float x1; //-5��10 ������ֵ
    float y1;
    float x2; //5��0
    float y2;
    float x3; //0,0
    int lenColor = 6;
    QColor colorArray[6] = { Qt::red,Qt::blue,Qt::green,Qt::GlobalColor::magenta,Qt::yellow,Qt::gray};
    int queueSize = 13;
    float minLightness = 100;
    float maxLightness = 200.0;


};