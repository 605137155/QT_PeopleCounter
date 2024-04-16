#include "PeopleCounterWindow.h"
#include <QGraphicsView>
#include <QSerialPortInfo>

PeopleCounter::PeopleCounter(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    
    //PointWidget配置
    pointWidget = new PointCloundWidget(ui.centralWidget);
    pointWidget->setGeometry(ui.widget->geometry());
    pointWidget->setObjectName(ui.widget->objectName());
    pointWidget_2 = new DrawingWidget(this);
    pointWidget_2->setGeometry(ui.widget_2->geometry());
    pointWidget_2->setObjectName(ui.widget_2->objectName());
    QWidget* p = ui.widget;
    ui.widget = pointWidget;
    p->deleteLater();
    QWidget* p2 = ui.widget_2;
    ui.widget_2 = pointWidget_2;
    p2->deleteLater();
    int stretchWindow = 12;
    int stretchSpace = 1;
    int stretchText = 8;
    ui.horizontalLayout_2->removeWidget(ui.widget);
    ui.horizontalLayout_2->removeWidget(ui.widget_2);
    ui.horizontalLayout_2->removeItem(ui.verticalLayout);
    ui.horizontalLayout_2->addWidget(ui.widget,1);
    ui.horizontalLayout_2->addWidget(ui.widget_2, 2);
    ui.horizontalLayout_2->addItem(ui.verticalLayout);
    ui.horizontalLayout_2->setStretch(2, 10);
    ui.horizontalLayout_2->setStretch(3, 10);
    ui.horizontalLayout_2->setStretch(4, 5);
    ui.widget_2->setStyleSheet("background-color: rgb(255, 255, 255);");

    //UartThread线程配置
    radar = new UartThread(this);
    connect(radar, &UartThread::clusterReceived, this, &PeopleCounter::updateClusterPoints);
    connect(radar, &UartThread::targetReceived, this, &PeopleCounter::updateTargetPoints);
    connect(radar, &UartThread::targetReceived, this, &PeopleCounter::updateTargetPointsWithCluster);
    connect(pointWidget_2, &DrawingWidget::sendFrame, this, &PeopleCounter::writeFrame);
    connect(radar, &UartThread::sendTargetNum, this, &PeopleCounter::updateTargetNum);

    //PureCppThread线程配置
    /*pureThread = new PureCppThread(this);
    connect(pureThread, &PureCppThread::targetReceived, this, &PeopleCounter::updateTargetPoints);
    connect(pureThread, &PureCppThread::targetReceived, this, &PeopleCounter::updateTargetPointsWithCluster);*/
    //pureThread->setPriority(QThread::HighestPriority);
    
    //左上角图标
    ui.label->setPixmap(QPixmap("images/andar_logo.png").scaled(QSize(120,90), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui.label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resized = false;

    //button配置
    button = ui.pushButton;
    connect(this->button, SIGNAL(clicked()), this, SLOT(startRadar()));
    button_2 = ui.pushButton_2;
    connect(this->button_2, SIGNAL(clicked()), this, SLOT(clearSecondWindow()));
    
    //定时器
    timer = new QTimer(this);

    //comboBox
    comboBox = ui.comboBox;
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& serialPortInfo : serialPortInfos) {
        comboBox->addItem(serialPortInfo.portName());
    }
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComBoBoxSelectionSelection(int)));

    //comboBox_2波特率
    comboBox_2 = ui.comboBox_2;
    comboBox_2->addItem("115200", QSerialPort::Baud115200);
    comboBox_2->addItem("1382400", 1382400);
    connect(comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBaudrate(int)));

    //滑动条改动队列长度
    ui.horizontalSlider->setMinimum(1);
    ui.horizontalSlider->setMaximum(30);
    ui.horizontalSlider->setValue(10);
    connect(ui.horizontalSlider, &QSlider::valueChanged, this, &PeopleCounter::updataQueueLength);

}

PeopleCounter::~PeopleCounter()
{
    if (radar->isRunning()) {
        radar->quit();
        radar->wait();
    }
}




void PeopleCounter::startRadar() {
    
    if (ui.pushButton->text() == "开始") {
        radar->start();
        //pureThread->start();
        ui.pushButton->setText(QString::fromUtf8("点击暂停"));
    }  

    else
    {
        ui.pushButton->setText(QString::fromUtf8("开始"));
        radar->requestInterruption();
        radar->wait();
        //pureThread->requestInterruption();
        //pureThread->wait();
    }
}

void PeopleCounter::clearSecondWindow(){
    
    pointWidget_2->clearWindow();

}


void PeopleCounter::updateClusterPoints(const  QVector<DataPoint> dataPoints) {
    QVector<DataPoint> a = deepCopy(dataPoints);
    lock.lock();
    pointWidget->pointCloundReady(a);
    //pointWidget_2->pointCloundReady(QVector<DataPoint>());
    qDebug() << "cluster槽函数处的a点的大小" << a.size();
    lock.unlock();
   
}

void PeopleCounter::updateTargetPoints(TargetQueue * targetQueue ) {
    qDebug() << "运行到里了3";
    pointWidget_2->pointCloundReady(targetQueue);
    //pointWidget->pointTargetReady(targetQueue);
}

void PeopleCounter::updateTargetPointsWithCluster(TargetQueue* targetQueue) {
    qDebug() << "运行到里了4";
    lock.lock();
    pointWidget->pointTargetReady(targetQueue);
    lock.unlock();
}





void PeopleCounter::resizeEvent(QResizeEvent* event) {
    QPixmap pixmap("images/andar_logo.png");
    if (!pixmap.isNull() && resized == true) {
        int w = ui.pushButton->width();
        int h = ui.pushButton->height();
        //QSize a(w, h);
        ui.label->setPixmap(pixmap.scaled(QSize(w, h), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qDebug() << w << h << "ssss" << ui.pushButton->size().width() << ui.pushButton->size().height();
    }
    else {
        resized = true;
        //QSize a(w, h);
        ui.label->setPixmap(pixmap.scaled(QSize(120, 120*1.209), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
    


QVector<DataPoint> PeopleCounter::deepCopy(const QVector<DataPoint>& original) {
    QVector<DataPoint> copy;
    for (const DataPoint& item : original) {
        copy.append(item);
    }
    return copy;
}


void PeopleCounter::onComBoBoxSelectionSelection(int index) {
    QString selectedPortName = comboBox->itemText(index);
    radar->setPortName(selectedPortName);

}


void PeopleCounter::writeFrame(QByteArray rectFrame) {
    radar->writeFrame(rectFrame);

}

void PeopleCounter::updataQueueLength(int length) {
    radar->updateQueue(length);
    pointWidget->updataQueueSize(length);
    pointWidget_2->updataQueueSize(length);
}


void PeopleCounter::updateTargetNum(int num){
    QString a = "人数:" + QString::number(num);
    ui.label_3->setText(a);
}

void PeopleCounter::updateBaudrate(int baud) {
    if (baud == 0)
        baud = 115200;
    else if (baud == 1)
        baud = 1382400;
    radar->updateBaud(baud);
}