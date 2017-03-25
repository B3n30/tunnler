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
#include <QSplitter>
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
#endif
    ConnectWidgetEvents();

    setWindowTitle(QString("Room"));
    show();


#if 1
    //FIXME!
    room_member.Join("NoName");
#endif

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

#if 0
    //FIXME: !!!! Either make this a headline or move it into the status bar
    QLabel* room_name_label = new QLabel();
    room_name_label->setText("Tunnler Battlefield");
    verticalLayout->addWidget(room_name_label);
#endif

    QSplitter* splitter = new QSplitter();

    QWidget* splitter_left = new QWidget();
    QVBoxLayout* splitter_left_layout = new QVBoxLayout();
    splitter_left->setLayout(splitter_left_layout);
    splitter->insertWidget(0, splitter_left);

    QWidget* splitter_right = new QWidget();
    QVBoxLayout* splitter_right_layout = new QVBoxLayout();
    splitter_right->setLayout(splitter_right_layout);
    splitter->insertWidget(1, splitter_right);

    chatWidget = new QWidget();
    gridLayout = new QGridLayout();
    chatWidget->setLayout(gridLayout);

    chatLog = new QTextEdit();
    chatLog->setReadOnly(true);
    splitter_left_layout->addWidget(chatLog);

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
        COLUMN_NAME,
        COLUMN_GAME,
        COLUMN_MAC_ADDRESS, // This one would only confuse users
        COLUMN_ACTIVITY,
        COLUMN_PING,
        COLUMN_COUNT, // Number of columns
    };

    item_model->insertColumns(0, COLUMN_COUNT);
    item_model->setHeaderData(COLUMN_NAME, Qt::Horizontal, "Name");
    item_model->setHeaderData(COLUMN_GAME, Qt::Horizontal, "Game");
    item_model->setHeaderData(COLUMN_MAC_ADDRESS, Qt::Horizontal, "MAC Address");
    item_model->setHeaderData(COLUMN_ACTIVITY, Qt::Horizontal, "Activity");
    item_model->setHeaderData(COLUMN_PING, Qt::Horizontal, "Ping");


    memberList->header()->setStretchLastSection(false);
//    memberList->header()->setSectionResizeMode(COLUMN_NAME, QHeaderView::Stretch);   
    memberList->header()->setSectionResizeMode(COLUMN_GAME, QHeaderView::Stretch);   

    splitter_right_layout->addWidget(memberList);

    sayLineEdit = new QLineEdit(); // <item row="1" column="0" colspan="2" >           
    splitter_left_layout->addWidget(sayLineEdit);

    sayButton = new QPushButton(tr("Say")); // <item row="1" column="2" >
    sayButton->show();
    splitter_right_layout->addWidget(sayButton);

    verticalLayout->addWidget(splitter);

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

#if 0
    html += "<font color=\"gray\"><i>" + QString(tr("Connecting..")).toHtmlEscaped() + "</i></font><br>";
    html += "<font color=\"gray\"><i>" + QString(tr("Connected")).toHtmlEscaped() + "</i></font><br>";
#else
    html += "<font color=\"green\"><b>" + QString(tr("Connecting..")).toHtmlEscaped() + "</b></font><br>";

#endif

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
#endif

void RoomViewWindow::UpdateMemberList() {
    //FIXME: Remember which row is selected

    auto member_list = room_member.GetMemberInformation();

    auto MacAddressString = [&](const MacAddress& mac_address) -> QString {
        QString str;
        for(unsigned int i = 0; i < mac_address.size(); i++) {
            if (i > 0) {
                str += ":";
            }
            str += QString("%1").arg(mac_address[i], 2, 16, QLatin1Char('0')).toUpper(); 
        }
        return str;
    };

    item_model->removeRows(0, item_model->rowCount());
    for(const auto& member : member_list) {
        QList<QStandardItem*> l;

        std::vector<std::string> elements = {
            member.nickname,
            member.game_name,
            MacAddressString(member.mac_address).toStdString(),
            "- %",
            "- ms"
        };
        for(auto& item : elements) {
		        QStandardItem *child = new QStandardItem(QString::fromStdString(item));
		        child->setEditable( false );
            l.append(child);
        }
        item_model->invisibleRootItem()->appendRow(l);
    }

    //FIXME: Restore row selection
}

void RoomViewWindow::OnSay() {
    QString message = sayLineEdit->text();
    room_member.SendChatMessage(message.toStdString());
    AddChatMessage(QString("NoName"), message, true);
    sayLineEdit->setText("");
    sayLineEdit->setFocus();


    //FIXME: Temporary hack to test memberlist + room member state
    UpdateMemberList();
    OnStateChange();
}

