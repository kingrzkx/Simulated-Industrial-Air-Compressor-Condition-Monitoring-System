#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include "loginwidget.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 在登录前先初始化数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("compressor_data.db");
    if (!db.open()) {
        QMessageBox::critical(nullptr, "数据库错误",
                              "无法打开数据库文件:\n" + db.lastError().text());
        return -1;
    }

    // 创建表（如果不存在）
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS sensor_data ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "exhaust_pressure REAL, exhaust_temperature REAL, "
               "lube_oil_temperature REAL, motor_current REAL, "
               "motor_voltage REAL, tank_pressure REAL, "
               "air_inlet_temperature REAL, vibration REAL, "
               "running_status INTEGER, alarm_status INTEGER)");

    query.exec("CREATE TABLE IF NOT EXISTS alarm_records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "sensor_name VARCHAR(50), sensor_value REAL, "
               "threshold_low REAL, threshold_high REAL, alarm_type VARCHAR(20))");

    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username VARCHAR(50) UNIQUE, password VARCHAR(50), "
               "role INTEGER, fullname VARCHAR(50))");
    query.exec("INSERT OR IGNORE INTO users (username, password, role, fullname) "
               "VALUES ('admin', 'admin123', 0, '管理员')");
    query.exec("INSERT OR IGNORE INTO users (username, password, role, fullname) "
               "VALUES ('user', 'user123', 1, '普通用户')");

    query.exec("CREATE TABLE IF NOT EXISTS alarm_thresholds ("
               "sensor_id INTEGER PRIMARY KEY, "
               "sensor_name VARCHAR(50), low_limit REAL, high_limit REAL)");

    // 初始化阈值（如果为空）
    query.exec("SELECT COUNT(*) FROM alarm_thresholds");
    if (query.next() && query.value(0).toInt() == 0) {
        QStringList names = {"排气压力","排气温度","润滑油温度","电机电流","电机电压",
                             "储气罐压力","空气入口温度","振动加速度","运行状态","报警状态"};
        QVector<double> lows = {400,30,20,30,340,350,-10,0,0,0};
        QVector<double> highs = {800,100,85,100,440,850,45,7.5,1,1};
        for (int i = 0; i < 10; ++i) {
            query.prepare("INSERT INTO alarm_thresholds (sensor_id, sensor_name, low_limit, high_limit) "
                          "VALUES (?, ?, ?, ?)");
            query.addBindValue(i);
            query.addBindValue(names[i]);
            query.addBindValue(lows[i]);
            query.addBindValue(highs[i]);
            query.exec();
        }
    }

    LoginWidget login;
    if (login.exec() == QDialog::Accepted) {
        MainWindow w(login.getUsername(), login.getRole());
        w.show();
        return a.exec();
    }
    return 0;
}