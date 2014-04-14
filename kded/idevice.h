/**
 * Copyright 2014 Alexandr Akulich <akulichalexander@gmail.com>
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

#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <QString>

#include "networkpackage.h"

class DeviceLink;
class KdeConnectPlugin;

class IDevice
    : public QObject
{
    Q_OBJECT

public:
    IDevice(QObject* parent) : QObject(parent) { }

    virtual ~IDevice() { }

    virtual QString id() const = 0;
    virtual QString name() const = 0 ;
    virtual QString dbusPath() const = 0;

    //Add and remove links
    virtual void addLink(const NetworkPackage& identityPackage, DeviceLink*) = 0;
    virtual void removeLink(DeviceLink*) = 0;

    virtual QString privateKeyPath() const = 0;
    
    virtual Q_SCRIPTABLE bool isPaired() const = 0;
    virtual Q_SCRIPTABLE bool pairRequested() const = 0;

    virtual Q_SCRIPTABLE QStringList availableLinks() const = 0;
    virtual Q_SCRIPTABLE bool isReachable() const = 0;

    virtual Q_SCRIPTABLE QStringList loadedPlugins() const = 0;
    virtual Q_SCRIPTABLE bool hasPlugin(const QString& name) const = 0;

public Q_SLOTS:
    ///sends a @p np package to the device
    virtual bool sendPackage(NetworkPackage& np) = 0;

    //Dbus operations
public Q_SLOTS:
    virtual Q_SCRIPTABLE void requestPair() = 0;
    virtual Q_SCRIPTABLE void unpair() = 0;
    virtual Q_SCRIPTABLE void reloadPlugins() = 0; //From kconf
    virtual Q_SCRIPTABLE void sendPing() = 0;
    virtual void acceptPairing() = 0;
    virtual void rejectPairing() = 0;

Q_SIGNALS:
    Q_SCRIPTABLE void reachableStatusChanged();
    Q_SCRIPTABLE void pluginsChanged();
    Q_SCRIPTABLE void pairingSuccesful();
    Q_SCRIPTABLE void pairingFailed(const QString& error);
    Q_SCRIPTABLE void unpaired();

};

Q_DECLARE_METATYPE(IDevice*)

#endif // IDEVICE_H
