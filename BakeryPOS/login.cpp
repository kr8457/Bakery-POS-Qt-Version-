#include "login.h"
#include "ui_login.h"
#include "dashboard.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

login::login(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::login)
{
    ui->setupUi(this);
    this->setFixedSize(400, 540);

    // Setup database connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("mydb");
    db.setUserName("root");
    db.setPassword("khalid");

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error",
                              "Error connecting to database: " + db.lastError().text());
    }
}

login::~login()
{
    delete ui;
}


void login::on_checkBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->passwordLineEdit->setEchoMode(QLineEdit::Normal); // Show password
    } else {
        ui->passwordLineEdit->setEchoMode(QLineEdit::Password); // Hide password
    }
}

void login::on_btnLogin_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = ? AND password = ?");
    query.bindValue(0, username);
    query.bindValue(1, password);

    if (query.exec()) {
        if (query.next()) {
            // Login successful
            int userId = query.value("UserID").toInt();
            qDebug() << "User logged in with ID:" << userId; // Debug output
            
            dashboard* dash = new dashboard(nullptr, userId);
            dash->show();
            this->close();
        } else {
            // Login failed
            QMessageBox::warning(this, "Login Failed",
                               "Invalid username or password");
        }
    } else {
        QMessageBox::critical(this, "Query Error",
                            "Database query failed: " + query.lastError().text());
    }
}



