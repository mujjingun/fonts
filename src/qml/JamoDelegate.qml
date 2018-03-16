import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import QtQuick.Shapes 1.0
import fontmaker 1.0

ColumnLayout {
    property FormModel formModel: model.formModel

    ColumnLayout {
        id: mainglyph
        spacing: 0

        Rectangle {
            id: jamorect
            width: 100
            height: 100
            color: "white"

            JamoView {
                anchors.fill: parent
                id: jamo
                glyph: formModel.defaultGlyph
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    jamoedit.glyph = jamo.glyph;
                }
            }
        }

        Rectangle {
            id: button
            width: 100
            height: charnametext.height
            color: mousearea.pressed? "grey" :
                   button.toggled? "darkgrey" : "lightgrey"

            property bool toggled: false

            Text {
                id: charnametext
                text: model.name
                leftPadding: 5
            }

            Shape {
                width: 10
                height: 5
                anchors.centerIn: parent
                visible: !button.toggled
                data: ShapePath {
                    strokeWidth: 0
                    fillColor: "black"
                    startX: 0; startY: 0
                    PathLine { x: 10; y: 0 }
                    PathLine { x: 5; y: 5 }
                }
            }

            Shape {
                width: 10
                height: 5
                anchors.centerIn: parent
                visible: button.toggled
                data: ShapePath {
                    strokeWidth: 0
                    fillColor: "black"
                    startX: 0; startY: 5
                    PathLine { x: 10; y: 5 }
                    PathLine { x: 5; y: 0 }
                }
            }

            MouseArea {
                id: mousearea
                anchors.fill: parent
                onClicked: {
                    button.toggled = !button.toggled
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            border.width: 2
            border.color: "#BBB"
            color: "transparent"
        }

    }

    ColumnLayout {
        id: forms
        spacing: 2
        visible: button.toggled

        Repeater {
            id: treeview

            model: formModel

            delegate: Item {
                width: 100
                height: 100

                JamoView {
                    anchors.fill: parent
                    glyph: model.glyph
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        jamoedit.glyph = jamo.glyph;
                    }
                }
            }
        }
    }
}
