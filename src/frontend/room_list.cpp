// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QThreadPool>
#include <QVBoxLayout>
/*
#include "common/common_paths.h"
#include "common/logging/log.h"
#include "common/string_util.h"
#include "core/loader/loader.h"
#include "ui_settings.h"
*/

#include "room_list.h"
#include "room_list_p.h"

RoomList::RoomList(QWidget* parent) : QWidget{parent} {
    QVBoxLayout* layout = new QVBoxLayout;

    tree_view = new QTreeView;
    item_model = new QStandardItemModel(tree_view);
    tree_view->setModel(item_model);

    tree_view->setAlternatingRowColors(true);
    tree_view->setSelectionMode(QHeaderView::SingleSelection);
    tree_view->setSelectionBehavior(QHeaderView::SelectRows);
    tree_view->setVerticalScrollMode(QHeaderView::ScrollPerPixel);
    tree_view->setHorizontalScrollMode(QHeaderView::ScrollPerPixel);
    tree_view->setSortingEnabled(true);
    tree_view->setEditTriggers(QHeaderView::NoEditTriggers);
    tree_view->setUniformRowHeights(true);
    tree_view->setContextMenuPolicy(Qt::CustomContextMenu);

    item_model->insertColumns(0, COLUMN_COUNT);
    item_model->setHeaderData(COLUMN_NAME, Qt::Horizontal, "Name");
    item_model->setHeaderData(COLUMN_PLAYERS, Qt::Horizontal, "Members");
    item_model->setHeaderData(COLUMN_SERVER, Qt::Horizontal, "Server");
    item_model->setHeaderData(COLUMN_PING, Qt::Horizontal, "Ping");
    item_model->setHeaderData(COLUMN_FLAGS, Qt::Horizontal, "Flags"); // Password | LAN

    connect(tree_view, &QTreeView::activated, this, &RoomList::ValidateEntry);
    connect(tree_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(SelectionChanged(const QItemSelection&,const QItemSelection&)));
#if 0
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &RoomList::PopupContextMenu);
    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &RoomList::RefreshGameDirectory);
#endif

    // We must register all custom types with the Qt Automoc system so that we are able to use it
    // with signals/slots. In this case, QList falls under the umbrells of custom types.
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");

    tree_view->header()->setStretchLastSection(false);
    tree_view->header()->setSectionResizeMode(COLUMN_NAME, QHeaderView::Stretch);   

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tree_view);
    setLayout(layout);
}

RoomList::~RoomList() {
    emit ShouldCancelWorker();
}

#if 0
//FIXME: Stolen from the internet
bool SortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QList<QModelIndex> children;
    children << sourceModel()->index(source_row, 0, source_parent);
 
    bool show = false;
    for(int i = 0; i < children.length(); i++)
    {
        if(show) break;
 
        // Add sub Nodos
        //
        for(int c = 0; c < sourceModel()->rowCount(children[i]) ;c++)
            children.append(children[i].child(c,0));
 
        QString type = sourceModel()->data(children[i], Qt::DisplayRole).toString();
        show = type.contains(filterRegExp());        
    }
    return show;
}
#endif

//#if 0

void RoomList::ClearSelection() {
    tree_view->clearSelection();
}

void RoomList::AddEntry(const QList<QStandardItem*>& entry_items) {
    item_model->invisibleRootItem()->appendRow(entry_items);
}

void RoomList::SelectionChanged(const QItemSelection&,const QItemSelection&) {
    //FIXME: get selection etc.
    emit RoomChosen("google.com", 1337, false); //FIXME!
}

void RoomList::ValidateEntry(const QModelIndex& item) {
    // We don't care about the individual QStandardItem that was selected, but its row.
    int row = item_model->itemFromIndex(item)->row();
    QStandardItem* child_server = item_model->invisibleRootItem()->child(row, COLUMN_SERVER);
    QString server = child_server->data(/*RoomListItemPath::FullPathRole*/).toString();

#if 0


    if (file_path.isEmpty())
        return;
    std::string std_file_path(file_path.toStdString());
    if (!FileUtil::Exists(std_file_path) || FileUtil::IsDirectory(std_file_path))
        return;
#endif
    emit RoomChosen(server, 1337, true); //FIXME!
}

#if 0
void RoomList::DonePopulating() {
    tree_view->setEnabled(true);
}

void RoomList::PopupContextMenu(const QPoint& menu_location) {
    QModelIndex item = tree_view->indexAt(menu_location);
    if (!item.isValid())
        return;

    int row = item_model->itemFromIndex(item)->row();
    QStandardItem* child_file = item_model->invisibleRootItem()->child(row, COLUMN_NAME);
    u64 program_id = child_file->data(RoomListItemPath::ProgramIdRole).toULongLong();

    QMenu context_menu;
    QAction* open_save_location = context_menu.addAction(tr("Open Save Data Location"));
    open_save_location->setEnabled(program_id != 0);
    connect(open_save_location, &QAction::triggered,
            [&]() { emit OpenSaveFolderRequested(program_id); });
    context_menu.exec(tree_view->viewport()->mapToGlobal(menu_location));
}

