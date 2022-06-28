/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "optical.h"
#include "utils/opticalhelper.h"
#include "utils/opticalfileshelper.h"
#include "utils/opticalsignalmanager.h"
#include "mastered/masteredmediafileinfo.h"
#include "mastered/masteredmediafilewatcher.h"
#include "mastered/masteredmediadiriterator.h"
#include "views/opticalmediawidget.h"
#include "menus/opticalmenuscene.h"
#include "events/opticaleventreceiver.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/common/propertydialog/propertydialogservice.h"
#include "services/filemanager/workspace/workspace_defines.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(QList<QVariantMap> *)

using namespace dfmplugin_optical;

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE

void Optical::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kBurn, "/", OpticalHelper::icon(), true);
    InfoFactory::regClass<MasteredMediaFileInfo>(Global::Scheme::kBurn, InfoFactory::kNoCache);
    WatcherFactory::regClass<MasteredMediaFileWatcher>(Global::Scheme::kBurn, WatcherFactory::kNoCache);
    DirIteratorFactory::regClass<MasteredMediaDirIterator>(Global::Scheme::kBurn);

    bindEvents();

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this,
            [this]() {
                addOpticalCrumbToTitleBar();
            },
            Qt::DirectConnection);

    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted,
            this, [this](const QString &id) {
                onDeviceChanged(id, true);
            },
            Qt::DirectConnection);
    // for blank disc
    connect(DevProxyMng, &DeviceProxyManager::blockDevPropertyChanged, this,
            [this](const QString &id, const QString &property, const QVariant &val) {
                if (id.contains(QRegularExpression("/sr[0-9]*$"))
                    && property == GlobalServerDefines::DeviceProperty::kOptical && !val.toBool()) {
                    onDeviceChanged(id);
                }
            },
            Qt::DirectConnection);
}

bool Optical::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(OpticalMenuSceneCreator::name(), new OpticalMenuSceneCreator);

    addFileOperations();
    addCustomTopWidget();
    addDelegateSettings();
    addPropertySettings();

    // follow event
    dpfHookSequence->follow("dfmplugin_utils", "hook_UrlsTransform", OpticalHelper::instance(), &OpticalHelper::urlsToLocal);

    return true;
}

dpf::Plugin::ShutdownFlag Optical::stop()
{
    return kSync;
}

void Optical::addOpticalCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kBurn), QVariantMap {});
    });
}

void Optical::addFileOperations()
{
    OpticalHelper::workspaceServIns()->addScheme(Global::Scheme::kBurn);
    OpticalHelper::workspaceServIns()->setWorkspaceMenuScene(Global::Scheme::kBurn, OpticalMenuSceneCreator::name());

    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &OpticalFilesHelper::openFilesHandle;
    fileOpeationsHandle->copy = [](const quint64 windowId,
                                   const QList<QUrl> sources,
                                   const QUrl target,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags) -> JobHandlePointer {
        Q_UNUSED(windowId)
        Q_UNUSED(flags)
        OpticalFilesHelper::pasteFilesHandle(sources, target);
        return {};
    };
    fileOpeationsHandle->cut = [](const quint64 windowId,
                                  const QList<QUrl> sources,
                                  const QUrl target,
                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags) -> JobHandlePointer {
        Q_UNUSED(windowId)
        Q_UNUSED(flags)
        OpticalFilesHelper::pasteFilesHandle(sources, target, false);
        return {};
    };
    fileOpeationsHandle->writeUrlsToClipboard = &OpticalFilesHelper::writeUrlToClipboardHandle;
    fileOpeationsHandle->openInTerminal = &OpticalFilesHelper::openInTerminalHandle;
    fileOpeationsHandle->deletes = &OpticalFilesHelper::deleteFilesHandle;
    fileOpeationsHandle->moveToTash = &OpticalFilesHelper::deleteFilesHandle;
    fileOpeationsHandle->linkFile = &OpticalFilesHelper::linkFileHandle;
    OpticalHelper::fileOperationsServIns()->registerOperations(Global::Scheme::kBurn, fileOpeationsHandle);
}

void Optical::addCustomTopWidget()
{
    Workspace::CustomTopWidgetInfo info;
    info.scheme = Global::Scheme::kBurn;
    info.keepShow = false;
    info.createTopWidgetCb = []() {
        return new OpticalMediaWidget;
    };
    info.showTopWidgetCb = [](QWidget *w, const QUrl &url) {
        bool ret { true };
        OpticalMediaWidget *mediaWidget = qobject_cast<OpticalMediaWidget *>(w);
        if (mediaWidget)
            ret = mediaWidget->updateDiscInfo(url);

        return ret;
    };

    OpticalHelper::workspaceServIns()->addCustomTopWidget(info);
}

void Optical::addDelegateSettings()
{
    OpticalHelper::dlgateServIns()->registerTransparentHandle(Global::Scheme::kBurn, [](const QUrl &url) -> bool {
        return !OpticalHelper::burnIsOnDisc(url);
    });
}

void Optical::addPropertySettings()
{
    // TODO(lixiang): change to slot event
    propertyServIns->registerFilterControlField(Global::Scheme::kBurn, Property::FilePropertyControlFilter::kPermission);
}

void Optical::bindEvents()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles", &OpticalEventReceiver::instance(),
                            &OpticalEventReceiver::handleDeleteFilesShortcut);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_CheckDragDropAction", &OpticalEventReceiver::instance(),
                            &OpticalEventReceiver::handleCheckDragDropAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FileDragMove", &OpticalEventReceiver::instance(),
                            &OpticalEventReceiver::handleCheckDragDropAction);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", &OpticalEventReceiver::instance(), &OpticalEventReceiver::sepateTitlebarCrumb);
}

void Optical::onDeviceChanged(const QString &id, bool isUnmount)
{
    if (id.contains(QRegularExpression("sr[0-9]*$"))) {
        const QString &&volTag { id.mid(id.lastIndexOf("/") + 1) };
        QUrl url { QString("burn:///dev/%1/disc_files/").arg(volTag) };
        if (isUnmount)
            emit OpticalSignalManager::instance()->discUnmounted(url);
        dpfSlotChannel->push("dfmplugin_workspace", "slot_CloseTab", url);
    }
}
