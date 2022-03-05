#include <QCoreApplication>
#include <QDebug>
#include "cmediacut.h"
#include "ch264tojpeg.h"
#include "cyuvtojpeg.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "hello world.";

//    CMediaCut cmc;
//    cmc.Cut(2, 5);

//    CH264toJPEG chj;
//    chj.Convert();

    CYUVtoJPEG cyj;
    cyj.Convert();

    return a.exec();
}
