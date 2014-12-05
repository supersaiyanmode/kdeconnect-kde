/**
 * Copyright 2015 Albert Vaca <albertvaka@gmail.com>
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

#include "kdeconnectpluginconfig.h"

#include <QDir>
#include <QSettings>
#include <QJsonDocument>

#include "kdeconnectconfig.h"

struct KdeConnectPluginConfigPrivate
{
    QDir mConfigDir;
    QSettings* mConfig = 0;
};

KdeConnectPluginConfig::KdeConnectPluginConfig(const QString& deviceId, const QString& pluginName)
    : d(new KdeConnectPluginConfigPrivate())
{
    d->mConfigDir = KdeConnectConfig::instance()->pluginConfigDir(deviceId, pluginName);
    QDir().mkpath(d->mConfigDir.path());
}

KdeConnectPluginConfig::~KdeConnectPluginConfig()
{
    delete d->mConfig;
}

QDir KdeConnectPluginConfig::privateDirectory()
{
    return d->mConfigDir;
}

QVariant KdeConnectPluginConfig::get(const QString& key, const QVariant& defaultValue)
{
    lazyLoad();
    d->mConfig->sync();
    return d->mConfig->value(key, defaultValue);
}

void KdeConnectPluginConfig::set(const QString& key, const QVariant& value)
{
    lazyLoad();
    d->mConfig->setValue(key, value);
    d->mConfig->sync();
}

void KdeConnectPluginConfig::lazyLoad()
{
    if (!d->mConfig) {
        QString path = d->mConfigDir.absoluteFilePath("config");
        d->mConfig = new QSettings(path, QSettings::IniFormat);
    }
}

QJsonObject KdeConnectPluginConfig::jsonConfig()
{
    QString path = d->mConfigDir.absoluteFilePath("jsonConfig");
    QFile jsonFile(path);
    jsonFile.open(QFile::ReadOnly);
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(jsonFile.readAll(), &error);
    //if (error != QJsonParseError::NoError) {
        return document.object();
    //} else {
    //    return QJsonObject();
    //}
}

void KdeConnectPluginConfig::storeJsonConfig(QJsonObject json)
{
    QString path = d->mConfigDir.absoluteFilePath("jsonConfig");
    QFile jsonFile(path);
    jsonFile.open(QFile::WriteOnly);
    QJsonDocument toStore;
    toStore.setObject(json);
    jsonFile.write(toStore.toJson());
}
