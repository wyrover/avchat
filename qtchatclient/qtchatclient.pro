TEMPLATE = app
TARGET = qtchatclient
QT += core gui widgets
CONFIG += c++11
QMAKE_CXXFLAGS += -pthread
LIBS += -L../ -lchatclient -lcommon -lcryptopp -pthread
TARGETDEPS += ../libcommon.a ../libchatclient.a
# Input
HEADERS += \
           ChatClientController.h \
           CustomShadowEffect.h \
           debugout.h \
           DropShadowWidget.h \
           FileRequestItem.h \
           FileRequestWidget.h \
           GifManager.h \
           ImageLoader.h \
           ImageViewer.h \
           LoginDialog.h \
           OneToOneRoom.h \
           PictureLabel.h \
           qtchatclient.h \
           ResizeableStackedWidget.h \
           resource.h \
           TextViewer.h \
           TitleBar.h \
           Utils.h \
           ../common/FileUtils.h \
           ../common/buffer.h \
           ../chatclient/ChatClient.h \
           ../common/errcode.h \
           ../chatclient/CommandCenter.h \
           ../chatclient/ErrorManager.h \
           ../chatclient/IChatClientController.h \
           ../chatclient/RequestFilesInfo.h \
           ../chatclient/TcpPeerManager.h \
           ../common/WSAStarter.h \
           ../common/trace.h \
           ../chatclient/MessageError.h \
           ../chatclient/ChatError.h \
           ../chatclient/Utils.h
FORMS += filetransfer.ui \
         filetransitem.ui \
         ImageViewer.ui \
         logindialog.ui \
         OneToOneRoom.ui \
         qtchatclient.ui
SOURCES += \
           CustomShadowEffect.cpp \
           DropShadowWidget.cpp \
           FileRequestItem.cpp \
           FileRequestWidget.cpp \
           GifManager.cpp \
           ImageLoader.cpp \
           ImageViewer.cpp \
           LoginDialog.cpp \
           main.cpp \
           OneToOneRoom.cpp \
           PictureLabel.cpp \
           qtchatclient.cpp \
           ResizeableStackedWidget.cpp \
           TextViewer.cpp \
           TitleBar.cpp \
           Utils.cpp \
           ChatClientController.cpp
RESOURCES += qtchatclient.qrc
TRANSLATIONS += qtchatclient_zh.ts
