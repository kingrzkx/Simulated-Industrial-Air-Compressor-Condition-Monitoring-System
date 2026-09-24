#include "databasehelper.h"
#include <QDebug>
#include <QSqlError>

DatabaseHelper::DatabaseHelper(QObject *parent) : QObject(parent)
{
}

DatabaseHelper::~DatabaseHelper()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseHelper::initDatabase()
{
    if (!QSqlDatabase::contains()) {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName("compressor_data.db");
    } else {
        m_db = QSqlDatabase::database();
    }
    if (!m_db.isOpen()) {
        if (!m_db.open()) {
            qDebug() << "数据库打开失败:" << m_db.lastError().text();
            return false;
        }
    }

    QSqlQuery query(m_db);
    // 传感器数据表
    query.exec("CREATE TABLE IF NOT EXISTS sensor_data ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "exhaust_pressure REAL, exhaust_temperature REAL, "
               "lube_oil_temperature REAL, motor_current REAL, "
               "motor_voltage REAL, tank_pressure REAL, "
               "air_inlet_temperature REAL, vibration REAL, "
               "running_status INTEGER, alarm_status INTEGER)");

    // 报警记录表
    query.exec("CREATE TABLE IF NOT EXISTS alarm_records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "sensor_name VARCHAR(50), sensor_value REAL, "
               "threshold_low REAL, threshold_high REAL, alarm_type VARCHAR(20))");

    // 用户表
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username VARCHAR(50) UNIQUE, password VARCHAR(50), "
               "role INTEGER, fullname VARCHAR(50))");
    query.exec("INSERT OR IGNORE INTO users (username, password, role, fullname) "
               "VALUES ('admin', 'admin123', 0, '管理员')");
    query.exec("INSERT OR IGNORE INTO users (username, password, role, fullname) "
               "VALUES ('user', 'user123', 1, '普通用户')");

    // 报警阈值表
    query.exec("CREATE TABLE IF NOT EXISTS alarm_thresholds ("
               "sensor_id INTEGER PRIMARY KEY, "
               "sensor_name VARCHAR(50), low_limit REAL, high_limit REAL)");
    QStringList names = {"排气压力","排气温度","润滑油温度","电机电流","电机电压",
                         "储气罐压力","空气入口温度","振动加速度","运行状态","报警状态"};
    QVector<double> lows = {400,30,20,30,340,350,-10,0,0,0};
    QVector<double> highs = {800,100,85,100,440,850,45,7.5,1,1};
    for (int i = 0; i < 10; ++i) {
        query.prepare("INSERT OR IGNORE INTO alarm_thresholds (sensor_id, sensor_name, low_limit, high_limit) "
                      "VALUES (?, ?, ?, ?)");
        query.addBindValue(i);
        query.addBindValue(names[i]);
        query.addBindValue(lows[i]);
        query.addBindValue(highs[i]);
        query.exec();
    }
    return true;
}

bool DatabaseHelper::insertSensorData(const QVector<double> &values)
{
    if (values.size() < 10) return false;
    QSqlQuery query;
    query.prepare("INSERT INTO sensor_data ("
                  "exhaust_pressure, exhaust_temperature, lube_oil_temperature, "
                  "motor_current, motor_voltage, tank_pressure, air_inlet_temperature, "
                  "vibration, running_status, alarm_status) "
                  "VALUES (?,?,?,?,?,?,?,?,?,?)");
    for (int i = 0; i < 10; ++i)
        query.addBindValue(values[i]);
    return query.exec();
}

bool DatabaseHelper::insertAlarmRecord(const QString &sensorName, double value, double low, double high)
{
    QString type = (value > high) ? "HIGH" : "LOW";
    QSqlQuery query;
    query.prepare("INSERT INTO alarm_records (sensor_name, sensor_value, threshold_low, threshold_high, alarm_type) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(sensorName);
    query.addBindValue(value);
    query.addBindValue(low);
    query.addBindValue(high);
    query.addBindValue(type);
    return query.exec();
}

bool DatabaseHelper::loadThresholds(QVector<double> &lowLimits, QVector<double> &highLimits)
{
    lowLimits.fill(0, 10);
    highLimits.fill(0, 10);
    QSqlQuery query("SELECT sensor_id, low_limit, high_limit FROM alarm_thresholds ORDER BY sensor_id");
    while (query.next()) {
        int id = query.value(0).toInt();
        if (id >= 0 && id < 10) {
            lowLimits[id] = query.value(1).toDouble();
            highLimits[id] = query.value(2).toDouble();
        }
    }
    return true;
}

bool DatabaseHelper::saveThreshold(int sensorId, double low, double high)
{
    QSqlQuery query;
    query.prepare("UPDATE alarm_thresholds SET low_limit = :low, high_limit = :high WHERE sensor_id = :id");
    query.bindValue(":low", low);
    query.bindValue(":high", high);
    query.bindValue(":id", sensorId);
    return query.exec();
}