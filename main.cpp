#include "qglobal.h"
#include "qpointer.h"
#include "statwidget.h"
#include <QApplication>
#include <QDebug>
#include <QPointer>
#include <QScreen>
int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QList<QPointer<StatWidget>> widgets;

  QRect screenGeometry = QApplication::primaryScreen()->geometry();
  int screenWidth = screenGeometry.width();
  int startX = screenWidth - 210;
  int startY = 100;
  widgets.append(new StatWidget("CPU"));
  widgets.append(new StatWidget("RAM"));
  widgets.append(new StatWidget("Disk"));
  widgets.append(new StatWidget("Battery"));
  for (int i = 0; i < widgets.size(); ++i) {
    widgets[i]->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    widgets[i]->move(startX, startY + (i * 70));
    widgets[i]->setAttribute(Qt::WA_DeleteOnClose);
    widgets[i]->show();
  }
  QObject::connect(&app, &QApplication::aboutToQuit,
                   [&]() { widgets.clear(); });

  return app.exec();
}
