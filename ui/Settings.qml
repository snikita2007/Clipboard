import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl 

Page {
    id: settingsPage
    background: null
    signal navigate(string page)
    
    function applySearch(query) {}
    function resetMode() {}

    property string currentMode: networkSwitch.currentModeString()
    property var udpDevicesList: []
    property var bluetoothDevicesList: []

    Connections {
        target: networkScanner 
        function onDevicesUpdated(ipList) { settingsPage.udpDevicesList = ipList }
    }

    Connections {
        target: bluetoothClient 
        function onDevicesDiscovered(devicesList) { settingsPage.bluetoothDevicesList = devicesList }
    }

    Connections {
        target: networkSwitch
        function onModeChanged(newModeStr) {
            settingsPage.currentMode = newModeStr
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12
        
        RowLayout {
            visible: Qt.platform.os !== "android"
            Layout.fillWidth: true; spacing: 8
            MouseArea {
                Layout.preferredWidth: 80; Layout.preferredHeight: 35
                cursorShape: Qt.PointingHandCursor
                onClicked: { bottomNav.currentTab = 0; root.updateWindowSize(0) }
                Row { anchors.centerIn: parent; spacing: 6
                    Text { text: "Назад"; font.pixelSize: 13; font.weight: Font.Medium; color: palette.windowText }
                }
            }
        }

        Label { text: "Это окно настроек"; font.bold: true; font.pixelSize: 16 }

        Button {
            id: switchButton
            Layout.fillWidth: true
            text: settingsPage.currentMode === "Wifi" ? "Работает: Wi-Fi" : "Работает: Bluetooth"

            onClicked: {
                settingsPage.currentMode = networkSwitch.toggleMode()
            }
        }

        Label { text: "Найденные IP по Wi-Fi:"; font.pixelSize: 12; color: "gray" }
        ListView {
            Layout.fillWidth: true; Layout.preferredHeight: 120; clip: true
            model: settingsPage.udpDevicesList
            delegate: ItemDelegate { width: parent.width; text: modelData }
        }

        Label { text: "Найденные Bluetooth устройства:"; font.pixelSize: 12; color: "gray" }
        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: settingsPage.bluetoothDevicesList
            delegate: ItemDelegate { width: parent.width; text: modelData.name ? modelData.name : "Неизвестное устройство" }
        }
    }
}

