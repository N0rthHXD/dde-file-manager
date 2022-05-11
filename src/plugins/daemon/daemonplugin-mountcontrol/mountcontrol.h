/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef MOUNTCONTROL_H
#define MOUNTCONTROL_H

#include "daemonplugin_mountcontrol_global.h"

#include <dfm-framework/framework.h>

class MountControlDBus;
DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE

class MountControl : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "mountcontrol.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<MountControlDBus> mng;
};

DAEMONPMOUNTCONTROL_END_NAMESPACE
#endif   // MOUNTCONTROL_H
