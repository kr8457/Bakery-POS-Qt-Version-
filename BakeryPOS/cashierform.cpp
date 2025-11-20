#include "cashierform.h"
#include <QMessageBox>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

CashierForm::CashierForm(QWidget *parent, int userId) : QWidget(parent)
{
    currentUserId = userId;
    setupUI();
    loadProducts();
    connectSignals();
}

void CashierForm::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Product selection section
    QVBoxLayout* productLayout = new QVBoxLayout();
    
    // Search box
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Search products...");
    
    // Products table
    productsTable = new QTableView(this);
    productsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    productsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    productsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productsTable->verticalHeader()->hide();
    productsTable->setAlternatingRowColors(true);
    
    // Quantity spinner and add button
    QHBoxLayout* addItemLayout = new QHBoxLayout();
    quantitySpinBox = new QDoubleSpinBox(this);
    quantitySpinBox->setRange(0.1, 999.99);
    quantitySpinBox->setValue(1.0);
    quantitySpinBox->setDecimals(2);
    addItemButton = new QPushButton("Add Item", this);
    
    addItemLayout->addWidget(new QLabel("Quantity:"));
    addItemLayout->addWidget(quantitySpinBox);
    addItemLayout->addWidget(addItemButton);
    addItemLayout->addStretch();

    productLayout->addWidget(searchBox);
    productLayout->addWidget(productsTable);
    productLayout->addLayout(addItemLayout);

    // Cart section
    cartTable = new QTableWidget(this);
    cartTable->setColumnCount(5);
    cartTable->setHorizontalHeaderLabels({"Product", "Quantity", "Unit Price", "Total", "ProductID"});
    cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable->hideColumn(4); // Hide ProductID column
    cartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Cart buttons
    QHBoxLayout* cartButtonsLayout = new QHBoxLayout();
    removeItemButton = new QPushButton("Remove Selected", this);
    clearCartButton = new QPushButton("Clear Cart", this);
    cartButtonsLayout->addWidget(removeItemButton);
    cartButtonsLayout->addWidget(clearCartButton);
    cartButtonsLayout->addStretch();

    // Totals section
    QGridLayout* totalsLayout = new QGridLayout();
    subtotalLabel = new QLabel("$0.00", this);
    taxLabel = new QLabel("$0.00", this);
    totalLabel = new QLabel("$0.00", this);
    
    totalsLayout->addWidget(new QLabel("Subtotal:"), 0, 0);
    totalsLayout->addWidget(subtotalLabel, 0, 1);
    totalsLayout->addWidget(new QLabel("Tax:"), 1, 0);
    totalsLayout->addWidget(taxLabel, 1, 1);
    totalsLayout->addWidget(new QLabel("Total:"), 2, 0);
    totalsLayout->addWidget(totalLabel, 2, 1);
    
    checkoutButton = new QPushButton("Checkout", this);
    checkoutButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 10px; }");

    // Add everything to main layout
    mainLayout->addLayout(productLayout);
    mainLayout->addWidget(cartTable);
    mainLayout->addLayout(cartButtonsLayout);
    mainLayout->addLayout(totalsLayout);
    mainLayout->addWidget(checkoutButton);
}

