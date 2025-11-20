#include "Dashboard.h"
#include "EditProductForm.h"
#include "EditUserForm.h"
#include "EditCategoryForm.h"
#include "ui_Dashboard.h"

#include "Utils.h"
#include <login.h>

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QString>

#include <QButtonGroup>
#include <QMessageBox>
#include <QPushButton>

dashboard::dashboard(QWidget *parent, int userId)
    : QMainWindow(parent)
    , ui(new Ui::dashboard)
    , currentUserId(userId)      // Match header order
    , Model(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    
    // Initialize member variables
    productsTable = nullptr;
    updateProductsTable = nullptr;
    usersTable = nullptr;
    updateUsersTable = nullptr;
    categoryTable = nullptr;
    updateCategoryTable = nullptr;
    cashierForm = nullptr;
    analyticsForm = nullptr;

    // Set base query
    BaseQuery = "SELECT * FROM products";  // Default query

    // Setup UI and load data
    setupUI();
    loadData();
    connectSignals();

    // Set window properties
    this->setWindowTitle("BakeryPOS - Dashboard");
    this->showMaximized();
}

dashboard::~dashboard()
{
    // Clean up analytics resources
    if (analyticsForm) {
        delete analyticsForm;
        analyticsForm = nullptr;
    }
    
    // Clean up cashier form
    if (cashierForm) {
        delete cashierForm;
        cashierForm = nullptr;
    }
    
    // Clean up database connections
    QString connectionName = QString("CashierConnection_%1").arg(currentUserId);
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }
    
    if (analyticsPageIndex != -1) {
        QWidget* widget = ui->MainDisplayStackedWidget->widget(analyticsPageIndex);
        ui->MainDisplayStackedWidget->removeWidget(widget);
        delete widget;
        analyticsPageIndex = -1;
    }

    delete ui;
}

void dashboard::setupUI()
{
    // painting the table
    ui->ProductPageTableView->setItemDelegate(
        new CustomTableDelegate(ui->ProductPageTableView));
    ui->UserPageTableView->setItemDelegate(
        new CustomTableDelegate(ui->ProductPageTableView));

    // Distribute columns based on content size
    ui->ProductPageTableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    // left-align header text, vertically centered
    ui->ProductPageTableView->horizontalHeader()->setDefaultAlignment(
        Qt::AlignLeft | Qt::AlignVCenter);
    // Distribute columns based on content size
    ui->UserPageTableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    // left-align header text, vertically centered
    ui->UserPageTableView->horizontalHeader()->setDefaultAlignment(
        Qt::AlignLeft | Qt::AlignVCenter);

    // Setup Category Table View
    ui->CategoryPageTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->CategoryPageTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->CategoryPageTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->CategoryPageTableView->setItemDelegate(
        new CustomTableDelegate(ui->CategoryPageTableView));
    ui->CategoryPageTableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    ui->CategoryPageTableView->horizontalHeader()->setDefaultAlignment(
        Qt::AlignLeft | Qt::AlignVCenter);

    // Setup sidebar buttons
    QButtonGroup *SidebarGroup = new QButtonGroup(this);
    SidebarGroup->setExclusive(true);

    SidebarGroup->addButton(ui->UsersButton);
    SidebarGroup->addButton(ui->InvoiceButton);
    SidebarGroup->addButton(ui->ProductsButton);
    SidebarGroup->addButton(ui->CategoriesButton);
    SidebarGroup->addButton(ui->AnalyticsButton);

    // Connect table header clicks
    connect(ui->ProductPageTableView->horizontalHeader(),
            &QHeaderView::sectionClicked, this,
            &dashboard::OnProductHeaderSectionClicked);
    connect(ui->UserPageTableView->horizontalHeader(),
            &QHeaderView::sectionClicked, this,
            &dashboard::OnUserHeaderSectionClicked);
    connect(ui->CategoryPageTableView->horizontalHeader(),
            &QHeaderView::sectionClicked,
            this,
            &dashboard::OnCategoryHeaderSectionClicked);

    // Set initial page
    ui->MainDisplayStackedWidget->setCurrentIndex(0);

    // Debug output
    qDebug() << "MainDisplayStackedWidget setup:";
    qDebug() << "Total pages:" << ui->MainDisplayStackedWidget->count();
    for(int i = 0; i < ui->MainDisplayStackedWidget->count(); i++) {
        QWidget* page = ui->MainDisplayStackedWidget->widget(i);
        qDebug() << "Page" << i << ":" << page->objectName();
    }
}

