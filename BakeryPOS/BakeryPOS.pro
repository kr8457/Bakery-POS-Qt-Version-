QT       += core gui sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Dashboard.cpp \
    EditProductForm.cpp \
    EditUserForm.cpp \
    analyticsform.cpp \
    cashierform.cpp \
    editcategoryform.cpp \
    login.cpp \
    main.cpp \
    CustomTableDelegate.cpp

HEADERS += \
    Dashboard.h \
    EditProductForm.h \
    EditUserForm.h \
    analyticsform.h \
    cashierform.h \
    editcategoryform.h \
    login.h \
    CustomTableDelegate.h

FORMS += \
    dashboard.ui \
    analyticsform.ui \
    editcategoryform.ui \
    login.ui \
    editproductform.ui \
    edituserform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    fonts/Poppins-Bold.ttf \
    fonts/Poppins-Italic.ttf \
    fonts/Poppins-Medium.ttf \
    fonts/Poppins-MediumItalic.ttf \
    fonts/Poppins-Regular.ttf \
    fonts/Poppins-SemiBold.ttf \
    fonts/Poppins-SemiBoldItalic.ttf \
    icons/search.png

