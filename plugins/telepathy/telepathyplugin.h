/**
 * Copyright 2014 Alexandr Akulich <akulichalexander@gmail.com>
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

#ifndef TELEPATHYPLUGIN_H
#define TELEPATHYPLUGIN_H

#include <QLoggingCategory>

#include <core/kdeconnectplugin.h>

#include "telepathy-cm/kdeconnecttelepathyprotocol.h"

#define PACKAGE_TYPE_TELEPATHY QLatin1String("kdeconnect.telepathy")

class TelepathyPlugin
    : public KdeConnectPlugin
{
    Q_OBJECT
public:
    explicit TelepathyPlugin(QObject *parent, const QVariantList &args);
    ~TelepathyPlugin();

public Q_SLOTS:
    bool receivePackage(const NetworkPackage& np);
    void connected();

private Q_SLOTS:
    void sendMessage(const QString &receiver, const QString &message);

private:
    SimpleProtocolPtr m_protocol;
};

#endif // TELEPATHYPLUGIN_H

