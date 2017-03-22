// Copyright 2017 Citra Emulator Project
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
#include <QGridLayout>
#include "room_list.h"
#include "room_list_window.h"

RoomListWindow::RoomListWindow() {
#if 0
    setAcceptDrops(true);
    ui.setupUi(this);
    statusBar()->hide();
#endif

    InitializeWidgets();
#if 0
    InitializeDebugWidgets();
    InitializeRecentFileMenuActions();
    InitializeHotkeys();

    SetDefaultUIGeometry();
    RestoreUIState();

    ConnectMenuEvents();
#endif
    ConnectWidgetEvents();

    setWindowTitle(QString("Room List"));
    show();

#if 0
    room_list->PopulateAsync(UISettings::values.gamedir, UISettings::values.gamedir_deepscan);

    QStringList args = QApplication::arguments();
    if (args.length() >= 2) {
        BootGame(args[1]);
    }
#endif
}

RoomListWindow::~RoomListWindow() {
#if 0
    // will get automatically deleted otherwise
    if (render_window->parent() == nullptr)
        delete render_window;

    Pica::g_debug_context.reset();
#endif
}

void RoomListWindow::InitializeWidgets() {
    QGridLayout* centralWidgetLayout = new QGridLayout();
    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(centralWidgetLayout);

    //FIXME: Automaticly keep this in sync with the select lobby server
    QWidget* direct_connection_widget = new QWidget();
    QGridLayout* direct_connection_widget_layout = new QGridLayout();
    direct_connection_widget->setLayout(direct_connection_widget_layout);
    {
        QLabel* server_label = new QLabel(tr("Server"));
        server = new QLineEdit();
        direct_connection_widget_layout->addWidget(server_label);
        direct_connection_widget_layout->addWidget(server);

        QLabel* port_label = new QLabel(tr("Port"));
        port = new QSpinBox();
        port->setRange(1, 65535);
        port->setValue(1234); // FIXME: Pick a proper port and / or retrieve it from tunnler
        direct_connection_widget_layout->addWidget(port_label);
        direct_connection_widget_layout->addWidget(port);

        QLabel* nickname_label = new QLabel(tr("Nickname"));
        QLineEdit* nickname = new QLineEdit();
        direct_connection_widget_layout->addWidget(nickname_label);
        direct_connection_widget_layout->addWidget(nickname);

        QPushButton* join_button = new QPushButton(tr("Join room"));
        direct_connection_widget_layout->addWidget(join_button);
        //FIXME: Disable if IP is empty
    }
    centralWidgetLayout->addWidget(direct_connection_widget);

    QWidget* filter_widget = new QWidget();
    QGridLayout* filter_widget_layout = new QGridLayout();
    filter_widget->setLayout(filter_widget_layout);

    room_list = new RoomList();
    centralWidgetLayout->addWidget(room_list);

    QLabel* gameFilterLabel = new QLabel(tr("Game"));
    gameFilter = new QLineEdit();
    filter_widget_layout->addWidget(gameFilterLabel, 0, 0);
    filter_widget_layout->addWidget(gameFilter, 0, 0);

    QLabel* nameFilterLabel = new QLabel(tr("Name"));
    nameFilter = new QLineEdit();
    filter_widget_layout->addWidget(nameFilterLabel, 1, 0);
    filter_widget_layout->addWidget(nameFilter, 1, 0);

    QCheckBox* show_internet = new QCheckBox(tr("Show internet servers"));
    filter_widget_layout->addWidget(show_internet, 0, 1);

    QCheckBox* show_lan = new QCheckBox(tr("Show LAN servers"));
    filter_widget_layout->addWidget(show_lan, 0, 1);

    not_full = new QCheckBox(tr("Hide full"));
    filter_widget_layout->addWidget(not_full, 0, 1);

    not_empty = new QCheckBox("Hide empty");
    filter_widget_layout->addWidget(not_empty, 1, 1);

    QLabel* ping_filter_label = new QLabel(tr("Ping"));
    pingFilter = new QSpinBox();
    unsigned int maximumPing = 9999;
    pingFilter->setRange(0, maximumPing);
    pingFilter->setValue(maximumPing);
    pingFilter->show();
    filter_widget_layout->addWidget(ping_filter_label, 2, 1);
    filter_widget_layout->addWidget(pingFilter, 2, 1);

    centralWidgetLayout->addWidget(filter_widget);
    setCentralWidget(centralWidget);    

#if 1
    // Create status bar
    emu_speed_label = new QLabel();
    emu_speed_label->setToolTip(tr("Current emulation speed. Values higher or lower than 100% "
                                   "indicate emulation is running faster or slower than a 3DS."));
    game_fps_label = new QLabel();
    game_fps_label->setToolTip(tr("How many frames per second the game is currently displaying. "
                                  "This will vary from game to game and scene to scene."));
    emu_frametime_label = new QLabel();
    emu_frametime_label->setToolTip(
        tr("Time taken to emulate a 3DS frame, not counting framelimiting or v-sync. For "
           "full-speed emulation this should be at most 16.67 ms."));

    for (auto& label : {emu_speed_label, game_fps_label, emu_frametime_label}) {
        //label->setVisible(false);
        label->setFrameStyle(QFrame::NoFrame);
        label->setContentsMargins(4, 0, 4, 0);
        statusBar()->addPermanentWidget(label);
    }
    statusBar()->setVisible(true);



emu_speed_label->setText(tr("5 servers found, 3 hidden / filtered."));

#endif


    for (unsigned i = 0; i < 10; i++) {
        QList<QStandardItem*> l;
        for(unsigned int j = 0; j < 5; j++) {
		        QStandardItem *child = new QStandardItem( QString("Item %0").arg(i)); //, QString("Foo"));
		        child->setEditable( false );
            l.append(child);
        }
        room_list->AddEntry(l);
    }


}

