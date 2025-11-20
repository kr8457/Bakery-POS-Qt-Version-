#include "EditUserForm.h"
#include "./ui_EditUserForm.h"  // Note the ./ prefix
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

EditUserForm::EditUserForm(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditUserForm)
{
    ui->setupUi(this);
    setWindowTitle("Edit User");
    
    // Setup role combo box
    ui->roleComboBox->clear();
    ui->roleComboBox->addItem("Admin");
    ui->roleComboBox->addItem("Cashier");
    ui->roleComboBox->addItem("Manager");

    // Setup status combo box with all possible statuses from your table
    ui->statusComboBox->clear();
    ui->statusComboBox->addItem("Active");
    ui->statusComboBox->addItem("Approval");
    ui->statusComboBox->addItem("Inactive");

    // Set default status to "Active" for new users
    if (currentUserId == 0) {
        ui->statusComboBox->setCurrentText("Active");
    }
}

EditUserForm::~EditUserForm() 
{
    delete ui;
}

void EditUserForm::loadUserData(int userId) 
{
    currentUserId = userId;
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE UserID = ?");  // Changed 'id' to 'UserID'
    query.bindValue(0, userId);
    
    if (query.exec() && query.next()) {
        ui->usernameLineEdit->setText(query.value("username").toString());
        ui->roleComboBox->setCurrentText(query.value("role").toString());
        ui->statusComboBox->setCurrentText(query.value("status").toString());
    }
}

void EditUserForm::on_saveButton_clicked() 
{
    QString username = ui->usernameLineEdit->text();
    QString role = ui->roleComboBox->currentText();
    QString password = ui->passwordLineEdit->text();
    QString status = ui->statusComboBox->currentText();

    // Validate input
    if (username.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Username cannot be empty");
        return;
    }

    if (currentUserId == 0 && password.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Password is required for new users");
        return;
    }

    QSqlQuery query;
    if (currentUserId > 0) {
        // Update existing user
        if (password.isEmpty()) {
            query.prepare("UPDATE users SET username = ?, role = ?, status = ? WHERE UserID = ?");  // Changed 'id' to 'UserID'
            query.bindValue(0, username);
            query.bindValue(1, role);
            query.bindValue(2, status);
            query.bindValue(3, currentUserId);
        } else {
            query.prepare("UPDATE users SET username = ?, role = ?, password = ?, status = ? WHERE UserID = ?");  // Changed 'id' to 'UserID'
            query.bindValue(0, username);
            query.bindValue(1, role);
            query.bindValue(2, password);
            query.bindValue(3, status);
            query.bindValue(4, currentUserId);
        }
    } else {
        // Insert new user
        query.prepare("INSERT INTO users (username, password, role, status, date) VALUES (?, ?, ?, ?, CURRENT_DATE)");
        query.bindValue(0, username);
        query.bindValue(1, password);
        query.bindValue(2, role);
        query.bindValue(3, status);
    }

    if (query.exec()) {
        emit userUpdated();
        accept();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save user: " + query.lastError().text());
    }
}

void EditUserForm::on_cancelButton_clicked() 
{
    reject();
}