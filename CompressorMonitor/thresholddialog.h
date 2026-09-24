#ifndef THRESHOLDDIALOG_H
#define THRESHOLDDIALOG_H

#include <QDialog>
#include <QVector>

class QTableWidget;

class ThresholdDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ThresholdDialog(const QVector<double> &low, const QVector<double> &high, QWidget *parent = nullptr);
    QVector<double> getLowLimits() const;
    QVector<double> getHighLimits() const;

private slots:
    void onSave();

private:
    QTableWidget *m_table;
    QVector<double> m_low, m_high;
};

#endif // THRESHOLDDIALOG_H