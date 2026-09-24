#include "historydialog.h"
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QSqlQuery>
#include <QTextStream>
#include <QDateTimeEdit>

HistoryDialog::HistoryDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("历史运行数据");
    resize(900, 500);

    m_model = new QSqlTableModel(this);
    m_model->setTable("sensor_data");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->select();
    m_model->setHeaderData(1, Qt::Horizontal, "时间");
    m_model->setHeaderData(2, Qt::Horizontal, "排气压力(kPa)");
    m_model->setHeaderData(3, Qt::Horizontal, "排气温度(℃)");
    m_model->setHeaderData(4, Qt::Horizontal, "润滑油温(℃)");
    m_model->setHeaderData(5, Qt::Horizontal, "电机电流(A)");
    m_model->setHeaderData(6, Qt::Horizontal, "电机电压(V)");
    m_model->setHeaderData(7, Qt::Horizontal, "储气罐压力(kPa)");
    m_model->setHeaderData(8, Qt::Horizontal, "入口温度(℃)");
    m_model->setHeaderData(9, Qt::Horizontal, "振动(mm/s)");
    m_model->setHeaderData(10, Qt::Horizontal, "运行状态");
    m_model->setHeaderData(11, Qt::Horizontal, "报警状态");

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->setAlternatingRowColors(true);
    m_view->horizontalHeader()->setStretchLastSection(true);

    m_startTime = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-1));
    m_startTime->setCalendarPopup(true);
    m_endTime = new QDateTimeEdit(QDateTime::currentDateTime());
    m_endTime->setCalendarPopup(true);
    m_queryBtn = new QPushButton("查询");
    m_exportBtn = new QPushButton("导出CSV");

    QHBoxLayout *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel("起始时间:"));
    filterLayout->addWidget(m_startTime);
    filterLayout->addWidget(new QLabel("结束时间:"));
    filterLayout->addWidget(m_endTime);
    filterLayout->addWidget(m_queryBtn);
    filterLayout->addWidget(m_exportBtn);
    filterLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(m_view);

    connect(m_queryBtn, &QPushButton::clicked, this, &HistoryDialog::onQuery);
    connect(m_exportBtn, &QPushButton::clicked, this, &HistoryDialog::onExport);
}

void HistoryDialog::onQuery()
{
    QString filter = QString("timestamp BETWEEN '%1' AND '%2'")
    .arg(m_startTime->dateTime().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(m_endTime->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    m_model->setFilter(filter);
    m_model->select();
}

void HistoryDialog::onExport()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出CSV", "history.csv", "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法保存文件");
        return;
    }
    QTextStream out(&file);
    // 写表头
    for (int i = 0; i < m_model->columnCount(); ++i) {
        out << m_model->headerData(i, Qt::Horizontal).toString() << (i == m_model->columnCount()-1 ? "\n" : ",");
    }
    // 写数据
    for (int r = 0; r < m_model->rowCount(); ++r) {
        for (int c = 0; c < m_model->columnCount(); ++c) {
            out << m_model->data(m_model->index(r, c)).toString() << (c == m_model->columnCount()-1 ? "\n" : ",");
        }
    }
    file.close();
    QMessageBox::information(this, "导出完成", "数据已导出到 " + fileName);
}