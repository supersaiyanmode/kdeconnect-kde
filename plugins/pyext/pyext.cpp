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

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_PYEXT, "kdeconnect.plugin.pyext")

PyExtPlugin::PyExtPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args) {
    scripts.insert(std::map<std::string, Script>::value_type ("123", Script("/home/thrustmaster/test.py"))); 
}

PyExtPlugin::~PyExtPlugin()
{
//     qCDebug(KDECONNECT_PLUGIN_PING) << "Ping plugin destructor for device" << device()->name();
}

bool PyExtPlugin::receivePackage(const NetworkPackage& np)
{
    if (np.get<QString>("pyext_type") == QString("ScriptListRequest")) {
        NetworkPackage response("kdeconnect.pyext", QVariantMap {
            {"pyext_type", "ScriptListResponse"},
            {"pyext_response", getScripts() }
        });
        sendPackage(response);
        return true;
    } else if (np.get<QString>("pyext_type") == QString("ScriptExecuteRequest")) {
        QString guid = np.get<QString>("script_guid");
        qCWarning(KDECONNECT_PLUGIN_PYEXT) << "Got NP for script guid: "<<guid;
        std::map<std::string, Script>::const_iterator it = scripts.find(guid.toStdString());
        if (it != scripts.end()) {
            it->second.invoke({});
        }
    }
    return false;
}

void PyExtPlugin::connected()
{
    QDBusConnection::sessionBus().registerObject(dbusPath(), this, QDBusConnection::ExportAllContents);
}

QVariantList PyExtPlugin::getScripts() const {
    QVariantList list;
    for (auto const& iter :scripts) {
        QVariantMap map;
        map["name"] = QString::fromUtf8(iter.second.name().c_str());
        map["guid"] = QString::fromUtf8(iter.first.c_str());
        list.append(map);
    }
    return list;
}

QString PyExtPlugin::dbusPath() const
{
    return "/modules/kdeconnect/devices/" + device()->id() + "/pyext";
}

#include "pyext.moc"

