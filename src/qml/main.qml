import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.0
import hangul.backend 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hangul Font Maker v0.0.1")

    SystemPalette { id: activePalette }

    BackEnd {
        id: backend
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

    Flow {
        objectName: "consonants_view"
        id: consonants_view
        anchors {
            top: consonants_title.bottom;
            left: parent.left; right: parent.right
            margins: 4
        }
        spacing: 10

        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
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

    Flow {
        id: vowels_view
        anchors {
            top: vowels_title.bottom;
            left: parent.left; right: parent.right
            margins: 4
        }
        spacing: 10

        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
        }
        Button {
            text: "Ok"
        }
    }

    footer: Rectangle {
        id: statusbar
        width: parent.width; height: 32
        color: activePalette.window
    }
}
