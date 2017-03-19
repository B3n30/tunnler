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
#include <QStandardItemModel>
#include "room_view_window.h"

RoomViewWindow::RoomViewWindow() {
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
    ConnectWidgetEvents();
#endif

    setWindowTitle(QString("Room"));
    show();

#if 0
    game_list->PopulateAsync(UISettings::values.gamedir, UISettings::values.gamedir_deepscan);

    QStringList args = QApplication::arguments();
    if (args.length() >= 2) {
        BootGame(args[1]);
    }
#endif
}

RoomViewWindow::~RoomViewWindow() {
#if 0
    // will get automatically deleted otherwise
    if (render_window->parent() == nullptr)
        delete render_window;

    Pica::g_debug_context.reset();
#endif
}

void RoomViewWindow::InitializeWidgets() {
#if 0
    game_list = new GameList();
    setCentralWidget(game_list);    
//    ui.horizontalLayout->addWidget(game_list);
#endif


centralwidget = new QWidget();
verticalLayout = new QVBoxLayout();
centralwidget->setLayout(verticalLayout);

//FIXME: !!!! Either make this a headline or move it into the status bar
QLabel* room_name_label = new QLabel();
room_name_label->setText("Tunnler Battlefield");
verticalLayout->addWidget(room_name_label);


chatWidget = new QWidget();
gridLayout = new QGridLayout();
chatWidget->setLayout(gridLayout);

chatLog = new QTextEdit();
chatLog->setReadOnly(true);
gridLayout->addWidget(chatLog, 0, 0);

QStandardItemModel* item_model = nullptr;
memberList = new QTreeView(); // <item row="0" column="1" colspan="2" >
item_model = new QStandardItemModel(memberList);
memberList->setModel(item_model);

memberList->setAlternatingRowColors(true);
memberList->setSelectionMode(QHeaderView::SingleSelection);
memberList->setSelectionBehavior(QHeaderView::SelectRows);
memberList->setVerticalScrollMode(QHeaderView::ScrollPerPixel);
memberList->setHorizontalScrollMode(QHeaderView::ScrollPerPixel);
memberList->setSortingEnabled(true);
memberList->setEditTriggers(QHeaderView::NoEditTriggers);
memberList->setUniformRowHeights(true);
memberList->setContextMenuPolicy(Qt::CustomContextMenu);

enum {
    COLUMN_ACTIVITY,
    COLUMN_NAME,
    COLUMN_GAME,
    // COLUMN_MAC_ADDRESS, // This one would only confuse users
    COLUMN_PING,
    COLUMN_COUNT, // Number of columns
};

item_model->insertColumns(0, COLUMN_COUNT);
item_model->setHeaderData(COLUMN_ACTIVITY, Qt::Horizontal, "Activity");
item_model->setHeaderData(COLUMN_NAME, Qt::Horizontal, "Name");
item_model->setHeaderData(COLUMN_GAME, Qt::Horizontal, "Game");
//item_model->setHeaderData(COLUMN_MAC_ADDRESS, Qt::Horizontal, "MAC Address");
item_model->setHeaderData(COLUMN_PING, Qt::Horizontal, "Ping");


gridLayout->addWidget(memberList, 0, 1);

sayLineEdit = new QLineEdit(); // <item row="1" column="0" colspan="2" >           
gridLayout->addWidget(sayLineEdit, 1, 0);

sayButton = new QPushButton(tr("Say")); // <item row="1" column="2" >
sayButton->show();
gridLayout->addWidget(sayButton, 1, 1);

verticalLayout->addWidget(chatWidget);

setCentralWidget(centralwidget);

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
        label->setVisible(false);
        label->setFrameStyle(QFrame::NoFrame);
        label->setContentsMargins(4, 0, 4, 0);
        statusBar()->addPermanentWidget(label);
    }
    statusBar()->setVisible(true);


emu_speed_label->setText("Connected to 127.0.0.1");
emu_speed_label->show();

#endif




// Some testing
QString html;

std::vector<std::string> chatMessages;

chatMessages.emplace_back("JayFoxRox: Heyho");
chatMessages.emplace_back("JayFoxRox: meep <b>meep</b>");

