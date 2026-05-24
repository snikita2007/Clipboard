import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

Page {
    id: settingsPage
    background: null
    
    function applySearch(query) {
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12
        
        TitleBar {
            id: titleBar
            showSettingsButton: false
            curentTitle: "Settings"
        }

        Label {text: "Это окно настроек"}

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}