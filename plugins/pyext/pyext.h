/**
 * Copyright 2016 Srivatsan Iyer <supersaiyanmode.rox@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PINGPLUGIN_H
#define PINGPLUGIN_H

#include <map>
#include <QVariantList>

#include <core/kdeconnectplugin.h>

#include "script.h"

//#define PACKAGE_TYPE_PING QLatin1String("kdeconnect.ping")

class Q_DECL_EXPORT PyExtPlugin
    : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.pyext")

public:
    explicit PyExtPlugin(QObject *parent, const QVariantList &args);
    ~PyExtPlugin() override;

public Q_SLOTS:
    bool receivePackage(const NetworkPackage& np) override;
    void connected() override;

private:
    QVariantList getScripts() const;
    QString dbusPath() const;
    
    
    std::map<std::string, Script> scripts;
};

#endif
