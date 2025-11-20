#include "EditProductForm.h"
#include "ui_EditProductForm.h"

EditProductForm::EditProductForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::EditProductForm), currentProductId(-1) {
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);
    this->setWindowTitle("Edit Product");

    setupCategoryComboBox();
    setupValidation();

    // Initially hide price fields until category is selected
    updatePriceFieldsVisibility();
}

EditProductForm::~EditProductForm() { delete ui; }

void EditProductForm::setupCategoryComboBox() {
    // Replace the ProductCategoryLineEdit with a ComboBox
    categoryComboBox = new QComboBox(this);
    categoryComboBox->setObjectName("ProductCategoryComboBox");
    categoryComboBox->addItem("Select Category");
    categoryComboBox->addItem("Sweet");
    categoryComboBox->addItem("Bakery");
    categoryComboBox->addItem("General");

    // Get the layout and replace the line edit with combo box
    QVBoxLayout *layout = ui->verticalLayout;
    int          index  = layout->indexOf(ui->ProductCategoryLineEdit);

    if (index != -1) {
        layout->removeWidget(ui->ProductCategoryLineEdit);
        ui->ProductCategoryLineEdit->hide();
        layout->insertWidget(index, categoryComboBox);
    }

    connect(categoryComboBox, &QComboBox::currentTextChanged, this,
            &EditProductForm::on_CategoryComboBox_currentTextChanged);
}

void EditProductForm::setupValidation() {
    // Set input validators for price and quantity fields
    QDoubleValidator *priceValidator =
        new QDoubleValidator(0.00, 99999.99, 2, this);
    QDoubleValidator *quantityValidator =
        new QDoubleValidator(0.00, 99999.99, 2, this);

    ui->ProductPricePerKgLineEdit->setValidator(priceValidator);
    ui->ProductPricePerPcsLineEdit->setValidator(priceValidator);
    ui->ProductStockQuantityLineEdit->setValidator(quantityValidator);
}

void EditProductForm::updatePriceFieldsVisibility() {
    QString category = categoryComboBox->currentText();

    if (category == "Select Category") {
        ui->ProductPricePerKgLineEdit->setEnabled(false);
        ui->ProductPricePerPcsLineEdit->setEnabled(false);
        ui->ProductPricePerKgLineEdit->setPlaceholderText(
            "Select category first");
        ui->ProductPricePerPcsLineEdit->setPlaceholderText(
            "Select category first");
    } else {
        ui->ProductPricePerKgLineEdit->setEnabled(true);
        ui->ProductPricePerPcsLineEdit->setEnabled(true);
        ui->ProductPricePerKgLineEdit->setPlaceholderText(
            "Price Per KG (Optional)");
        ui->ProductPricePerPcsLineEdit->setPlaceholderText(
            "Price Per Unit (Optional)");
    }
}

void EditProductForm::loadProductData(int productId) {
    currentProductId = productId;

    QSqlQuery query;
    query.prepare("SELECT Name, Category, PricePerKg, PricePerUnit, "
                  "StockQuantity, UnitType FROM products WHERE ProductID = ?");
    query.bindValue(0, productId);

    if (query.exec() && query.next()) {
        // Load data into form fields
        ui->ProductNameLineEdit->setText(query.value("Name").toString());

        QString category = query.value("Category").toString();
        categoryComboBox->setCurrentText(category);

        // Handle NULL values for prices
        QVariant pricePerKg   = query.value("PricePerKg");
        QVariant pricePerUnit = query.value("PricePerUnit");

        if (!pricePerKg.isNull()) {
            ui->ProductPricePerKgLineEdit->setText(
                QString::number(pricePerKg.toDouble(), 'f', 2));
        } else {
            ui->ProductPricePerKgLineEdit->clear();
        }

        if (!pricePerUnit.isNull()) {
            ui->ProductPricePerPcsLineEdit->setText(
                QString::number(pricePerUnit.toDouble(), 'f', 2));
        } else {
            ui->ProductPricePerPcsLineEdit->clear();
        }

        ui->ProductStockQuantityLineEdit->setText(
            QString::number(query.value("StockQuantity").toDouble(), 'f', 2));

        // Note: UnitType is loaded but we don't display it in the form
        // It will be automatically determined based on which price fields are
        // filled

        updatePriceFieldsVisibility();
    } else {
        QMessageBox::critical(this, "Error",
                              "Failed to load product data: " +
                                  query.lastError().text());
        this->close();
    }
}

