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

#include "mounter.h"

#include <QDir>
#include <QDebug>

#include <KLocalizedString>

#include "mountloop.h"
#include "sftp_debug.h"
#include "kdeconnectconfig.h"

Mounter::Mounter(SftpPlugin* sftp)
    : QObject(sftp)
    , m_sftp(sftp)
    , m_proc(nullptr)
    , m_mountPoint(sftp->mountPoint())
    , m_started(false)
{

    connect(m_sftp, &SftpPlugin::packageReceived, this, &Mounter::onPakcageReceived);

    connect(&m_connectTimer, &QTimer::timeout, this, &Mounter::onMountTimeout);

    connect(this, &Mounter::mounted, &m_connectTimer, &QTimer::stop);
    connect(this, &Mounter::failed, &m_connectTimer, &QTimer::stop);

    m_connectTimer.setInterval(10000);
    m_connectTimer.setSingleShot(true);

    QTimer::singleShot(0, this, &Mounter::start);
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Created mounter";
}

Mounter::~Mounter()
{
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Destroy mounter";
    //FIXME: We don't unmount becuase it crashes if we try to unmount while the filesystem is being used. Potential memory leak.
    //unmount();
}

bool Mounter::wait()
{
    if (m_started)
    {
        return true;
    }
    
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Starting loop to wait for mount";
    
    MountLoop loop;
    connect(this, &Mounter::mounted, &loop, &MountLoop::successed);
    connect(this, &Mounter::failed, &loop, &MountLoop::failed);
    return loop.exec();
}

void Mounter::onPakcageReceived(const NetworkPackage& np)
{
    if (np.get<bool>(QStringLiteral("stop"), false))
    {
        qCDebug(KDECONNECT_PLUGIN_SFTP) << "SFTP server stopped";
        unmount();
        return;
    }

    //This is the previous code, to access sftp server using KIO. Now we are
    //using the external binary sshfs, and accessing it as a local filesystem.
  /*
   *    QUrl url;
   *    url.setScheme("sftp");
   *    url.setHost(np.get<QString>("ip"));
   *    url.setPort(np.get<QString>("port").toInt());
   *    url.setUserName(np.get<QString>("user"));
   *    url.setPassword(np.get<QString>("password"));
   *    url.setPath(np.get<QString>("path"));
   *    new KRun(url, 0);
   *    Q_EMIT mounted();
   */

    unmount();

    m_proc = new KProcess(this);
    m_proc->setOutputChannelMode(KProcess::MergedChannels);

    connect(m_proc, &QProcess::started, this, &Mounter::onStarted);
    connect(m_proc, SIGNAL(error(QProcess::ProcessError)), SLOT(onError(QProcess::ProcessError)));
    connect(m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onFinished(int,QProcess::ExitStatus)));

    QDir().mkpath(m_mountPoint);

    const QString program = QStringLiteral("sshfs");

    QString path;
    if (np.has(QStringLiteral("multiPaths"))) path = '/';
    else path = np.get<QString>(QStringLiteral("path"));

    const QStringList arguments = QStringList()
        << QStringLiteral("%1@%2:%3").arg(
            np.get<QString>(QStringLiteral("user")),
            np.get<QString>(QStringLiteral("ip")),
            path)
        << m_mountPoint
        << QStringLiteral("-p") << np.get<QString>(QStringLiteral("port"))
        << QStringLiteral("-f")
        << QStringLiteral("-F") << QStringLiteral("/dev/null") //Do not use ~/.ssh/config
        << QStringLiteral("-o") << "IdentityFile=" + KdeConnectConfig::instance()->privateKeyPath()
        << QStringLiteral("-o") << QStringLiteral("StrictHostKeyChecking=no") //Do not ask for confirmation because it is not a known host
        << QStringLiteral("-o") << QStringLiteral("UserKnownHostsFile=/dev/null") //Prevent storing as a known host
        << QStringLiteral("-o") << QStringLiteral("HostKeyAlgorithms=ssh-dss") //https://bugs.kde.org/show_bug.cgi?id=351725
        << QStringLiteral("-o") << QStringLiteral("password_stdin")
        ;

    m_proc->setProgram(program, arguments);

    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Starting process: " << m_proc->program().join(QStringLiteral(" "));
    m_proc->start();

    //qCDebug(KDECONNECT_PLUGIN_SFTP) << "Passing password: " << np.get<QString>("password").toLatin1();
    m_proc->write(np.get<QString>(QStringLiteral("password")).toLatin1());
    m_proc->write("\n");

}

void Mounter::onStarted()
{
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Process started";
    m_started = true;
    Q_EMIT mounted();

    //m_proc->setStandardOutputFile("/tmp/kdeconnect-sftp.out");
    //m_proc->setStandardErrorFile("/tmp/kdeconnect-sftp.err");
    connect(m_proc, &KProcess::readyReadStandardError, [this]() {
        qCDebug(KDECONNECT_PLUGIN_SFTP) << "stderr: " << m_proc->readAll();
    });
    connect(m_proc, &KProcess::readyReadStandardOutput, [this]() {
        qCDebug(KDECONNECT_PLUGIN_SFTP) << "stdout:" << m_proc->readAll();
    });
}

void Mounter::onError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart)
    {
        qCDebug(KDECONNECT_PLUGIN_SFTP) << "Process failed to start";
        m_started = false;
        Q_EMIT failed(i18n("Failed to start sshfs"));
    }
}

void Mounter::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit)
    {
        qCDebug(KDECONNECT_PLUGIN_SFTP) << "Process finished (exit code: " << exitCode << ")";
        Q_EMIT unmounted();
    }
    else
    {
        qCDebug(KDECONNECT_PLUGIN_SFTP) << "Process failed (exit code:" << exitCode << ")";
        Q_EMIT failed(i18n("Error when accessing to filesystem"));
    }
    
    unmount();
}

void Mounter::onMountTimeout()
{
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Timeout: device not responding";
    Q_EMIT failed(i18n("Failed to mount filesystem: device not responding"));
}

void Mounter::start()
{
    NetworkPackage np(PACKAGE_TYPE_SFTP_REQUEST, {{"startBrowsing", true}});
    m_sftp->sendPackage(np);
    
    m_connectTimer.start();
}

void Mounter::unmount()
{
    qCDebug(KDECONNECT_PLUGIN_SFTP) << "Unmount" << m_proc;
    if (m_proc)
    {
        auto toDestroy = m_proc;
        m_proc = nullptr; //So we don't reenter this code path when onFinished gets called
        toDestroy->kill();
        delete toDestroy;
        //Free mount point (won't always succeed if the path is in use)
        KProcess::execute(QStringList() << QStringLiteral("fusermount") << QStringLiteral("-u") << m_mountPoint, 10000);
    }
    m_started = false;
}