void dashboard::refreshProductData() {
    // Refresh the product data in the table
    ApplyFiltersForProducts();
    qDebug() << "Product data refreshed after edit";
}

void dashboard::ApplyFiltersForProducts(const QString &SortColumn,
                                        const QString &SortOrder) {
    QString     Query = BaseQuery;
    QStringList Conditions;

    if (!CurrentCategoryFilter.isEmpty() && CurrentCategoryFilter != "All") {
        Conditions << QString("Category = '%1'").arg(CurrentCategoryFilter);
    }

    if (!CurrentSearchFilter.isEmpty()) {
        Conditions << QString("Name LIKE '%1%'").arg(CurrentSearchFilter);
    }

    if (!Conditions.isEmpty()) {
        Query += " WHERE " + Conditions.join(" AND ");
    }

    // Add ORDER BY if sorting is specified
    if (!SortColumn.isEmpty()) {
        Query += QString(" ORDER BY `%1` %2").arg(SortColumn, SortOrder);
    }

    qDebug() << "Final query:" << Query;
    Model->setQuery(Query);
    UpdateProductRecordCountLabel();
}

void dashboard::ApplyFiltersForUsers(const QString &SortColumn,
                                     const QString &SortOrder) {
    QString     Query = BaseQuery;
    QStringList Conditions;

    if (!CurrentCategoryFilter.isEmpty() && CurrentCategoryFilter != "All") {
        Conditions << QString("Role = '%1'").arg(CurrentCategoryFilter);
    }

    if (!CurrentSearchFilter.isEmpty()) {
        Conditions << QString("username LIKE '%1%'").arg(CurrentSearchFilter);
    }

    if (!Conditions.isEmpty()) {
        Query += " WHERE " + Conditions.join(" AND ");
    }

    // Add ORDER BY if sorting is specified
    if (!SortColumn.isEmpty()) {
        Query += QString(" ORDER BY `%1` %2").arg(SortColumn, SortOrder);
    }

    qDebug() << "Final query:" << Query;
    Model->setQuery(Query);
    UpdateUserRecordCountLabel();
}

void dashboard::OnProductHeaderSectionClicked(int LogicalIndex) {
    static int  LastSortedColumn = -1;
    static bool Ascending        = true;

    QString ColumnName =
        Model->headerData(LogicalIndex, Qt::Horizontal).toString();
    ColumnName = ColumnName.trimmed().replace(" ", "_");

    if (LastSortedColumn == LogicalIndex) {
        Ascending = !Ascending;
    } else {
        Ascending        = true;
        LastSortedColumn = LogicalIndex;
    }

    QString Order = Ascending ? "ASC" : "DESC";

    // Now apply both filtering and sorting
    ApplyFiltersForProducts(ColumnName, Order);
    UpdateProductRecordCountLabel();
}

void dashboard::OnUserHeaderSectionClicked(int LogicalIndex) {
    static int  LastSortedColumn = -1;
    static bool Ascending        = true;

    QString ColumnName =
        Model->headerData(LogicalIndex, Qt::Horizontal).toString();
    ColumnName = ColumnName.trimmed().replace(" ", "_");

    if (LastSortedColumn == LogicalIndex) {
        Ascending = !Ascending;
    } else {
        Ascending        = true;
        LastSortedColumn = LogicalIndex;
    }

    QString Order = Ascending ? "ASC" : "DESC";

    // Now apply both filtering and sorting
    ApplyFiltersForUsers(ColumnName, Order);
    UpdateProductRecordCountLabel();
}

