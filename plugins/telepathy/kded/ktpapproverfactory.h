/*
    Copyright (C) 2014 David Edmundson <davidedmundson@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KDECONNECTTELEAPTHYLOADER_H
#define KDECONNECTTELEAPTHYLOADER_H

#include <KPluginFactory>
#include <KDEDModule>

K_PLUGIN_FACTORY_DECLARATION(KDEConnectTelepathyCM)

class KDEConnectTelepathyCMLoader:
    KDEDModule(parent)
{
public:
    KDEConnectTelepathyCMLoader(QObject *parent, const QVariantList & args);
private:
    SimpleProtocolPtr m_protocol;
};

#endif //KDECONNECTTELEAPTHYLOADER_H