void RoomViewWindow::ConnectWidgetEvents() {


    connect(sayLineEdit, SIGNAL(returnPressed()), sayButton, SIGNAL(clicked()));
    connect(sayButton, SIGNAL(clicked()), this, SLOT(OnSay()));

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

#if 0
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

#if 0

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

#endif

static void AppendHtml(QTextEdit* text_edit, QString html) {
    //FIXME: Keep cursor / selection where it is etc

    auto* scrollbar = text_edit->verticalScrollBar();
    auto scrollbar_value = scrollbar->value();
    bool scroll_down = (scrollbar_value == scrollbar->maximum());

    QString new_html = text_edit->toHtml();
    new_html += html;
    text_edit->setHtml(new_html);

    // Scroll to the very bottom
    if (scroll_down) {
      scrollbar->setValue(scrollbar->maximum());
    } else {
      scrollbar->setValue(scrollbar_value);
    }
}

void RoomViewWindow::AddRoomMessage(QString message) {
    QString html;
    html += "<font color=\"gray\"><i>" + message.toHtmlEscaped() + "</i></font><br>";
    AppendHtml(chatLog, html);
}

void RoomViewWindow::AddConnectionMessage(QString message) {
    QString html;
    html += "<font color=\"green\"><b>" + message.toHtmlEscaped() + "</b></font><br>";
    AppendHtml(chatLog, html);
}

void RoomViewWindow::AddChatMessage(QString nickname, QString message, bool outbound) {
    QString html;
    if (outbound) {
        html += "<font color=\"RoyalBlue\"><b>" + nickname.toHtmlEscaped() + ":</b></font> ";
    } else {
        html += "<font color=\"Red\"><b>" + nickname.toHtmlEscaped() + ":</b></font> ";
    }
    html += message.toHtmlEscaped();// + "<br>";
    AppendHtml(chatLog, html);
}

void RoomViewWindow::SetUiState(bool connected) {
    if (connected) {
        // FIXME: Enable playerlist
        //Enable chat button and chat typing
    } else {
        // FIXME: Clear playerlist or at least disable it
        // Disable chat button and chat typing
    }
}

void RoomViewWindow::OnStateChange() {
    switch(room_member.GetState()) {
    case RoomMember::State::Idle:
        break;
    case RoomMember::State::Error:
        AddConnectionMessage(tr("The network could not be used. Make sure your system is connected to the network and you have the necessary permissions"));
        break;
    case RoomMember::State::Joining:
        chatLog->setHtml(""); //FIXME: Only do this when the server has changed, not in case of reconnect attempts!
        AddConnectionMessage(tr("Attempting to join room (Connecting)..."));
        break;
    case RoomMember::State::Joined:
        AddConnectionMessage(tr("Room joined successfully (Connected)"));
//        emit OnConnected();
        break;
    case RoomMember::State::LostConnection:
        AddConnectionMessage(tr("Disconnected (Lost connection to room)"));
//        emit OnDisconnected();
        break;
    case RoomMember::State::RoomFull:
        AddConnectionMessage(tr("Unable to join (The room is full)"));
        break;
    case RoomMember::State::RoomDestroyed:
        AddConnectionMessage(tr("Unable to join (The room could not be found)"));
        break;
    case RoomMember::State::NameCollision:
        AddConnectionMessage(tr("Unable to join (The nickname is already in use)"));
        break;
    case RoomMember::State::MacCollision:
        AddConnectionMessage(tr("Unable to join (The preferred mac address is already in use)"));
        break;
    case RoomMember::State::WrongVersion:
        AddConnectionMessage(tr("Unable to join (Room is using another Citra version)"));
        break;
    default:
        assert(false);
        break;
    }
}

void RoomViewWindow::OnRoomGameChange(std::string game_name) {
    AddRoomMessage(QString(tr("The room is intended for playing %1")).arg(QString::fromStdString(game_name)));
}

void RoomViewWindow::OnMemberGameChange(std::string nickname, std::string game_name) {
    AddRoomMessage(QString(tr("%1 is playing %2")).arg(QString::fromStdString(nickname)).arg(QString::fromStdString(game_name)));
    UpdateMemberList();
}

void RoomViewWindow::OnDisconnected() {
    SetUiState(false);
}

void RoomViewWindow::OnMemberLeft(std::string nickname) {
    QString reason(tr("Unknown reason"));
    AddRoomMessage(QString(tr("%1 is no longer in the room (%2)")).arg(QString::fromStdString(nickname)).arg(reason));
    UpdateMemberList();
}

void RoomViewWindow::OnMemberJoined(std::string nickname) {
    AddRoomMessage(QString(tr("%1 joined the room")).arg(QString::fromStdString(nickname)));
    UpdateMemberList();
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
        tr("Are you sure you want to leave this room? Your simulated WiFi connection to all other members will be lost."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return answer != QMessageBox::No;
}

void RoomViewWindow::closeEvent(QCloseEvent* event) {
    if (!ConfirmClose()) {
        event->ignore();
        return;
    }

    QWidget::closeEvent(event);
}
