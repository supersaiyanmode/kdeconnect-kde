/*
 * Copyright 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
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

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import org.kde.kdeconnect 1.0

ApplicationWindow
{
    id: root
    visible: true
    width: 400
    height: 500

    DevicesModel {
        id: connectDeviceModel
        displayFilter: DevicesModel.NoFilter
    }

    Instantiator {
        model: connectDeviceModel
        delegate: QtObject {
            Component.onCompleted: {
                console.log("got a device:", device.name, ".", device.id(), ".")

                device.requestPair()
                device.setPluginEnabled("kdeconnect_castdisplay", true)
            }

            property var fu: Connections {
                target: device
                onNameChanged: console.log("name changed:", device.name, device.id());
            }
        }
    }

    CastDisplayDbusInterface {
        onStartProgram: {
            console.log("should start program", programPath, JSON.stringify(data))

            view.clear();
            view.push(programPath, data);
        }
    }

    Component {
        id: unknownViewer
        Item {
            Text {
                property string source
                anchors.centerIn: parent
//                 text: i18n ("Received file: %1", source)
                text: "Received file: " + source
            }
        }
    }

    StackView {
        id: view
        anchors.fill: parent
        initialItem: Image {
            source: "http://loremflickr.com/" + root.Screen.width + "/" + root.Screen.height + "/barcelona"
            fillMode: Image.PreserveAspectCrop

            Text {
//                 text: i18n("%1 devices", connectDeviceModel.count)
                text: connectDeviceModel.count + " devices"
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                    margins: 20
                }
                color: "white"
                styleColor: "black"
                style: Text.Raised
                font.pointSize: Math.max(parent.height/10, 10)
            }
        }
    }
}
