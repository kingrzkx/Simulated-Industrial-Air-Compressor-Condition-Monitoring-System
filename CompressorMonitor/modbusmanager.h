#ifndef MODBUSMANAGER_H
#define MODBUSMANAGER_H
#include <QObject>
#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QTimer>

// 连接类型枚举
enum class ModbusConnectType
{
    SerialRTU,  // 串口RTU
    TcpIP       // Modbus TCP
};

class ModbusManager : public QObject
{
    Q_OBJECT
public:
    explicit ModbusManager(QObject *parent = nullptr);
    ~ModbusManager();

    // 通用连接接口：区分串口/TCP
    bool connectDevice(ModbusConnectType type,
                       const QString &addr,    // 串口名(COM1~COM9) / TCP IP地址
                       int port = 502,        // TCP端口，串口时无效
                       int baudRate = 9600);  // 串口波特率，TCP时无效

    void disconnectDevice();
    bool isConnected() const;

signals:
    void dataReceived(const QVector<quint16> &values);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);

private slots:
    void pollData();
    void onReadReady();
    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);

private:
    QModbusClient *m_modbusClient;
    QTimer *m_pollTimer;
    bool m_isConnected;
    ModbusConnectType m_connectType;
};

#endif // MODBUSMANAGER_H