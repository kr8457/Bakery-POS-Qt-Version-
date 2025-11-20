#include "login.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[]) {

    QApplication App(argc, argv);

    // Loading and setting the font
    int ID = QFontDatabase::addApplicationFont(":/fonts/Poppins-Medium.ttf");
    QString Family = QFontDatabase::applicationFontFamilies(ID).at(0);

    QFont AppFont(Family);

    App.setFont(AppFont);

    // Making the login form
    login w;
    w.show();

    return App.exec();
}
