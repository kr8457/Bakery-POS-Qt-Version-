#include "analyticsform.h"
#include <QSqlError>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>

AnalyticsForm::AnalyticsForm(QWidget *parent)
    : QWidget(parent)
{
    try {
        // Check database connection
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isValid()) {
            qDebug() << "Invalid database connection!";
            throw std::runtime_error("Database connection invalid");
        }

        setupUI();

        // Initialize with current data
        QTimer::singleShot(100, this, [this]() {
            updateStats();
        });

        qDebug() << "AnalyticsForm initialized successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Error in AnalyticsForm initialization:" << e.what();
        throw;
    }
}

void AnalyticsForm::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Period selector
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *periodLabel = new QLabel("Select Period:", this);
    periodComboBox = new QComboBox(this);
    periodComboBox->addItems({"Today", "This Week", "This Month", "This Year"});
    headerLayout->addWidget(periodLabel);
    headerLayout->addWidget(periodComboBox);
    headerLayout->addStretch();

    // Stats cards
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);  // Increase spacing between cards
    cardsLayout->setContentsMargins(0, 10, 0, 10);  // Add vertical margins

    // Create cards and add them to layout
    QFrame* revenueCard = createStatsCard("Total Revenue", "$0.00");
    QFrame* ordersCard = createStatsCard("Total Orders", "0");
    QFrame* avgOrderCard = createStatsCard("Avg Order Value", "$0.00");
    QFrame* topProductCard = createStatsCard("Top Product", "-");

    cardsLayout->addWidget(revenueCard);
    cardsLayout->addWidget(ordersCard);
    cardsLayout->addWidget(avgOrderCard);
    cardsLayout->addWidget(topProductCard);
    cardsLayout->addStretch();  // Add stretch at the end to keep cards left-aligned

    // Tables section
    QGridLayout *tablesLayout = new QGridLayout();
    tablesLayout->setSpacing(10);

    // Sales table setup
    QLabel *salesTitle = new QLabel("Sales by Product", this);
    salesTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding: 10px;");
    salesTable = new QTableWidget(this);
    salesTable->setColumnCount(3);
    salesTable->setHorizontalHeaderLabels({"Product", "Quantity", "Revenue"});
    salesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    salesTable->setAlternatingRowColors(true);

    // Category table setup
    QLabel *categoryTitle = new QLabel("Sales by Category", this);
    categoryTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding: 10px;");
    categoryTable = new QTableWidget(this);
    categoryTable->setColumnCount(2);
    categoryTable->setHorizontalHeaderLabels({"Category", "Total Sales"});
    categoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    categoryTable->setAlternatingRowColors(true);

    // Add tables to layout
    tablesLayout->addWidget(salesTitle, 0, 0);
    tablesLayout->addWidget(salesTable, 1, 0);
    tablesLayout->addWidget(categoryTitle, 0, 1);
    tablesLayout->addWidget(categoryTable, 1, 1);

    // Add all layouts to main layout
    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(cardsLayout);
    mainLayout->addLayout(tablesLayout, 1);  // Give tables more vertical space

    // Connect signals
    connectSignals();

    // Set some reasonable default sizes
    salesTable->setMinimumHeight(200);
    categoryTable->setMinimumHeight(200);
}

