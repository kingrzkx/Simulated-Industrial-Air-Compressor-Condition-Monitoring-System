#include "loginwidget.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

LoginWidget::LoginWidget(QWidget *parent) : QDialog(parent), m_role(1)
{
    setWindowTitle("用户登录 - 工业空压机监控系统");
    setFixedSize(450, 300);

    this->setStyleSheet(R"(
        QDialog {
            border-image: url(:/res/login_bg.png) stretch;
        }
        QLineEdit, QComboBox, QPushButton {
            background-color: rgba(255,255,255,180);
        }
    )");

    QLabel *userLabel = new QLabel("用户名:", this);
    QLabel *passLabel = new QLabel("密  码:", this);
    QLabel *roleLabel = new QLabel("角  色:", this);
    m_userEdit = new QLineEdit(this);
    m_passEdit = new QLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_roleCombo = new QComboBox(this);
    m_roleCombo->addItem("管理员", 0);
    m_roleCombo->addItem("普通用户", 1);

    m_loginBtn = new QPushButton("登录", this);
    m_cancelBtn = new QPushButton("取消", this);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(userLabel, 0, 0);
    grid->addWidget(m_userEdit, 0, 1);
    grid->addWidget(passLabel, 1, 0);
    grid->addWidget(m_passEdit, 1, 1);
    grid->addWidget(roleLabel, 2, 0);
    grid->addWidget(m_roleCombo, 2, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_cancelBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(grid);
    mainLayout->addLayout(btnLayout);

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void LoginWidget::onLoginClicked()
{
    QString username = m_userEdit->text().trimmed();
    QString password = m_passEdit->text().trimmed();
    int selectedRole = m_roleCombo->currentData().toInt();

    QSqlQuery query;
    query.prepare("SELECT password, role FROM users WHERE username = :username");
    query.bindValue(":username", username);
    if (query.exec() && query.next()) {
        QString dbPass = query.value(0).toString();
        int dbRole = query.value(1).toInt();
        if (dbPass == password && dbRole == selectedRole) {
            m_username = username;
            m_role = dbRole;
            accept();
            return;
        }
    }
    QMessageBox::warning(this, "登录失败", "用户名、密码或角色不正确");
}

QString LoginWidget::getUsername() const
{
    return m_username;
}

int LoginWidget::getRole() const
{
    return m_role;
}