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
import QtQuick.Dialogs 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 1024
    height: 600
    color: activePalette.window
    title: qsTr("Hangul Font Maker v0.0.1")

    SystemPalette { id: activePalette }

    signal fileImportSignal(var filename)
    signal fileLoaded()

    onFileLoaded: {
        busy.visible = false;
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home
        onAccepted: {
            window.fileImportSignal(fileDialog.fileUrl);
            busy.visible = true;
        }
    }

    Rectangle {
        id: busy
        anchors.fill: parent
        z: 10000
        visible: false
        color: "transparent"
        BusyIndicator {
            anchors.centerIn: parent
            running: true
            z: 10001
        }
    }

    menuBar: MenuBar {
        id: menu
        Menu {
            title: qsTr("&File")
            Action {
                text: qsTr("&New...")
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
                text: qsTr("&Import from OTF...")
                onTriggered: fileDialog.open()
            }
            Action {
                text: qsTr("&Export to OTF...")
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
            color: "transparent"
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
                    model: consonantJamoModel

                    delegate: JamoDelegate {}
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
