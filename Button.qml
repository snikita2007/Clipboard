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
            if (Qt.platform.os === "windows") return 4    
            if (Qt.platform.os === "osx")     return 12     
            if (Qt.platform.os === "android") return 20     
            if (Qt.platform.os === "ios")     return 10 
            return 8                                    
        }

        color: {
            if (control.pressed) return Qt.rgba(1, 1, 1, 0.2)
            if (Qt.platform.os === "android" || Qt.platform.os === "ios") 
                return palette.button
            return Qt.rgba(1, 1, 1, 0.08) 
        }
    }
}
