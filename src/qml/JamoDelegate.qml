import QtQuick 2.7
import QtQuick.Shapes 1.0

Rectangle {
    width: 100
    height: 100

    Shape {
        id: shape
        anchors.fill: parent
        Component.onCompleted: {
            var scale = 100;
            for (var j = 0; j < paths.length; ++i) {
                var path = paths[j];
                var pstr = "";
                for (var i = 0; i < path.length; ++i) {
                    var seg = path[i];
                    pstr += seg["name"] + '{' +
                            'x: ' + seg["x"] * scale + ';' +
                            'y: ' + seg["y"] * scale + '}';
                }
                var code =
                    'import QtQuick 2.7;' +
                    'import QtQuick.Shapes 1.0;' +
                    'ShapePath {' +
                        'strokeWidth: 2;' +
                        'strokeColor: "red";' +
                        'strokeStyle: ShapePath.SolidLine;' +
                        'startX: ' + start.x * scale + '; startY: ' + start.y * scale + ';' +
                        pstr +
                    '}';
            }
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
