#ifndef EDITUSERFORM_H
#define EDITUSERFORM_H

#include <QDialog>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui {
class EditUserForm;
}
QT_END_NAMESPACE

class EditUserForm : public QDialog {
    Q_OBJECT

public:
    explicit EditUserForm(QWidget *parent = nullptr);
    ~EditUserForm();
    void loadUserData(int userId);

signals:
    void userUpdated();

private slots:
    void on_saveButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::EditUserForm *ui;
    int currentUserId;
};

#endif // EDITUSERFORM_H