void CashierForm::loadProducts()
{
    productsModel = new QSqlQueryModel(this);
    productsModel->setQuery(
        "SELECT ProductID, Name, Category, PricePerUnit, UnitType, StockQuantity "
        "FROM products "
        "WHERE status = 'Available' "
        "ORDER BY Name"
    );
    
    // Set headers
    productsModel->setHeaderData(0, Qt::Horizontal, "ID");
    productsModel->setHeaderData(1, Qt::Horizontal, "Product");
    productsModel->setHeaderData(2, Qt::Horizontal, "Category");
    productsModel->setHeaderData(3, Qt::Horizontal, "Price");
    productsModel->setHeaderData(4, Qt::Horizontal, "Unit");
    productsModel->setHeaderData(5, Qt::Horizontal, "Stock");
    
    productsTable->setModel(productsModel);
    productsTable->hideColumn(0); // Hide ID column
    
    // Adjust column widths
    productsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void CashierForm::connectSignals()
{
    connect(addItemButton, &QPushButton::clicked, this, &CashierForm::onAddItemClicked);
    connect(removeItemButton, &QPushButton::clicked, this, &CashierForm::onRemoveItemClicked);
    connect(clearCartButton, &QPushButton::clicked, this, &CashierForm::onClearCartClicked);
    connect(checkoutButton, &QPushButton::clicked, this, &CashierForm::onCheckoutClicked);
    connect(searchBox, &QLineEdit::textChanged, this, [this](const QString& text) {
        QString filter = text.trimmed();
        QString query = QString(
            "SELECT ProductID, Name, Category, PricePerUnit, UnitType, StockQuantity "
            "FROM products "
            "WHERE status = 'Available' "
            "AND (Name LIKE '%%1%' OR Category LIKE '%%1%') "
            "ORDER BY Name"
        ).arg(filter);
        
        productsModel->setQuery(query);
    });
    
    connect(productsTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &CashierForm::onProductSelectionChanged);
}

void CashierForm::onAddItemClicked()
{
    QModelIndex current = productsTable->currentIndex();
    if (!current.isValid()) {
        QMessageBox::warning(this, "Warning", "Please select a product first.");
        return;
    }

    int row = current.row();
    int productId = productsModel->data(productsModel->index(row, 0)).toInt();
    QString productName = productsModel->data(productsModel->index(row, 1)).toString();
    double unitPrice = productsModel->data(productsModel->index(row, 3)).toDouble();
    double quantity = quantitySpinBox->value();

    if (quantity <= 0) {
        QMessageBox::warning(this, "Warning", "Please enter a valid quantity.");
        return;
    }

    double total = quantity * unitPrice;

    // Check if product already exists in cart
    for (int row = 0; row < cartTable->rowCount(); ++row) {
        if (cartTable->item(row, 4)->text().toInt() == productId) {
            // Update quantity and total instead of adding new row
            double currentQty = cartTable->item(row, 1)->text().toDouble();
            double newQty = currentQty + quantity;
            double newTotal = newQty * unitPrice;
            
            cartTable->item(row, 1)->setText(QString::number(newQty, 'f', 2));
            cartTable->item(row, 3)->setText(formatCurrency(newTotal));
            
            updateTotals();
            quantitySpinBox->setValue(1.0);
            return;
        }
    }

    // Add new row if product not in cart
    int cartRow = cartTable->rowCount();
    cartTable->insertRow(cartRow);
    
    QTableWidgetItem* nameItem = new QTableWidgetItem(productName);
    QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(quantity, 'f', 2));
    QTableWidgetItem* priceItem = new QTableWidgetItem(formatCurrency(unitPrice));
    QTableWidgetItem* totalItem = new QTableWidgetItem(formatCurrency(total));
    QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(productId));

    cartTable->setItem(cartRow, 0, nameItem);
    cartTable->setItem(cartRow, 1, qtyItem);
    cartTable->setItem(cartRow, 2, priceItem);
    cartTable->setItem(cartRow, 3, totalItem);
    cartTable->setItem(cartRow, 4, idItem);

    // Update totals and reset quantity
    updateTotals();
    quantitySpinBox->setValue(1.0);

    // Debug output
    qDebug() << "Added product to cart:";
    qDebug() << "Product ID:" << productId;
    qDebug() << "Name:" << productName;
    qDebug() << "Quantity:" << quantity;
    qDebug() << "Unit Price:" << unitPrice;
    qDebug() << "Total:" << total;
}

void CashierForm::onRemoveItemClicked()
{
    int currentRow = cartTable->currentRow();
    if (currentRow >= 0) {
        cartTable->removeRow(currentRow);
        updateTotals();
    }
}

