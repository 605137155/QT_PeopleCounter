#pragma once
#pragma execution_character_set("utf-8")
#include <QtWidgets/QMainWindow>
#include "ui_PeopleCounter.h"
#include "UartThread.h"
#include "PointCloundWidget.h"
#include "DrawingWidget.h"
#include "PureCppThread.h"
#include "TargetQueue.h"
#include <QTimer>

class PeopleCounter : public QMainWindow
{
    Q_OBJECT

public:
    PeopleCounter(QWidget *parent = nullptr);
    ~PeopleCounter();
    PointCloundWidget* pointWidget = nullptr;
    DrawingWidget* pointWidget_2 = nullptr;
    QPushButton* button = nullptr;
    QPushButton* button_2 = nullptr;
    UartThread* radar = nullptr;
    PureCppThread* pureThread = nullptr;
    QComboBox* comboBox = nullptr;
    QComboBox* comboBox_2 = nullptr;

private:
    Ui::PeopleCounterClass ui;
    void resizeEvent(QResizeEvent* event);
    bool resized;
    QMutex lock;
    QTimer* timer = nullptr;
    QVector<DataPoint> deepCopy(const QVector<DataPoint>& original);

public slots:

private slots:
    void startRadar();
    void updateClusterPoints(const  QVector<DataPoint> dataPoints);
    void updateTargetPoints(TargetQueue* targetQueue);
    void updateTargetPointsWithCluster(TargetQueue* targetQueue);
    void clearSecondWindow();
    void onComBoBoxSelectionSelection(int index);
    void writeFrame(QByteArray frame);
    void updataQueueLength(int length);
    void updateTargetNum(int num);
    void updateBaudrate(int baud);
};
