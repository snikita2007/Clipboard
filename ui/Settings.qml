import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

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
        
        TitleBar {
            id: titleBar
            showSettingsButton: false
            curentTitle: "Settings"
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