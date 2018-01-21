import QtQuick 2.7
import QtQuick.Shapes 1.0
import QtGraphicalEffects 1.0
import fontmaker 1.0

Rectangle {
    width: 100
    height: 100

    DropShadow {
        anchors.fill: jamo
        horizontalOffset: 0
        verticalOffset: 0
        radius: 8
        samples: 17
        color: "#40000000"
        source: jamo
    }

    JamoView {
        id: jamo
        anchors.fill: parent
        name: model.name
        glyph: model.glyph
    }

    Text {
        text: name
    }

}