void CashierForm::onClearCartClicked()
{
    clearCart();
}

void CashierForm::onCheckoutClicked()
{
    if (cartTable->rowCount() == 0) {
        QMessageBox::warning(this, "Error", "Cart is empty!");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Checkout", 
                                "Do you want to complete this transaction?",
                                QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (saveOrder()) {
            QMessageBox::information(this, "Success", "Order completed successfully!");
            clearCart();
        }
    }
}

void CashierForm::updateTotals()
{
    double subtotal = calculateSubtotal();
    double tax = subtotal * TAX_RATE;
    double total = subtotal + tax;

    subtotalLabel->setText(formatCurrency(subtotal));
    taxLabel->setText(formatCurrency(tax));
    totalLabel->setText(formatCurrency(total));
}

double CashierForm::calculateSubtotal()
{
    double subtotal = 0.0;
    for (int row = 0; row < cartTable->rowCount(); ++row) {
        subtotal += cartTable->item(row, 3)->text().remove("$").toDouble();
    }
    return subtotal;
}

void CashierForm::clearCart()
{
    cartTable->setRowCount(0);
    updateTotals();
}

QString CashierForm::formatCurrency(double amount)
{
    return QString("$%1").arg(amount, 0, 'f', 2);
}

bool CashierForm::saveOrder()
{
    if (cartTable->rowCount() == 0) return false;

    QSqlDatabase::database().transaction();

    try {
        QSqlQuery query;
        double subtotal = calculateSubtotal();
        double tax = subtotal * TAX_RATE;
        double total = subtotal + tax;

        // Create new order - match column names from database
        query.prepare("INSERT INTO Orders (OrderDate, UserID, TotalAmount, payment_method) "
                     "VALUES (NOW(), :userId, :total, 'Cash')");
        query.bindValue(":userId", currentUserId);
        query.bindValue(":total", total);
        
        if (!query.exec()) {
            throw std::runtime_error(query.lastError().text().toStdString());
        }

        int orderId = query.lastInsertId().toInt();

        // Add order details
        for (int row = 0; row < cartTable->rowCount(); ++row) {
            query.prepare("INSERT INTO OrderDetails (OrderID, ProductID, Quantity, Price) "
                        "VALUES (:orderId, :productId, :quantity, :price)");
                        
            int productId = cartTable->item(row, 4)->text().toInt();
            double quantity = cartTable->item(row, 1)->text().toDouble();
            double unitPrice = cartTable->item(row, 2)->text().remove("$").toDouble();

            query.bindValue(":orderId", orderId);
            query.bindValue(":productId", productId);
            query.bindValue(":quantity", quantity);
            query.bindValue(":price", unitPrice);

            if (!query.exec()) {
                throw std::runtime_error(query.lastError().text().toStdString());
            }

            // Update product stock
            query.prepare("UPDATE products SET StockQuantity = StockQuantity - :qty "
                        "WHERE ProductID = :pid");
            query.bindValue(":qty", quantity);
            query.bindValue(":pid", productId);
            
            if (!query.exec()) {
                throw std::runtime_error(query.lastError().text().toStdString());
            }
        }

        QSqlDatabase::database().commit();
        
        // Show invoice after successful save
        showInvoice(orderId, subtotal, tax, total);
        clearCart();
        return true;
    }
    catch (const std::exception& e) {
        QSqlDatabase::database().rollback();
        QMessageBox::critical(this, "Error", QString("Failed to save order: %1").arg(e.what()));
        return false;
    }
}

