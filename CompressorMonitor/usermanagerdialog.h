#ifndef USERMANAGERDIALOG_H
#define USERMANAGERDIALOG_H

#include <QDialog>
#include <QSqlTableModel>

class QTableView;
class QPushButton;

class UserManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserManagerDialog(QWidget *parent = nullptr);

private slots:
    void onAddUser();
    void onDeleteUser();
    void onModifyUser();

private:
    QSqlTableModel *m_model;
    QTableView *m_view;
    QPushButton *m_addBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_modifyBtn;
};

#endif // USERMANAGERDIALOG_H