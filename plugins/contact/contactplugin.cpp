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

#include <Akonadi/AgentManager>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/Control>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <akonadi/itemfetchjob.h>
#include <Akonadi/ChangeRecorder>
#include <akonadi/itemfetchscope.h>
#include <KABC/VCardConverter>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QDBusMessage>
 
K_PLUGIN_FACTORY( KdeConnectPluginFactory, registerPlugin< ContactPlugin >(); )
K_EXPORT_PLUGIN( KdeConnectPluginFactory("kdeconnect_contact", "kdeconnect-plugins") )

ContactPlugin::ContactPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args), isRequest(false)
{
	if ( !Akonadi::Control::start() ) {
           kDebug(debugArea()) <<"Akonadi failed to start";
    }
    setupResource();
    kDebug(debugArea()) << "Contact plugin constructor for device" << device()->name();
}

bool ContactPlugin::receivePackage(const NetworkPackage& np)
{
    kDebug(debugArea()) << "Contact plugin received a package" ;
    QString vcards=np.get<QString>("vcard");
    QString op=np.get<QString>("op");
    if(np.has("request")){
        
    }
    if (op=="delet"){
        if (np.has("uid"))
        {
            QString uid=np.get<QString>("uid");
            deleteContact(uid);
        }
        else if (np.has("id"))
        {
            int id=np.get<int>("id");
            QString rid=device()->id()+QString::number(id);
            deleteContact(rid);
        }
    }
    else if (op=="merge")
    {
        int id=np.get<int>("id");
        QString rid=device()->id()+QString::number(id);
        kDebug(debugArea())<<"merge vcard"<<"rid:"<<rid<<"\n"<<vcards;
        KABC::VCardConverter convertor;
        KABC::Addressee addr=convertor.parseVCard(vcards.toLocal8Bit());
        mergeContacts(addr,rid);
    }
    return true;
}

void ContactPlugin::connected()
{
    // fetchItems();
	// QTimer::singleShot( 3000, this, SLOT( delayedRequest() ) );
}

void ContactPlugin::delayedRequest()
{
    NetworkPackage np(PACKAGE_TYPE_CONTACT);
	np.set("request",true);
	sendPackage(np);
}

int ContactPlugin::setupResource()
{
    KSharedConfigPtr config = KSharedConfig::openConfig("kdeconnectrc");
    KConfigGroup data = config->group("trusted_devices").group(device()->id());

    resID= data.readEntry<QString>("contacts_resource_id", QLatin1String(""));
    if (resID!="")
    {
        kDebug(debugArea())<<" res id in config file"<<resID;
        Akonadi::AgentInstance mAgentInstance=Akonadi::AgentManager::self()->instance(resID);
        if (!mAgentInstance.isValid()){
            resID="";
            kDebug(debugArea())<<"instance non valid";
        }
    }

    if (resID=="")
    {
        Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( "akonadi_contacts_resource", this );
        job->exec();
        if ( job->error() ) {
            kDebug(debugArea())<< "Create calendar failed:" << job->errorString();
            return -1;
        }
        resID=job->instance().identifier();
        config->group("trusted_devices").group(device()->id()).writeEntry("contacts_resource_id",resID);
        config->sync();
        QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Akonadi.Agent."+resID, "/Settings", "org.kde.Akonadi.Contacts.Settings", "setPath");
        QList<QVariant> args;
        args.append("kdeconnect_contact/"+device()->id());
        msg.setArguments(args);
        bool queued= QDBusConnection::sessionBus().send(msg);
        job->instance().synchronize();
    }
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
            Akonadi::CollectionFetchJob::Recursive,
            this);
    job->fetchScope().setContentMimeTypes(QStringList() << KABC::Addressee::mimeType() );
    job->setResource(resID);
    connect(job, SIGNAL(result(KJob*)), SLOT(collectionFetchResult(KJob*)));
    return 0;
}

void ContactPlugin::collectionFetchResult(KJob* job)
{
    Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob*>( job );
    if (fetchJob->collections().count()==0){
        kDebug(debugArea())<<"no collection";
    }else{
        kDebug(debugArea())<<"got the collection";
        mCollection=fetchJob->collections().first();
    }
    
    if (!mCollection.isValid()){
        kDebug(debugArea())<<"collection fetch failed";
    }
    mCollection.setName(device()->name());
    Akonadi::CollectionModifyJob *job2 = new Akonadi::CollectionModifyJob( mCollection , this);
    job2->exec();
    mCollection=job2->collection();
    kDebug(debugArea())<<mCollection;
}

void ContactPlugin::sendContacts()
{
    KABC::VCardConverter convertor;
    KABC::Addressee::List list;
    foreach(QString uid,mItemMap.keys())
    {
        Akonadi::Item item=mItemMap[uid];
        if ( !item.isValid() ) {
            kWarning() << "Item not valid";
            // return;
        }
        KABC::Addressee addr=item.payload<KABC::Addressee>();
        QString str=convertor.createVCard(addr,KABC::VCardConverter::v3_0);
        NetworkPackage np(PACKAGE_TYPE_CONTACT);
        np.set("vcard",str);
        np.set("op","merge");
        np.set("uid",uid);
        sendPackage(np);
    }
}

void  ContactPlugin::deleteContact(QString& uid)
{
    if (mItemMap.contains(uid))
    {
        Akonadi::Item item=mItemMap[uid];
        Akonadi::ItemDeleteJob* job=new Akonadi::ItemDeleteJob(item , this);
        connect( job, SIGNAL( result( KJob* ) ), SLOT( contactRemoveResult( KJob* ) ) );
    }
}

void ContactPlugin::addContacts(KABC::Addressee addr,QString& uid)
{
    kDebug(debugArea())<<"adding contact"<<uid;
    Akonadi::Item item;
    item.setPayload<KABC::Addressee>( addr );
    item.setMimeType( KABC::Addressee::mimeType() );
    Akonadi::ItemCreateJob* job=new Akonadi::ItemCreateJob(item,mCollection,this);
    connect( job, SIGNAL( result( KJob* ) ), SLOT( contactCreateResult( KJob* ) ) );
}

void ContactPlugin::modifyContact(KABC::Addressee addr,QString& uid)
{
    Akonadi::Item item=mItemMap[uid];
    item.setPayload<KABC::Addressee>( addr );
    Akonadi::ItemModifyJob* job=new Akonadi::ItemModifyJob(item,this);
    connect( job, SIGNAL( result( KJob* ) ), SLOT( contactModifyResult( KJob* ) ) );
}

void ContactPlugin::mergeContacts(KABC::Addressee addr,QString& uid)
{
    if (mItemMap.contains(uid)){
        modifyContact(addr,uid);
    }else{
        addContacts(addr,uid);
    }
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
    mItemMap.clear();
    foreach(Akonadi::Item item, fetchJob->items()){
        KABC::Addressee addr=item.payload<KABC::Addressee>();
        mItemMap.insert(addr.uid(),item);
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

void ContactPlugin::contactRemoveResult( KJob* job)
{
    if ( job->error() != KJob::NoError ) {
        kDebug(debugArea())<<"remove error:"<<job->errorString();;
        return;
    }
    kDebug(debugArea())<<"contact removed ";
}

void ContactPlugin::contactModifyResult( KJob* job)
{
    if ( job->error() != KJob::NoError ) {
        kDebug(debugArea())<<"modify error:"<<job->errorString();;
        return;
    }
    kDebug(debugArea())<<"contact modified ";
}