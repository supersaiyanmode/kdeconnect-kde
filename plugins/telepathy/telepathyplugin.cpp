/**
 * Copyright 2014 Alexandr Akulich <akulichalexander@gmail.com>
 * Copyright 2014 David Edmundson <davidedmundson@kde.org>
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

#include "telepathyplugin.h"

// #include "../../telepathy-cm/simple

#include <KLocalizedString>
#include <KPluginFactory>

#include <QDebug>

K_PLUGIN_FACTORY( KdeConnectPluginFactory, registerPlugin< TelepathyPlugin >(); )
Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_TELEPHONY, "kdeconnect.plugin.telepathy")

TelepathyPlugin::TelepathyPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
    m_protocol = KDEConnectTelepathyProtocolFactory::simpleProtocol();
    connect(m_protocol.constData(), SIGNAL(messageReceived(QString,QString)), SLOT(sendMessage(QString,QString)));
}

TelepathyPlugin::~TelepathyPlugin()
{
}

bool TelepathyPlugin::receivePackage(const NetworkPackage& np)
{
    if (np.type() == QLatin1String("kdeconnect.telephony")) {
        const QString event = np.get<QString>("event");
        if (event == "sms") {
            const QString phoneNumber = np.get<QString>("phoneNumber", i18n("unknown number"));
            const QString messageBody = np.get<QString>("messageBody", "");

            m_protocol->sendMessage(phoneNumber, messageBody);
        }
        return true;
    }
    return true;
}

void TelepathyPlugin::connected()
{
    NetworkPackage np(PACKAGE_TYPE_TELEPATHY);
    np.set("requestNumbers", true);
    sendPackage(np);
    return;
}

void TelepathyPlugin::sendMessage(const QString &receiver, const QString &message)
{
    NetworkPackage np(PACKAGE_TYPE_TELEPATHY);
    np.set("sendSms", true);
    np.set("receiver", receiver);
    np.set("message", message);
    sendPackage(np);

    qDebug() << "sending message";

    //sending needed here
}

#include "telepathyplugin.moc"
