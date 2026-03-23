import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: settingsWindow
    width: 300
    height: 400
    title: "Settings"
    color: "transparent"

    flags: Qt.Window | Qt.CustomizeWindowHint 

    Rectangle {
        id: bg
        anchors.fill: parent
        visible: Qt.platform.os = "osx"
        color: (palette.windowText.r > 0.5) ? "#1E1E1E" : "#F5F5F5"
        radius: 28
        clip: true
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.1)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12
        
        Item {
            id: titleBar
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            Layout.leftMargin: -10
            Layout.rightMargin: -10
            Layout.topMargin: -10

            RowLayout {
                anchors.fill: parent
                spacing: (Qt.platform.os == "osx") ? 7 : 0

                Item {Layout.preferredWidth: (Qt.platform.os == "osx") ? 5 : 10}

                Loader{
                    id: btnMacClose
                    source: "Button.qml"
                    Layout.preferredHeight: 14
                    Layout.preferredWidth: 14
                    visible: Qt.platform.os == "osx"

                    onLoaded:{
                        item.btnText = Qt.binding(function() { return item.hovered ? "✕" : "" })
                        item.btnFontColor = Qt.rgba(0, 0, 0, 0.8)
                        item.btnRadius = 50
                        item.btnPadding = 0
                        item.btnColor = Qt.binding(function() {
                            return root.active ? Qt.rgba(1, 0.37, 0.34, 1) : (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
                        })
                        item.btnHoveredColor = Qt.rgba(1, 0.37, 0.34, 1)
                        item.btnPressedColor = Qt.rgba(0.9, 0.37, 0.34)
                        item.clicked.connect(() => settingsWindow.softClose())
                    }
                }

                Loader{
                    source: "Button.qml"
                    Layout.preferredHeight: 14
                    Layout.preferredWidth: 14
                    visible: Qt.platform.os == "osx"

                    onLoaded:{
                        item.btnText = ""
                        item.btnRadius = 50
                        item.btnPadding = 0
                        item.btnHoveredColor = (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
                        item.btnPressedColor = (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
                    }
                }

                Loader{
                    source: "Button.qml"
                    Layout.preferredHeight: 14
                    Layout.preferredWidth: 14
                    visible: Qt.platform.os == "osx"

                    onLoaded:{
                        item.btnText = ""
                        item.btnRadius = 50
                        item.btnPadding = 0
                        item.btnHoveredColor = (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
                        item.btnPressedColor = (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
                    }
                }
                Item {
                    Layout.preferredWidth: 3
                    visible: Qt.platform.os === "osx" 
                }

                Label {
                    text: "Settings"
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                    color: palette.windowText 
                    Layout.fillWidth: true
                    elide: Label.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                Item {
                    Layout.preferredWidth: 5
                    visible: Qt.platform.os === "osx" 
                }

                Item {
                    Layout.fillWidth: true
                    visible: Qt.platform.os !== "osx" 
                }

                Loader{
                    source: "Button.qml"
                    Layout.preferredHeight: 35
                    Layout.preferredWidth: 50
                    visible: Qt.platform.os != "osx"
                    onLoaded: {
                        item.btnText = "✕"
                        item.btnRadius = 8
                        item.btnPadding = 0
                        item.btnHoveredColor = Qt.rgba(0.9098, 0.0667, 0.1373)
                        item.btnColor = Qt.rgba(0, 0 , 0, 0)
                        item.clicked.connect(() => settingsWindow.softClose())
                    }
                }
                DragHandler {
                    target: null 
                    onActiveChanged: if (active) settingsWindow.startSystemMove() 
                }
            }  
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Label {text: "Это окно настроек"}
    }

    signal softClose()
    onSoftClose: closeAnimation.start()
    ParallelAnimation {
        id: closeAnimation
        NumberAnimation { target: settingsWindow; property: "opacity"; to: 0; duration: 150; easing.type: Easing.OutCubic }
        onFinished: settingsWindow.close()
    }
}