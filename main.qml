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
                        item.clicked.connect(() => root.softClose())
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
                    text: "Clipboard"
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                    color: palette.windowText 
                    Layout.fillWidth: true
                    elide: Label.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                Loader{
                    id: btnSettings
                    source: "Button.qml"
                    Layout.fillHeight: (Qt.platform.os !== "osx")
                    Layout.preferredHeight: (Qt.platform.os === "osx") ? 30 : -1
                    Layout.preferredWidth: (Qt.platform.os === "osx") ? 45 : 50 

                    onLoaded:{
                        item.btnText = "⛭" 
                        item.btnRadius = (Qt.platform.os === "osx") ? 30 : 8 
                        item.btnFontSize = 18
                        item.padding = 0
                        item.btnColor = ((Qt.platform.os === "osx") ? (palette.windowText.r > 0.5 ? Qt.rgba(1, 1, 1, 0.05)  : Qt.rgba(0, 0, 0, 0.05)) : Qt.rgba(0, 0 , 0, 0) )
                        item.clicked.connect(function() {
                            console.log("Settings Button");
                            settingsWindow.active = true;
                        })
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
                        item.clicked.connect(() => root.softClose())
                    }
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
            state: ""

            states: [
                State {
                    name: "searchmode"
                    PropertyChanges { 
                        target: btnClear
                        opacity: 0
                        visible: false
                        Layout.preferredWidth: 0
                        Layout.fillWidth: false 
                    }
                    PropertyChanges { 
                        target: btnSearch
                        Layout.fillWidth: true 
                    }
                    PropertyChanges {
                        target: btnSearch.item
                        btnText: ""
                    }
                    PropertyChanges { 
                        target: searchInput
                        opacity: 1
                        visible: true
                    }
                    PropertyChanges {
                        target: historyList
                        opacity: 0
                        visible: false
                    }
                }
            ]

            transitions: Transition {
                NumberAnimation { properties: "opacity,Layout.preferredWidth"; duration: 200; easing.type: Easing.OutCubic }
            }

            Loader {
                id: btnSearch
                source: "Button.qml"
                Layout.preferredHeight: 30 
                Layout.fillWidth: true

                onLoaded: {
                    item.btnText = topBar.state === "searchmode" ? "" : "Search"
                    item.clicked.connect(() => {
                        if (topBar.state !== "searchmode") {
                            topBar.state = "searchmode"
                            searchInput.forceActiveFocus()
                        }
                    })
                }

                TextField {
                    id: searchInput
                    anchors.fill: parent 
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    z: 2
                    visible: false
                    opacity: 0
                    placeholderText: "Type to search..."
                    color: palette.windowText
                    background: Item{}

                    onTextChanged: {
                        console.log("Searched for: " + text)
                    }
                    
                    Text {
                        text: "✕"
                        opacity: 0.5
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        visible: topBar.state === "searchmode"
                        color: palette.windowText
                        font.pixelSize: 16
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                topBar.state = ""
                                searchInput.text = ""
                            }
                        }
                    }
                }
            }

            Loader {
                id: btnClear
                source: "Button.qml"
                Layout.preferredHeight: 30
                Layout.preferredWidth: 70
                onLoaded: item.btnText = "Clear All"
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
                        item.btnRadius = ((Qt.platform.os === "osx") ? 12 : 8 )
                        item.btnLineCount = 2
                        item.clicked.connect(() => {
                            console.log("Нажат элемент: " + model.title)
                        })
                    }
                }
            }
        }
    }
    Loader {
        id: settingsWindow
        active: false
        source: "settings.qml"
        onLoaded: {
            windowEffects.applyEffects(item)
            item.show()
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