void RoomList::PopulateAsync(const QString& dir_path, bool deep_scan) {
    if (!FileUtil::Exists(dir_path.toStdString()) ||
        !FileUtil::IsDirectory(dir_path.toStdString())) {
        LOG_ERROR(Frontend, "Could not find game list folder at %s", dir_path.toLocal8Bit().data());
        return;
    }

    tree_view->setEnabled(false);
    // Delete any rows that might already exist if we're repopulating
    item_model->removeRows(0, item_model->rowCount());

    emit ShouldCancelWorker();

    auto watch_dirs = watcher.directories();
    if (!watch_dirs.isEmpty()) {
        watcher.removePaths(watch_dirs);
    }
    UpdateWatcherList(dir_path.toStdString(), deep_scan ? 256 : 0);
    RoomListWorker* worker = new RoomListWorker(dir_path, deep_scan);

    connect(worker, &RoomListWorker::EntryReady, this, &RoomList::AddEntry, Qt::QueuedConnection);
    connect(worker, &RoomListWorker::Finished, this, &RoomList::DonePopulating,
            Qt::QueuedConnection);
    // Use DirectConnection here because worker->Cancel() is thread-safe and we want it to cancel
    // without delay.
    connect(this, &RoomList::ShouldCancelWorker, worker, &RoomListWorker::Cancel,
            Qt::DirectConnection);

    QThreadPool::globalInstance()->start(worker);
    current_worker = std::move(worker);
}

void RoomList::SaveInterfaceLayout() {
    UISettings::values.gamelist_header_state = tree_view->header()->saveState();
}

void RoomList::LoadInterfaceLayout() {
    auto header = tree_view->header();
    if (!header->restoreState(UISettings::values.gamelist_header_state)) {
        // We are using the name column to display icons and titles
        // so make it as large as possible as default.
        header->resizeSection(COLUMN_NAME, header->width());
    }

    item_model->sort(header->sortIndicatorSection(), header->sortIndicatorOrder());
}

const QStringList RoomList::supported_file_extensions = {"3ds", "3dsx", "elf", "axf",
                                                         "cci", "cxi",  "app"};

static bool HasSupportedFileExtension(const std::string& file_name) {
    QFileInfo file = QFileInfo(file_name.c_str());
    return RoomList::supported_file_extensions.contains(file.suffix(), Qt::CaseInsensitive);
}

void RoomList::RefreshGameDirectory() {
    if (!UISettings::values.gamedir.isEmpty() && current_worker != nullptr) {
        LOG_INFO(Frontend, "Change detected in the games directory. Reloading game list.");
        PopulateAsync(UISettings::values.gamedir, UISettings::values.gamedir_deepscan);
    }
}

/**
 * Adds the game list folder to the QFileSystemWatcher to check for updates.
 *
 * The file watcher will fire off an update to the game list when a change is detected in the game
 * list folder.
 *
 * Notice: This method is run on the UI thread because QFileSystemWatcher is not thread safe and
 * this function is fast enough to not stall the UI thread. If performance is an issue, it should
 * be moved to another thread and properly locked to prevent concurrency issues.
 *
 * @param dir folder to check for changes in
 * @param recursion 0 if recursion is disabled. Any positive number passed to this will add each
 *        directory recursively to the watcher and will update the file list if any of the folders
 *        change. The number determines how deep the recursion should traverse.
 */
void RoomList::UpdateWatcherList(const std::string& dir, unsigned int recursion) {
    const auto callback = [this, recursion](unsigned* num_entries_out, const std::string& directory,
                                            const std::string& virtual_name) -> bool {
        std::string physical_name = directory + DIR_SEP + virtual_name;

        if (FileUtil::IsDirectory(physical_name)) {
            UpdateWatcherList(physical_name, recursion - 1);
        }
        return true;
    };

    watcher.addPath(QString::fromStdString(dir));
    if (recursion > 0) {
        FileUtil::ForeachDirectoryEntry(nullptr, dir, callback);
    }
}

void RoomListWorker::AddFstEntriesToRoomList(const std::string& dir_path, unsigned int recursion) {
    const auto callback = [this, recursion](unsigned* num_entries_out, const std::string& directory,
                                            const std::string& virtual_name) -> bool {
        std::string physical_name = directory + DIR_SEP + virtual_name;

        if (stop_processing)
            return false; // Breaks the callback loop.

        if (!FileUtil::IsDirectory(physical_name) && HasSupportedFileExtension(physical_name)) {
            std::unique_ptr<Loader::AppLoader> loader = Loader::GetLoader(physical_name);
            if (!loader)
                return true;

            std::vector<u8> smdh;
            loader->ReadIcon(smdh);

            u64 program_id = 0;
            loader->ReadProgramId(program_id);

            emit EntryReady({
                new RoomListItemPath(QString::fromStdString(physical_name), smdh, program_id),
                new RoomListItem(
                    QString::fromStdString(Loader::GetFileTypeString(loader->GetFileType()))),
                new RoomListItemSize(FileUtil::GetSize(physical_name)),
            });
        } else if (recursion > 0) {
            AddFstEntriesToRoomList(physical_name, recursion - 1);
        }

        return true;
    };

    FileUtil::ForeachDirectoryEntry(nullptr, dir_path, callback);
}

void RoomListWorker::run() {
    stop_processing = false;
    AddFstEntriesToRoomList(dir_path.toStdString(), deep_scan ? 256 : 0);
    emit Finished();
}

void RoomListWorker::Cancel() {
    this->disconnect();
    stop_processing = true;
}

#endif