void dashboard::UpdateProductRecordCountLabel() {
    ui->NumberOfProductRecordsShownLabel->setText(
        QString("Showing %1 records").arg(Model->rowCount()));
}

void dashboard::UpdateUserRecordCountLabel() {
    ui->NumberOfUserRecordsShownLabel->setText(
        QString("Showing %1 records").arg(Model->rowCount()));
}

void dashboard::on_EditProductButton_clicked() {
    QModelIndexList selectedIndexes =
        ui->ProductPageTableView->selectionModel()->selectedRows();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();

        // Get ProductID from the first column (assuming ProductID is in column
        // 0)
        QModelIndex productIdIndex =
            ui->ProductPageTableView->model()->index(selectedRow, 0);
        int productId =
            ui->ProductPageTableView->model()->data(productIdIndex).toInt();

        // Get Product Name for confirmation
        QModelIndex nameIndex = ui->ProductPageTableView->model()->index(
            selectedRow, 1); // column 1 = Name
        QString productName =
            ui->ProductPageTableView->model()->data(nameIndex).toString();

        qDebug() << "Selected Product ID:" << productId;
        qDebug() << "Product Name:" << productName;

        // Create and show the edit form
        EditProductForm *editForm = new EditProductForm(this);
        editForm->loadProductData(productId);

        // Connect the productUpdated signal to refresh the table
        connect(editForm, &EditProductForm::productUpdated, this,
                &dashboard::refreshProductData);

        editForm->show();

    } else {
        QMessageBox::warning(this, "No Selection",
                             "Please select a product to edit.");
    }
}
void dashboard::on_DeleteProductButton_clicked() {
    QModelIndexList selectedIndexes =
        ui->ProductPageTableView->selectionModel()->selectedRows();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();

        // Get ProductID
        QModelIndex productIdIndex =
            ui->ProductPageTableView->model()->index(selectedRow, 0);
        int productId =
            ui->ProductPageTableView->model()->data(productIdIndex).toInt();

        // Get detailed product information from database
        QSqlQuery query;
        query.prepare("SELECT ProductID, Name, Category, PricePerKg, "
                      "PricePerUnit, StockQuantity, UnitType, date_added, "
                      "status FROM products WHERE ProductID = ?");
        query.bindValue(0, productId);

        if (query.exec() && query.next()) {
            // Extract all product details
            QString  productName   = query.value("Name").toString();
            QString  category      = query.value("Category").toString();
            QVariant pricePerKg    = query.value("PricePerKg");
            QVariant pricePerUnit  = query.value("PricePerUnit");
            double   stockQuantity = query.value("StockQuantity").toDouble();
            QString  unitType      = query.value("UnitType").toString();
            QString  dateAdded     = query.value("date_added").toString();
            QString  status        = query.value("status").toString();

            // Format pricing information
            QString priceInfo;
            if (!pricePerKg.isNull() && !pricePerUnit.isNull()) {
                priceInfo = QString("Price per KG: $%1\nPrice per Unit: $%2")
                                .arg(pricePerKg.toDouble(), 0, 'f', 2)
                                .arg(pricePerUnit.toDouble(), 0, 'f', 2);
            } else if (!pricePerKg.isNull()) {
                priceInfo = QString("Price per KG: $%1")
                                .arg(pricePerKg.toDouble(), 0, 'f', 2);
            } else if (!pricePerUnit.isNull()) {
                priceInfo = QString("Price per Unit: $%1")
                                .arg(pricePerUnit.toDouble(), 0, 'f', 2);
            } else {
                priceInfo = "No pricing information available";
            }

            // Create detailed confirmation message
            QString detailedMessage =
                QString("Product Details:\n\n"
                        "Product ID: %1\n"
                        "Name: %2\n"
                        "Category: %3\n"
                        "%4\n"
                        "Stock Quantity: %5 %6\n"
                        "Status: %7\n"
                        "Date Added: %8\n\n"
                        "⚠️ WARNING: This action cannot be undone!")
                    .arg(productId)
                    .arg(productName)
                    .arg(category)
                    .arg(priceInfo)
                    .arg(stockQuantity, 0, 'f', 2)
                    .arg(unitType)
                    .arg(status)
                    .arg(dateAdded);

            // Create message box with explicit styling to fix white text on
            // white background
            QMessageBox confirmBox;
            confirmBox.setParent(this);
            confirmBox.setWindowTitle("Confirm Product Deletion");
            confirmBox.setIcon(QMessageBox::Question);

            // Set explicit styling to ensure visibility
            confirmBox.setStyleSheet("QMessageBox { "
                                     "   background-color: #f0f0f0; "
                                     "   color: #000000; "
                                     "} "
                                     "QMessageBox QLabel { "
                                     "   color: #000000; "
                                     "   background-color: transparent; "
                                     "} "
                                     "QPushButton { "
                                     "   background-color: #e0e0e0; "
                                     "   border: 1px solid #a0a0a0; "
                                     "   color: #000000; "
                                     "   padding: 5px 15px; "
                                     "   border-radius: 3px; "
                                     "} "
                                     "QPushButton:hover { "
                                     "   background-color: #d0d0d0; "
                                     "} "
                                     "QPushButton:pressed { "
                                     "   background-color: #c0c0c0; "
                                     "}");

            confirmBox.setText(
                QString("Are you sure you want to delete product: \"%1\"?\n\n"
                        "Product ID: %2\n"
                        "Category: %3\n"
                        "%4\n"
                        "Stock: %5 %6\n\n"
                        "⚠️ WARNING: This action cannot be undone!")
                    .arg(productName)
                    .arg(productId)
                    .arg(category)
                    .arg(priceInfo)
                    .arg(stockQuantity, 0, 'f', 2)
                    .arg(unitType));

            confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmBox.setDefaultButton(QMessageBox::No);

            int result = confirmBox.exec();

            if (result == QMessageBox::Yes) {
                // Proceed with deletion
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM products WHERE ProductID = ?");
                deleteQuery.bindValue(0, productId);

                if (deleteQuery.exec()) {
                    QMessageBox::information(
                        this, "Success",
                        QString("Product \"%1\" has been deleted successfully!")
                            .arg(productName));
                    refreshProductData();
                } else {
                    QMessageBox::critical(
                        this, "Database Error",
                        "Failed to delete product from database:\n" +
                            deleteQuery.lastError().text());
                }
            }
            // If result is QMessageBox::No, the dialog simply closes normally

        } else {
            QMessageBox::critical(this, "Error",
                                  "Failed to retrieve product details:\n" +
                                      query.lastError().text());
        }
    } else {
        QMessageBox::warning(
            this, "No Selection",
            "Please select a product from the table to delete.");
    }
}
void dashboard::on_AddProductButton_clicked() {
    // Create a new EditProductForm for adding (without loading existing data)
    EditProductForm *addForm = new EditProductForm(this);
    addForm->setWindowTitle("Add New Product");

    // Connect the productUpdated signal to refresh the table
    connect(addForm, &EditProductForm::productUpdated, this,
            &dashboard::refreshProductData);

    addForm->show();
}

