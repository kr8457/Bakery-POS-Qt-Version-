#include "editcategoryform.h"
#include "ui_editcategoryform.h"
#include <QMessageBox>
#include <QSqlError>

EditCategoryForm::EditCategoryForm(QWidget *parent)
    : QDialog(parent), ui(new Ui::EditCategoryForm)
{
    ui->setupUi(this);
    setWindowTitle("Edit Category");
}

EditCategoryForm::~EditCategoryForm()
{
    delete ui;
}

void EditCategoryForm::loadCategoryData(int categoryId)
{
    currentCategoryId = categoryId;
    QSqlQuery query;
    query.prepare("SELECT * FROM categories WHERE ID = ?");
    query.bindValue(0, categoryId);
    
    if (query.exec() && query.next()) {
        ui->categoryNameLineEdit->setText(query.value("Category").toString());
    }
}

void EditCategoryForm::on_saveButton_clicked()
{
    QString categoryName = ui->categoryNameLineEdit->text();

    if (categoryName.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Category name cannot be empty");
        return;
    }

    QSqlQuery query;
    if (currentCategoryId > 0) {
        query.prepare("UPDATE categories SET Category = ? WHERE ID = ?");
        query.bindValue(0, categoryName);
        query.bindValue(1, currentCategoryId);
    } else {
        query.prepare("INSERT INTO categories (Category, Date) VALUES (?, CURRENT_DATE)");
        query.bindValue(0, categoryName);
    }

    if (query.exec()) {
        emit categoryUpdated();
        accept();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save category: " + query.lastError().text());
    }
}

void EditCategoryForm::on_cancelButton_clicked()
{
    reject();
}
