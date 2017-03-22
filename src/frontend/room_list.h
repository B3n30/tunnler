// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstdint>
typedef uint64_t u64;
typedef uint16_t u16;

#pragma once

#include <QFileSystemWatcher>
#include <QModelIndex>
#include <QSettings>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTreeView>
#include <QWidget>

class RoomListWorker;

class RoomList : public QWidget {
    Q_OBJECT

public:
    enum {
        COLUMN_NAME,
        COLUMN_SERVER,
        COLUMN_PLAYERS,
        COLUMN_PING,
        COLUMN_FLAGS,
        COLUMN_COUNT, // Number of columns
    };

    explicit RoomList(QWidget* parent = nullptr);
    ~RoomList() override;

#if 0
    void PopulateAsync(const QString& dir_path, bool deep_scan);

    void SaveInterfaceLayout();
    void LoadInterfaceLayout();

    static const QStringList supported_file_extensions;
#endif
signals:
    void RoomChosen(QString server, u16 serverPort, bool join);
    void ShouldCancelWorker();
    void OpenSaveFolderRequested(u64 program_id);

public:
    void ClearSelection();
    void AddEntry(const QList<QStandardItem*>& entry_items);
private slots:
    void SelectionChanged(const QItemSelection&, const QItemSelection&); 
private:
    void ValidateEntry(const QModelIndex& item);
#if 0
    void DonePopulating();

    void PopupContextMenu(const QPoint& menu_location);
    void UpdateWatcherList(const std::string& path, unsigned int recursion);
    void RefreshGameDirectory();


    RoomListWorker* current_worker = nullptr;
    QFileSystemWatcher watcher;
#endif

    QTreeView* tree_view = nullptr;
    QStandardItemModel* item_model = nullptr;
};