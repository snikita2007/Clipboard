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

    flags: Qt.Window | Qt.CustomizeWindowHint 

    background: Rectangle {
        id: bg
        anchors.fill: parent
        visible: Qt.platform.os === "osx"
        color: (palette.windowText.r > 0.5) ? "#1E1E1E" : "#F5F5F5"
        radius: 28
        clip: true
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.1)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10
        
        Item {
            id: titleBar
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
                    btnColor: root.active ? Qt.rgba(1, 0.37, 0.34, 1) : (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.05))
                    btnHoveredColor: Qt.rgba(1, 0.37, 0.34, 1)
                    btnPressedColor: Qt.rgba(0.9, 0.37, 0.34)
                    onClicked: root.softClose()
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
                    text: "Clipboard"
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
                    onClicked: root.softClose()
                }

                DragHandler {
                    target: null 
                    onActiveChanged: if (active) root.startSystemMove() 
                }
            }  
        }

        RowLayout {
            id: topBar
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            spacing: 5
            state: ""

            states: [
                State {
                    name: "searchmode"
                    PropertyChanges { 
                        target: btnClear
                        Layout.preferredWidth: 0
                        opacity: 0
                        visible: false 
                    }
                    PropertyChanges { 
                        target: btnSearch
                        btnText: "" 
                    }
                    PropertyChanges {
                        target: searchInput
                        visible: true
                        opacity: 1
                    }
                    PropertyChanges {
                        target: historyList
                        visible: false
                    }
                }
            ]

            transitions: Transition {
                NumberAnimation { 
                    properties: "Layout.preferredWidth,opacity"; 
                    duration: 200; 
                    easing.type: Easing.OutCubic  
                }
            }

            Item {
                id: searchWrapper
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                MyButton {
                    id: btnSearch
                    anchors.fill: parent
                    btnText: topBar.state === "searchmode" ? "" : "Search"
                    onClicked: {
                        topBar.state = "searchmode"
                        searchInput.forceActiveFocus()
                    }
                }

                TextField {
                    id: searchInput
                    anchors.fill: parent
                    visible: false
                    opacity: 0
                    placeholderText: "Type to search..."
                    color: palette.windowText
                    leftPadding: 10
                    rightPadding: 30
                    background: Item {} 

                    Text {
                        text: "✕"
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        color: palette.windowText
                        opacity: 0.5
                        visible: topBar.state === "searchmode"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                searchInput.text = ""
                                topBar.state = ""
                            }
                        }
                    }
                }
            }

            MyButton {
                id: btnClear
                Layout.preferredWidth: 70
                Layout.preferredHeight: 30
                btnText: "Clear All"
                clip: true
                
                onClicked: {
                    console.log("Cleared")
                    clipList.clear();
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
                    id: clipList
                    ListElement { title: "First copied item" }
                    ListElement { title: "Second copied item (a bit longer example)" }
                    ListElement { title: "Third copied item (a much longer text example for test button view)"}
                    ListElement { title: "Fourth copied item (as longest title as i can try to fit to the frame of text. It can be longer but it longest that can be displayed)" }
                    ListElement { title: "Fifth copied item" }
                    ListElement { title: "Sixth copied item" }
                    ListElement { title: "Seventh copied item" }
                    ListElement { title: "Another copied item" }
                }

                delegate: MyButton{
                    id: delegateLoader
                    width: historyList.width  
                    height: 70
                    btnText: title || "loading..."
                    btnAlignH: Text.AlignLeft
                    btnRadius: ((Qt.platform.os === "osx") ? 12 : 8 )
                    btnLineCount: 2
                    onClicked: console.log("Нажат элемент: " + title)
                }
            }

            Label {
                text: "Clipboard is empty"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.centerIn: historyList
                font.weight: Font.Light
                visible: clipList.count === 0 && historyList.visible === true
            }
        }
    }

    Loader {
        id: settingsWindow
        active: false
        source: "settings.qml"
        onLoaded: {
            windowEffects.applyEffects(item)
            show()
            console.log("Settings window loaded and shown")
        }
        Connections {
            target: settingsWindow.item
            ignoreUnknownSignals: true
        
            function onClosing() {
                settingsWindow.active = false
            }
        }
    }

    signal softClose()
    onSoftClose: closeAnimation.start()
    ParallelAnimation {
        id: closeAnimation
        NumberAnimation { target: root; property: "opacity"; to: 0; duration: 150; easing.type: Easing.OutCubic }
        onFinished: root.close()
    }
}