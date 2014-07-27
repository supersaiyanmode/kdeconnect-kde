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

#include "reminderplugin.h"
#include <core/kdebugnamespace.h>

#include <Akonadi/AgentManager>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/Calendar/IncidenceChanger>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>
#include <Akonadi/Collection>
#include <Akonadi/Control>
#include <KCalCore/Incidence>
#include <KCalCore/ICalFormat>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QDBusMessage>

K_PLUGIN_FACTORY( KdeConnectPluginFactory, registerPlugin< ReminderPlugin >(); )
K_EXPORT_PLUGIN( KdeConnectPluginFactory("kdeconnect_reminder", "kdeconnect-plugins") )

ReminderPlugin::ReminderPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args),mCalendar(QStringList()<<KCalCore::Todo::todoMimeType())

{
    if ( !Akonadi::Control::start() ) {
           kDebug(debugArea()) <<"Akonadi failed to start";
    }
    kDebug(debugArea()) << "Reminder plugin constructor for device" << device()->name();
    setupResource();
    connect(&mCalendar,SIGNAL(createFinished(bool,QString)),SLOT(createFinished(bool,QString)));
    connect(&mCalendar,SIGNAL(modifyFinished(bool,QString)),SLOT(modifyFinished(bool,QString)));
    connect(&mCalendar,SIGNAL(deleteFinished(bool,QString)),SLOT(deleteFinished(bool,QString)));
    connect(&mCalendar,SIGNAL(calendarChanged()),SLOT(calendarChanged()));
}

ReminderPlugin::~ReminderPlugin()
{
    kDebug(debugArea()) << "Reminder plugin destructor for device" << device()->name();
}

bool ReminderPlugin::receivePackage(const NetworkPackage& np)
{   
	kDebug(debugArea()) << "Reminder plugin received an package from device" << device()->name();
	QString iCal=np.get<QString>("iCal");
    QString op=np.get<QString>("op");
    if (np.has("request")){
        sendCalendar();
    }
    if (op=="delete")
    {
        QString uid=np.get<QString>("uid");
        deleteIncidence(uid);
    }
    else if (op=="merge"){
        KCalCore::Incidence::Ptr incidence=parseICal(iCal);
        mergeIncidence(incidence);
    }
    return true;
}

void ReminderPlugin::connected()
{
    QTimer::singleShot( 3000, this, SLOT( delayedRequest() ) );
}

void ReminderPlugin::delayedRequest()
{
    if (mCalendar.items(mCollection.id()).count()==0)
    {   
        NetworkPackage np(PACKAGE_TYPE_REMINDER);
        np.set("request",true);
        sendPackage(np);
    }
}

int ReminderPlugin::setupResource()
{
    KSharedConfigPtr config = KSharedConfig::openConfig("kdeconnectrc");
    KConfigGroup data = config->group("trusted_devices").group(device()->id());

    mResourceId= data.readEntry<QString>("ical_resource_id", QLatin1String(""));
    if (mResourceId!="")
    {
        kDebug(debugArea())<<" res id in config file"<<mResourceId;
        Akonadi::AgentInstance mAgentInstance=Akonadi::AgentManager::self()->instance(mResourceId);
        if (!mAgentInstance.isValid()){
            mResourceId="";
            kDebug(debugArea())<<"instance non valid";
        }
    }

    if (mResourceId=="")
    {
        Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( "akonadi_icaldir_resource", this );
        job->exec();
        if ( job->error() ) {
            kDebug(debugArea())<< "Create calendar failed:" << job->errorString();
            return -1;
        }
        mResourceId=job->instance().identifier();
        config->group("trusted_devices").group(device()->id()).writeEntry("ical_resource_id",mResourceId);
        config->sync();
        QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Akonadi.Agent."+mResourceId, "/Settings", "org.kde.Akonadi.ICalDirectory.Settings", "setPath");
        QList<QVariant> args;
        args.append("kdeconnect_ical/"+device()->id());
        msg.setArguments(args);
        bool queued= QDBusConnection::sessionBus().send(msg);
        job->instance().synchronize();
    }
    //wait a bit then fetch collection
    mCalendar.setCollectionFilteringEnabled(false);
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
            Akonadi::CollectionFetchJob::Recursive,
            this);
    job->fetchScope().setContentMimeTypes(QStringList() <<  KCalCore::Event::eventMimeType()<<KCalCore::Todo::todoMimeType());
    job->setResource(mResourceId);
    job->startTimer(100);
    connect(job, SIGNAL(result(KJob*)), SLOT(collectionFetchResult(KJob*)));
    return 0;
}

