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
        : (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.05)  : Qt.rgba(0, 0, 0, 0.05)) // (Mac/Linux)
    property color btnPressedColor: Qt.rgba(1, 1, 1, 0.08)
    property color btnHoveredColor: Qt.rgba(1, 1, 1, 0.08)
    property real btnRadius: 0
    property int btnFontSize: 13 
    property color btnFontColor: palette.windowText

    width: control.btnWidth
    height: control.btnHeight

    contentItem: Text {
        text: control.btnText
        anchors.centerIn: parent
        lineHeight: 0.8 
        anchors.verticalCenterOffset: (control.height < 15) ? -0.75 : 0
        anchors.horizontalCenterOffset: (control.width < 15) ? 0.25 : 0
        font.pixelSize: control.btnFontSize
        color: control.btnFontColor
        padding: control.btnPadding
        font.weight: Font.Medium
        horizontalAlignment: control.btnAlignH
        verticalAlignment: control.btnAlignV
        wrapMode: Text.WordWrap 
        maximumLineCount: control.btnLineCount
        elide: Text.ElideNone

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