QFrame* AnalyticsForm::createStatsCard(const QString &title, const QString &value)
{
    QFrame* card = new QFrame(this);
    card->setObjectName("card");
    card->setFixedSize(220, 100);  // Make cards slightly wider
    card->setStyleSheet(
        "QFrame#card {"
        "   background-color: white;"
        "   border-radius: 8px;"
        "   padding: 15px;"
        "   margin: 5px;"
        "   border: 1px solid #bdc3c7;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setSpacing(5);
    layout->setContentsMargins(10, 10, 10, 10);  // Add padding inside cards

    QLabel* titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* valueLabel = new QLabel(value, card);
    valueLabel->setObjectName("valueLabel");
    valueLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    valueLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    // Store the value label directly
    if (title == "Total Revenue") totalRevenueCard = valueLabel;
    else if (title == "Total Orders") totalOrdersCard = valueLabel;
    else if (title == "Avg Order Value") avgOrderCard = valueLabel;
    else if (title == "Top Product") topProductCard = valueLabel;

    return card;
}

void AnalyticsForm::updateStats()
{
    // Reset all cards first
    if (totalRevenueCard) totalRevenueCard->setText("$0.00");
    if (totalOrdersCard) totalOrdersCard->setText("0");
    if (avgOrderCard) avgOrderCard->setText("$0.00");
    if (topProductCard) topProductCard->setText("-");

    // Then update with new data
    loadSalesData();
    loadCategoryData();
    updateDashboardCards();
}

void AnalyticsForm::loadSalesData()
{
    QSqlQuery query;

    try {
        QString periodCondition;
        switch(periodComboBox->currentIndex()) {
            case 0: // Today
                periodCondition = "DATE(o.OrderDate) = CURRENT_DATE";
                break;
            case 1: // This Week
                periodCondition = "YEARWEEK(o.OrderDate) = YEARWEEK(CURRENT_DATE)";
                break;
            case 2: // This Month
                periodCondition = "MONTH(o.OrderDate) = MONTH(CURRENT_DATE)";
                break;
            case 3: // This Year
                periodCondition = "YEAR(o.OrderDate) = YEAR(CURRENT_DATE)";
                break;
        }

        // Fixed query to match actual database structure
        QString salesQuery = QString(
            "SELECT p.Name, SUM(od.Quantity) as TotalQty, "
            "SUM(od.Quantity * od.Price) as Revenue "
            "FROM OrderDetails od "
            "JOIN Orders o ON od.OrderID = o.OrderID "
            "JOIN products p ON od.ProductID = p.ProductID "
            "WHERE %1 "
            "GROUP BY p.ProductID, p.Name "
            "ORDER BY Revenue DESC").arg(periodCondition);

        qDebug() << "Executing sales query:" << salesQuery;

        if (!query.exec(salesQuery)) {
            throw std::runtime_error(query.lastError().text().toStdString());
        }

        salesTable->setRowCount(0);
        double totalRevenue = 0;
        int totalOrders = 0;

        while(query.next()) {
            int row = salesTable->rowCount();
            salesTable->insertRow(row);
            salesTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            salesTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));

            double revenue = query.value(2).toDouble();
            totalRevenue += revenue;
            totalOrders++;

            salesTable->setItem(row, 2, new QTableWidgetItem(formatCurrency(revenue)));
        }

        if (totalRevenueCard) {
            totalRevenueCard->setText(formatCurrency(totalRevenue));
        }
        if (totalOrdersCard) {
            totalOrdersCard->setText(QString::number(totalOrders));
        }

    } catch (const std::exception& e) {
        qDebug() << "Error in loadSalesData:" << e.what();
    }
}

void AnalyticsForm::loadCategoryData()
{
    try {
        QString periodCondition;
        switch(periodComboBox->currentIndex()) {
            case 0: // Today
                periodCondition = "DATE(o.OrderDate) = CURRENT_DATE";
                break;
            case 1: // This Week
                periodCondition = "YEARWEEK(o.OrderDate) = YEARWEEK(CURRENT_DATE)";
                break;
            case 2: // This Month
                periodCondition = "MONTH(o.OrderDate) = MONTH(CURRENT_DATE)";
                break;
            case 3: // This Year
                periodCondition = "YEAR(o.OrderDate) = YEAR(CURRENT_DATE)";
                break;
        }

        // Fixed category query
        QString categoryQuery = QString(
            "SELECT c.Category, COUNT(DISTINCT o.OrderID) as TotalSales "
            "FROM categories c "
            "LEFT JOIN products p ON p.Category = c.Category "
            "LEFT JOIN OrderDetails od ON od.ProductID = p.ProductID "
            "LEFT JOIN Orders o ON o.OrderID = od.OrderID "
            "WHERE %1 "
            "GROUP BY c.Category "
            "ORDER BY TotalSales DESC").arg(periodCondition);

        QSqlQuery query;
        if (!query.exec(categoryQuery)) {
            throw std::runtime_error(query.lastError().text().toStdString());
        }

        categoryTable->setRowCount(0);
        while(query.next()) {
            int row = categoryTable->rowCount();
            categoryTable->insertRow(row);
            categoryTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            categoryTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        }

    } catch (const std::exception& e) {
        qDebug() << "Error in loadCategoryData:" << e.what();
    }
}

