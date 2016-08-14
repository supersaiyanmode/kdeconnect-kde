/**
 * Copyright 2016 Srivatsan Iyer<supersaiyanmode.rox@gmail.com>
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

#include "pyext.h"

#include <KNotification>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDebug>
#include <QDBusConnection>
#include <QLoggingCategory>

#include <core/device.h>

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_pyext.json", registerPlugin< PyExtPlugin >(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_PING, "kdeconnect.plugin.pyext")

PyExtPlugin::PyExtPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
//     qCDebug(KDECONNECT_PLUGIN_PING) << "Ping plugin constructor for device" << device()->name();
}

PyExtPlugin::~PyExtPlugin()
{
//     qCDebug(KDECONNECT_PLUGIN_PING) << "Ping plugin destructor for device" << device()->name();
}

bool PyExtPlugin::receivePackage(const NetworkPackage& np)
{
    KNotification* notification = new KNotification("pingReceived"); //KNotification::Persistent
    notification->setIconName(QStringLiteral("dialog-ok"));
    notification->setComponentName("kdeconnect");
    notification->setTitle(device()->name());
    notification->setText(np.get<QString>("message",i18n("Ping!"))); //This can be a source of spam
    notification->sendEvent();
    return true;
}

void PyExtPlugin::connected()
{
    QDBusConnection::sessionBus().registerObject(dbusPath(), this, QDBusConnection::ExportAllContents);
}

QString PyExtPlugin::dbusPath() const
{
    return "/modules/kdeconnect/devices/" + device()->id() + "/pyext";
}

#include "pyext.moc"

