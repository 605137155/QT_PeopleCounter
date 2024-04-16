#include "PeopleCounterWindow.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
#include "Datastruct.h"
#include <vld.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QVector<DataPoint>>("QVector<DataPoint>");
    PeopleCounter w;
    QIcon icon("./images/AIR.ico");
    w.setWindowIcon(icon);
    w.show();
    return a.exec();

    //≤‚ ‘
    /*QByteArray data = "0";
    quint32 dataLength = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(QByteArray::fromHex(data.mid(4, 8)).constData()));*/

}
