import QtQuick 2.7
import QtQuick.Shapes 1.0
import fontmaker 1.0

Rectangle {
    width: 100
    height: 100

    JamoView {
        anchors.fill: parent
        name: model.name
        glyph: model.glyph
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
