/**
 * Copyright 2014 YANG Qiao <yangqiao0505@me.com>
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

#ifndef CONTACTPLUGIN_H
#define CONTACTPLUGIN_H

#include <QObject>

#include <core/kdeconnectplugin.h>

#include <KABC/Addressee>
#include <KJob>
#include <Akonadi/Collection>
 #include <Akonadi/Item>

#define PACKAGE_TYPE_CONTACT QLatin1String("kdeconnect.contact")

class ContactPlugin
    : public KdeConnectPlugin
{
    Q_OBJECT
private:
	Akonadi::Collection mCollection;
	Akonadi::Item::List mlist;
	bool isRequest;
public:
    explicit ContactPlugin(QObject *parent, const QVariantList &args);
    void sendContacts();
    void deleteContact(QString& uid);
    void addContacts(KABC::Addressee::List& list);
    void mergeContacts(KABC::Addressee::List& list);
    void fetchCollection();
    void fetchItems();
public Q_SLOTS:
    virtual bool receivePackage(const NetworkPackage& np);
    virtual void connected();
    void delayedRequest();
    void contactCreateResult( KJob* job);
    void itemFetchDone(KJob* job);
};

#endif
