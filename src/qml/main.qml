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
import QtQuick.Layouts 1.0
import hangul.jamoview 1.0

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

    Text {
        id: consonants_title
        text: "Consonants"
        anchors {
            top: menu.bottom;
            left: parent.left;
            margins: 4
        }
    }

    function myQmlFunction(msg) {
        jamoModel.append({"name": msg, "number": "test"});
    }

    ListModel {
        id: jamoModel
        ListElement {
            name: "ㄱ"
        }
        ListElement {
            name: "ㄴ"
        }
        ListElement {
            name: "ㄷ"
        }
    }

    Rectangle {
        id: consonants_view
        height: 110
        anchors {
            top: consonants_title.bottom;
            left: parent.left; right: parent.right
            margins: 4
        }
        MouseArea {
            anchors.fill: parent
            onWheel: {
                listview.flick(wheel.angleDelta.y * 7, 0);
            }
        }
        ListView {
            id: listview
            anchors.fill: parent
            orientation: Qt.Horizontal
            ScrollBar.horizontal: ScrollBar {}
            spacing: 10
            model: jamoModel
            delegate: Rectangle {
                width: 100
                height: 100
                JamoView {
                    anchors.fill: parent
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
            boundsBehavior: Flickable.StopAtBounds
            highlight: Rectangle { color: "lightsteelblue"; }
            focus: true
        }
    }

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

    footer: Rectangle {
        id: statusbar
        width: parent.width; height: 32
        color: activePalette.window
    }
}