void dashboard::on_FilterRoleComboBox_currentIndexChanged() {
    CurrentCategoryFilter = ui->FilterRoleComboBox->currentText();
    ApplyFiltersForUsers();
}

void dashboard::on_UsersButton_clicked() {
    ui->MainDisplayStackedWidget->setCurrentIndex(3);
    this->BaseQuery = "SELECT * FROM users";
    this->Model->setQuery(BaseQuery);
    ui->UserPageTableView->setModel(Model);
    UpdateUserRecordCountLabel();
}

void dashboard::on_ProductsButton_clicked() {
    ui->MainDisplayStackedWidget->setCurrentIndex(0);
    this->BaseQuery = "SELECT * FROM products";
    this->Model->setQuery(BaseQuery);
    ui->ProductPageTableView->setModel(Model);
}

void dashboard::on_SearchUserByNameLineEdit_returnPressed() {
    CurrentSearchFilter = ui->SearchUserByNameLineEdit->text();
    ApplyFiltersForUsers();
}

void dashboard::on_CategoriesButton_clicked()
{
    // Set correct index for CategoryManagementPage
    ui->MainDisplayStackedWidget->setCurrentIndex(6);
    
    QString query = "SELECT ID as 'ID', "
                   "Category as 'Category Name', "
                   "Date as 'Date Added' "
                   "FROM categories";
    
    // Add debug output
    qDebug() << "Executing query:" << query;
    
    Model->setQuery(query);
    
    // Check for query errors
    if (Model->lastError().isValid()) {
        qDebug() << "Query error:" << Model->lastError().text();
        return;
    }
    
    qDebug() << "Row count:" << Model->rowCount();
    
    ui->CategoryPageTableView->setModel(Model);
    ui->CategoryPageTableView->resizeColumnsToContents();
    UpdateCategoryRecordCountLabel();
    
    // Add visibility debug
    qDebug() << "CategoryPageTableView visible:" << ui->CategoryPageTableView->isVisible();
    qDebug() << "StackedWidget current index:" << ui->MainDisplayStackedWidget->currentIndex();
}

