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

#include "castdisplayplugin.h"
#include <QJsonDocument>

#include <QDBusConnection>
#include <QStandardPaths>
#include <QDataStream>
#include <QMimeDatabase>
#include <QLoggingCategory>

#include <KPluginFactory>

#include <core/device.h>

Q_DECLARE_LOGGING_CATEGORY(KDECONNECT_PLUGIN_CASTDISPLAY)

#define PACKAGE_TYPE_CASTDISPLAY QLatin1String("kdeconnect.castdisplay")
#define PACKAGE_TYPE_CASTREMOTE QLatin1String("kdeconnect.castremote")

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_castdisplay.json", registerPlugin<CastDisplayPlugin>(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_CAST, "kdeconnect.plugin.castdisplay")

CastDisplay::CastDisplay()
{}

CastDisplay * CastDisplay::self()
{
    static CastDisplay* disp = nullptr;
    if (!disp) {
        disp = new CastDisplay;
        const QString dbusPath = QStringLiteral("/modules/kdeconnect/devices/castdisplay");
        QDBusConnection::sessionBus().registerObject(dbusPath, disp, QDBusConnection::ExportAllContents);
    }
    return disp;
}

void CastDisplayPlugin::setCurrentProgram(const QString &current)
{
    NetworkPackage np(PACKAGE_TYPE_CASTDISPLAY);
    np.set<QString>("currentProgram", current);
    sendPackage(np);
}

void CastDisplay::emitStartProgram(const QString &programPath, const QJsonObject &data)
{
    Q_EMIT startProgramSerialized(programPath, QJsonDocument(data).toJson());
}

void CastDisplay::startProgramForDocument(const QString &programPath, const QUrl &url)
{
    emitStartProgram(programPath, { {"source", url.toString()} });
}

CastDisplayPlugin::CastDisplayPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
}

bool CastDisplayPlugin::receivePackage (const NetworkPackage& np)
{
    if (np.has("command")) {

    } else { //it's a program
        const QString name = np.get<QString>("name");
        const QString source = np.get<QString>("source");

        const QString location = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir().mkpath(location);
        const QString path = location + '/' + QString(name).replace(QChar('/'), QString()) + ".qml";

        QFile sourceFile(path);
        bool b = sourceFile.open(QIODevice::WriteOnly);
        if (!b) {
            qWarning() << "Couldn't write in " << path;
            return true;
        }
        sourceFile.write(source.toUtf8());

        m_programForName[name] = path;

        const QUrl url = m_pending.take(name);
        qDebug() << "url:" << url << "name:" << name << "fuuu" << m_pending;
        if (!url.isEmpty()) {
            CastDisplay::self()->startProgramForDocument(path, url);
        }
    }
    return true;
}

void CastDisplayPlugin::fileReceived(const QUrl& url)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForUrl(url);
    QString program = m_programForName.value(mime.name());

    qDebug() << "File Received" << url << program;

    if (!program.isEmpty()) {
        CastDisplay::self()->startProgramForDocument(program, url);
    } else {
        NetworkPackage np(PACKAGE_TYPE_CASTDISPLAY);
        np.set<bool>("programRequest", true);
        np.set<QString>("name", mime.name());
        sendPackage(np);

        m_pending.insert(mime.name(), url);
    }
}

void CastDisplayPlugin::connected()
{
    CastDisplay::self(); //make sure it's on

    if (device()->plugin("kdeconnect_share")) {
        connect(device()->plugin("kdeconnect_share"), SIGNAL(fileReceived(QUrl)), this, SLOT(fileReceived(QUrl)), Qt::UniqueConnection);
    } else
        qWarning() << "no share plugin installed!";

    qDebug() << "cast display connected to " << device()->plugin("kdeconnect_share");
}

#include "castdisplayplugin.moc"
