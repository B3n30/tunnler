// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#define u64 uint64_t

#ifndef _CITRA_QT_ROOM_VIEW_WINDOW_HXX_
#define _CITRA_QT_ROOM_VIEW_WINDOW_HXX_

#include <memory>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QTreeView>
#include <QPushButton>
#include <QTextEdit>
#if 0
#include "ui_main.h"

class CallstackWidget;
class Config;
class DisassemblerWidget;
class EmuThread;
#endif
class RoomMemberList;
#if 0
class GImageInfo;
class GPUCommandStreamWidget;
class GPUCommandListWidget;
class GraphicsBreakPointsWidget;
class GraphicsTracingWidget;
class GraphicsVertexShaderWidget;
class GRenderWindow;
class MicroProfileDialog;
class ProfilerWidget;
class RegistersWidget;
class WaitTreeWidget;

#endif

class RoomViewWindow : public QMainWindow {
    Q_OBJECT

    typedef uint8_t EmuThread;

    /// Max number of recently loaded items to keep track of
    static const int max_recent_files_item = 10;

public:
    RoomViewWindow();
    ~RoomViewWindow();

private:
    void InitializeWidgets();

    void SetDefaultUIGeometry();
    void RestoreUIState();

    void ConnectWidgetEvents();

    /**
     * Ask the user if they really want to leave a room / close the window.
     *
     * @return true if the user confirmed
     */
    bool ConfirmClose();
    void closeEvent(QCloseEvent* event) override;

private slots:
    void OnStartGame();
    void OnPauseGame();
    void OnStopGame();
    /// Called whenever a user selects a game in the game list widget.
//    void OnGameListLoadFile(QString game_path);
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

private:
    void UpdateStatusBar();

    //Ui::MainWindow ui;

/*
    GRenderWindow* render_window;
*/

QWidget* centralwidget = nullptr;
QVBoxLayout* verticalLayout = nullptr;

QWidget* chatWidget = nullptr;
QGridLayout* gridLayout = nullptr;
QTextEdit* chatLog = nullptr;
QTreeView* memberList = nullptr; // <item row="0" column="1" colspan="2" >
QLineEdit* sayLineEdit = nullptr; // <item row="1" column="0" colspan="2" >           
QPushButton* sayButton = nullptr; // <item row="1" column="2" >

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

#endif // _CITRA_QT_ROOM_VIEW_WINDOW_HXX_
