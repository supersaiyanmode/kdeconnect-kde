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

#include "contactplugin.h"
#include <core/kdebugnamespace.h>
#include <Akonadi/Control>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <akonadi/itemfetchjob.h>
#include <Akonadi/ChangeRecorder>
#include <akonadi/itemfetchscope.h>
#include <KABC/VCardConverter>

K_PLUGIN_FACTORY( KdeConnectPluginFactory, registerPlugin< ContactPlugin >(); )
K_EXPORT_PLUGIN( KdeConnectPluginFactory("kdeconnect_contact", "kdeconnect-plugins") )

ContactPlugin::ContactPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args), isRequest(false)
{
	if ( !Akonadi::Control::start() ) {
           kDebug(debugArea()) <<"Akonadi failed to start";
    }
    kDebug(debugArea()) << "Contact plugin constructor for device" << device()->name();
}

bool ContactPlugin::receivePackage(const NetworkPackage& np)
{
    kDebug(debugArea()) << "Contact plugin received a package" ;
    QString vcards=np.get<QString>("vcard");
    QString op=np.get<QString>("op");
    if(np.has("request")){
        isRequest=true;
        fetchItems();
    }
    if (op=="delete"){
        QString uid=np.get<QString>("uid");
        deleteContact(uid);
    }
    else if (op=="merge")
    {
        kDebug(debugArea())<<"merge vcard"<<vcards;
        KABC::VCardConverter convertor;
        KABC::Addressee::List addr=convertor.parseVCards(vcards.toLocal8Bit());
        mergeContacts(addr);
    }
    return true;
}

void ContactPlugin::connected()
{
    fetchCollection();
    fetchItems();
	// QTimer::singleShot( 3000, this, SLOT( delayedRequest() ) );
}

void ContactPlugin::delayedRequest()
{
    if (1)
    {   
        NetworkPackage np(PACKAGE_TYPE_CONTACT);
		np.set("request",true);
		sendPackage(np);
    }
}

void ContactPlugin::sendContacts()
{
    KABC::VCardConverter convertor;
    KABC::Addressee::List list;
    foreach(Akonadi::Item item,mlist)
    {
        if ( !item.isValid() ) {
            kWarning() << "Item not valid";
            // return;
        }
        KABC::Addressee addr=item.payload<KABC::Addressee>();
        QString str=convertor.createVCard(addr,KABC::VCardConverter::v3_0);
        NetworkPackage np(PACKAGE_TYPE_CONTACT);
        np.set("vcard",str);
        np.set("op","merge");
        sendPackage(np);
        kDebug(debugArea())<<"generate vcard";
        kDebug(debugArea())<<str;
    }
}

void  ContactPlugin::deleteContact(QString& uid)
{

}

void ContactPlugin::addContacts(KABC::Addressee::List& list)
{
    foreach(KABC::Addressee addr, list)
    {
        Akonadi::Item item;
        item.setPayload<KABC::Addressee>( addr );
        item.setMimeType( KABC::Addressee::mimeType() );

        Akonadi::ItemCreateJob* job=new Akonadi::ItemCreateJob(item,mCollection,this);
        connect( job, SIGNAL( result( KJob* ) ), SLOT( contactCreateResult( KJob* ) ) );
    }
}

void ContactPlugin::mergeContacts(KABC::Addressee::List& list)
{
    addContacts(list);
}

void ContactPlugin::fetchCollection()
{
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
            Akonadi::CollectionFetchJob::Recursive,
            this);
    // Get list of collections
    job->fetchScope().setContentMimeTypes(QStringList() << KABC::Addressee::mimeType());
    job->exec();

    Akonadi::Collection::List collections = job->collections();
    if (collections.count()<=0){
        kDebug(debugArea())<<"no collection";
    }
    mCollection=collections.first();    
}

void ContactPlugin::fetchItems()
{
    kDebug(debugArea())<<"fetchItems";
    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob( mCollection, this );
    fetchJob->fetchScope().fetchFullPayload();
    connect( fetchJob, SIGNAL( result( KJob* ) ), SLOT( itemFetchDone( KJob*) ) );
}

void ContactPlugin::itemFetchDone(KJob* job)
{
    kDebug(debugArea())<<"items fetch done";
    Akonadi::ItemFetchJob *fetchJob = static_cast<Akonadi::ItemFetchJob*>( job );
    if ( job->error() ) {
        kError() << job->errorString();
        return;
    }

    if ( fetchJob->items().isEmpty() ) {
        kWarning() << "Job did not retrieve any items";
        return;
    }
    mlist = fetchJob->items();
    if (isRequest){
        isRequest=false;
        sendContacts();
    }
}

void ContactPlugin::contactCreateResult( KJob* job)
{
    if ( job->error() != KJob::NoError ) {
        kDebug(debugArea())<<"create error:"<<job->errorString();;
        return;
    }
    kDebug(debugArea())<<"contact created ";
}