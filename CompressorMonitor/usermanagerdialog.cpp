#include "usermanagerdialog.h"
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QSqlError>
UserManagerDialog::UserManagerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("用户管理");
    resize(600, 400);

    m_model = new QSqlTableModel(this);
    m_model->setTable("users");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->select();
    m_model->setHeaderData(1, Qt::Horizontal, "用户名");
    m_model->setHeaderData(2, Qt::Horizontal, "密码");
    m_model->setHeaderData(3, Qt::Horizontal, "角色");
    m_model->setHeaderData(4, Qt::Horizontal, "姓名");

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_addBtn = new QPushButton("添加用户");
    m_deleteBtn = new QPushButton("删除用户");
    m_modifyBtn = new QPushButton("修改密码");

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_modifyBtn);
    btnLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_view);
    mainLayout->addLayout(btnLayout);

    connect(m_addBtn, &QPushButton::clicked, this, &UserManagerDialog::onAddUser);
    connect(m_deleteBtn, &QPushButton::clicked, this, &UserManagerDialog::onDeleteUser);
    connect(m_modifyBtn, &QPushButton::clicked, this, &UserManagerDialog::onModifyUser);
}

void UserManagerDialog::onAddUser()
{
    QString username = QInputDialog::getText(this, "添加用户", "用户名:");
    if (username.isEmpty()) return;
    QString password = QInputDialog::getText(this, "添加用户", "密码:", QLineEdit::Password);
    if (password.isEmpty()) return;
    int role = QInputDialog::getInt(this, "添加用户", "角色 (0=管理员, 1=普通用户):", 1, 0, 1);
    QString fullname = QInputDialog::getText(this, "添加用户", "姓名:");

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, role, fullname) VALUES (?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(role);
    query.addBindValue(fullname);
    if (query.exec()) {
        m_model->select();
        QMessageBox::information(this, "成功", "用户已添加");
    } else {
        QMessageBox::warning(this, "失败", "添加用户失败: " + query.lastError().text());
    }
}

void UserManagerDialog::onDeleteUser()
{
    int row = m_view->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的用户");
        return;
    }
    int id = m_model->data(m_model->index(row, 0)).toInt();
    if (QMessageBox::question(this, "确认", "确定删除该用户吗？") == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM users WHERE id = ?");
        query.addBindValue(id);
        if (query.exec()) {
            m_model->select();
            QMessageBox::information(this, "成功", "用户已删除");
        } else {
            QMessageBox::warning(this, "失败", "删除失败");
        }
    }
}

void UserManagerDialog::onModifyUser()
{
    int row = m_view->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要修改密码的用户");
        return;
    }
    int id = m_model->data(m_model->index(row, 0)).toInt();
    QString newPass = QInputDialog::getText(this, "修改密码", "新密码:", QLineEdit::Password);
    if (newPass.isEmpty()) return;

    QSqlQuery query;
    query.prepare("UPDATE users SET password = ? WHERE id = ?");
    query.addBindValue(newPass);
    query.addBindValue(id);
    if (query.exec()) {
        QMessageBox::information(this, "成功", "密码已更新");
    } else {
        QMessageBox::warning(this, "失败", "更新失败");
    }
}