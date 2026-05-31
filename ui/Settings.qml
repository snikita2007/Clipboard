import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl 

Page {
    id: settingsPage
    background: null
    signal navigate(string page)
    
    function applySearch(query) {
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12
        
        RowLayout {
            visible: Qt.platform.os !== "android"
            Layout.fillWidth: true
            spacing: 8

            MouseArea {
                Layout.preferredWidth: 80
                Layout.preferredHeight: 35
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    bottomNav.currentTab = 0
                    root.updateWindowSize(0)
                }

                Row {
                    anchors.centerIn: parent
                    spacing: 6
                    
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Назад"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: palette.windowText
                    }
                }
            }
        }

        Label {text: "Это окно настроек"}

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}