#if 0
void RoomListWindow::InitializeDebugWidgets() {
#if 0
    connect(ui.action_Create_Pica_Surface_Viewer, &QAction::triggered, this,
            &RoomListWindow::OnCreateGraphicsSurfaceViewer);

    QMenu* debug_menu = ui.menu_View_Debugging;

#if MICROPROFILE_ENABLED
    microProfileDialog = new MicroProfileDialog(this);
    microProfileDialog->hide();
    debug_menu->addAction(microProfileDialog->toggleViewAction());
#endif

    disasmWidget = new DisassemblerWidget(this, emu_thread.get());
    addDockWidget(Qt::BottomDockWidgetArea, disasmWidget);
    disasmWidget->hide();
    debug_menu->addAction(disasmWidget->toggleViewAction());
    connect(this, &RoomListWindow::EmulationStarting, disasmWidget,
            &DisassemblerWidget::OnEmulationStarting);
    connect(this, &RoomListWindow::EmulationStopping, disasmWidget,
            &DisassemblerWidget::OnEmulationStopping);

    registersWidget = new RegistersWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, registersWidget);
    registersWidget->hide();
    debug_menu->addAction(registersWidget->toggleViewAction());
    connect(this, &RoomListWindow::EmulationStarting, registersWidget,
            &RegistersWidget::OnEmulationStarting);
    connect(this, &RoomListWindow::EmulationStopping, registersWidget,
            &RegistersWidget::OnEmulationStopping);

    callstackWidget = new CallstackWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, callstackWidget);
    callstackWidget->hide();
    debug_menu->addAction(callstackWidget->toggleViewAction());

    graphicsWidget = new GPUCommandStreamWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, graphicsWidget);
    graphicsWidget->hide();
    debug_menu->addAction(graphicsWidget->toggleViewAction());

    graphicsCommandsWidget = new GPUCommandListWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, graphicsCommandsWidget);
    graphicsCommandsWidget->hide();
    debug_menu->addAction(graphicsCommandsWidget->toggleViewAction());

    graphicsBreakpointsWidget = new GraphicsBreakPointsWidget(Pica::g_debug_context, this);
    addDockWidget(Qt::RightDockWidgetArea, graphicsBreakpointsWidget);
    graphicsBreakpointsWidget->hide();
    debug_menu->addAction(graphicsBreakpointsWidget->toggleViewAction());

    graphicsVertexShaderWidget = new GraphicsVertexShaderWidget(Pica::g_debug_context, this);
    addDockWidget(Qt::RightDockWidgetArea, graphicsVertexShaderWidget);
    graphicsVertexShaderWidget->hide();
    debug_menu->addAction(graphicsVertexShaderWidget->toggleViewAction());

    graphicsTracingWidget = new GraphicsTracingWidget(Pica::g_debug_context, this);
    addDockWidget(Qt::RightDockWidgetArea, graphicsTracingWidget);
    graphicsTracingWidget->hide();
    debug_menu->addAction(graphicsTracingWidget->toggleViewAction());
    connect(this, &RoomListWindow::EmulationStarting, graphicsTracingWidget,
            &GraphicsTracingWidget::OnEmulationStarting);
    connect(this, &RoomListWindow::EmulationStopping, graphicsTracingWidget,
            &GraphicsTracingWidget::OnEmulationStopping);

    waitTreeWidget = new WaitTreeWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, waitTreeWidget);
    waitTreeWidget->hide();
    debug_menu->addAction(waitTreeWidget->toggleViewAction());
    connect(this, &RoomListWindow::EmulationStarting, waitTreeWidget,
            &WaitTreeWidget::OnEmulationStarting);
    connect(this, &RoomListWindow::EmulationStopping, waitTreeWidget,
            &WaitTreeWidget::OnEmulationStopping);
#endif
}

