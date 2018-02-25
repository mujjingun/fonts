import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
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

        Button {
            width: parent.width
            height: charnametext.height

            Text {
                id: charnametext
                text: model.name
                leftPadding: 5
            }
        }


        Rectangle {
            id: overlayrect
            anchors.fill: parent
            border.width: 2
            border.color: "#BBB"
            color: "transparent"
        }

    }

    ColumnLayout {
        id: forms
        spacing: 2

        Repeater {
            id: treeview

            model: formModel

            delegate: ColumnLayout {

                JamoView {
                    width: 100
                    height: 100
                    glyph: model.glyph
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        //jamoedit.glyph = jamo.glyph;
                    }
                }
            }
        }
    }
}