void dashboard::on_FilterRoleComboBox_2_currentIndexChanged()
{
    CurrentCategoryFilter = ui->FilterRoleComboBox_2->currentText();
    ApplyFiltersForCategories();
}

void dashboard::UpdateCategoryRecordCountLabel()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM categories");
    if (query.next()) {
        int count = query.value(0).toInt();
        ui->CategoryRecordCountLabel->setText(QString::number(count) + " Records Found");
    }
}

void dashboard::ApplyFiltersForCategories(const QString &SortColumn, const QString &SortOrder)
{
    QString queryStr = "SELECT * FROM categories";
    
    // Update to use correct widget name
    QString searchText = ui->SearchCategoryByNameLineEdit->text();
    if (!searchText.isEmpty()) {
        queryStr += " WHERE Category LIKE '%" + searchText + "%'";
    }
    
    // Add sorting if specified
    if (!SortColumn.isEmpty() && !SortOrder.isEmpty()) {
        queryStr += " ORDER BY " + SortColumn + " " + SortOrder;
    }
    
    Model->setQuery(queryStr);
    UpdateCategoryRecordCountLabel();  // Fixed: Changed from UpdateCategoryRecordCount to UpdateCategoryRecordCountLabel
}

void dashboard::on_SearchCategoryByNameLineEdit_returnPressed()
{
    ApplyFiltersForCategories();
}

void dashboard::OnCategoryHeaderSectionClicked(int LogicalIndex)
{
    // Get the column name from the model
    QString columnName = Model->headerData(LogicalIndex, Qt::Horizontal).toString();
    
    static bool ascending = true;
    QString sortOrder = ascending ? "ASC" : "DESC";
    
    ApplyFiltersForCategories(columnName, sortOrder);
    ascending = !ascending;
}

void dashboard::on_LogoutButton_clicked() {
    if (QMessageBox::question(this, "Confirm Logout",
                              "Are you sure you want to logout?") ==
        QMessageBox::Yes) {
        this->close();
        login *Lgn = new login();
        Lgn->show();
    }
}

