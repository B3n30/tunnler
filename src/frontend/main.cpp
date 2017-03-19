// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cinttypes>
#include <clocale>
#include <memory>
#include <thread>
#define QT_NO_OPENGL
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QtGui>
#include <QtWidgets>
#include "main.h"
#include "room_view_window.h"
#include "room_list_window.h"

#ifdef QT_STATICPLUGIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif


#ifdef main
#undef main
#endif

int main(int argc, char* argv[]) {
    // Init settings params
    QCoreApplication::setOrganizationName("Tunnler team");
    QCoreApplication::setApplicationName("Tunnler");

    QApplication::setAttribute(Qt::AA_X11InitThreads);
    QApplication app(argc, argv);

    // Qt changes the locale and causes issues in float conversion using std::to_string()
    setlocale(LC_ALL, "C");

    RoomListWindow room_list_window;
    room_list_window.show();

    RoomViewWindow room_view_window;
    room_view_window.show();

    return app.exec();
}
