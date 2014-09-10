/*
    Copyright (C) 2014 Collabora Ltd. <info@collabora.co.uk>

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
#include <KAboutData>
#include <KLocale>
#include <KComponentData>
#include <KDEDModule>


KDEConnectTelepathyCMLoader(QObject *parent, const QVariantList & args)
    : KDEDModule(parent)
{
    Q_UNUSED(args);
    m_protocol = KDEConnectTelepathyProtocolFactory::simpleProtocol();
}


K_PLUGIN_FACTORY_DEFINITION(KDEConnectTelepathyCMLoaderFactory, registerPlugin<KDEConnectTelepathyCMLoader>();)
K_EXPORT_PLUGIN(KDEConnectTelepathyCMLoaderFactory(KDEConnectTelepathyCMLoader::aboutData()))