void dashboard::on_EditUserButton_clicked() {
    QModelIndexList selectedIndexes = ui->UserPageTableView->selectionModel()->selectedRows();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex userIdIndex = ui->UserPageTableView->model()->index(selectedRow, 0);
        int userId = ui->UserPageTableView->model()->data(userIdIndex).toInt();

        EditUserForm *editForm = new EditUserForm(this);
        editForm->loadUserData(userId);

        connect(editForm, &EditUserForm::userUpdated, this, [this]() {
            this->Model->setQuery(this->BaseQuery);
            UpdateUserRecordCountLabel();
        });

        editForm->show();
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a user to edit.");
    }
}

void dashboard::on_DeleteUserButton_clicked() {
    QModelIndexList selectedIndexes = ui->UserPageTableView->selectionModel()->selectedRows();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex userIdIndex = ui->UserPageTableView->model()->index(selectedRow, 0);
        int userId = ui->UserPageTableView->model()->data(userIdIndex).toInt();
        
        QModelIndex usernameIndex = ui->UserPageTableView->model()->index(selectedRow, 1);
        QString username = ui->UserPageTableView->model()->data(usernameIndex).toString();

        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "Confirm Deletion",
            QString("Are you sure you want to delete user '%1'?\nThis action cannot be undone.").arg(username),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            QSqlQuery query;
            query.prepare("DELETE FROM users WHERE id = ?");
            query.bindValue(0, userId);

            if (query.exec()) {
                Model->setQuery(BaseQuery);
                UpdateUserRecordCountLabel();
                QMessageBox::information(this, "Success", "User deleted successfully.");
            } else {
                QMessageBox::critical(this, "Error", 
                    "Failed to delete user: " + query.lastError().text());
            }
        }
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a user to delete.");
    }
}

void dashboard::on_AddUserButton_clicked() {
    EditUserForm *addForm = new EditUserForm(this);
    addForm->setWindowTitle("Add New User");

    connect(addForm, &EditUserForm::userUpdated, this, [this]() {
        this->Model->setQuery(this->BaseQuery);
        UpdateUserRecordCountLabel();
    });

    addForm->show();
}

void dashboard::on_AddCategoryButton_clicked()
{
    EditCategoryForm *addForm = new EditCategoryForm(this);
    addForm->setWindowTitle("Add New Category");

    connect(addForm, &EditCategoryForm::categoryUpdated, this, [this]() {
        this->Model->setQuery("SELECT * FROM categories");
        UpdateCategoryRecordCountLabel();
    });

    addForm->show();
}

void dashboard::on_EditCategoryButton_clicked()
{
    QModelIndexList selectedIndexes = ui->CategoryPageTableView->selectionModel()->selectedRows();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex categoryIdIndex = ui->CategoryPageTableView->model()->index(selectedRow, 0);
        int categoryId = ui->CategoryPageTableView->model()->data(categoryIdIndex).toInt();

        EditCategoryForm *editForm = new EditCategoryForm(this);
        editForm->loadCategoryData(categoryId);

        connect(editForm, &EditCategoryForm::categoryUpdated, this, [this]() {
            this->Model->setQuery("SELECT * FROM categories");
            UpdateCategoryRecordCountLabel();
        });

        editForm->show();
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a category to edit.");
    }
}

void dashboard::on_DeleteCategoryButton_clicked()
{
    QModelIndexList selectedIndexes = ui->CategoryPageTableView->selectionModel()->selectedRows();

    if (!selectedIndexes.isEmpty()) {
        int selectedRow = selectedIndexes.first().row();
        QModelIndex categoryIdIndex = ui->CategoryPageTableView->model()->index(selectedRow, 0);
        int categoryId = ui->CategoryPageTableView->model()->data(categoryIdIndex).toInt();
        
        QModelIndex categoryNameIndex = ui->CategoryPageTableView->model()->index(selectedRow, 1);
        QString categoryName = ui->CategoryPageTableView->model()->data(categoryNameIndex).toString();

        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Confirm Deletion",
            QString("Are you sure you want to delete category '%1'?\nThis action cannot be undone.").arg(categoryName),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            QSqlQuery query;
            query.prepare("DELETE FROM categories WHERE ID = ?");
            query.bindValue(0, categoryId);

            if (query.exec()) {
                Model->setQuery("SELECT * FROM categories");
                UpdateCategoryRecordCountLabel();
                QMessageBox::information(this, "Success", "Category deleted successfully.");
            } else {
                QMessageBox::critical(this, "Error",
                    "Failed to delete category: " + query.lastError().text());
            }
        }
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a category to delete.");
    }
}

