#ifndef CASHIERFORM_H
#define CASHIERFORM_H

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlQueryModel>  // Add this
#include <QTableView>      // Add this
#include <QDoubleSpinBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QDateTime>

class CashierForm : public QWidget
{
    Q_OBJECT

public:
    // Update constructor declaration to include userId parameter
    explicit CashierForm(QWidget *parent = nullptr, int userId = -1);
    ~CashierForm();

private slots:
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onClearCartClicked();
    void onCheckoutClicked();
    void onProductSelectionChanged();  // Add this
    void updateTotals();

private:
    // UI Elements
    QDoubleSpinBox* quantitySpinBox;
    QPushButton* addItemButton;
    QPushButton* removeItemButton;
    QPushButton* clearCartButton;
    QPushButton* checkoutButton;
    QTableWidget* cartTable;
    QLabel* subtotalLabel;
    QLabel* taxLabel;
    QLabel* totalLabel;
    QTableView* productsTable;
    QSqlQueryModel* productsModel;
    QLineEdit* searchBox;
    QPrinter* printer = nullptr;

    // Helper methods
    void setupUI();
    void loadProducts();
    void connectSignals();
    double calculateSubtotal();
    QString formatCurrency(double amount);
    void clearCart();
    bool saveOrder();
    void showInvoice(int orderId, double subtotal, double tax, double total);
    void printInvoice(QWidget* invoice);
    const double TAX_RATE = 0.15;

    // ...existing members...
    int currentUserId;
};

#endif