#include "mainwindow.h"
#include "thresholddialog.h"
#include "historydialog.h"
#include "usermanagerdialog.h"
#include "curvedialog.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QGroupBox>
#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QComboBox>
#include <QLineEdit>

MainWindow::MainWindow(const QString &username, int role, QWidget *parent)
    : QMainWindow(parent),
    m_username(username),
    m_userRole(role),
    m_curveDialog(nullptr)
{
    m_isAdmin = (role == 0);
    m_db = new DatabaseHelper(this);
    if (!m_db->initDatabase())
        QMessageBox::critical(this, "错误", "数据库初始化失败");
    m_db->loadThresholds(m_lowTh, m_highTh);
    m_modbus = new ModbusManager(this);
    m_alarm = new AlarmManager(m_db, this);
    m_alarm->setThresholds(m_lowTh, m_highTh);

    // 信号绑定
    connect(m_modbus, &ModbusManager::dataReceived, this, &MainWindow::onDataReceived);
    connect(m_modbus, &ModbusManager::connectionStatusChanged, this, &MainWindow::onConnectionChanged);
    connect(m_modbus, &ModbusManager::errorOccurred, this, &MainWindow::onError);
    connect(m_alarm, &AlarmManager::alarmTriggered, this, &MainWindow::onAlarm);

    initUI();

    // 系统时间定时器
    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, &QTimer::timeout, this, &MainWindow::onRefreshTime);
    m_timeTimer->start(1000);
    onRefreshTime();

    statusBar()->showMessage("请选择连接方式并手动连接设备");
}

MainWindow::~MainWindow()
{
    m_modbus->disconnectDevice();
}

