#include <TelepathyQt/Types>
#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

#include "protocol.h"

#include "kdeconnecttelepathyprotocol.h"

Tp::WeakPtr<SimpleProtocol> KDEConnectTelepathyProtocolFactory::s_protocol;

SimpleProtocolPtr KDEConnectTelepathyProtocolFactory::simpleProtocol() {
    if (s_protocol.isNull()) {
        Tp::registerTypes();
        Tp::enableDebug(true);
        Tp::enableWarnings(true);

        SimpleProtocolPtr protocol = Tp::BaseProtocol::create<SimpleProtocol>(
                QDBusConnection::sessionBus(),
                QLatin1String("kdeconnect"));
        s_protocol = protocol;

        static Tp::BaseConnectionManagerPtr cm = Tp::BaseConnectionManager::create(
                QDBusConnection::sessionBus(), QLatin1String("kdeconnect"));

        protocol->setConnectionManagerName(cm->name());
        protocol->setEnglishName(QLatin1String("KDE Connect"));
        protocol->setIconName(QLatin1String("kdeconnect"));
        protocol->setVCardField(QLatin1String("phone_number"));

        cm->addProtocol(protocol);
        cm->registerObject();
    }
    return s_protocol.toStrongRef();
}
