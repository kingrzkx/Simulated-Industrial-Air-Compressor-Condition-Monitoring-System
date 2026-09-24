#include "alarmmanager.h"
#include "databasehelper.h"
#include <QApplication>
#include <QDebug>

AlarmManager::AlarmManager(DatabaseHelper *dbHelper, QObject *parent)
    : QObject(parent), m_dbHelper(dbHelper)
{
    m_alarmActive.fill(false, 10);
}

void AlarmManager::setThresholds(const QVector<double> &low, const QVector<double> &high)
{
    m_lowLimits = low;
    m_highLimits = high;
}

void AlarmManager::checkAlarms(const QVector<double> &values)
{
    static QStringList names = {"排气压力","排气温度","润滑油温度","电机电流","电机电压",
                                "储气罐压力","空气入口温度","振动加速度","运行状态","报警状态"};
    for (int i = 0; i < 10 && i < values.size(); ++i) {
        bool isAlarm = (values[i] > m_highLimits[i] || values[i] < m_lowLimits[i]);
        if (isAlarm && !m_alarmActive[i]) {
            m_alarmActive[i] = true;
            emit alarmTriggered(names[i], values[i], m_lowLimits[i], m_highLimits[i]);
            if (m_dbHelper) {
                m_dbHelper->insertAlarmRecord(names[i], values[i], m_lowLimits[i], m_highLimits[i]);
            }
            QApplication::beep();
        } else if (!isAlarm && m_alarmActive[i]) {
            m_alarmActive[i] = false;
        }
    }
}