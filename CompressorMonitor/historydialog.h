#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QSqlTableModel>

class QTableView;
class QDateTimeEdit;
class QPushButton;

class HistoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDialog(QWidget *parent = nullptr);

private slots:
    void onQuery();
    void onExport();

private:
    QSqlTableModel *m_model;
    QTableView *m_view;
    QDateTimeEdit *m_startTime;
    QDateTimeEdit *m_endTime;
    QPushButton *m_queryBtn;
    QPushButton *m_exportBtn;
};

#endif // HISTORYDIALOG_H