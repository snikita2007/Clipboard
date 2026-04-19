import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: control 
    property var settingsWindow: null
    property bool showSettingsButton: false
    property string curentTitle: ""

    Layout.fillWidth: true
    Layout.preferredHeight: 35
    Layout.leftMargin: -10
    Layout.rightMargin: -10
    Layout.topMargin: (Qt.platform.os === "osx") ? -8 : -12

    RowLayout {
        anchors.fill: parent
        spacing: (Qt.platform.os == "osx") ? 7 : 0

        Item {Layout.preferredWidth: (Qt.platform.os == "osx") ? 5 : 10}

        MyButton{
            id: btnMacClose
            Layout.preferredHeight: 14
            Layout.preferredWidth: 14
            visible: Qt.platform.os == "osx"
            btnText: hovered ? "✕" : ""
            btnFontColor: Qt.rgba(0, 0, 0, 0.8)
            btnRadius: 50
            btnPadding: 0
            btnColor: control.Window.window.active ? Qt.rgba(1, 0.37, 0.34, 1) : (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.05) : Qt.rgba(0, 0, 0, 0.05))
            btnHoveredColor: Qt.rgba(1, 0.37, 0.34, 1)
            btnPressedColor: Qt.rgba(0.9, 0.37, 0.34)
            onClicked: control.Window.window.softClose()
        }

        MyButton{
            Layout.preferredHeight: 14
            Layout.preferredWidth: 14
            visible: Qt.platform.os == "osx"
            btnText: ""
            btnRadius: 50
            btnPadding: 0
            btnHoveredColor: (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
            btnPressedColor: (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
        }

        MyButton{
            Layout.preferredHeight: 14
            Layout.preferredWidth: 14
            visible: Qt.platform.os == "osx"
            btnText: ""
            btnRadius: 50
            btnPadding: 0
            btnHoveredColor: (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
            btnPressedColor: (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
        }

        Item {
            Layout.preferredWidth: 3
            visible: Qt.platform.os === "osx" 
        }

        Label {
            text: control.curentTitle
            font.pixelSize: 18
            font.weight: Font.DemiBold
            color: palette.windowText 
            Layout.fillWidth: true
            elide: Label.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        MyButton{
            id: btnSettings
            Layout.fillHeight: (Qt.platform.os !== "osx")
            Layout.preferredHeight: (Qt.platform.os === "osx") ? 30 : -1
            Layout.preferredWidth: (Qt.platform.os === "osx") ? 45 : 50 
            visible: control.showSettingsButton 

            btnText: "⛭" 
            btnRadius: (Qt.platform.os === "osx") ? 30 : 8 
            btnFontSize: 14
            padding: 0
            btnColor: ((Qt.platform.os === "osx") ? (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.05)  : Qt.rgba(0, 0, 0, 0.05)) : Qt.rgba(0, 0 , 0, 0) )
            onClicked:{
                console.log("Settings Button");
                settingsWindow.active = true;
            }
        }

        Item {
            Layout.preferredWidth: 5
            visible: Qt.platform.os === "osx" 
        }

        Item {
            Layout.fillWidth: true
            visible: Qt.platform.os !== "osx" 
        }

        MyButton{
            Layout.preferredHeight: 35
            Layout.preferredWidth: 50
            visible: Qt.platform.os != "osx"
            btnText: "✕"
            btnRadius: 8
            btnPadding: 0
            btnHoveredColor: Qt.rgba(0.9098, 0.0667, 0.1373)
            btnColor: Qt.rgba(0, 0 , 0, 0)
            onClicked: control.Window.window.softClose()
        }

        DragHandler {
            target: null 
            onActiveChanged:  if (active && control.Window.window) control.Window.window.startSystemMove() 
        }
    }  
}