void MainWindow::initUI()
{
    setWindowTitle(QString("工业空压机物联网测控系统 - 用户: %1").arg(m_username));
    resize(1200, 700);
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ========== 顶部信息栏 ==========
    QHBoxLayout *topBar = new QHBoxLayout;
    QLabel *titleLabel = new QLabel("工业空压机实时监控");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    m_timeLabel = new QLabel;
    topBar->addWidget(titleLabel);
    topBar->addStretch();
    topBar->addWidget(m_timeLabel);
    mainLayout->addLayout(topBar);

    // ========== Modbus 连接配置区域 ==========
    QGroupBox *connGroup = new QGroupBox("Modbus 连接配置");
    QHBoxLayout *connLayout = new QHBoxLayout(connGroup);
    connLayout->addWidget(new QLabel("连接类型:"));
    m_cmbConnectType = new QComboBox;
    m_cmbConnectType->addItem("串口 RTU", (int)ModbusConnectType::SerialRTU);
    m_cmbConnectType->addItem("Modbus TCP", (int)ModbusConnectType::TcpIP);
    connLayout->addWidget(m_cmbConnectType);

    connLayout->addWidget(new QLabel("串口号:"));
    m_cmbSerialPort = new QComboBox;
    for(int i = 1; i <= 9; ++i)
    {
        m_cmbSerialPort->addItem(QString("COM%1").arg(i));
    }
    m_cmbSerialPort->setCurrentText("COM1");
    connLayout->addWidget(m_cmbSerialPort);

    connLayout->addWidget(new QLabel("TCP地址:"));
    m_edtTcpAddr = new QLineEdit("127.0.0.1");
    connLayout->addWidget(m_edtTcpAddr);

    connLayout->addWidget(new QLabel("端口:"));
    m_edtTcpPort = new QLineEdit("502");
    connLayout->addWidget(m_edtTcpPort);

    m_btnConnect = new QPushButton("连接设备");
    m_btnDisconnect = new QPushButton("断开连接");
    m_btnDisconnect->setEnabled(false);
    connLayout->addWidget(m_btnConnect);
    connLayout->addWidget(m_btnDisconnect);
    connLayout->addStretch();
    mainLayout->addWidget(connGroup);

    connect(m_cmbConnectType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onConnectTypeChanged);
    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onModbusConnect);
    connect(m_btnDisconnect, &QPushButton::clicked, this, &MainWindow::onModbusDisconnect);
    onConnectTypeChanged(0);

    // ========== 传感器实时数据（流程图背景 + 点位 + 中文标注） ==========
    QGroupBox *dataGroup = new QGroupBox("传感器实时数据");
    dataGroup->setObjectName("dataGroupBox");
    dataGroup->setStyleSheet(R"(
        #dataGroupBox {
            border-image: url(:/res/main_bg.png) stretch;
            border: none;
        }
    )");
    dataGroup->setFixedSize(1000, 450);

    // 1. 排气压力
    QLabel *labName1 = new QLabel("排气压力", dataGroup);
    labName1->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName1->setGeometry(820, 180, 60, 15);
    lbl_pressure_out = new QLabel("0.00 kPa", dataGroup);
    lbl_pressure_out->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_pressure_out->setGeometry(820, 200, 90, 26);

    // 2. 排气温度
    QLabel *labName2 = new QLabel("排气温度", dataGroup);
    labName2->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName2->setGeometry(660, 130, 60, 15);
    lbl_temp_out = new QLabel("0.00 ℃", dataGroup);
    lbl_temp_out->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_temp_out->setGeometry(660, 150, 90, 26);

    // 3. 润滑油温度
    QLabel *labName3 = new QLabel("润滑油温度", dataGroup);
    labName3->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName3->setGeometry(410, 230, 70, 15);
    lbl_temp_oil = new QLabel("0.00 ℃", dataGroup);
    lbl_temp_oil->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_temp_oil->setGeometry(410, 250, 90, 26);

    // 4. 电机电流
    QLabel *labName4 = new QLabel("电机电流", dataGroup);
    labName4->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName4->setGeometry(310, 280, 60, 15);
    lbl_current = new QLabel("0.00 A", dataGroup);
    lbl_current->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_current->setGeometry(310, 300, 90, 26);

    // 5. 电机电压
    QLabel *labName5 = new QLabel("电机电压", dataGroup);
    labName5->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName5->setGeometry(210, 280, 60, 15);
    lbl_voltage = new QLabel("0.00 V", dataGroup);
    lbl_voltage->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_voltage->setGeometry(210, 300, 90, 26);

    // 6. 储气罐压力
    QLabel *labName6 = new QLabel("储气罐压力", dataGroup);
    labName6->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName6->setGeometry(510, 80, 70, 15);
    lbl_pressure_tank = new QLabel("0.00 kPa", dataGroup);
    lbl_pressure_tank->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_pressure_tank->setGeometry(510, 100, 90, 26);

    // 7. 空气入口温度
    QLabel *labName7 = new QLabel("入口温度", dataGroup);
    labName7->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName7->setGeometry(110, 130, 60, 15);
    lbl_temp_in = new QLabel("0.00 ℃", dataGroup);
    lbl_temp_in->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_temp_in->setGeometry(110, 150, 90, 26);

    // 8. 振动加速度
    QLabel *labName8 = new QLabel("振动加速度", dataGroup);
    labName8->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName8->setGeometry(360, 160, 70, 15);
    lbl_vibration = new QLabel("0.00 mm/s", dataGroup);
    lbl_vibration->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_vibration->setGeometry(360, 180, 100, 26);

    // 9. 运行状态
    QLabel *labName9 = new QLabel("运行状态", dataGroup);
    labName9->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName9->setGeometry(560, 180, 60, 15);
    lbl_status = new QLabel(dataGroup);
    lbl_status->setStyleSheet("background-color: green; border-radius: 10px;");
    lbl_status->setGeometry(560, 200, 20, 20);

    // 10. 报警状态
    QLabel *labName10 = new QLabel("报警状态", dataGroup);
    labName10->setStyleSheet("color: black; font-size: 12px; font-weight: bold;");
    labName10->setGeometry(710, 280, 60, 15);
    lbl_alarm = new QLabel("正常", dataGroup);
    lbl_alarm->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    lbl_alarm->setGeometry(710, 300, 65, 26);

    mainLayout->addWidget(dataGroup);

    // ========== 底部功能按钮栏 ==========
    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *btnThreshold = new QPushButton("阈值设置");
    QPushButton *btnHistory = new QPushButton("历史数据");
    QPushButton *btnPrint = new QPushButton("打印报表");
    QPushButton *btnUserMgr = new QPushButton("用户管理");
    QPushButton *btnCurve = new QPushButton("曲线窗口");
    QPushButton *btnExit = new QPushButton("退出系统");
    btnLayout->addStretch();
    btnLayout->addWidget(btnThreshold);
    btnLayout->addWidget(btnHistory);
    btnLayout->addWidget(btnPrint);
    btnLayout->addWidget(btnUserMgr);
    btnLayout->addWidget(btnCurve);
    btnLayout->addWidget(btnExit);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // 权限控制
    if (!m_isAdmin)
    {
        btnThreshold->setEnabled(false);
        btnThreshold->setToolTip("普通用户无权修改阈值");
        btnUserMgr->setEnabled(false);
        btnUserMgr->setToolTip("普通用户无权管理用户");
    }

    // 绑定按钮信号
    connect(btnThreshold, &QPushButton::clicked, this, &MainWindow::onThresholdSettings);
    connect(btnHistory, &QPushButton::clicked, this, &MainWindow::onHistoryQuery);
    connect(btnPrint, &QPushButton::clicked, this, &MainWindow::onPrintReport);
    connect(btnUserMgr, &QPushButton::clicked, this, &MainWindow::onUserManagement);
    connect(btnCurve, &QPushButton::clicked, this, &MainWindow::onOpenCurveWindow);
    connect(btnExit, &QPushButton::clicked, this, &QMainWindow::close);

    statusBar()->showMessage("就绪");
}

