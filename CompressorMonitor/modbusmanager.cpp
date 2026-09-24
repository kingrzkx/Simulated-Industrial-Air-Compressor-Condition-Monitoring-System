#include "modbusmanager.h"
#include <QDebug>
#include <QSerialPort>
#include <QVariant>

ModbusManager::ModbusManager(QObject *parent)
    : QObject(parent),
    m_modbusClient(nullptr),
    m_isConnected(false),
    m_connectType(ModbusConnectType::SerialRTU)
{
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(1000);
    connect(m_pollTimer, &QTimer::timeout, this, &ModbusManager::pollData);
}

ModbusManager::~ModbusManager()
{
    disconnectDevice();
}

bool ModbusManager::connectDevice(ModbusConnectType type,
                                  const QString &addr,
                                  int port,
                                  int baudRate)
{
    // 先断开已有连接
    disconnectDevice();
    m_connectType = type;

    if (type == ModbusConnectType::SerialRTU)
    {
        // 串口RTU模式
        QModbusRtuSerialClient *rtuClient = new QModbusRtuSerialClient(this);
        rtuClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, QVariant(addr));
        rtuClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QVariant(baudRate));
        rtuClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QVariant(8));
        rtuClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QVariant(1));
        rtuClient->setConnectionParameter(QModbusDevice::SerialParityParameter, QVariant(QSerialPort::NoParity));
        m_modbusClient = rtuClient;
    }
    else
    {
        // Modbus TCP 模式，默认端口502
        QModbusTcpClient *tcpClient = new QModbusTcpClient(this);
        tcpClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, QVariant(port));
        tcpClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(addr));
        m_modbusClient = tcpClient;
    }

    // 通用超时、重试配置
    m_modbusClient->setTimeout(1000);
    m_modbusClient->setNumberOfRetries(2);

    // 信号绑定
    connect(m_modbusClient, &QModbusClient::stateChanged, this, &ModbusManager::onStateChanged);
    connect(m_modbusClient, &QModbusClient::errorOccurred, this, &ModbusManager::onErrorOccurred);

    if (!m_modbusClient->connectDevice())
    {
        emit errorOccurred("连接失败: " + m_modbusClient->errorString());
        return false;
    }
    return true;
}

void ModbusManager::disconnectDevice()
{
    if (m_modbusClient)
    {
        m_modbusClient->disconnectDevice();
        m_modbusClient->deleteLater();
        m_modbusClient = nullptr;
    }
    m_pollTimer->stop();
    m_isConnected = false;
}

bool ModbusManager::isConnected() const
{
    return m_isConnected;
}

void ModbusManager::pollData()
{
    if (!m_modbusClient || m_modbusClient->state() != QModbusDevice::ConnectedState)
        return;

    // 读取保持寄存器：起始地址0，共10个寄存器
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 10);
    if (auto *reply = m_modbusClient->sendReadRequest(readUnit, 1))
    {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, this, &ModbusManager::onReadReady);
        }
        else
        {
            delete reply;
        }
    }
}

void ModbusManager::onReadReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) return;

    if (reply->error() == QModbusDevice::NoError)
    {
        const QModbusDataUnit unit = reply->result();
        QVector<quint16> values;
        for (uint i = 0; i < unit.valueCount(); ++i)
        {
            values.append(unit.value(i));
        }
        emit dataReceived(values);
    }
    else
    {
        emit errorOccurred("读取错误: " + reply->errorString());
    }
    reply->deleteLater();
}

void ModbusManager::onStateChanged(QModbusDevice::State state)
{
    bool connected = (state == QModbusDevice::ConnectedState);
    if (m_isConnected != connected)
    {
        m_isConnected = connected;
        emit connectionStatusChanged(connected);
        if (connected)
        {
            m_pollTimer->start();
        }
        else
        {
            m_pollTimer->stop();
        }
    }
}

void ModbusManager::onErrorOccurred(QModbusDevice::Error error)
{
    if (error == QModbusDevice::NoError) return;
    emit errorOccurred(m_modbusClient->errorString());
}