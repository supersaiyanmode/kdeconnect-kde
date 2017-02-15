/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
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

#include "daemon.h"

#include <QDBusConnection>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QPointer>

#include "core_debug.h"
#include "kdeconnectconfig.h"
#include "networkpackage.h"
#include "backends/lan/lanlinkprovider.h"
#include "backends/loopback/loopbacklinkprovider.h"
#include "device.h"
#include "backends/devicelink.h"
#include "backends/linkprovider.h"

static Daemon* s_instance = nullptr;

struct DaemonPrivate
{
    //Different ways to find devices and connect to them
    QSet<LinkProvider*> mLinkProviders;

    //Every known device
    QMap<QString, Device*> mDevices;

    QSet<QString> mDiscoveryModeAcquisitions;
};

Daemon* Daemon::instance()
{
    Q_ASSERT(s_instance != nullptr);
    return s_instance;
}

Daemon::Daemon(QObject *parent, bool testMode)
    : QObject(parent)
    , d(new DaemonPrivate)
{
    Q_ASSERT(!s_instance);
    s_instance = this;
    qCDebug(KDECONNECT_CORE) << "KdeConnect daemon starting";

    //Load backends
    if (testMode)
        d->mLinkProviders.insert(new LoopbackLinkProvider());
    else
        d->mLinkProviders.insert(new LanLinkProvider());

    //Read remebered paired devices
    const QStringList& list = KdeConnectConfig::instance()->trustedDevices();
    Q_FOREACH (const QString& id, list) {
        addDevice(new Device(this, id));
    }

    //Listen to new devices
    Q_FOREACH (LinkProvider* a, d->mLinkProviders) {
        connect(a, &LinkProvider::onConnectionReceived,
                this, &Daemon::onNewDeviceLink);
        a->onStart();
    }

    //Register on DBus
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.kdeconnect"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/modules/kdeconnect"), this, QDBusConnection::ExportScriptableContents);

    qCDebug(KDECONNECT_CORE) << "KdeConnect daemon started";
}

void Daemon::acquireDiscoveryMode(const QString &key)
{
    bool oldState = d->mDiscoveryModeAcquisitions.isEmpty();

    d->mDiscoveryModeAcquisitions.insert(key);

    if (oldState != d->mDiscoveryModeAcquisitions.isEmpty()) {
        forceOnNetworkChange();
    }
}

void Daemon::releaseDiscoveryMode(const QString &key)
{
    bool oldState = d->mDiscoveryModeAcquisitions.isEmpty();

    d->mDiscoveryModeAcquisitions.remove(key);

    if (oldState != d->mDiscoveryModeAcquisitions.isEmpty()) {
        cleanDevices();
    }
}

void Daemon::removeDevice(Device* device)
{
    d->mDevices.remove(device->id());
    device->deleteLater();
    Q_EMIT deviceRemoved(device->id());
}

void Daemon::cleanDevices()
{
    Q_FOREACH (Device* device, d->mDevices) {
        if (device->isTrusted()) {
            continue;
        }
        device->cleanUnneededLinks();
        //If there are no links remaining
        if (!device->isReachable()) {
            removeDevice(device);
        }
    }
}

void Daemon::forceOnNetworkChange()
{
    qCDebug(KDECONNECT_CORE) << "Sending onNetworkChange to " << d->mLinkProviders.size() << " LinkProviders";
    Q_FOREACH (LinkProvider* a, d->mLinkProviders) {
        a->onNetworkChange();
    }
}

Device*Daemon::getDevice(const QString& deviceId)
{
    Q_FOREACH (Device* device, d->mDevices) {
        if (device->id() == deviceId) {
            return device;
        }
    }
    return Q_NULLPTR;
}

QStringList Daemon::devices(bool onlyReachable, bool onlyTrusted) const
{
    QStringList ret;
    Q_FOREACH (Device* device, d->mDevices) {
        if (onlyReachable && !device->isReachable()) continue;
        if (onlyTrusted && !device->isTrusted()) continue;
        ret.append(device->id());
    }
    return ret;
}

void Daemon::onNewDeviceLink(const NetworkPackage& identityPackage, DeviceLink* dl)
{
    const QString& id = identityPackage.get<QString>(QStringLiteral("deviceId"));

    //qCDebug(KDECONNECT_CORE) << "Device discovered" << id << "via" << dl->provider()->name();

    if (d->mDevices.contains(id)) {
        qCDebug(KDECONNECT_CORE) << "It is a known device" << identityPackage.get<QString>(QStringLiteral("deviceName"));
        Device* device = d->mDevices[id];
        bool wasReachable = device->isReachable();
        device->addLink(identityPackage, dl);
        if (!wasReachable) {
            Q_EMIT deviceVisibilityChanged(id, true);
        }
    } else {
        qCDebug(KDECONNECT_CORE) << "It is a new device" << identityPackage.get<QString>(QStringLiteral("deviceName"));
        Device* device = new Device(this, identityPackage, dl);

        //we discard the connections that we created but it's not paired.
        if (!isDiscoveringDevices() && !device->isTrusted() && !dl->linkShouldBeKeptAlive()) {
            device->deleteLater();
        } else {
            addDevice(device);
        }
    }
}

void Daemon::onDeviceStatusChanged()
{
    Device* device = (Device*)sender();

    //qCDebug(KDECONNECT_CORE) << "Device" << device->name() << "status changed. Reachable:" << device->isReachable() << ". Paired: " << device->isPaired();

    if (!device->isReachable() && !device->isTrusted()) {
        //qCDebug(KDECONNECT_CORE) << "Destroying device" << device->name();
        removeDevice(device);
    } else {
        Q_EMIT deviceVisibilityChanged(device->id(), device->isReachable());
    }

}

void Daemon::setAnnouncedName(const QString &name)
{
    qCDebug(KDECONNECT_CORE()) << "Announcing name";
    KdeConnectConfig::instance()->setName(name);
    forceOnNetworkChange();
    Q_EMIT announcedNameChanged(name);
}

QString Daemon::announcedName()
{
    return KdeConnectConfig::instance()->name();
}

QNetworkAccessManager* Daemon::networkAccessManager()
{
    static QPointer<QNetworkAccessManager> manager;
    if (!manager) {
        manager = new QNetworkAccessManager(this);
    }
    return manager;
}

QList<Device*> Daemon::devicesList() const
{
    return d->mDevices.values();
}

bool Daemon::isDiscoveringDevices() const
{
    return !d->mDiscoveryModeAcquisitions.isEmpty();
}

QString Daemon::deviceIdByName(const QString &name) const
{
    Q_FOREACH (Device* d, d->mDevices) {
        if (d->name() == name && d->isTrusted())
            return d->id();
    }
    return {};
}

void Daemon::addDevice(Device* device)
{
    const QString id = device->id();
    connect(device, &Device::reachableChanged, this, &Daemon::onDeviceStatusChanged);
    connect(device, &Device::trustedChanged, this, &Daemon::onDeviceStatusChanged);
    connect(device, &Device::hasPairingRequestsChanged, this, &Daemon::pairingRequestsChanged);
    connect(device, &Device::hasPairingRequestsChanged, this, [this, device](bool hasPairingRequests) {
        if (hasPairingRequests)
            askPairingConfirmation(device);
    } );
    d->mDevices[id] = device;

    Q_EMIT deviceAdded(id);
}

QStringList Daemon::pairingRequests() const
{
    QStringList ret;
    for(Device* dev: d->mDevices) {
        if (dev->hasPairingRequests())
            ret += dev->id();
    }
    return ret;
}

Daemon::~Daemon()
{

}
