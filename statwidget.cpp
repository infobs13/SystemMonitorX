#include "statwidget.h"
#include "qdebug.h"
#include "qglobal.h"
#include "qnamespace.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStorageInfo>
#include <QString>
#include <QTextStream>
#include <QTimer>
StatWidget::StatWidget(QString name, QWidget *parent)
    : QWidget(parent), widgetName(name) {
  /*setWindowFlags(Qt::FramelessWindowHint | Qt::Tool |
   * Qt::WindowStaysOnTopHint);*/
  /*setAttribute(Qt::WA_TranslucentBackground);*/
  /*setAttribute(Qt::WA_ShowWithoutActivating);*/
  /*setFixedSize(200, 50);*/
  setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
  setAutoFillBackground(true);
  setMinimumSize(200, 50);
  setStyleSheet("background-color: rgba(0, 0, 0, 150);");

  installEventFilter(this);
  connect(qApp, &QApplication::focusChanged, this,
          &StatWidget::ensureVisibility);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 10, 10, 10);

  label = new QLabel(name, this);
  label->setWordWrap(true);
  label->setStyleSheet("font-size: 16px; color: white;");
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  layout->addWidget(label);

  progress = new QProgressBar(this);
  progress->setFixedHeight(5);
  progress->setStyleSheet(
      "QProgressBar { background: #111; height: 5px; border-radius: 2px; }"
      "QProgressBar::chunk { background: lightgrey; border-radius: 2px; }");
  progress->setTextVisible(false);
  layout->addWidget(progress);

  if (name == "CPU") {
    connect(&updateTimer, &QTimer::timeout, this, &StatWidget::updateCPUUsage);
    updateTimer.start(1000); // Update every second
  }
  if (name == "RAM") {
    connect(&updateTimer, &QTimer::timeout, this, &StatWidget::updateRAMUsage);
    updateTimer.start(1000);
  }
  if (name == "Battery") {
    connect(&updateTimer, &QTimer::timeout, this,
            &StatWidget::updateBatteryStatus);
    updateTimer.start(3000);
  }
  if (name == "Disk") {
    connect(&updateTimer, &QTimer::timeout, this, &StatWidget::updateDiskUsage);
    updateTimer.start(3000);
  }
}

void StatWidget::setValue(int value) { progress->setValue(value); }

void StatWidget::setText(const QString &text) { label->setText(text); }
void StatWidget::ensureVisibility(QWidget *old, QWidget *now) {
  Q_UNUSED(old);
  Q_UNUSED(now);
  if (!isVisible()) {
    qDebug() << "Forcing" << widgetName << "to show";
    show();
  }
}
bool StatWidget::eventFilter(QObject *obj, QEvent *event) {
  if (obj == this) {
    switch (event->type()) {
    case QEvent::Hide:
      qDebug() << "Widget" << widgetName << "hidden unexpectedly";
      break;
    case QEvent::Show:
      qDebug() << "Widget" << widgetName << "shown";
      break;
    case QEvent::WindowDeactivate:
      qDebug() << "Widget" << widgetName << "deactivated";
      break;
    default:
      break;
    }
  }
  return QWidget::eventFilter(obj, event);
}

void StatWidget::updateCPUUsage() {
  static qint64 prevIdle = 0, prevTotal = 0;

  QFile file("/proc/stat");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream in(&file);
  QString line = in.readLine();
  file.close();

  QStringList values = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  if (values.size() < 8)
    return;

  qint64 user = values[1].toLongLong();
  qint64 nice = values[2].toLongLong();
  qint64 system = values[3].toLongLong();
  qint64 idle = values[4].toLongLong();
  qint64 iowait = values[5].toLongLong();
  qint64 irq = values[6].toLongLong();
  qint64 softirq = values[7].toLongLong();

  qint64 total = user + nice + system + idle + iowait + irq + softirq;
  qint64 totalDiff = total - prevTotal;
  qint64 idleDiff = idle - prevIdle;

  if (totalDiff > 0) {
    double cpuUsage = 100.0 * (totalDiff - idleDiff) / totalDiff;
    setValue(cpuUsage * 100);
    progress->setMaximum(10000);
    setText(widgetName + ": " + QString::number(cpuUsage, 'f', 2) +
            "%"); // 2 decimal places
  }

  prevTotal = total;
  prevIdle = idle;
}

void StatWidget::updateRAMUsage() {
  QFile file("/proc/meminfo");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }
  QTextStream in(&file);
  qint64 memTotal = 0, memAvailable = 0;
  do {
    QString line = in.readLine();
    if (line.startsWith("MemTotal:")) {
      memTotal = line.split(" ", Qt::SkipEmptyParts)[1].toLongLong();
    } else if (line.startsWith("MemAvailable:")) {
      memAvailable = line.split(" ", Qt::SkipEmptyParts)[1].toLongLong();
    }
  } while (!in.atEnd());
  file.close();
  if (memTotal > 0) {
    double ramUsage = 100.0 * (memTotal - memAvailable) / memTotal;
    setValue(ramUsage);
    setText(widgetName + ": " + QString::number(ramUsage, 'f', 2) + "%");
  }
}
void StatWidget::updateBatteryStatus() {
  QStringList batteryPaths = {"/sys/class/power_supply/BAT0",
                              "/sys/class/power_supply/BAT1"};
  QString batteryPath;

  for (const QString &path : batteryPaths) {
    if (QFile::exists(path)) {
      batteryPath = path;
      break;
    }
  }

  if (batteryPath.isEmpty()) {
    setText("Battery: Not Found");
    setValue(0);
    return;
  }

  QFile capacityFile(batteryPath + "/capacity");
  if (!capacityFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setText("Battery: Error");
    return;
  }
  QTextStream in(&capacityFile);
  int batteryLevel = in.readLine().toInt();
  capacityFile.close();

  QFile statusFile(batteryPath + "/status");
  QString statusText = "Unknown";
  if (statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream statusStream(&statusFile);
    statusText = statusStream.readLine();
    statusFile.close();
  }
  QString color;
  if (statusText == "Charging") {
    color = "green";
  } else if (batteryLevel < 20) {
    color = "red";
  } else {
    color = "lightgrey";
  }
  progress->setStyleSheet(
      QString(
          "QProgressBar { background: #111; height: 5px; border-radius: 2px; }"
          "QProgressBar::chunk { background: %1; border-radius: 2px; }")
          .arg(color));
  setValue(batteryLevel);
  setText(QString("Battery: %1%\n(%2)").arg(batteryLevel).arg(statusText));
}
#include <QStorageInfo>

void StatWidget::updateDiskUsage() {
  QStorageInfo storage = QStorageInfo::root();

  if (!storage.isValid() || !storage.isReady()) {
    setText("Disk: Error");
    setValue(0);
    return;
  }

  qint64 total = storage.bytesTotal();
  qint64 free = storage.bytesAvailable();
  qint64 used = total - free;

  if (total > 0) {
    double usage = 100.0 * used / total;
    setValue(usage);
    setText(QString("Disk: %1% Used").arg(usage, 0, 'f', 2));
  }
}

void StatWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    dragPosition = event->globalPos() - frameGeometry().topLeft();
    event->accept();
  }
}

void StatWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    move(event->globalPos() - dragPosition);
    event->accept();
  }
}