void AnalyticsForm::onPeriodComboBoxChanged(int)
{
    updateStats();
}

AnalyticsForm::~AnalyticsForm()
{
    // No ui member to delete
}

void AnalyticsForm::updateDashboardCards()
{
    QSqlQuery query;
    QString periodCondition;

    // Get period condition
    switch(periodComboBox->currentIndex()) {
        case 0: // Today
            periodCondition = "DATE(o.OrderDate) = CURRENT_DATE";
            break;
        case 1: // This Week
            periodCondition = "YEARWEEK(o.OrderDate) = YEARWEEK(CURRENT_DATE)";
            break;
        case 2: // This Month
            periodCondition = "MONTH(o.OrderDate) = MONTH(CURRENT_DATE)";
            break;
        case 3: // This Year
            periodCondition = "YEAR(o.OrderDate) = YEAR(CURRENT_DATE)";
            break;
    }

    // Updated top product query with period condition
    QString topProductQuery = QString(
        "SELECT p.Name, SUM(od.Quantity) as Qty "
        "FROM OrderDetails od "
        "JOIN products p ON od.ProductID = p.ProductID "
        "JOIN Orders o ON od.OrderID = o.OrderID "
        "WHERE %1 "
        "GROUP BY p.ProductID, p.Name "
        "ORDER BY Qty DESC "
        "LIMIT 1").arg(periodCondition);

    if (!query.exec(topProductQuery)) {
        qDebug() << "Top product query error:" << query.lastError().text();
        return;
    }

    if (query.next() && topProductCard) {
        topProductCard->setText(query.value(0).toString());
    }

    // Updated average order query with period condition
    QString avgOrderQuery = QString(
        "SELECT AVG(TotalAmount) "
        "FROM Orders o "
        "WHERE %1").arg(periodCondition);

    if (!query.exec(avgOrderQuery)) {
        qDebug() << "Average order query error:" << query.lastError().text();
        return;
    }

    if (query.next() && avgOrderCard) {
        double avgOrder = query.value(0).toDouble();
        avgOrderCard->setText(formatCurrency(avgOrder));
    }
}

void AnalyticsForm::connectSignals()
{
    // Update stats whenever period changes
    connect(periodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) {
                updateStats();
            });

    // Set up a timer for periodic updates (every 30 seconds)
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &AnalyticsForm::updateStats);
    updateTimer->start(30000); // 30 seconds
}

QString AnalyticsForm::formatCurrency(double amount)
{
    return QString("$%1").arg(amount, 0, 'f', 2);
}


// Add missing member function implementations
void AnalyticsForm::updatePeriodText()
{
    QString periodText;
    switch(periodComboBox->currentIndex()) {
        case 0:
            periodText = "Today";
            break;
        case 1:
            periodText = "This Week";
            break;
        case 2:
            periodText = "This Month";
            break;
        case 3:
            periodText = "This Year";
            break;
    }
    setWindowTitle(QString("Analytics - %1").arg(periodText));
}

void AnalyticsForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // Refresh data when form becomes visible
    updateStats();
    updatePeriodText();
}

void AnalyticsForm::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    // Stop timer when form is hidden to save resources
    if (updateTimer && updateTimer->isActive()) {
        updateTimer->stop();
    }
}

void AnalyticsForm::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Adjust table columns on resize
    if (salesTable) {
        salesTable->setColumnWidth(0, salesTable->width() * 0.4);  // Product name
        salesTable->setColumnWidth(1, salesTable->width() * 0.3);  // Quantity
        salesTable->setColumnWidth(2, salesTable->width() * 0.3);  // Revenue
    }
    if (categoryTable) {
        categoryTable->setColumnWidth(0, categoryTable->width() * 0.6);  // Category
        categoryTable->setColumnWidth(1, categoryTable->width() * 0.4);  // Total Sales
    }
}

// Make sure to update the header file (analyticsform.h) to include these new functions:
/*
protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updatePeriodText();
    QTimer* updateTimer = nullptr;
*/