// 切换连接类型
void MainWindow::onConnectTypeChanged(int idx)
{
    int type = m_cmbConnectType->itemData(idx).toInt();
    if((ModbusConnectType)type == ModbusConnectType::SerialRTU)
    {
        m_cmbSerialPort->setEnabled(true);
        m_edtTcpAddr->setEnabled(false);
        m_edtTcpPort->setEnabled(false);
    }
    else
    {
        m_cmbSerialPort->setEnabled(false);
        m_edtTcpAddr->setEnabled(true);
        m_edtTcpPort->setEnabled(true);
    }
}

// 连接设备（修复 idx 未定义问题）
void MainWindow::onModbusConnect()
{
    // 获取当前下拉框选中索引
    int curIdx = m_cmbConnectType->currentIndex();
    int type = m_cmbConnectType->itemData(curIdx).toInt();

    QString addr;
    int port = 502;
    int baud = 9600;
    if((ModbusConnectType)type == ModbusConnectType::SerialRTU)
    {
        addr = m_cmbSerialPort->currentText();
    }
    else
    {
        addr = m_edtTcpAddr->text().trimmed();
        port = m_edtTcpPort->text().toInt();
    }
    bool ret = m_modbus->connectDevice((ModbusConnectType)type, addr, port, baud);
    if(ret)
    {
        m_btnConnect->setEnabled(false);
        m_btnDisconnect->setEnabled(true);
    }
}

// 断开设备
void MainWindow::onModbusDisconnect()
{
    m_modbus->disconnectDevice();
    m_btnConnect->setEnabled(true);
    m_btnDisconnect->setEnabled(false);
}

// 接收Modbus数据
void MainWindow::onDataReceived(const QVector<quint16> &rawValues)
{
    if (rawValues.size() != 10) return;
    QVector<double> scaled(10);
    for (int i = 0; i < 10; ++i) scaled[i] = static_cast<double>(rawValues[i]);

    m_db->insertSensorData(scaled);
    m_alarm->checkAlarms(scaled);
    updateDisplay(scaled);

    if (m_curveDialog)
    {
        m_curveDialog->appendData(scaled);
    }
}

