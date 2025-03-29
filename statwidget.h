#ifndef STATWIDGET_H
#define STATWIDGET_H

/*#include "qboxlayout.h"*/
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
class QPushButton;
class StatWidget : public QWidget {
  Q_OBJECT

public:
  explicit StatWidget(QString name, QWidget *parent = nullptr);
  void setValue(int value);
  void setText(const QString &text);
  bool eventFilter(QObject *obj, QEvent *event) override;

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
private slots:
  void ensureVisibility(QWidget *old, QWidget *now);

private:
  QLabel *label;
  QProgressBar *progress;
  QPoint dragPosition;
  QTimer updateTimer;
  QString widgetName;
  QPushButton *closeButton;
  void openSettings();
  void updateCPUUsage();
  void updateRAMUsage();
  void updateDiskUsage();
  void updateBatteryStatus();
};

#endif
