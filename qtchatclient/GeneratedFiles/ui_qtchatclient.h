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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "textviewer.h"

QT_BEGIN_NAMESPACE

class Ui_qtchatclientClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout;
    TextViewer *textBrowser;
    QWidget *toolbar;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *addPicBtn;
    QPushButton *addVoiceBtn;
    QSpacerItem *horizontalSpacer_2;
    QTextEdit *textEdit;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_2;
    QPushButton *pushButton;
    QListView *userListView;

    void setupUi(QMainWindow *qtchatclientClass)
    {
        if (qtchatclientClass->objectName().isEmpty())
            qtchatclientClass->setObjectName(QStringLiteral("qtchatclientClass"));
        qtchatclientClass->resize(754, 522);
        qtchatclientClass->setStyleSheet(QStringLiteral("background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(235,236,246,255), stop:1 rgba(219,222,233,255))"));
        centralWidget = new QWidget(qtchatclientClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setStyleSheet(QStringLiteral(""));
        horizontalLayout_3 = new QHBoxLayout(centralWidget);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, -1, -1, -1);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        textBrowser = new TextViewer(centralWidget);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        textBrowser->setStyleSheet(QStringLiteral(""));

        verticalLayout->addWidget(textBrowser);

        toolbar = new QWidget(centralWidget);
        toolbar->setObjectName(QStringLiteral("toolbar"));
        toolbar->setStyleSheet(QStringLiteral("border-top:1px solid rgb(209,211,221);"));
        horizontalLayout_2 = new QHBoxLayout(toolbar);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        addPicBtn = new QPushButton(toolbar);
        addPicBtn->setObjectName(QStringLiteral("addPicBtn"));
        addPicBtn->setStyleSheet(QLatin1String("QPushButton {\n"
"	background-color: transparent;\n"
"	border:none;\n"
"	margin-left: 5px;\n"
"	margin-top: 2px;\n"
"    padding: 3px;\n"
"}\n"
"QPushButton:hover {\n"
"	border: 1px solid rgb(173,175,176);\n"
"	border-radius: 2px;\n"
"}\n"
""));
        addPicBtn->setFlat(true);

        horizontalLayout_2->addWidget(addPicBtn);

        addVoiceBtn = new QPushButton(toolbar);
        addVoiceBtn->setObjectName(QStringLiteral("addVoiceBtn"));
        addVoiceBtn->setStyleSheet(QLatin1String("QPushButton {\n"
"	background-color: transparent;\n"
"	border:none;\n"
"	margin-left: 5px;\n"
"	margin-top: 2px;\n"
"    padding: 3px;\n"
"}\n"
"QPushButton:hover {\n"
"	border: 1px solid rgb(173,175,176);\n"
"	border-radius: 2px;\n"
"}"));
        addVoiceBtn->setFlat(true);

        horizontalLayout_2->addWidget(addVoiceBtn);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addWidget(toolbar);

        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setMinimumSize(QSize(0, 100));
        textEdit->setMaximumSize(QSize(16777215, 100));
        textEdit->setStyleSheet(QStringLiteral(""));
        textEdit->setFrameShape(QFrame::Panel);
        textEdit->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(textEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, -1, -1, 8);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setStyleSheet(QLatin1String("\n"
"                        background: #4c70b1;\n"
"                        border-radius: 8;\n"
"                        border:1px solid #4c70b1;\n"
"                        color:white;\n"
"                        padding:5px 10px;\n"
"                        margin:0px 5px;\n"
"                      "));

        horizontalLayout->addWidget(pushButton_2);

        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setStyleSheet(QLatin1String("\n"
"                        background: #4c70b1;\n"
"                        border-radius: 8;\n"
"\n"
"                        border:1px solid #4c70b1;\n"
"                        color:white;\n"
"                        padding:5px 10px;\n"
"                        margin-right: 15px;\n"
"                        margin-left: 10px;\n"
"                      "));

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);


        horizontalLayout_3->addLayout(verticalLayout);

        userListView = new QListView(centralWidget);
        userListView->setObjectName(QStringLiteral("userListView"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(userListView->sizePolicy().hasHeightForWidth());
        userListView->setSizePolicy(sizePolicy);
        userListView->setMinimumSize(QSize(200, 0));
        userListView->setMaximumSize(QSize(200, 16777215));
        userListView->setFrameShape(QFrame::NoFrame);

        horizontalLayout_3->addWidget(userListView);

        qtchatclientClass->setCentralWidget(centralWidget);

        retranslateUi(qtchatclientClass);

        QMetaObject::connectSlotsByName(qtchatclientClass);
    } // setupUi

    void retranslateUi(QMainWindow *qtchatclientClass)
    {
        qtchatclientClass->setWindowTitle(QApplication::translate("qtchatclientClass", "qtchatclient", 0));
        addPicBtn->setText(QString());
        addVoiceBtn->setText(QString());
        pushButton_2->setText(QApplication::translate("qtchatclientClass", "\345\205\263\351\227\255(&C)", 0));
        pushButton->setText(QApplication::translate("qtchatclientClass", "\345\217\221\351\200\201(&S)", 0));
    } // retranslateUi

};

namespace Ui {
    class qtchatclientClass: public Ui_qtchatclientClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTCHATCLIENT_H