void CashierForm::showInvoice(int orderId, double subtotal, double tax, double total)
{
    QWidget* invoice = new QWidget(nullptr, Qt::Window);
    invoice->setWindowTitle("Invoice #" + QString::number(orderId));
    invoice->setMinimumWidth(400);

    QVBoxLayout* layout = new QVBoxLayout(invoice);

    // Header
    QLabel* header = new QLabel("BakeryPOS Receipt");
    header->setAlignment(Qt::AlignCenter);
    QFont headerFont = header->font();
    headerFont.setPointSize(16);
    headerFont.setBold(true);
    header->setFont(headerFont);
    layout->addWidget(header);

    // Order info
    layout->addWidget(new QLabel("Order #: " + QString::number(orderId)));
    layout->addWidget(new QLabel("Date: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    layout->addWidget(new QLabel("Cashier ID: " + QString::number(currentUserId)));

    // Separator
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    // Items table
    QTableWidget* items = new QTableWidget();
    items->setColumnCount(4);
    items->setHorizontalHeaderLabels({"Item", "Qty", "Price", "Total"});
    items->setRowCount(cartTable->rowCount());
    items->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    for (int row = 0; row < cartTable->rowCount(); ++row) {
        items->setItem(row, 0, new QTableWidgetItem(cartTable->item(row, 0)->text()));
        items->setItem(row, 1, new QTableWidgetItem(cartTable->item(row, 1)->text()));
        items->setItem(row, 2, new QTableWidgetItem(cartTable->item(row, 2)->text()));
        items->setItem(row, 3, new QTableWidgetItem(cartTable->item(row, 3)->text()));
    }
    
    items->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(items);

    // Totals
    QGridLayout* totals = new QGridLayout();
    totals->addWidget(new QLabel("Subtotal:"), 0, 0, Qt::AlignRight);
    totals->addWidget(new QLabel(formatCurrency(subtotal)), 0, 1, Qt::AlignRight);
    totals->addWidget(new QLabel("Tax:"), 1, 0, Qt::AlignRight);
    totals->addWidget(new QLabel(formatCurrency(tax)), 1, 1, Qt::AlignRight);
    totals->addWidget(new QLabel("Total:"), 2, 0, Qt::AlignRight);
    totals->addWidget(new QLabel(formatCurrency(total)), 2, 1, Qt::AlignRight);
    layout->addLayout(totals);

    // Print button
    QPushButton* printButton = new QPushButton("Print");
    connect(printButton, &QPushButton::clicked, this, [this, invoice]() {
        printInvoice(invoice);
    });
    layout->addWidget(printButton);

    invoice->show();
}

void CashierForm::printInvoice(QWidget* invoice)
{
    if (!printer) {
        printer = new QPrinter(QPrinter::HighResolution);
    }

    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter(printer);
        invoice->render(&painter);
    }
}

void CashierForm::onProductSelectionChanged()
{
    QModelIndex current = productsTable->currentIndex();
    if (current.isValid()) {
        int row = current.row();
        QString category = productsModel->data(productsModel->index(row, 2)).toString();
        QString unitType = productsModel->data(productsModel->index(row, 4)).toString();
        
        // Adjust quantity spinner based on category
        if (category == "Sweet") {
            quantitySpinBox->setSuffix(" kg");
            quantitySpinBox->setDecimals(3);
            quantitySpinBox->setSingleStep(0.100);
            quantitySpinBox->setMinimum(0.100);
        } else {
            quantitySpinBox->setSuffix(unitType.isEmpty() ? "" : " " + unitType);
            quantitySpinBox->setDecimals(0);
            quantitySpinBox->setSingleStep(1);
            quantitySpinBox->setMinimum(1);
        }
        
        addItemButton->setEnabled(true);
    } else {
        addItemButton->setEnabled(false);
    }
}

CashierForm::~CashierForm()
{
    // Clean up printer if it exists
    if (printer) {
        delete printer;
        printer = nullptr;
    }

    // Clean up model if it exists
    if (productsModel) {
        delete productsModel;
        productsModel = nullptr;
    }

    // Note: Qt will automatically delete child widgets
    // so we don't need to manually delete UI elements
}