bool EditProductForm::validateInput() {
    // Check if name is empty
    if (ui->ProductNameLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Product name cannot be empty.");
        ui->ProductNameLineEdit->setFocus();
        return false;
    }

    // Check if category is selected
    if (categoryComboBox->currentText() == "Select Category") {
        QMessageBox::warning(this, "Validation Error",
                             "Please select a category.");
        categoryComboBox->setFocus();
        return false;
    }

    // Check if at least one price is provided
    bool hasPricePerKg =
        !ui->ProductPricePerKgLineEdit->text().trimmed().isEmpty();
    bool hasPricePerUnit =
        !ui->ProductPricePerPcsLineEdit->text().trimmed().isEmpty();

    if (!hasPricePerKg && !hasPricePerUnit) {
        QMessageBox::warning(
            this, "Validation Error",
            "Please provide at least one price (Per KG or Per Unit).");
        return false;
    }

    // Check if stock quantity is provided
    if (ui->ProductStockQuantityLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Stock quantity cannot be empty.");
        ui->ProductStockQuantityLineEdit->setFocus();
        return false;
    }

    // Validate that stock quantity is not negative
    double stockQuantity = ui->ProductStockQuantityLineEdit->text().toDouble();
    if (stockQuantity < 0) {
        QMessageBox::warning(this, "Validation Error",
                             "Stock quantity cannot be negative.");
        ui->ProductStockQuantityLineEdit->setFocus();
        return false;
    }

    // Check for duplicate product names (only when adding new products)
    if (currentProductId == -1) {
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM products WHERE Name = ?");
        checkQuery.bindValue(0, ui->ProductNameLineEdit->text().trimmed());

        if (checkQuery.exec() && checkQuery.next()) {
            int count = checkQuery.value(0).toInt();
            if (count > 0) {
                QMessageBox::warning(this, "Validation Error",
                                     "A product with this name already exists. "
                                     "Please choose a different name.");
                ui->ProductNameLineEdit->setFocus();
                return false;
            }
        }
    }

    return true;
}

void EditProductForm::on_CategoryComboBox_currentTextChanged(
    const QString & /*text*/) {
    updatePriceFieldsVisibility();
}

void EditProductForm::on_SaveButton_clicked() {
    if (!validateInput()) { return; }

    QSqlQuery query;
    QString   queryString;

    if (currentProductId == -1) {
        // Adding new product - include UnitType and let date_added and status
        // use defaults
        queryString = "INSERT INTO products (Name, Category, PricePerKg, "
                      "PricePerUnit, StockQuantity, UnitType) "
                      "VALUES (?, ?, ?, ?, ?, ?)";
    } else {
        // Updating existing product
        queryString =
            "UPDATE products SET Name = ?, Category = ?, PricePerKg = ?, "
            "PricePerUnit = ?, StockQuantity = ?, UnitType = ? WHERE ProductID "
            "= ?";
    }

    query.prepare(queryString);
    query.bindValue(0, ui->ProductNameLineEdit->text().trimmed());
    query.bindValue(1, categoryComboBox->currentText());

    // Handle price per kg (can be NULL)
    QString pricePerKgText = ui->ProductPricePerKgLineEdit->text().trimmed();
    if (pricePerKgText.isEmpty()) {
        query.bindValue(2, QVariant(QMetaType(QMetaType::Double)));
    } else {
        query.bindValue(2, pricePerKgText.toDouble());
    }

    // Handle price per unit (can be NULL)
    QString pricePerUnitText = ui->ProductPricePerPcsLineEdit->text().trimmed();
    if (pricePerUnitText.isEmpty()) {
        query.bindValue(3, QVariant(QMetaType(QMetaType::Double)));
    } else {
        query.bindValue(3, pricePerUnitText.toDouble());
    }

    query.bindValue(4, ui->ProductStockQuantityLineEdit->text().toDouble());

    // Determine unit type based on which price field is filled
    // Your enum values are 'kg' and 'unit' (not 'pieces')
    QString unitType = "unit"; // default
    if (!pricePerKgText.isEmpty() && pricePerUnitText.isEmpty()) {
        unitType = "kg";
    } else if (pricePerKgText.isEmpty() && !pricePerUnitText.isEmpty()) {
        unitType = "unit";
    } else if (!pricePerKgText.isEmpty() && !pricePerUnitText.isEmpty()) {
        // If both prices are provided, default to 'unit' since we can't use
        // 'both'
        unitType = "unit";
    }
    query.bindValue(5, unitType);

    if (currentProductId != -1) {
        // For update, bind the product ID
        query.bindValue(6, currentProductId);
    }

    if (query.exec()) {
        QString successMessage;
        if (currentProductId == -1) {
            successMessage = "Product added successfully!";
        } else {
            successMessage = "Product updated successfully!";
        }

        QMessageBox::information(this, "Success", successMessage);
        emit productUpdated(); // Notify parent to refresh data
        this->close();
    } else {
        QString errorMessage;
        if (currentProductId == -1) {
            errorMessage = "Failed to add product: " + query.lastError().text();
        } else {
            errorMessage =
                "Failed to update product: " + query.lastError().text();
        }

        QMessageBox::critical(this, "Error", errorMessage);
    }
}

void EditProductForm::on_DiscardButton_clicked() {
    // Ask for confirmation before discarding changes
    if (QMessageBox::question(this, "Confirm Discard",
                              "Are you sure you want to discard changes?",
                              QMessageBox::Yes | QMessageBox::No) ==
        QMessageBox::Yes) {
        this->close();
    }
}

void EditProductForm::clearForm() {
    ui->ProductNameLineEdit->clear();
    categoryComboBox->setCurrentIndex(0);
    ui->ProductPricePerKgLineEdit->clear();
    ui->ProductPricePerPcsLineEdit->clear();
    ui->ProductStockQuantityLineEdit->clear();
    currentProductId = -1;
}
