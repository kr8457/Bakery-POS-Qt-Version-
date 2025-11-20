#ifndef ANALYTICSFORM_H
#define ANALYTICSFORM_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QTimer>
#include <QSqlQuery>

class AnalyticsForm : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyticsForm(QWidget *parent = nullptr);
    ~AnalyticsForm();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateStats();
    void onPeriodComboBoxChanged(int index);

private:
    // UI Elements
    QComboBox* periodComboBox = nullptr;
    QTableWidget* salesTable = nullptr;
    QTableWidget* categoryTable = nullptr;
    QLabel* totalRevenueCard = nullptr;
    QLabel* totalOrdersCard = nullptr;
    QLabel* avgOrderCard = nullptr;
    QLabel* topProductCard = nullptr;
    QTimer* updateTimer = nullptr;

    // Helper functions
    void setupUI();
    void connectSignals();
    void loadSalesData();
    void loadCategoryData();
    void updateDashboardCards();
    void updatePeriodText();
    QString formatCurrency(double amount);
    QFrame* createStatsCard(const QString& title, const QString& value);

    // Constants
    const double TAX_RATE = 0.15; // 15% tax rate
};

#endif // ANALYTICSFORM_H