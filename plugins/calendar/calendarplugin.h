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

#ifndef CALENDARPLUGIN_H
#define CALENDARPLUGIN_H

#include <QObject>

#include <core/kdeconnectplugin.h>
#include <Akonadi/Calendar/ETMCalendar>
#include <Akonadi/Collection>
#include <KCalCore/Incidence>

#define PACKAGE_TYPE_CALENDAR QLatin1String("kdeconnect.calendar")

class KDE_EXPORT CalendarPlugin
    : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.calendar")
private:
    typedef struct incidencInfo
    {
        QString uid;
        QString summary;
        QList<KDateTime> s_date;
        QList<KDateTime> e_date;
    }IncidenceInfo;
	Akonadi::ETMCalendar mCalendar;
    Akonadi::Collection mCollection;
    QList<IncidenceInfo> mSentInfoList;
    QString mResourceId;
    KCalCore::Incidence::Ptr itemToIncidence(const Akonadi::Item &item);
    IncidenceInfo  incidenceToInfo(KCalCore::Incidence::Ptr& incidence);
    bool incidenceIsIden(KCalCore::Incidence::Ptr& incidence1,KCalCore::Incidence::Ptr& incidence2);
    bool incidenceIsIden(IncidenceInfo info1,IncidenceInfo info2);

    int setupResource();
    bool calendarDidChanged();
    void sendCalendar();
    void addIncidence(KCalCore::Incidence::Ptr& incidence);
    void modifyIncidence(KCalCore::Incidence::Ptr& incidence);
    void deleteIncidence(KCalCore::Incidence::Ptr& incidence);
    void deleteIncidence(QString& uid);
    void mergeIncidence(KCalCore::Incidence::Ptr& incidence);
    KCalCore::Incidence::Ptr parseICal(QString& iCal);

public:
    explicit CalendarPlugin(QObject *parent, const QVariantList &args);
    virtual ~CalendarPlugin();

public Q_SLOTS:
    virtual bool receivePackage(const NetworkPackage& np);
    virtual void connected();

private slots:
    void delayedRequest();
    void calendarChanged();
    void collectionFetchResult(KJob*);
    void createFinished(bool success, const QString &errorMessage);
    void deleteFinished(bool success, const QString &errorMessage);
    void modifyFinished(bool success, const QString &errorMessage);
};
#endif