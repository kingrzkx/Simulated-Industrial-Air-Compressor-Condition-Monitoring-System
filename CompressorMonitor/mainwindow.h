#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "modbusmanager.h"
#include "databasehelper.h"
#include "alarmmanager.h"

class QTableWidget;
class QLabel;
class QTimer;
class CurveDialog;
class QComboBox;
class QLineEdit;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &username, int role, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDataReceived(const QVector<quint16> &rawValues);
    void onConnectionChanged(bool connected);
    void onError(const QString &error);
    void onAlarm(const QString &name, double value, double low, double high);
    void onThresholdSettings();
    void onHistoryQuery();
    void onRefreshTime();
    void onPrintReport();
    void onUserManagement();
    void onOpenCurveWindow();
    // Modbus 连接相关槽函数
    void onModbusConnect();
    void onModbusDisconnect();
    void onConnectTypeChanged(int idx);

private:
    void initUI();
    void updateDisplay(const QVector<double> &values);

    // 核心业务对象
    ModbusManager *m_modbus;
    DatabaseHelper *m_db;
    AlarmManager *m_alarm;
    QVector<double> m_lowTh, m_highTh;

    // 原有UI控件
    QTableWidget *m_tableWidget;
    QLabel *m_timeLabel;
    QTimer *m_timeTimer;
    QString m_username;
    int m_userRole;
    bool m_isAdmin;
    CurveDialog *m_curveDialog;

    // Modbus 连接区域控件
    QComboBox *m_cmbConnectType;
    QComboBox *m_cmbSerialPort;
    QLineEdit *m_edtTcpAddr;
    QLineEdit *m_edtTcpPort;
    QPushButton *m_btnConnect;
    QPushButton *m_btnDisconnect;

    // ===================== 流程图上的传感器标签控件 =====================
    QLabel *lbl_pressure_out;    // 排气压力
    QLabel *lbl_temp_out;        // 排气温度
    QLabel *lbl_temp_oil;        // 润滑油温度
    QLabel *lbl_current;         // 电机电流
    QLabel *lbl_voltage;         // 电机电压
    QLabel *lbl_pressure_tank;   // 储气罐压力
    QLabel *lbl_temp_in;         // 空气入口温度
    QLabel *lbl_vibration;       // 振动加速度
    QLabel *lbl_status;          // 运行状态指示灯
    QLabel *lbl_alarm;           // 报警状态标签
};

#endif // MAINWINDOW_H