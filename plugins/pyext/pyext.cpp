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
#include <QDirIterator>

#include <core/device.h>

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_pyext.json", registerPlugin< PyExtPlugin >(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_PYEXT, "kdeconnect.plugin.pyext")

namespace {
    QDir getPluginDir() {
        QDir home = QDir::home();
        QString path = home.filePath(".kde/share/apps/kdeconnect/pyext/plugins");
        QDir dir(path);
        dir.mkpath(".");
        return dir;
    }
    
    std::vector<std::string> listDir(const QDir& path) {
        std::vector<std::string> paths;
        QDirIterator it(path.absolutePath(), QDir::AllDirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            it.next();
            paths.push_back(it.fileName().toStdString());
        }
        return paths;
    }
}

PyExtPlugin::PyExtPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args), pluginDir(getPluginDir()) {
    scripts = listScripts(pluginDir);
    interpreter.initialize();
    interpreter.addSysPath(pluginDir.absoluteFilePath(".").toStdString());
}

PyExtPlugin::~PyExtPlugin() {
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
        QString entryPoint = np.get<QString>("script_entrypoint");
        qCDebug(KDECONNECT_PLUGIN_PYEXT) << "Got package for script guid: "<<guid;
        std::map<std::string, Script>::iterator it = scripts.find(guid.toStdString());
        if (it == scripts.end()) {
            qCDebug(KDECONNECT_PLUGIN_PYEXT) << "Unable to find script for guid: "<<guid;
            return false;
        }
        it->second.invoke(entryPoint.toStdString(), {});
        return true;
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
        map["description"] = QString::fromUtf8(iter.second.description().c_str());
        map["guid"] = QString::fromUtf8(iter.first.c_str());
        QVariantList capabilities;
        for (auto const& capability: iter.second.capabilities()) {
            capabilities.append(QString::fromStdString(capability));
        }
        map["capabilities"] = capabilities;
        list.append(map);
    }
    return list;
}

std::map<std::string, Script > PyExtPlugin::listScripts(const QDir& dir) {
    qCDebug(KDECONNECT_PLUGIN_PYEXT) << "Searching for plugins in: " << dir;
    std::map<std::string, Script> result;
    for (auto const& dirName: listDir(dir)) {
        auto path = dir.absoluteFilePath(".").toStdString();
        Script s(interpreter, path, dirName);
        if (!s.valid()) {
            qCWarning(KDECONNECT_PLUGIN_PYEXT) << "Failed to load PyExt plugin name: " <<  QString(dirName.c_str());
        } else {
            result.insert(
                    std::map<std::string, Script>::value_type {dirName, s});
            qCDebug(KDECONNECT_PLUGIN_PYEXT) << "Found PyExt plugin name: " <<  QString(dirName.c_str());
        }
    }
    return result;
}


QString PyExtPlugin::dbusPath() const {
    return "/modules/kdeconnect/devices/" + device()->id() + "/pyext";
}

#include "pyext.moc"