void ReminderPlugin::collectionFetchResult(KJob* job)
{
    Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob*>( job );
    if (fetchJob->collections().count()==0){
        kDebug(debugArea())<<"no collection";
    }else{
        kDebug(debugArea())<<"got the collection";
        mCollection=fetchJob->collections().first();
    }
    
    if (mCollection.isValid()){
        kDebug(debugArea())<<"collection fetch failed";
        kDebug(debugArea())<<mCollection;
    }
    
    mCalendar.incidenceChanger()->setDefaultCollection(mCollection);
    mCalendar.incidenceChanger()->setDestinationPolicy(Akonadi::IncidenceChanger::DestinationPolicyNeverAsk);
}

void ReminderPlugin::sendCalendar()
{
    if(!calendarDidChanged())
        return;
    //renew incidence list
    Akonadi::Item::List newItemList=mCalendar.items(mCollection.id());
    mSentInfoList.clear();
    foreach(Akonadi::Item item, newItemList)
    {
        KCalCore::Incidence::Ptr incidence=itemToIncidence(item);
        mSentInfoList.append(incidenceToInfo(incidence));
    }

    //send reminder
    KCalCore::ICalFormat ical;
    foreach(Akonadi::Item item,mCalendar.items(mCollection.id()))
    {
        KCalCore::Incidence::Ptr incidence=itemToIncidence(item);
        QList<KDateTime> datelist=incidence->startDateTimesForDate(QDate::currentDate(),KDateTime::LocalZone);
        datelist+=incidence->startDateTimesForDate(QDate::currentDate().addDays(1),KDateTime::LocalZone);
        if (datelist.length()==0){
            continue;
        }
        QString str=ical.toICalString(incidence);
        kDebug(debugArea()) << "send reminder :\n" <<str;
        NetworkPackage np(PACKAGE_TYPE_REMINDER);
        np.set("iCal",str);
        np.set("op","merge");
        sendPackage(np);
    }
}

bool ReminderPlugin::incidenceIsIden(ReminderPlugin::IncidenceInfo info1,ReminderPlugin::IncidenceInfo info2)
{
    if ( info1.uid!=info2.uid ||
        info1.summary!=info2.summary ||
        info1.s_date !=info2.s_date ||
        info1.e_date !=info2.e_date )
    {
        return false;
    }
    return true;
}

bool ReminderPlugin::incidenceIsIden(KCalCore::Incidence::Ptr& incidence1,KCalCore::Incidence::Ptr& incidence2)
{
    IncidenceInfo info1=incidenceToInfo(incidence1);
    IncidenceInfo info2=incidenceToInfo(incidence2);
    return incidenceIsIden(info1,info2);
}

KCalCore::Incidence::Ptr ReminderPlugin::itemToIncidence(const Akonadi::Item &item)
{
    if(item.hasPayload()){
        return item.payload<KCalCore::Incidence::Ptr>();
    }
    else{
        return KCalCore::Incidence::Ptr();
    }
}

void ReminderPlugin::addIncidence(KCalCore::Incidence::Ptr& incidence)
{
    kDebug(debugArea())<<"add incidence"<<incidence->uid();
    mCalendar.addIncidence(incidence);
}

void ReminderPlugin::modifyIncidence(KCalCore::Incidence::Ptr& incidence)
{
    kDebug(debugArea())<<"modify incidence"<<incidence->uid();
    mCalendar.modifyIncidence(incidence);
}

void ReminderPlugin::deleteIncidence(KCalCore::Incidence::Ptr& incidence)
{
    kDebug(debugArea())<<"delete incidence"<<incidence->uid();
    mCalendar.deleteIncidence(incidence);
}

