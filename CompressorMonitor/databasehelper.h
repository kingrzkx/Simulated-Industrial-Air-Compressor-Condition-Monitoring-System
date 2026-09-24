#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVector>

class DatabaseHelper : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseHelper(QObject *parent = nullptr);
    ~DatabaseHelper();
    bool initDatabase();
    bool insertSensorData(const QVector<double> &values);
    bool insertAlarmRecord(const QString &sensorName, double value, double low, double high);
    bool loadThresholds(QVector<double> &lowLimits, QVector<double> &highLimits);
    bool saveThreshold(int sensorId, double low, double high);
    // 可添加其他查询方法

private:
    QSqlDatabase m_db;
};

#endif // DATABASEHELPER_H