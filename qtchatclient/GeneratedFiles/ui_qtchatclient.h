/********************************************************************************
** Form generated from reading UI file 'qtchatclient.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTCHATCLIENT_H
#define UI_QTCHATCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include <textviewer.h>

QT_BEGIN_NAMESPACE

class Ui_qtchatclientClass
{
public:
    QWidget *centralWidget;
    TextViewer *textBrowser;
    QListView *userListView;
    QTextEdit *textEdit;
    QPushButton *pushButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *qtchatclientClass)
    {
        if (qtchatclientClass->objectName().isEmpty())
            qtchatclientClass->setObjectName(QStringLiteral("qtchatclientClass"));
        qtchatclientClass->resize(600, 533);
        centralWidget = new QWidget(qtchatclientClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        textBrowser = new TextViewer(centralWidget);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        textBrowser->setGeometry(QRect(20, 20, 451, 261));
        userListView = new QListView(centralWidget);
        userListView->setObjectName(QStringLiteral("userListView"));
        userListView->setGeometry(QRect(490, 20, 71, 261));
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(20, 310, 451, 111));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(390, 450, 75, 23));
        qtchatclientClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(qtchatclientClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 23));
        qtchatclientClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(qtchatclientClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        qtchatclientClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(qtchatclientClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        qtchatclientClass->setStatusBar(statusBar);

        retranslateUi(qtchatclientClass);

        QMetaObject::connectSlotsByName(qtchatclientClass);
    } // setupUi

    void retranslateUi(QMainWindow *qtchatclientClass)
    {
        qtchatclientClass->setWindowTitle(QApplication::translate("qtchatclientClass", "qtchatclient", 0));
        pushButton->setText(QApplication::translate("qtchatclientClass", "PushButton", 0));
    } // retranslateUi

};

namespace Ui {
    class qtchatclientClass: public Ui_qtchatclientClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTCHATCLIENT_H
