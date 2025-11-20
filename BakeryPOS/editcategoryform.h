#ifndef EDITCATEGORYFORM_H
#define EDITCATEGORYFORM_H

#include <QDialog>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui {
class EditCategoryForm;
}
QT_END_NAMESPACE

class EditCategoryForm : public QDialog {
    Q_OBJECT

public:
    explicit EditCategoryForm(QWidget *parent = nullptr);
    ~EditCategoryForm();
    void loadCategoryData(int categoryId);

signals:
    void categoryUpdated();

private slots:
    void on_saveButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::EditCategoryForm *ui;
    int currentCategoryId;
};

#endif // EDITCATEGORYFORM_H