void RoomListWindow::InitializeRecentFileMenuActions() {
#if 0
    for (int i = 0; i < max_recent_files_item; ++i) {
        actions_recent_files[i] = new QAction(this);
        actions_recent_files[i]->setVisible(false);
        connect(actions_recent_files[i], SIGNAL(triggered()), this, SLOT(OnMenuRecentFile()));

        ui.menu_recent_files->addAction(actions_recent_files[i]);
    }

    UpdateRecentFiles();
#endif
}

void RoomListWindow::InitializeHotkeys() {
#if 0
    RegisterHotkey("Main Window", "Load File", QKeySequence::Open);
    RegisterHotkey("Main Window", "Swap Screens", QKeySequence::NextChild);
    RegisterHotkey("Main Window", "Start Emulation");
    LoadHotkeys();

    connect(GetHotkey("Main Window", "Load File", this), SIGNAL(activated()), this,
            SLOT(OnMenuLoadFile()));
    connect(GetHotkey("Main Window", "Start Emulation", this), SIGNAL(activated()), this,
            SLOT(OnStartGame()));
    connect(GetHotkey("Main Window", "Swap Screens", render_window), SIGNAL(activated()), this,
            SLOT(OnSwapScreens()));
#endif
}

#endif

void RoomListWindow::SetDefaultUIGeometry() {
#if 0
    // geometry: 55% of the window contents are in the upper screen half, 45% in the lower half
    const QRect screenRect = QApplication::desktop()->screenGeometry(this);

    const int w = screenRect.width() * 2 / 3;
    const int h = screenRect.height() / 2;
    const int x = (screenRect.x() + screenRect.width()) / 2 - w / 2;
    const int y = (screenRect.y() + screenRect.height()) / 2 - h * 55 / 100;

    setGeometry(x, y, w, h);
#endif
}


void RoomListWindow::OnRoomListSelectRoom(QString server, u16 serverPort, bool join) {
  this->server->setText(server);
  this->port->setValue(serverPort);

  if (join) {
    //FIXME: Pretend we pressed the join button
    printf("Pretending to press join lolol");
  }
}

void RoomListWindow::ConnectWidgetEvents() {
    connect(room_list, SIGNAL(RoomChosen(QString, u16, bool)), this, SLOT(OnRoomListSelectRoom(QString, u16, bool)));
#if 0
    connect(room_list, SIGNAL(OpenSaveFolderRequested(u64)), this,
            SLOT(OnRoomListOpenSaveFolder(u64)));

    connect(this, SIGNAL(EmulationStarting(EmuThread*)), render_window,
            SLOT(OnEmulationStarting(EmuThread*)));
    connect(this, SIGNAL(EmulationStopping()), render_window, SLOT(OnEmulationStopping()));

    connect(&status_bar_update_timer, &QTimer::timeout, this, &RoomListWindow::UpdateStatusBar);
#endif
}



void RoomListWindow::UpdateStatusBar() {
#if 0
    if (emu_thread == nullptr) {
        status_bar_update_timer.stop();
        return;
    }

    auto results = Core::System::GetInstance().GetAndResetPerfStats();

    emu_speed_label->setText(tr("Speed: %1%").arg(results.emulation_speed * 100.0, 0, 'f', 0));
    game_fps_label->setText(tr("Game: %1 FPS").arg(results.game_fps, 0, 'f', 0));
    emu_frametime_label->setText(tr("Frame: %1 ms").arg(results.frametime * 1000.0, 0, 'f', 2));

    emu_speed_label->setVisible(true);
    game_fps_label->setVisible(true);
    emu_frametime_label->setVisible(true);
#endif
}

bool RoomListWindow::ConfirmClose() {
    auto answer = QMessageBox::question(
        this, tr("Citra"),
        tr("Are you sure you want to leave this room? All other members will loose connection to you."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return answer != QMessageBox::No;
}

void RoomListWindow::closeEvent(QCloseEvent* event) {
#if 0
    if (!ConfirmClose()) {
        event->ignore();
        return;
    }

    UISettings::values.geometry = saveGeometry();
    UISettings::values.state = saveState();
    UISettings::values.renderwindow_geometry = render_window->saveGeometry();
#if MICROPROFILE_ENABLED
    UISettings::values.microprofile_geometry = microProfileDialog->saveGeometry();
    UISettings::values.microprofile_visible = microProfileDialog->isVisible();
#endif
    UISettings::values.single_window_mode = ui.action_Single_Window_Mode->isChecked();
    UISettings::values.display_titlebar = ui.action_Display_Dock_Widget_Headers->isChecked();
    UISettings::values.show_status_bar = ui.action_Show_Status_Bar->isChecked();
    UISettings::values.first_start = false;

    room_list->SaveInterfaceLayout();
    SaveHotkeys();

    // Shutdown session if the emu thread is active...
    if (emu_thread != nullptr)
        ShutdownGame();

    render_window->close();

    QWidget::closeEvent(event);
#endif
}

