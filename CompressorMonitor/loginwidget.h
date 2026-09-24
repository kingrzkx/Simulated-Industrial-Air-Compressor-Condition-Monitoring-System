#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class LoginWidget : public QDialog
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);
    QString getUsername() const;
    int getRole() const;

private slots:
    void onLoginClicked();

private:
    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
    QComboBox *m_roleCombo;
    QPushButton *m_loginBtn;
    QPushButton *m_cancelBtn;
    QString m_username;
    int m_role;
};

#endif // LOGINWIDGET_H