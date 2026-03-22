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
                spacing: 0

                Item {
                    Layout.preferredWidth: 10
                    visible: Qt.platform.os !== "osx" 
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
                    Layout.fillWidth: true
                    visible: Qt.platform.os !== "osx" 
                }

                Loader{
                    source: "Button.qml"
                    Layout.preferredHeight: 35
                    Layout.preferredWidth: 50
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
                Item {
                    Layout.fillWidth: true
                    visible: Qt.platform.os === "osx"
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