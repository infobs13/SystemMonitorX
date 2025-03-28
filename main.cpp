#include "qglobal.h"
#include "statwidget.h"
#include <QApplication>
#include <QDebug>
#include <QScreen>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QRect screenGeometry = QApplication::primaryScreen()->geometry();
  int screenWidth = screenGeometry.width();
  int startX = screenWidth - 210;
  int startY = 100;
  qDebug() << "creating widgets ";
  StatWidget *cpu = new StatWidget("CPU");
  StatWidget *ram = new StatWidget("RAM");
  StatWidget *disk = new StatWidget("Disk");
  StatWidget *battery = new StatWidget("Battery");

  qDebug() << "cpu wid : " << cpu;
  qDebug() << "ram wid : " << ram;
  qDebug() << "disk wid : " << disk;
  qDebug() << "battery wid : " << battery;
  cpu->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                      Qt::WindowStaysOnTopHint);
  ram->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                      Qt::WindowStaysOnTopHint);
  disk->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                       Qt::WindowStaysOnTopHint);
  battery->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                          Qt::WindowStaysOnTopHint);

  cpu->move(startX, startY);
  cpu->show();

  ram->move(startX, startY + 70);
  ram->show();

  disk->move(startX, startY + 140);
  disk->show();

  battery->move(startX, startY + 210);
  battery->show();

  return app.exec();
}
