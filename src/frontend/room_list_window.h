// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstdint>
typedef uint16_t u16;
typedef uint64_t u64;

#ifndef _CITRA_QT_ROOM_LIST_WINDOW_HXX_
#define _CITRA_QT_ROOM_LIST_WINDOW_HXX_

#include <memory>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>

class RoomList;

class RoomListWindow : public QMainWindow {
    Q_OBJECT

    typedef uint8_t EmuThread;

    /// Max number of recently loaded items to keep track of
    static const int max_recent_files_item = 10;

public:
    RoomListWindow();
    ~RoomListWindow();

private:
    void InitializeWidgets();

    void SetDefaultUIGeometry();

    void ConnectWidgetEvents();

    /**
     * Ask the user if they really want to leave a room / close the window.
     *
     * @return true if the user confirmed
     */
    bool ConfirmClose();
    void closeEvent(QCloseEvent* event) override;

private slots:
    void OnRoomListSelectRoom(QString server, u16 serverPort, bool join);
#if 0
    void OnStartGame();
    void OnPauseGame();
    void OnStopGame();
    /// Called whenever a user selects a game in the game list widget.
    void OnGameListOpenSaveFolder(u64 program_id);
    void OnMenuLoadFile();
    void OnMenuLoadSymbolMap();
    /// Called whenever a user selects the "File->Select Game List Root" menu item
    void OnMenuSelectGameListRoot();
    void OnMenuRecentFile();
    void OnSwapScreens();
    void OnConfigure();
    void OnDisplayTitleBars(bool);
    void ToggleWindowMode();
    void OnCreateGraphicsSurfaceViewer();
#endif

private:
    void UpdateStatusBar();

    //Ui::MainWindow ui;

/*
    GRenderWindow* render_window;
*/
    RoomList* room_list;

    QLineEdit* server = nullptr;
    QSpinBox* port = nullptr;

    QCheckBox* not_full = nullptr;
    QCheckBox* not_empty = nullptr;
//FIXME: Var naming for these 3..
    QLineEdit* gameFilter = nullptr;
    QLineEdit* nameFilter = nullptr;
    QSpinBox* pingFilter = nullptr;

    // Status bar elements
    QLabel* emu_speed_label = nullptr;
    QLabel* game_fps_label = nullptr;
    QLabel* emu_frametime_label = nullptr;
    QTimer status_bar_update_timer;

#if 0
    std::unique_ptr<Config> config;

    // Whether emulation is currently running in Citra.
    bool emulation_running = false;
    std::unique_ptr<EmuThread> emu_thread;

    // Debugger panes
    ProfilerWidget* profilerWidget;
    MicroProfileDialog* microProfileDialog;
    DisassemblerWidget* disasmWidget;
    RegistersWidget* registersWidget;
    CallstackWidget* callstackWidget;
    GPUCommandStreamWidget* graphicsWidget;
    GPUCommandListWidget* graphicsCommandsWidget;
    GraphicsBreakPointsWidget* graphicsBreakpointsWidget;
    GraphicsVertexShaderWidget* graphicsVertexShaderWidget;
    GraphicsTracingWidget* graphicsTracingWidget;
    WaitTreeWidget* waitTreeWidget;

    QAction* actions_recent_files[max_recent_files_item];
#endif
};

#endif // _CITRA_QT_ROOM_LIST_WINDOW_HXX_
