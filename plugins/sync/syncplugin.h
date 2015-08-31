/**
 * Copyright 2015 Ashish Bansal <bansal.ashish096@gmail.com>
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

#ifndef SYNCPLUGIN_H
#define SYNCPLUGIN_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QTimer>

#include <KIO/Job>

#include <core/kdeconnectplugin.h>
#include <core/networkpackage.h>

#define PACKAGE_TYPE_SYNC QLatin1String("kdeconnect.sync")

class Q_DECL_EXPORT SyncPlugin
    : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.sync")

    struct FileProperties {
        QString hash;
        QString modifiedDate;
        QString relativePath;
    };

public:
    explicit SyncPlugin(QObject *parent, const QVariantList &args);
    virtual ~SyncPlugin();

    Q_SCRIPTABLE QString syncDirectory();
    Q_SCRIPTABLE bool setSyncDirectory(QString path);
    Q_SCRIPTABLE QStringList listFilesInDirectory();
    Q_SCRIPTABLE QList<FileProperties> getFilesDesc(QStringList& fileList);


private Q_SLOTS:
    virtual bool receivePackage(const NetworkPackage& np);
    virtual void connected();
    Q_SCRIPTABLE void startSyncing();
    void finished(KJob* job);
    void directoryChanged(QString path);

private:
    QString dbusPath() const;
    void receiveFile(const NetworkPackage& np);
    void compareAndRequestFiles(const NetworkPackage &np);
    void sendRequestedFiles(const NetworkPackage &np);
    QFileSystemWatcher syncDirWatcher;
    QDir m_syncDir;
    QStringList m_incomingFiles;
    QTimer m_timer;
};

#endif // SYNCPLUGIN_H
