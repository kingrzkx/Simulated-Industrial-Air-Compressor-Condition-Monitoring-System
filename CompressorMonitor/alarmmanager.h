#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <QObject>
#include <QVector>

class DatabaseHelper;

class AlarmManager : public QObject
{
    Q_OBJECT
public:
    explicit AlarmManager(DatabaseHelper *dbHelper, QObject *parent = nullptr);
    void setThresholds(const QVector<double> &low, const QVector<double> &high);
    void checkAlarms(const QVector<double> &values);

signals:
    void alarmTriggered(const QString &sensorName, double value, double low, double high);

private:
    DatabaseHelper *m_dbHelper;
    QVector<double> m_lowLimits;
    QVector<double> m_highLimits;
    QVector<bool> m_alarmActive;
};

#endif // ALARMMANAGER_H