#ifndef CURVEDIALOG_H
#define CURVEDIALOG_H
#include <QDialog>
#include <QVector>
#include <QCheckBox>
#include "qcustomplot.h"
#include <QDateTime>

class CurveDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CurveDialog(QWidget *parent = nullptr);
    ~CurveDialog();

public slots:
    void appendData(const QVector<double> &values);

private slots:
    void onCheckBoxToggled(int index, bool checked);
    void onMouseMove(QMouseEvent *event);

private:
    void setupUI();

    const int MAX_SENSOR_COUNT = 10;
    const int m_maxPoints = 600;

    QStringList m_sensorNames;
    QList<QColor> m_sensorColors;

    // 数据缓存
    QVector<QVector<double>> m_dataPool;
    QVector<QDateTime> m_timePool;
    QDateTime m_startTime; // 曲线启动时间，用于计算相对秒数

    QCustomPlot *m_customPlot;
    QList<QCheckBox*> m_checkBoxes;
};

#endif // CURVEDIALOG_H