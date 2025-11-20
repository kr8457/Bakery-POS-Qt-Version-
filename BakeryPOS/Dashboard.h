#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QMainWindow>
#include <QTableView>
#include "cashierform.h"
#include "analyticsform.h"

namespace Ui {
class dashboard;
}

class dashboard : public QMainWindow {
    Q_OBJECT

  public:
    explicit dashboard(QWidget *parent = nullptr, int userId = -1);
    ~dashboard();

  private slots:
    void on_FilterCategoryComboBox_currentIndexChanged();
    void on_SearchProductByNameLineEdit_returnPressed();
    void on_EditProductButton_clicked();
    void on_DeleteProductButton_clicked();
    void on_AddProductButton_clicked();
    void on_FilterRoleComboBox_currentIndexChanged();
    void on_UsersButton_clicked();
    void on_ProductsButton_clicked();
    void on_SearchUserByNameLineEdit_returnPressed();
    void on_CategoriesButton_clicked();
    void on_LogoutButton_clicked();
    void on_EditUserButton_clicked();
    void on_DeleteUserButton_clicked();
    void on_AddUserButton_clicked();
    void on_AddCategoryButton_clicked();
    void on_EditCategoryButton_clicked();
    void on_DeleteCategoryButton_clicked();
    void on_SearchCategoryByNameLineEdit_returnPressed();
    void on_FilterRoleComboBox_2_currentIndexChanged();
    void OnProductHeaderSectionClicked(int LogicalIndex);
    void OnUserHeaderSectionClicked(int LogicalIndex);
    void OnCategoryHeaderSectionClicked(int LogicalIndex);
    void refreshProductData();
    void UpdateCategoryRecordCountLabel();
    void on_AnalyticsButton_clicked();
    void on_InvoiceButton_clicked();

  private:
    Ui::dashboard  *ui;
    int currentUserId;           // Move up
    QSqlQueryModel *Model;       // Then Model
    QString         BaseQuery;
    QString         CurrentCategoryFilter;
    QString         CurrentSearchFilter;
    AnalyticsForm  *analyticsForm = nullptr;
    CashierForm* cashierForm = nullptr;  // Initialize to nullptr
    int analyticsPageIndex = -1;  // Track the analytics page index

    // Table pointers
    QTableView* productsTable;
    QTableView* updateProductsTable;
    QTableView* usersTable;
    QTableView* updateUsersTable;
    QTableView* categoryTable;
    QTableView* updateCategoryTable;

    // Helper methods
    void setupUI();
    void loadData();
    void connectSignals();
    int getCurrentUserId() const;

    void ApplyFiltersForProducts(const QString &SortColumn = QString(),
                                 const QString &SortOrder = QString());
    void ApplyFiltersForUsers(const QString &SortColumn = QString(),
                              const QString &SortOrder = QString());
    void UpdateProductRecordCountLabel();
    void UpdateUserRecordCountLabel();
    void ApplyFiltersForCategories(const QString &SortColumn = QString(), 
                                 const QString &SortOrder = QString());
    void setupCashierPage();
};

#endif // DASHBOARD_H
