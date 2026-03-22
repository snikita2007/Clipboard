import QtQuick
import QtQuick.Controls

Button {
    id: control
    
    property string btnText: "Button"
    property int btnAlignH: Text.AlignHCenter
    property int btnAlignV: Text.AlignVCenter
    property int btnPadding: 12
    property int btnHeight: 100
    property int btnWidth: 100
    property int btnLineCount: 1
    property color btnColor: (Qt.platform.os === "windows") 
        ? (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(1, 1, 1, 0.2)) // Windows
        : (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1)  : Qt.rgba(0, 0, 0, 0.05)) // (Mac/Linux)
    property color btnPressedColor: Qt.rgba(1, 1, 1, 0.08)
    property color btnHoveredColor: Qt.rgba(1, 1, 1, 0.08)
    property real btnRadius: 0

    width: control.btnWidth
    height: control.btnHeight

    contentItem: Text {
        text: control.btnText
        color: palette.windowText
        padding: control.btnPadding
        font.weight: Font.Medium
        horizontalAlignment: control.btnAlignH
        verticalAlignment: control.btnAlignV
        wrapMode: Text.WordWrap 
        maximumLineCount: control.btnLineCount
    }

    background: Rectangle {
        anchors.fill: parent 
        radius: {
            if (control.btnRadius > 0) return control.btnRadius;
            
            return (Qt.platform.os === "windows") ? 4 :
                (Qt.platform.os === "osx")     ? 30 :
                (Qt.platform.os === "android") ? 20 :
                (Qt.platform.os === "ios")     ? 10 : 8;
        }
        color: control.pressed ? control.btnPressedColor : 
           control.hovered ? control.btnHoveredColor : control.btnColor
    }
}
