import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: false
    color: "transparent"
    width: 350; height: 370;
    maximumWidth: 350 
    maximumHeight: 370 
    minimumHeight: 370 
    minimumWidth: 350

    flags: Qt.Window | Qt.CustomizeWindowHint | Qt.WindowSystemMenuHint | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    background: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Clipboard history"
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: palette.windowText 
                Layout.fillWidth: true
                elide: Label.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Loader{
                source: "Button.qml"
                Layout.preferredHeight: 30 
                Layout.preferredWidth: 70

                onLoaded:{
                    item.btnText = "Clear All"
                    item.padding = 12
                    item.clicked.connect(function() {console.log("Clear All")})
                }
            }

            Loader{
                source: "Button.qml"
                Layout.preferredHeight: 30 
                Layout.preferredWidth: 70

                onLoaded:{
                    item.btnText = "Settings"
                    item.padding = 12
                    item.clicked.connect(function() {console.log("Settings Button")})
                }
            }
        }


        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0
            topPadding: 0
            bottomPadding: 0
            leftPadding: 0
            rightPadding: 0
            background: Rectangle {
                color: Qt.rgba(1, 1, 1, 0)
                radius: 12
            }

            ListView {
                id: historyList
                anchors.fill: parent
                clip: true
                spacing: 5
                boundsBehavior: Flickable.StopAtBounds

                model: ListModel {
                    ListElement { title: "First copied item" }
                    ListElement { title: "Second copied item (a bit longer example)" }
                    ListElement { title: "Third copied item (a much longer text example for test button view)"}
                    ListElement { title: "Fourth copied item (as longest title as i can try to fit to the frame of text. It can be longer but it longest that can be displayed)" }
                    ListElement { title: "Fifth copied item" }
                    ListElement { title: "Sixth copied item" }
                    ListElement { title: "Seventh copied item" }
                    ListElement { title: "Another copied item" }
                }

                delegate: Loader{
                    source: "Button.qml"
                    width: historyList.width  
                    height: 70

                    onLoaded:{
                        item.btnText = model.title
                        item.btnAlignH = Text.AlignLeft
                        item.btnLineCount = 2
                        item.clicked.connect(() => {
                            console.log("Нажат элемент: " + model.title)
                        })
                    }
                }
            }
        }
    }
}