// 更新界面所有数值、状态
void MainWindow::updateDisplay(const QVector<double> &values)
{
    lbl_pressure_out->setText(QString::number(values[0], 'f', 2) + " kPa");
    lbl_temp_out->setText(QString::number(values[1], 'f', 2) + " ℃");
    lbl_temp_oil->setText(QString::number(values[2], 'f', 2) + " ℃");
    lbl_current->setText(QString::number(values[3], 'f', 2) + " A");
    lbl_voltage->setText(QString::number(values[4], 'f', 2) + " V");
    lbl_pressure_tank->setText(QString::number(values[5], 'f', 2) + " kPa");
    lbl_temp_in->setText(QString::number(values[6], 'f', 2) + " ℃");
    lbl_vibration->setText(QString::number(values[7], 'f', 2) + " mm/s");

    // 运行状态灯
    if (values[8] > 0)
        lbl_status->setStyleSheet("background-color: green; border-radius: 10px;");
    else
        lbl_status->setStyleSheet("background-color: red; border-radius: 10px;");

    // 报警状态
    bool hasAlarm = (values[9] > 0);
    if(!hasAlarm)
    {
        lbl_alarm->setText("正常");
        lbl_alarm->setStyleSheet("background-color: rgba(255,255,255,180); color: black; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    }
    else
    {
        lbl_alarm->setText("报警");
        lbl_alarm->setStyleSheet("background-color: rgba(255,255,255,180); color: red; padding: 3px 8px; border-radius: 4px; font-weight: bold;");
    }
}

void MainWindow::onConnectionChanged(bool connected)
{
    if(connected)
        statusBar()->showMessage("Modbus 设备已成功连接");
    else
        statusBar()->showMessage("Modbus 连接已断开");
}

void MainWindow::onError(const QString &error)
{
    statusBar()->showMessage("错误: " + error);
    m_btnConnect->setEnabled(true);
    m_btnDisconnect->setEnabled(false);
}

// 报警弹窗
void MainWindow::onAlarm(const QString &name, double value, double low, double high)
{
    QMessageBox::warning(this, "参数超限报警", QString("%1 超出阈值！当前值: %2 阈值范围: %3 ~ %4").arg(name).arg(value).arg(low).arg(high));
}

// 阈值设置
void MainWindow::onThresholdSettings()
{
    if (!m_isAdmin)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以修改阈值");
        return;
    }
    ThresholdDialog dlg(m_lowTh, m_highTh, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        m_lowTh = dlg.getLowLimits();
        m_highTh = dlg.getHighLimits();
        m_alarm->setThresholds(m_lowTh, m_highTh);
        for (int i = 0; i < 10; ++i)
        {
            m_db->saveThreshold(i, m_lowTh[i], m_highTh[i]);
        }
    }
}

void MainWindow::onHistoryQuery()
{
    HistoryDialog dlg(this);
    dlg.exec();
}

void MainWindow::onRefreshTime()
{
    m_timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void MainWindow::onPrintReport()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString html = "<html><head><title>空压机实时数据报表</title></head><body>";
        html += "<h1>工业空压机运行数据</h1>";
        html += "<p>生成时间：" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
        html += "</body></html>";
        QTextDocument doc;
        doc.setHtml(html);
        doc.print(&printer);
    }
}

void MainWindow::onUserManagement()
{
    if (!m_isAdmin)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以管理用户");
        return;
    }
    UserManagerDialog dlg(this);
    dlg.exec();
}

void MainWindow::onOpenCurveWindow()
{
    if (!m_curveDialog)
    {
        m_curveDialog = new CurveDialog(this);
        connect(m_curveDialog, &CurveDialog::destroyed, this, [this]()
                {
                    m_curveDialog = nullptr;
                });
    }
    m_curveDialog->show();
    m_curveDialog->raise();
    m_curveDialog->activateWindow();
}