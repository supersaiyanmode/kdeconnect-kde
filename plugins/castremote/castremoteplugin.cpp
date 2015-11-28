/**
 * Copyright 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "castremoteplugin.h"

#include <KPluginFactory>

#include <QDebug>
#include <QLoggingCategory>

#include <core/device.h>

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_castremote.json", registerPlugin< CastRemotePlugin >(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_CASTREMOTE, "kdeconnect.plugin.castremote")

#define PACKAGE_TYPE_CASTDISPLAY QLatin1String("kdeconnect.castdisplay")
#define PACKAGE_TYPE_CASTREMOTE QLatin1String("kdeconnect.castremote")

CastRemotePlugin::CastRemotePlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
}

CastRemotePlugin::~CastRemotePlugin()
{
}

bool CastRemotePlugin::receivePackage(const NetworkPackage& np)
{
    if (np.has("programRequest")) {
        const QString program = np.get<QString>("name");
        qDebug() << "program requested:" << program;

        static const QHash<QString, QString> codes = {
            { "image/png",          "import QtQuick 2.1\n\nImage{ fillMode: Image.PreserveAspectFit }\n"},
            { "video/x-matroska",   "import QtMultimedia 5.4\n"
                                    "import QtQuick 2.1\n"

                                    "Video {\n"
                                    "    Component.onCompleted: play();\n"
                                    "}\n"},
            };

        NetworkPackage np(PACKAGE_TYPE_CASTREMOTE);
        np.set<QString>("name", program);
        np.set<QString>("source", codes[program]);
        sendPackage(np);
    } else if(np.has("currentProgram")) {
        qWarning() << "current program is " << np.get<QString>("currentProgram");
    }

    return true;
}

void CastRemotePlugin::connected()
{
//     QDBusConnection::sessionBus().registerObject(dbusPath(), this, QDBusConnection::ExportAllContents);
}

#include "castremoteplugin.moc"
