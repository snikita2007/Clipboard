import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Clipboard
import "components"

ApplicationWindow {
    id: root
    visible: false
    color: "transparent"
    width: 350; height: 370;
    maximumWidth: 350 
    maximumHeight: 370 
    minimumHeight: 370 
    minimumWidth: 350

    flags: Qt.Window | Qt.CustomizeWindowHint | Qt.WindowStaysOnTopHint 
    background: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10
        
        TitleBar {
            id: titleBar
            settingsWindow: settingsWindow
            showSettingsButton: true
            curentTitle: "Clipboard"
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
                        btnClear.Layout.preferredWidth: 0 
                        btnClear.opacity: 0
                        btnClear.visible: false 
                        btnSearch.btnText: "" 
                        searchInput.visible: true
                        searchInput.opacity: 1
                        historyList.visible: false
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
                    backend.clearHistory();
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

                model: ListModel { id: clipList }

                delegate: MyButton {
                    id: delegateLoader
                    required property var modelData
                    width: historyList.width  
                    height: 70
                    clip: true
                    btnRadius: (Qt.platform.os === "windows") ? 4 : 12
                    btnAlignH: Qt.AlignLeft
                    btnText: modelData.type === "text" ? modelData.preview : ""
                    Image {
                        id: previewImage
                        opacity: 0.1
                        sourceSize.width: delegateLoader.width
                        sourceSize.height: delegateLoader.height 
                        visible: delegateLoader.modelData.type === "image"
                        source: visible ? delegateLoader.modelData.preview : ""
                        asynchronous: true
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        horizontalAlignment: Image.AlignHCenter
                        verticalAlignment: Image.AlignVCenter
                    }
                    onClicked: {
                        let manualMap = { "type": modelData.type, "data": modelData.data }
                        Backend.copyToClip(manualMap)
                    }
                }

                add: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 100 }
                    NumberAnimation { property: "scale"; from: 0.8; to: 1.0; duration: 150; easing.type: Easing.OutBack }
                }

                displaced: Transition {
                    NumberAnimation { 
                        properties: "y"; 
                        duration: 200; 
                        easing.type: Easing.OutExpo 
                    }
                }
                Connections {
                    target: backend
                    function onItemAdded(newItem) {
                        clipList.insert(0, newItem);
                        if (clipList.count > 50) clipList.remove(50);
                    }
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
        source: "Settings.qml"
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