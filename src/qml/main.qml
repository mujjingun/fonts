/****************************************************************************
**
** Copyright (C) 2018 Gun Park.
** Author: Gun Park
** Contact: mujjingun@gmail.com
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("Hangul Font Maker v0.0.1")

    SystemPalette { id: activePalette }

    menuBar: MenuBar {
        id: menu
        Menu {
            title: qsTr("&File")
            Action {
                text: qsTr("&New...")
                onTriggered: window.qmlSignal("hello world!")
            }
            Action {
                text: qsTr("&Open...")
            }
            Action {
                text: qsTr("&Save")
            }
            Action {
                text: qsTr("Save &As...")
            }
            MenuSeparator { }
            Action {
                text: qsTr("&Quit")
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: qsTr("&Edit")
            Action { text: qsTr("Cu&t") }
            Action { text: qsTr("&Copy") }
            Action { text: qsTr("&Paste") }
        }
        Menu {
            title: qsTr("&Help")
            Action { text: qsTr("&About") }
        }
    }

    Flickable {
        id: flicker
        anchors {
            top: menu.bottom;
            bottom: statusbar.top;
            left: parent.left;
            right: parent.right;
            margins: 4
        }
        contentWidth: width
        contentHeight: contentItem.childrenRect.height
        ScrollBar.vertical: ScrollBar {}
        boundsBehavior: Flickable.StopAtBounds
        focus: true
        clip: true

        Text {
            id: consonants_title
            text: "Consonants"
            anchors {
                top: parent.top;
                left: parent.left;
                margins: 4
            }
        }

        Rectangle {
            id: consonants_view
            height: flow.height
            anchors {
                top: consonants_title.bottom;
                left: parent.left; right: parent.right
                margins: 4
            }
            Flow {
                id: flow
                anchors.centerIn: parent
                width: parent.width
                spacing: 10

                Repeater {
                    id: jamorepeat
                    model: jamoModel

                    delegate: Rectangle {
                        width: 100
                        height: 100

                        Shape {
                            anchors.fill: parent
                            Component.onCompleted: {
                                var pstr = "";
                                for (var i = 0; i < path.length; ++i) {
                                    var seg = path[i];
                                    pstr += seg["name"] + '{' +
                                            'x: ' + seg["x"] + ';' +
                                            'y: ' + seg["y"] + '}';
                                }
                                var code =
                                    'import QtQuick 2.7;' +
                                    'import QtQuick.Shapes 1.0;' +
                                    'ShapePath {' +
                                        'strokeWidth: 2;' +
                                        'strokeColor: "red";' +
                                        'strokeStyle: ShapePath.SolidLine;' +
                                        'startX: ' + start.x + '; startY: ' + start.y + ';' +
                                        pstr +
                                    '}';
                                //console.log(code);
                                var pl = Qt.createQmlObject(
                                    code,
                                    window,
                                    'dynamicsnippet1'
                                );
                                this.data.push(pl)
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            border.color: "black"
                            border.width: 1
                            color: "transparent"
                        }

                        Text {
                            text: name
                        }
                    }
                } // Repeater
            } // Flow
        } // Rectangle

        Text {
            id: vowels_title
            text: "Vowels"
            anchors {
                top: consonants_view.bottom
                left: parent.left
                margins: 4
            }
        }

        ListView {
            id: vowels_view
            anchors {
                top: vowels_title.bottom;
                left: parent.left; right: parent.right
                margins: 4
            }
            spacing: 10
        }
    }

    footer: Rectangle {
        id: statusbar
        width: parent.width; height: 32
        color: activePalette.window
    }
}