void ReminderPlugin::deleteIncidence(QString& uid)
{
    kDebug(debugArea())<<"delete incidence"<<uid;
    Akonadi::Item item=mCalendar.item(uid);
    if (item.isValid())
    {
        kDebug(debugArea())<<"item valid "<<uid;
        KCalCore::Incidence::Ptr incidence=itemToIncidence(item);
        deleteIncidence(incidence);
    }
    else{
        kDebug(debugArea())<<"item invalid "<<uid;
    }
}

void ReminderPlugin::mergeIncidence(KCalCore::Incidence::Ptr& incidence)
{
    kDebug(debugArea())<<"merge incidence"<<incidence->summary();
    //FIXME it's dangerous when receive two same incidence to add at the same time
    // a duplicate will occur
    Akonadi::Item item=mCalendar.item(incidence->uid());
    if (item.isValid()){
        KCalCore::Incidence::Ptr oldInc=itemToIncidence(item);
        if (!incidenceIsIden(incidence,oldInc)){
                modifyIncidence(incidence);
        }
        else{
            kDebug(debugArea())<<"nothing changed with incidence"<<incidence->summary();
        }
    }
    else{
        addIncidence(incidence);
    }
}

bool ReminderPlugin::calendarDidChanged()
{
    Akonadi::Item::List newItemList=mCalendar.items(mCollection.id());
    if (newItemList.length()!=mSentInfoList.length()){
        kDebug(debugArea()) << "length not match";
        return true;
    }
    for (int i = 0; i < newItemList.length(); ++i){
        kDebug(debugArea()) << "comparing incidences";
        KCalCore::Incidence::Ptr incidence=itemToIncidence(newItemList[i]);
        IncidenceInfo newInfo=incidenceToInfo(incidence);
        QList<KDateTime> datelist=newInfo.s_date;
        if (datelist.empty()){
            kDebug(debugArea()) << "datelist empty";
            continue;
        }
        IncidenceInfo oldInfo=mSentInfoList[i];
        if (!incidenceIsIden(newInfo,oldInfo))
        {
            kDebug(debugArea()) << "an item not match";
            kDebug(debugArea()) << oldInfo.uid<<oldInfo.summary<<oldInfo.s_date<<oldInfo.e_date;
            kDebug(debugArea()) << newInfo.uid<<newInfo.summary<<newInfo.s_date<<newInfo.e_date;
            return true;
        }
    }
    return false;
}

KCalCore::Incidence::Ptr ReminderPlugin::parseICal(QString& iCal)
{
    //TODO find out a way to parse modify date so that we return the lated version of incidence
    //In this way we can save a lot
    kDebug(debugArea()) << "parse iCal :\n" << iCal;
    KCalCore::ICalFormat conventor;
    return conventor.fromString(iCal);
}

ReminderPlugin::IncidenceInfo  ReminderPlugin::incidenceToInfo(KCalCore::Incidence::Ptr& incidence)
{
    IncidenceInfo info;
    info.uid=incidence->uid();
    info.summary=incidence->summary();
    QList<KDateTime> datelist=incidence->startDateTimesForDate(QDate::currentDate(),KDateTime::LocalZone);
    datelist+=incidence->startDateTimesForDate(QDate::currentDate().addDays(1),KDateTime::LocalZone);
    info.s_date=datelist;
    foreach(KDateTime dt,info.s_date){
        info.e_date.append(incidence->endDateForStart(dt));
    }
    return info;
}

void ReminderPlugin::calendarChanged()
{
    sendCalendar();
}

void ReminderPlugin::createFinished(bool success, const QString &errorMessage)
{
    kDebug(debugArea()) <<"reminder add " <<success<<":"<<errorMessage;
}

void ReminderPlugin::deleteFinished(bool success, const QString &errorMessage)
{
    kDebug(debugArea()) <<"reminder delete " <<success<<":"<<errorMessage;
}

void ReminderPlugin::modifyFinished(bool success, const QString &errorMessage)
{
    kDebug(debugArea()) <<"reminder modify " <<success<<":"<<errorMessage;
}
