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
import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.1
import fontmaker 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 1024
    height: 600
    color: activePalette.window
    title: qsTr("Hangul Font Maker v0.0.1")

    SystemPalette { id: activePalette }

    signal fileImportSignal(string filename)
    signal fileLoadFinished()

    onFileLoadFinished: {
        busy.loading = false;
    }

    FileDialog {
        id: fileImportDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home
        selectExisting: true
        onAccepted: {
            window.fileImportSignal(fileUrl);
            busy.loading = true;
        }
    }

    signal fileExportSignal(string filename)
    signal fileExportFinished()

    onFileExportFinished: {
        busy.loading = false;
    }

    FileDialog {
        id: fileExportDialog
        title: qsTr("Please choose a location")
        folder: shortcuts.home
        selectExisting: false

        onAccepted: {
            window.fileExportSignal(fileUrl);
            busy.loading = true;
        }
    }

    signal alert(int type, string title, string text)

    onAlert: {
        messageDialog.icon = type;
        messageDialog.title = title;
        messageDialog.text = text;
        messageDialog.open();
    }

    MessageDialog {
        id: messageDialog
    }

    Rectangle {
        id: busy
        property bool loading: false
        anchors.fill: parent
        color: "#88ffffff"
        z: 10000
        visible: loading
        BusyIndicator {
            anchors.centerIn: parent
            running: parent.loading
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
                onTriggered: fileImportDialog.open()
            }
            Action {
                text: qsTr("&Export to OTF...")
                onTriggered: fileExportDialog.open()
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

    SwipeView {
        id: swipeview
        currentIndex: tabbar.currentIndex
        interactive: false

        anchors {
            top: menu.bottom;
            bottom: tabbar.top;
            left: parent.left;
            right: parent.right;
        }

        Flickable {
            id: flicker
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
                    margins: 10
                }
            }

            Item {
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
                    spacing: 5
                    padding: 5

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
                    margins: 10
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

        Item {

            DropShadow {
                anchors.fill: jamoedit
                horizontalOffset: 0
                verticalOffset: 0
                radius: 8
                samples: 17
                color: "#40000000"
                source: jamoedit
            }

            JamoView {
                id: jamoedit
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: height
                anchors.margins: 8
                editable: true
            }

            MouseArea {
                anchors.fill: jamoedit
                hoverEnabled: true
                onPressed: jamoedit.pressed(mouse.x, mouse.y)
                onPositionChanged: jamoedit.moved(mouse.x, mouse.y)
                onReleased: jamoedit.unpressed(mouse.x, mouse.y)
            }
        }

        Item {
        }
    }

    footer: TabBar {
        id: tabbar
        currentIndex: swipeview.currentIndex
        width: parent.width; height: 32
        TabButton {
            text: qsTr("Glyphs")
        }
        TabButton {
            text: qsTr("Design")
        }
        TabButton {
            text: qsTr("Preview")
        }
    }
}
