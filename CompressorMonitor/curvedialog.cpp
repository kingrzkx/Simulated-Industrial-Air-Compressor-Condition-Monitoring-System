#include "curvedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QToolTip>

CurveDialog::CurveDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("实时趋势曲线 - 工业空压机");
    resize(1000, 600);
    setAttribute(Qt::WA_DeleteOnClose);

    // 10路传感器配置
    m_sensorNames = {
        "排气压力","排气温度","润滑油温度","电机电流","电机电压",
        "储气罐压力","空气入口温度","振动加速度","运行状态","报警状态"
    };
    m_sensorColors = {
        Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta,
        Qt::darkYellow, Qt::darkCyan, Qt::darkMagenta, Qt::gray, Qt::black
    };

    // 初始化数据容器
    m_dataPool.resize(MAX_SENSOR_COUNT);
    m_startTime = QDateTime::currentDateTime(); // 记录曲线启动时间
    setupUI();
}

CurveDialog::~CurveDialog()
{

}

void CurveDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 复选框区域
    QWidget *checkWidget = new QWidget;
    QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
    checkLayout->setContentsMargins(0, 0, 0, 0);
    checkLayout->setSpacing(10);

    for (int i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        QCheckBox *cb = new QCheckBox(m_sensorNames[i]);
        cb->setChecked(i < 3);
        m_checkBoxes.append(cb);
        checkLayout->addWidget(cb);

        connect(cb, &QCheckBox::toggled, this, [this, i](bool visible)
                {
                    onCheckBoxToggled(i, visible);
                });
    }
    checkLayout->addStretch();
    mainLayout->addWidget(checkWidget);

    // 绘图控件
    m_customPlot = new QCustomPlot(this);
    mainLayout->addWidget(m_customPlot);

    // 创建10条曲线
    for (int i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        m_customPlot->addGraph();
        QCPGraph *graph = m_customPlot->graph(i);
        graph->setName(m_sensorNames[i]);
        graph->setPen(QPen(m_sensorColors[i], 2));
        graph->setVisible(i < 3);
    }

    // 基础轴配置（X轴改为相对时间，单位秒）
    m_customPlot->xAxis->setLabel("相对时间(秒)");
    m_customPlot->yAxis->setLabel("数值");
    m_customPlot->legend->setVisible(true);
    m_customPlot->legend->setFont(QFont("Arial", 9));

    m_customPlot->setMouseTracking(true);
    connect(m_customPlot, &QCustomPlot::mouseMove, this, &CurveDialog::onMouseMove);

    // 初始坐标范围
    m_customPlot->xAxis->setRange(0, 60);
    m_customPlot->yAxis->setRange(0, 1000);
}

void CurveDialog::appendData(const QVector<double> &values)
{
    if (values.size() != MAX_SENSOR_COUNT)
        return;

    QDateTime nowDt = QDateTime::currentDateTime();
    // 计算相对时间（从曲线启动开始的秒数）
    double relativeSec = m_startTime.secsTo(nowDt);

    // 存入数据
    for (int i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        m_dataPool[i].append(values[i]);
    }
    m_timePool.append(nowDt);

    // 限制最大点数
    if (m_timePool.size() > m_maxPoints)
    {
        for (int i = 0; i < MAX_SENSOR_COUNT; ++i)
        {
            m_dataPool[i].removeFirst();
        }
        m_timePool.removeFirst();
    }

    // 组装曲线数据（X轴使用相对时间）
    int total = m_timePool.size();
    for (int i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        QCPGraph *graph = m_customPlot->graph(i);
        QVector<double> xData, yData;
        xData.reserve(total);
        yData.reserve(total);

        for (int idx = 0; idx < total; ++idx)
        {
            double t = m_startTime.secsTo(m_timePool[idx]);
            xData.append(t);
            yData.append(m_dataPool[i][idx]);
        }
        graph->setData(xData, yData);
    }

    // X轴滚动，显示最近60秒数据
    if (total > 0)
    {
        double latestSec = m_startTime.secsTo(m_timePool.last());
        m_customPlot->xAxis->setRange(latestSec - 60, latestSec);
    }

    m_customPlot->rescaleAxes(true);
    m_customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void CurveDialog::onCheckBoxToggled(int index, bool checked)
{
    if (index < 0 || index >= MAX_SENSOR_COUNT)
        return;
    m_customPlot->graph(index)->setVisible(checked);
    m_customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void CurveDialog::onMouseMove(QMouseEvent *event)
{
    // 空数据拦截，防止崩溃
    if (m_timePool.isEmpty())
    {
        QToolTip::hideText();
        return;
    }

    QPoint mousePos = event->pos();
    double mouseX = mousePos.x();
    double mouseY = mousePos.y();

    // 像素转相对时间（秒）
    double mouseSec = m_customPlot->xAxis->pixelToCoord(mouseX);
    int total = m_timePool.size();

    // 查找最近采样点
    int findIndex = -1;
    double minDiff = 99999.0;
    for (int i = 0; i < total; ++i)
    {
        double t = m_startTime.secsTo(m_timePool[i]);
        double diff = qAbs(t - mouseSec);
        if (diff < minDiff)
        {
            minDiff = diff;
            findIndex = i;
        }
    }

    // 索引合法性校验
    if (findIndex < 0 || findIndex >= total)
    {
        QToolTip::hideText();
        return;
    }

    // 拼接提示（悬浮框依然显示完整采集时间）
    QString tipText = QString("采集时间: %1\n")
                          .arg(m_timePool[findIndex].toString("yyyy-MM-dd hh:mm:ss"));
    bool foundHover = false;

    for (int i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        QCPGraph *graph = m_customPlot->graph(i);
        if (!graph->visible())
            continue;

        if (findIndex >= m_dataPool[i].size())
            continue;

        double val = m_dataPool[i][findIndex];
        double pointY = m_customPlot->yAxis->coordToPixel(val);

        if (qAbs(mouseY - pointY) < 10)
        {
            tipText += QString("%1: %2\n")
            .arg(m_sensorNames[i])
                .arg(val, 0, 'f', 2);
            foundHover = true;
        }
    }

    if (foundHover)
    {
        QToolTip::showText(event->globalPos(), tipText, m_customPlot);
    }
    else
    {
        QToolTip::hideText();
    }
}