for(const auto& message : chatMessages) {
  //FIXME: We should probably adopt the established pidgin color scheme where:
  //- either every user has a different color and you appear bold and always in some navy blue
  //- OR you are navy blue and everyone else is read [all names bold]
  // This would allow to faster recognize who said something last / if there have been new messages
  html += QString::fromStdString(message).toHtmlEscaped() + "<br>";
}
html += "<font color=\"gray\"><i>" + QString(tr("XYZ joined the room")).toHtmlEscaped() + "</i></font><br>";
html += "<font color=\"gray\"><i>" + QString(tr("XYZ changed their game")).toHtmlEscaped() + "</i></font><br>";
html += "<font color=\"gray\"><i>" + QString(tr("XYZ left the room")).toHtmlEscaped() + "</i></font><br>";
html += "<font color=\"green\"><b>" + QString(tr("You were dropped from the room (Connection lost)")).toHtmlEscaped() + "</b></font><br>";
html += "<font color=\"blue\">" + QString(tr("RakNet debug message?!")).toHtmlEscaped() + "</font><br>";
chatLog->setHtml(html);


}

#if 0
void RoomViewWindow::InitializeDebugWidgets() {
#if 0
    connect(ui.action_Create_Pica_Surface_Viewer, &QAction::triggered, this,
            &RoomViewWindow::OnCreateGraphicsSurfaceViewer);

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
    connect(this, &RoomViewWindow::EmulationStarting, disasmWidget,
            &DisassemblerWidget::OnEmulationStarting);
    connect(this, &RoomViewWindow::EmulationStopping, disasmWidget,
            &DisassemblerWidget::OnEmulationStopping);

    registersWidget = new RegistersWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, registersWidget);
    registersWidget->hide();
    debug_menu->addAction(registersWidget->toggleViewAction());
    connect(this, &RoomViewWindow::EmulationStarting, registersWidget,
            &RegistersWidget::OnEmulationStarting);
    connect(this, &RoomViewWindow::EmulationStopping, registersWidget,
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
    connect(this, &RoomViewWindow::EmulationStarting, graphicsTracingWidget,
            &GraphicsTracingWidget::OnEmulationStarting);
    connect(this, &RoomViewWindow::EmulationStopping, graphicsTracingWidget,
            &GraphicsTracingWidget::OnEmulationStopping);

    waitTreeWidget = new WaitTreeWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, waitTreeWidget);
    waitTreeWidget->hide();
    debug_menu->addAction(waitTreeWidget->toggleViewAction());
    connect(this, &RoomViewWindow::EmulationStarting, waitTreeWidget,
            &WaitTreeWidget::OnEmulationStarting);
    connect(this, &RoomViewWindow::EmulationStopping, waitTreeWidget,
            &WaitTreeWidget::OnEmulationStopping);
#endif
}

