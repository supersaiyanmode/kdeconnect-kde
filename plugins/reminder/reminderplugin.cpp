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
#include <Akonadi/Collection>
#include <Akonadi/Control>
#include <KCalCore/Incidence>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QDBusMessage>

K_PLUGIN_FACTORY( KdeConnectPluginFactory, registerPlugin< ReminderPlugin >(); )
K_EXPORT_PLUGIN( KdeConnectPluginFactory("kdeconnect_reminder", "kdeconnect-plugins") )

ReminderPlugin::ReminderPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args),mCalendar(QStringList()<<KCalCore::Todo::todoMimeType()),isProcessing(false),shouldSend(false),mSentTodo()

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
	QString iCal=np.get<QString>("iCal");
    QString op=np.get<QString>("op");
    QString uid=np.get<QString>("uid");
    QString status=np.get<QString>("status");
    if (np.has("request")){
        shouldSend=true;
        sendCalendar();
    }
    if (op=="delete"){
        KCalCore::Todo::Ptr oldTodo=mCalendar.todo(uid);
        mCalendar.deleteTodo(oldTodo);
    }
    else if (op=="merge"){
        if (status == "begin"){
            isProcessing=true;
            return true;
        }
        else if (status=="end"){
            isProcessing=false;
            sendCalendar();
            return true;
        }
        else if (status=="process"){
            isProcessing=true;
        }
        KCalCore::Todo::Ptr oldTodo=mCalendar.todo(uid);
        KCalCore::Incidence::Ptr incidence=parseICal(iCal);
        KCalCore::Todo::Ptr newTodo=incidence.staticCast<KCalCore::Todo>();
        
        if (oldTodo.isNull()){
            mCalendar.addTodo(newTodo);
            shouldSend=true;
        }
        else{
            if (*oldTodo!=*newTodo && oldTodo->lastModified()< newTodo->lastModified()){
                mCalendar.modifyIncidence(newTodo);
                shouldSend=true;
            }
        }
        
    }
    return true;
}

void ReminderPlugin::connected()
{
    QTimer::singleShot( 3000, this, SLOT( delayedRequest() ) );
}

void ReminderPlugin::delayedRequest()
{ 
    NetworkPackage np(PACKAGE_TYPE_REMINDER);
    np.set("request",true);
    sendPackage(np);
}

int ReminderPlugin::setupResource()
{
    KSharedConfigPtr config = KSharedConfig::openConfig("kdeconnectrc");
    KConfigGroup data = config->group("trusted_devices").group(device()->id());

    mResourceId= data.readEntry<QString>("ical_resource_id", QLatin1String(""));
    if (mResourceId!=""){
        Akonadi::AgentInstance mAgentInstance=Akonadi::AgentManager::self()->instance(mResourceId);
        if (!mAgentInstance.isValid()){
            mResourceId="";
        }
    }

    if (mResourceId==""){
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
    mCalendar.setCollectionFilteringEnabled(false);
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
            Akonadi::CollectionFetchJob::Recursive,
            this);
    job->fetchScope().setContentMimeTypes(QStringList()<<KCalCore::Todo::todoMimeType());
    job->setResource(mResourceId);
    connect(job, SIGNAL(result(KJob*)), SLOT(collectionFetchResult(KJob*)));
    return 0;
}

void ReminderPlugin::collectionFetchResult(KJob* job)
{
    Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob*>( job );
    if (fetchJob->collections().count()!=0 ){
        mCollection=fetchJob->collections().first();
    }
    
    if (!mCollection.isValid()){
        kDebug(debugArea())<<"collection fetch failed";
    }
    
    mCalendar.incidenceChanger()->setDefaultCollection(mCollection);
    mCalendar.incidenceChanger()->setDestinationPolicy(Akonadi::IncidenceChanger::DestinationPolicyNeverAsk);
}

void ReminderPlugin::sendCalendar()
{
    //this list accumulate why?
    if (!shouldSend || isProcessing)
        return;

    NetworkPackage np(PACKAGE_TYPE_REMINDER);
    np.set("status","begin");
    np.set("op","merge");
    sendPackage(np);
    foreach(Akonadi::Item item,mCalendar.items(mCollection.id()))
    {
        KCalCore::Todo::Ptr incidence=itemToTodo(item);

        QString uid=incidence->uid();
        KCalCore::Todo::Ptr sent=mSentTodo[uid];
        if (sent.isNull() || (*incidence!=*sent) ){
            QString str=conventor.toICalString(incidence);
            kDebug(debugArea()) << "send reminder :\n" <<str;
            NetworkPackage np(PACKAGE_TYPE_REMINDER);
            np.set("iCal",str);
            np.set("op","merge");
            sendPackage(np);
            mSentTodo[uid]=incidence;
        }
    }
    NetworkPackage np2(PACKAGE_TYPE_REMINDER);
    np2.set("status","end");
    np2.set("op","merge");
    sendPackage(np2);
    shouldSend=false;
}

KCalCore::Todo::Ptr ReminderPlugin::itemToTodo(const Akonadi::Item &item)
{
    if(item.hasPayload()){
        KCalCore::Incidence::Ptr inc=item.payload<KCalCore::Incidence::Ptr>();
        if (inc->mimeType()==KCalCore::Todo::todoMimeType()){
            return inc.staticCast<KCalCore::Todo>();
        }
    }
    return KCalCore::Todo::Ptr();
}

KCalCore::Incidence::Ptr ReminderPlugin::parseICal(QString& iCal)
{
    kDebug(debugArea()) << "parse iCal :\n" << iCal;
    return conventor.fromString(iCal);
}

void ReminderPlugin::calendarChanged()
{
    shouldSend=true;
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