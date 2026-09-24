#include "thresholddialog.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>

ThresholdDialog::ThresholdDialog(const QVector<double> &low, const QVector<double> &high, QWidget *parent)
    : QDialog(parent), m_low(low), m_high(high)
{
    setWindowTitle("报警阈值设置");
    setModal(true);
    resize(600, 400);

    QStringList sensors = {"排气压力", "排气温度", "润滑油温度", "电机电流", "电机电压",
                           "储气罐压力", "空气入口温度", "振动加速度", "运行状态", "报警状态"};
    m_table = new QTableWidget(10, 3, this);
    m_table->setHorizontalHeaderLabels({"传感器", "下限阈值", "上限阈值"});
    m_table->horizontalHeader()->setStretchLastSection(true);

    for (int i = 0; i < 10; ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(sensors[i]));
        QDoubleSpinBox *lowSpin = new QDoubleSpinBox;
        lowSpin->setRange(-10000, 10000);
        lowSpin->setValue(m_low[i]);
        QDoubleSpinBox *highSpin = new QDoubleSpinBox;
        highSpin->setRange(-10000, 10000);
        highSpin->setValue(m_high[i]);
        m_table->setCellWidget(i, 1, lowSpin);
        m_table->setCellWidget(i, 2, highSpin);
    }

    QPushButton *saveBtn = new QPushButton("保存");
    QPushButton *cancelBtn = new QPushButton("取消");
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_table);
    mainLayout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, &ThresholdDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ThresholdDialog::onSave()
{
    for (int i = 0; i < 10; ++i) {
        QDoubleSpinBox *lowSpin = qobject_cast<QDoubleSpinBox*>(m_table->cellWidget(i, 1));
        QDoubleSpinBox *highSpin = qobject_cast<QDoubleSpinBox*>(m_table->cellWidget(i, 2));
        if (lowSpin && highSpin) {
            m_low[i] = lowSpin->value();
            m_high[i] = highSpin->value();
        }
    }
    accept();
}

QVector<double> ThresholdDialog::getLowLimits() const { return m_low; }
QVector<double> ThresholdDialog::getHighLimits() const { return m_high; }