void dashboard::on_FilterCategoryComboBox_currentIndexChanged()
{
    CurrentCategoryFilter = ui->FilterCategoryComboBox->currentText();
    ApplyFiltersForProducts();
}

void dashboard::on_SearchProductByNameLineEdit_returnPressed()
{
    CurrentSearchFilter = ui->SearchProductByNameLineEdit->text();
    ApplyFiltersForProducts();
}

void dashboard::on_AnalyticsButton_clicked()
{
    if (!analyticsForm) {
        analyticsForm = new AnalyticsForm(this);
        QWidget* page = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(analyticsForm);
        
        // Add to stacked widget
        int index = ui->MainDisplayStackedWidget->addWidget(page);
        analyticsPageIndex = index;
    }
    
    ui->MainDisplayStackedWidget->setCurrentIndex(analyticsPageIndex);
}

void dashboard::setupCashierPage()
{
    if (!cashierForm) {
        // Get current user ID
        int userId = getCurrentUserId();
        
        // Check if we need a new database connection
        QString connectionName = QString("CashierConnection_%1").arg(userId);
        if (!QSqlDatabase::contains(connectionName)) {
            QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connectionName);
            db.setHostName("localhost");
            db.setDatabaseName("mydb");
            db.setUserName("root");
            db.setPassword("khalid");
            
            if (!db.open()) {
                qDebug() << "Failed to open database for cashier:" << db.lastError().text();
                return;
            }
        }
        
        cashierForm = new CashierForm(this, userId);
        
        QWidget* cashierPage = ui->MainDisplayStackedWidget->widget(9);
        if (!cashierPage->layout()) {
            QVBoxLayout* layout = new QVBoxLayout(cashierPage);
            layout->setContentsMargins(0, 0, 0, 0);
        }
        cashierPage->layout()->addWidget(cashierForm);
    }
}

void dashboard::on_InvoiceButton_clicked()
{
    // Setup cashier page if not already done
    setupCashierPage();
    
    // Switch to cashier page (index 9)
    ui->MainDisplayStackedWidget->setCurrentIndex(9);
    setWindowTitle("BakeryPOS - Invoice");
}

void dashboard::loadData()
{
    try {
        // Initialize with products data first
        Model->setQuery(BaseQuery);
        
        if (Model->lastError().isValid()) {
            qDebug() << "Query error:" << Model->lastError().text();
            return;
        }
        
        // Set model for products table
        ui->ProductPageTableView->setModel(Model);
        
        // Load initial data for tables
        ApplyFiltersForProducts();
        ApplyFiltersForUsers();
        ApplyFiltersForCategories();
        
        // Update record count labels
        UpdateProductRecordCountLabel();
        UpdateUserRecordCountLabel();
        UpdateCategoryRecordCountLabel();
    }
    catch (const std::exception& e) {
        qDebug() << "Error in loadData:" << e.what();
    }
}

// Keep only one implementation of getCurrentUserId
int dashboard::getCurrentUserId() const 
{
    return currentUserId;
}

void dashboard::connectSignals()
{
    // Connect table header clicks for sorting with correct table view names
    connect(ui->ProductPageTableView->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &dashboard::OnProductHeaderSectionClicked);
    
    connect(ui->UserPageTableView->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &dashboard::OnUserHeaderSectionClicked);
    
    connect(ui->CategoryPageTableView->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &dashboard::OnCategoryHeaderSectionClicked);
}
