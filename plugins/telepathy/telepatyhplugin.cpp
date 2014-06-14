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

#include "telepathyplugin.h"

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

#include "../../kdebugnamespace.h"

#include <SimpleCM/Protocol>

#include <KLocalizedString>

#include <QDebug>

static SimpleProtocol *g_proto = 0;

K_PLUGIN_FACTORY( KdeConnectPluginFactory, registerPlugin< TelepathyPlugin >(); )
K_EXPORT_PLUGIN( KdeConnectPluginFactory("kdeconnect_telepathy", "kdeconnect-kded") )

TelepathyPlugin::TelepathyPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    if (g_proto) {
        connect(g_proto, SIGNAL(messageReceived(QString,QString)), SLOT(sendMessage(QString,QString)));
        return;
    }

    static Tp::BaseProtocolPtr proto = Tp::BaseProtocol::create<SimpleProtocol>(
            QDBusConnection::sessionBus(),
            QLatin1String("kdeconnect"));
    static Tp::BaseConnectionManagerPtr cm = Tp::BaseConnectionManager::create(
            QDBusConnection::sessionBus(), QLatin1String("kdeconnect"));

    g_proto = static_cast<SimpleProtocol*> (proto.data());

    g_proto->setConnectionManagerName(cm->name());
    g_proto->setEnglishName(QLatin1String("KDE Connect"));
    g_proto->setIconName(QLatin1String("kdeconnect"));
    g_proto->setVCardField(QLatin1String("phone_number"));

    cm->addProtocol(proto);
    cm->registerObject();
}

bool TelepathyPlugin::receivePackage(const NetworkPackage& np)
{
    if (np.type() == QLatin1String("kdeconnect.ping")) {

//        static bool odd = true;

//        odd = !odd;

//        if (odd)
//            return;

        NetworkPackage np(PACKAGE_TYPE_TELEPATHY);
        np.set("requestNumbers", true);
        device()->sendPackage(np);
    }
    if (np.type() == QLatin1String("kdeconnect.telephony")) {
        const QString event = np.get<QString>("event");
        if (event == "sms") {
            const QString phoneNumber = np.get<QString>("phoneNumber", i18n("unknown number"));
            const QString messageBody = np.get<QString>("messageBody", "");

            g_proto->sendMessage(phoneNumber, messageBody);
        }
        return true;
    } else if (np.type() == QLatin1String("kdeconnect.telepathy")) {
        QStringList cards = np.get<QStringList>("numbers");

        qDebug() << cards;
        g_proto->setContactList(cards);
    }

//    g_proto->sendMessage(np.type(), np.get<QStringList>("numbers").join(" "));

    return true;
}

void TelepathyPlugin::connected()
{
    NetworkPackage np(PACKAGE_TYPE_TELEPATHY);
    np.set("requestNumbers", true);
    device()->sendPackage(np);

    return;
}

void TelepathyPlugin::sendMessage(/*const QString &deviceId, */const QString &receiver, const QString &message)
{
//    if (device()->id() != deviceId) {
//        return;
//    }

    NetworkPackage np(PACKAGE_TYPE_TELEPATHY);
    np.set("sendSms", true);
    np.set("receiver", receiver);
    np.set("message", message);
    device()->sendPackage(np);

    qDebug() << "Called";
}