void RoomViewWindow::InitializeRecentFileMenuActions() {
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

void RoomViewWindow::InitializeHotkeys() {
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

void RoomViewWindow::SetDefaultUIGeometry() {
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

void RoomViewWindow::RestoreUIState() {
#if 0
    restoreGeometry(UISettings::values.geometry);
    restoreState(UISettings::values.state);
    render_window->restoreGeometry(UISettings::values.renderwindow_geometry);
#if MICROPROFILE_ENABLED
    microProfileDialog->restoreGeometry(UISettings::values.microprofile_geometry);
    microProfileDialog->setVisible(UISettings::values.microprofile_visible);
#endif

    game_list->LoadInterfaceLayout();

    ui.action_Single_Window_Mode->setChecked(UISettings::values.single_window_mode);
    ToggleWindowMode();

    ui.action_Display_Dock_Widget_Headers->setChecked(UISettings::values.display_titlebar);
    OnDisplayTitleBars(ui.action_Display_Dock_Widget_Headers->isChecked());

    ui.action_Show_Status_Bar->setChecked(UISettings::values.show_status_bar);
    statusBar()->setVisible(ui.action_Show_Status_Bar->isChecked());
#endif
}

void RoomViewWindow::ConnectWidgetEvents() {
#if 0
    connect(game_list, SIGNAL(GameChosen(QString)), this, SLOT(OnGameListLoadFile(QString)));
    connect(game_list, SIGNAL(OpenSaveFolderRequested(u64)), this,
            SLOT(OnGameListOpenSaveFolder(u64)));

    connect(this, SIGNAL(EmulationStarting(EmuThread*)), render_window,
            SLOT(OnEmulationStarting(EmuThread*)));
    connect(this, SIGNAL(EmulationStopping()), render_window, SLOT(OnEmulationStopping()));

    connect(&status_bar_update_timer, &QTimer::timeout, this, &RoomViewWindow::UpdateStatusBar);
#endif
}

void RoomViewWindow::ConnectMenuEvents() {
#if 0
    // File
    connect(ui.action_Load_File, &QAction::triggered, this, &RoomViewWindow::OnMenuLoadFile);
    connect(ui.action_Load_Symbol_Map, &QAction::triggered, this,
            &RoomViewWindow::OnMenuLoadSymbolMap);
    connect(ui.action_Select_Game_List_Root, &QAction::triggered, this,
            &RoomViewWindow::OnMenuSelectGameListRoot);
    connect(ui.action_Exit, &QAction::triggered, this, &QMainWindow::close);

    // Emulation
    connect(ui.action_Start, &QAction::triggered, this, &RoomViewWindow::OnStartGame);
    connect(ui.action_Pause, &QAction::triggered, this, &RoomViewWindow::OnPauseGame);
    connect(ui.action_Stop, &QAction::triggered, this, &RoomViewWindow::OnStopGame);
    connect(ui.action_Configure, &QAction::triggered, this, &RoomViewWindow::OnConfigure);

    // View
    connect(ui.action_Single_Window_Mode, &QAction::triggered, this,
            &RoomViewWindow::ToggleWindowMode);
    connect(ui.action_Display_Dock_Widget_Headers, &QAction::triggered, this,
            &RoomViewWindow::OnDisplayTitleBars);
    connect(ui.action_Show_Status_Bar, &QAction::triggered, statusBar(), &QStatusBar::setVisible);
#endif
}

#endif

void RoomViewWindow::OnDisplayTitleBars(bool show) {
#if 0
    QList<QDockWidget*> widgets = findChildren<QDockWidget*>();

    if (show) {
        for (QDockWidget* widget : widgets) {
            QWidget* old = widget->titleBarWidget();
            widget->setTitleBarWidget(nullptr);
            if (old != nullptr)
                delete old;
        }
    } else {
        for (QDockWidget* widget : widgets) {
            QWidget* old = widget->titleBarWidget();
            widget->setTitleBarWidget(new QWidget());
            if (old != nullptr)
                delete old;
        }
    }
#endif
}

void RoomViewWindow::OnGameListOpenSaveFolder(u64 program_id) {
#if 0
    std::string sdmc_dir = FileUtil::GetUserPath(D_SDMC_IDX);
    std::string path = FileSys::ArchiveSource_SDSaveData::GetSaveDataPathFor(sdmc_dir, program_id);
    QString qpath = QString::fromStdString(path);

    QDir dir(qpath);
    if (!dir.exists()) {
        QMessageBox::critical(this, tr("Error Opening Save Folder"), tr("Folder does not exist!"));
        return;
    }

    LOG_INFO(Frontend, "Opening save data path for program_id=%" PRIu64, program_id);
    QDesktopServices::openUrl(QUrl::fromLocalFile(qpath));
#endif
}

void RoomViewWindow::OnMenuLoadFile() {
#if 0
    QString extensions;
    for (const auto& piece : game_list->supported_file_extensions)
        extensions += "*." + piece + " ";

    QString file_filter = tr("3DS Executable") + " (" + extensions + ")";
    file_filter += ";;" + tr("All Files (*.*)");

    QString filename = QFileDialog::getOpenFileName(this, tr("Load File"),
                                                    UISettings::values.roms_path, file_filter);
    if (!filename.isEmpty()) {
        UISettings::values.roms_path = QFileInfo(filename).path();

        BootGame(filename);
    }
#endif
}

void RoomViewWindow::OnMenuLoadSymbolMap() {
#if 0
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Load Symbol Map"), UISettings::values.symbols_path, tr("Symbol Map (*.*)"));
    if (!filename.isEmpty()) {
        UISettings::values.symbols_path = QFileInfo(filename).path();

        LoadSymbolMap(filename.toStdString());
    }
#endif
}

void RoomViewWindow::OnMenuSelectGameListRoot() {
#if 0
    QString dir_path = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
    if (!dir_path.isEmpty()) {
        UISettings::values.gamedir = dir_path;
        game_list->PopulateAsync(dir_path, UISettings::values.gamedir_deepscan);
    }
#endif
}

void RoomViewWindow::OnMenuRecentFile() {
#if 0
    QAction* action = qobject_cast<QAction*>(sender());
    assert(action);

    QString filename = action->data().toString();
    QFileInfo file_info(filename);
    if (file_info.exists()) {
        BootGame(filename);
    } else {
        // Display an error message and remove the file from the list.
        QMessageBox::information(this, tr("File not found"),
                                 tr("File \"%1\" not found").arg(filename));

        UISettings::values.recent_files.removeOne(filename);
        UpdateRecentFiles();
    }
#endif
}

void RoomViewWindow::OnStartGame() {
#if 0
    emu_thread->SetRunning(true);

    ui.action_Start->setEnabled(false);
    ui.action_Start->setText(tr("Continue"));

    ui.action_Pause->setEnabled(true);
    ui.action_Stop->setEnabled(true);
#endif
}

void RoomViewWindow::OnPauseGame() {
#if 0
    emu_thread->SetRunning(false);

    ui.action_Start->setEnabled(true);
    ui.action_Pause->setEnabled(false);
    ui.action_Stop->setEnabled(true);
#endif
}

void RoomViewWindow::OnStopGame() {
#if 0
    ShutdownGame();
#endif
}

void RoomViewWindow::ToggleWindowMode() {
#if 0
    if (ui.action_Single_Window_Mode->isChecked()) {
        // Render in the main window...
        render_window->BackupGeometry();
        ui.horizontalLayout->addWidget(render_window);
        render_window->setFocusPolicy(Qt::ClickFocus);
        if (emulation_running) {
            render_window->setVisible(true);
            render_window->setFocus();
            game_list->hide();
        }

    } else {
        // Render in a separate window...
        ui.horizontalLayout->removeWidget(render_window);
        render_window->setParent(nullptr);
        render_window->setFocusPolicy(Qt::NoFocus);
        if (emulation_running) {
            render_window->setVisible(true);
            render_window->RestoreGeometry();
            game_list->show();
        }
    }
#endif
}

void RoomViewWindow::OnConfigure() {
#if 0
    ConfigureDialog configureDialog(this);
    auto result = configureDialog.exec();
    if (result == QDialog::Accepted) {
        configureDialog.applyConfiguration();
        render_window->ReloadSetKeymaps();
        config->Save();
    }
#endif
}

void RoomViewWindow::OnSwapScreens() {
#if 0
    Settings::values.swap_screen = !Settings::values.swap_screen;
    Settings::Apply();
#endif
}

void RoomViewWindow::OnCreateGraphicsSurfaceViewer() {
#if 0
    auto graphicsSurfaceViewerWidget = new GraphicsSurfaceWidget(Pica::g_debug_context, this);
    addDockWidget(Qt::RightDockWidgetArea, graphicsSurfaceViewerWidget);
    // TODO: Maybe graphicsSurfaceViewerWidget->setFloating(true);
    graphicsSurfaceViewerWidget->show();
#endif
}

void RoomViewWindow::UpdateStatusBar() {
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

bool RoomViewWindow::ConfirmClose() {
    auto answer = QMessageBox::question(
        this, tr("Citra"),
        tr("Are you sure you want to leave this room? All other members will loose connection to you."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return answer != QMessageBox::No;
}

void RoomViewWindow::closeEvent(QCloseEvent* event) {
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

    game_list->SaveInterfaceLayout();
    SaveHotkeys();

    // Shutdown session if the emu thread is active...
    if (emu_thread != nullptr)
        ShutdownGame();

    render_window->close();

    QWidget::closeEvent(event);
#endif
}
