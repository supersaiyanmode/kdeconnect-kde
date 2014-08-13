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

#ifndef REMINDERPLUGIN_H
#define REMINDERPLUGIN_H

#include <QObject>

#include <core/kdeconnectplugin.h>
#include <Akonadi/Calendar/ETMCalendar>
#include <KCalCore/ICalFormat>
 
#define PACKAGE_TYPE_REMINDER QLatin1String("kdeconnect.reminder")

class KDE_EXPORT ReminderPlugin
    : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.reminder")
private:
    bool isProcessing;
    bool shouldSend;
	Akonadi::ETMCalendar mCalendar;
    Akonadi::Collection mCollection;
    QString mResourceId;
    KCalCore::ICalFormat conventor;
    QMap<QString,KCalCore::Todo::Ptr> mSentTodo;
    KCalCore::Todo::Ptr itemToTodo(const Akonadi::Item &item);

    int setupResource();
    void sendCalendar();
    KCalCore::Incidence::Ptr parseICal(QString& iCal);

public:
    explicit ReminderPlugin(QObject *parent, const QVariantList &args);
    virtual ~ReminderPlugin();

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