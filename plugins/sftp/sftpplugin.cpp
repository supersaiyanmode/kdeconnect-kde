/**
 * Copyright 2014 Samoilenko Yuri<kinnalru@gmail.com>
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

#include "sftpplugin.h"

#include <QDBusConnection>
#include <QDir>
#include <QDebug>

#include <KLocalizedString>
#include <KNotification>
#include <KRun>
#include <KFilePlacesModel>
#include <KPluginFactory>

#include "mounter.h"
#include "sftp_debug.h"

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_sftp.json", registerPlugin< SftpPlugin >(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_SFTP, "kdeconnect.plugin.sftp")

static const QSet<QString> fields_c = QSet<QString>() << QStringLiteral("ip") << QStringLiteral("port") << QStringLiteral("user") << QStringLiteral("port") << QStringLiteral("path");

struct SftpPlugin::Pimpl
{
    Pimpl() : mounter(nullptr) {}
  
    //Add KIO entry to Dolphin's Places
    KFilePlacesModel  placesModel;
    Mounter* mounter;
};

SftpPlugin::SftpPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_d(new Pimpl())
{ 
    deviceId = device()->id();
    addToDolphin();
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Created device:" << device()->name();
}

SftpPlugin::~SftpPlugin()
{
    QDBusConnection::sessionBus().unregisterObject(dbusPath(), QDBusConnection::UnregisterTree);
    removeFromDolphin();    
    unmount();
}

void SftpPlugin::addToDolphin()
{
    removeFromDolphin();
    QUrl kioUrl("kdeconnect://"+deviceId+"/");
    m_d->placesModel.addPlace(device()->name(), kioUrl, QStringLiteral("kdeconnect"));
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "add to dolphin";
}

void SftpPlugin::removeFromDolphin()
{
    QUrl kioUrl("kdeconnect://"+deviceId+"/");
    QModelIndex index = m_d->placesModel.closestItem(kioUrl);
    while (index.row() != -1) {
        m_d->placesModel.removePlace(index);
        index = m_d->placesModel.closestItem(kioUrl);
    }
}

void SftpPlugin::connected()
{
    QDBusConnection::sessionBus().registerObject(dbusPath(), this, QDBusConnection::ExportScriptableContents);
}

void SftpPlugin::mount()
{
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Mount device:" << device()->name();
    if (m_d->mounter) {
        return;
    }

    m_d->mounter = new Mounter(this);
    connect(m_d->mounter, &Mounter::mounted, this, &SftpPlugin::onMounted);
    connect(m_d->mounter, &Mounter::unmounted, this, &SftpPlugin::onUnmounted);
    connect(m_d->mounter, &Mounter::failed, this, &SftpPlugin::onFailed);
}

void SftpPlugin::unmount()
{
    if (m_d->mounter)
    {
        m_d->mounter->deleteLater();
        m_d->mounter = nullptr;
    }
}

bool SftpPlugin::mountAndWait()
{
    mount();
    return m_d->mounter->wait();
}

bool SftpPlugin::isMounted() const
{
    return m_d->mounter && m_d->mounter->isMounted();
}

bool SftpPlugin::startBrowsing()
{
    if (mountAndWait()) {
        //return new KRun(QUrl::fromLocalFile(mountPoint()), 0);
        return new KRun(QUrl("kdeconnect://"+deviceId), nullptr);
    }
    return false;
}

bool SftpPlugin::receivePackage(const NetworkPackage& np)
{
    if (!(fields_c - np.body().keys().toSet()).isEmpty()) {
        // package is invalid
        return false;
    }
    
    Q_EMIT packageReceived(np);

    remoteDirectories.clear();
    if (np.has(QStringLiteral("multiPaths"))) {
        QStringList paths = np.get<QStringList>(QStringLiteral("multiPaths"),QStringList());
        QStringList names = np.get<QStringList>(QStringLiteral("pathNames"),QStringList());
        int size = qMin<int>(names.size(), paths.size());
        for (int i = 0; i < size; i++) {
            remoteDirectories.insert(mountPoint() + paths.at(i), names.at(i));
        }
    } else {
        remoteDirectories.insert(mountPoint(), i18n("All files"));
        remoteDirectories.insert(mountPoint() + "/DCIM/Camera", i18n("Camera pictures"));
    }

    return true;
}

QString SftpPlugin::mountPoint()
{
    QDir mountDir = config()->privateDirectory();
    return mountDir.absoluteFilePath(deviceId);
}

void SftpPlugin::onMounted()
{
    qCDebug(KDECONNECT_PLUGIN_SFTP) << device()->name() << QStringLiteral("Remote filesystem mounted at %1").arg(mountPoint());

    Q_EMIT mounted();
}

void SftpPlugin::onUnmounted()
{
    qCDebug(KDECONNECT_PLUGIN_SFTP) << device()->name() << "Remote filesystem unmounted";

    unmount();
    
    Q_EMIT unmounted();
}

void SftpPlugin::onFailed(const QString& message)
{
    KNotification::event(KNotification::Error, device()->name(), message);

    unmount();

    Q_EMIT unmounted();
}

QVariantMap SftpPlugin::getDirectories()
{
    return remoteDirectories;
}

#include "sftpplugin.moc"
