#ifndef EDITPRODUCTFORM_H
#define EDITPRODUCTFORM_H

#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class EditProductForm;
}
QT_END_NAMESPACE

class EditProductForm : public QWidget {
    Q_OBJECT

  public:
    EditProductForm(QWidget *parent = nullptr);
    ~EditProductForm();

    // Method to load product data for editing
    void loadProductData(int productId);

  signals:
    void productUpdated(); // Signal to notify parent when product is updated

  private slots:
    void on_SaveButton_clicked();
    void on_DiscardButton_clicked();
    void on_CategoryComboBox_currentTextChanged(const QString &text);

  private:
    Ui::EditProductForm *ui;
    int                  currentProductId;
    QComboBox           *categoryComboBox;

    void setupCategoryComboBox();
    void setupValidation();
    bool validateInput();
    void clearForm();
    void updatePriceFieldsVisibility();
};

#endif // EDITPRODUCTFORM_H
