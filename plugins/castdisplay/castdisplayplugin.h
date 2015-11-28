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

#ifndef CASTDISPLAYPLUGIN_H
#define CASTDISPLAYPLUGIN_H

#include <QString>
#include <QHash>
#include <QJsonArray>

#include <core/kdeconnectplugin.h>

class CastDisplay : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.castdisplay")

public:
    static CastDisplay* self();

    void startProgramForDocument(const QString &programPath, const QUrl &url);
    void emitStartProgram(const QString& programPath, const QJsonObject& data);

Q_SIGNALS:
    void startProgramSerialized(const QString &programPath, const QByteArray &data);

private:
    CastDisplay();
};

class CastDisplayPlugin
    : public KdeConnectPlugin
{
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.castdisplay")
    Q_OBJECT

public:
    explicit CastDisplayPlugin(QObject *parent, const QVariantList &args);

    bool receivePackage(const NetworkPackage& np) override;
    void connected() override;

private Q_SLOTS:
    void fileReceived(const QUrl &url);
    void setCurrentProgram(const QString &current);

private:
    QHash<QString, QString> m_programForName;
    QHash<QString, QUrl> m_pending;
};

#endif
