/*
 * Copyright (C) 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
// #include <KLocalizedContext>
#include <KAboutData>
#include <KLocalizedString>

#include <interfaces/dbusinterfaces.h>

int main(int argc, char *argv[])
{
    CastDisplayDbusInterface iface;

    QGuiApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("kdeconnect.livingroom"), i18n("Awesome Living Room"), QStringLiteral("1.0"), i18n("KDE Connect Living Room"), KAboutLicense::GPL, i18n("(c) 2015, Aleix Pol Gonzalez"));
    aboutData.addAuthor(QStringLiteral("Aleix Pol i Gonzalez"), i18n("Maintainer"), QStringLiteral("aleixpol@kde.org"));
    KAboutData::setApplicationData(aboutData);

    {
        QCommandLineParser parser;
        aboutData.setupCommandLine(&parser);
        parser.addVersionOption();
        parser.addHelpOption();
        parser.process(app);
        aboutData.processCommandLine(&parser);
    }
 
    QQmlApplicationEngine engine;

//     engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}

#include "main.moc"
