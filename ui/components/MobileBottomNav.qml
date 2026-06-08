import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root
    
    visible: Qt.platform.os === "android" && settingsPage.mobileDetailOpen === false

    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
    
    width: parent.width
    height: contentHolder.height + bottomSpacer.height

    property bool searchMode: false
    property int currentTab: 0
    
    readonly property color panelColor: palette.window.hslLightness > 0.5 ? "#FFFFFF" : "#1E1E1E"

    signal navigate(string page)
    signal searchTextChanged(string text)

    Timer {
        id: keyboardDelayTimer
        interval: 10
        repeat: false
        onTriggered: {
            mobileSearchField.forceActiveFocus()
            Qt.inputMethod.show()
        }
    }

    Connections {
        target: Qt.inputMethod
        function onVisibleChanged() {
            if (!Qt.inputMethod.visible && root.searchMode) {
                Qt.callLater(function() {
                    if (!Qt.inputMethod.visible) { 
                        mobileSearchField.text = "" 
                        mobileSearchField.focus = false
                        root.searchMode = false
                    }
                })
            }
        }
    }

    states: [
        State {
            name: "searchmode"
            when: root.searchMode
            
            PropertyChanges { target: searchCircle; x: 8; width: root.width - 16; height: 40; radius: 20; y: 20 }
            PropertyChanges { target: searchIcon; x: 16; y: 10 }
            PropertyChanges { target: pillContainer; opacity: 0 }
            PropertyChanges { target: searchModeLayout; opacity: 1 }
            PropertyChanges { target: bottomSpacer; height: 10 }
        }
    ]

    transitions: [
        Transition {
            from: ""; to: "searchmode"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: searchCircle; properties: "x,width"; duration: 140; easing.type: Easing.OutQuad }
                    NumberAnimation { target: searchIcon; property: "x"; duration: 140; easing.type: Easing.OutQuad }
                    NumberAnimation { target: pillContainer; property: "opacity"; duration: 100 }
                }
                ParallelAnimation {
                    NumberAnimation { target: searchCircle; properties: "height,radius,y"; duration: 320; easing.type: Easing.OutBack; easing.amplitude: 1.8 }
                    NumberAnimation { target: searchIcon; property: "y"; duration: 320; easing.type: Easing.OutBack; easing.amplitude: 1.8 }
                    NumberAnimation { target: searchModeLayout; property: "opacity"; duration: 150 }
                    NumberAnimation { target: bottomSpacer; property: "height"; duration: 320; easing.type: Easing.OutQuad }
                }
            }
        },
        Transition {
            from: "searchmode"; to: ""
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: searchCircle; properties: "height,radius,y"; duration: 150; easing.type: Easing.OutCubic }
                    NumberAnimation { target: searchIcon; property: "y"; duration: 150; easing.type: Easing.OutCubic }
                    NumberAnimation { target: searchModeLayout; property: "opacity"; duration: 100 }
                    NumberAnimation { target: bottomSpacer; property: "height"; duration: 150; easing.type: Easing.OutCubic }
                }
                ParallelAnimation {
                    NumberAnimation { target: searchCircle; properties: "x,width"; duration: 200; easing.type: Easing.OutCubic }
                    NumberAnimation { target: searchIcon; property: "x"; duration: 200; easing.type: Easing.OutCubic }
                    NumberAnimation { target: pillContainer; property: "opacity"; duration: 180 }
                }
            }
        }
    ]

    Item {
        id: contentHolder
        width: parent.width
        height: 60
        anchors.bottom: bottomSpacer.top

        Rectangle {
            id: pillContainer
            height: 60
            radius: height / 2
            color: root.panelColor
            
            x: 24
            y: 0
            width: parent.width - 120
            enabled: !root.searchMode

            Rectangle {
                id: lens
                width: (pillContainer.width / 2) - 12
                height: pillContainer.height - 12
                radius: height / 2
                color: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.06)
                anchors.verticalCenter: parent.verticalCenter
                x: root.currentTab === 0 ? 6 : (pillContainer.width / 2) + 6

                Behavior on x {
                    NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                }
            }

            Row {
                anchors.fill: parent

                MouseArea {
                    width: parent.width / 2
                    height: parent.height
                    onClicked: root.navigate("History.qml")

                    Column {
                        anchors.centerIn: parent
                        spacing: 0
                        
                        IconImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "qrc:/ui/assets/history.svg"
                            sourceSize.width: 25
                            sourceSize.height: 25
                            color: root.currentTab === 0 ? palette.windowText : palette.placeholderText
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "History"
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            color: root.currentTab === 0 ? palette.windowText : palette.placeholderText
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                    }
                }

                MouseArea {
                    width: parent.width / 2
                    height: parent.height
                    onClicked: root.navigate("Settings.qml")

                    Column {
                        anchors.centerIn: parent
                        spacing: 0
                        
                        IconImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "qrc:/ui/assets/settings.svg"
                            sourceSize.width: 25
                            sourceSize.height: 25
                            color: root.currentTab === 1 ? palette.windowText : palette.placeholderText
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Settings"
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            color: root.currentTab === 1 ? palette.windowText : palette.placeholderText
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                    }
                }
            }
        }

        MultiEffect {
            source: pillContainer
            anchors.fill: pillContainer
            visible: pillContainer.opacity > 0
            shadowEnabled: true
            shadowColor: Qt.rgba(0, 0, 0, 0.12)
            shadowBlur: 0.3
            z: -1
        }

        Rectangle {
            id: searchCircle
            color: root.panelColor
            
            x: parent.width - 84
            y: 0
            width: 60
            height: 60
            radius: height / 2

            IconImage {
                id: searchIcon
                source: "qrc:/ui/assets/search.svg"
                sourceSize.width: 20
                sourceSize.height: 20
                color: palette.windowText
                x: (searchCircle.width - width) / 2
                y: (searchCircle.height - height) / 2
                
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -10
                    enabled: root.searchMode
                    onClicked: {
                        mobileSearchField.text = ""
                        mobileSearchField.focus = false
                        root.searchMode = false
                        Qt.inputMethod.hide()
                    }
                }
            }

            MouseArea {
                id: searchCircleMouseArea
                anchors.fill: parent
                enabled: !root.searchMode
                onClicked: {
                    root.navigate("History.qml")
                    root.searchMode = true
                    keyboardDelayTimer.start()
                }
                onPressed: searchCircle.scale = 0.95
                onReleased: searchCircle.scale = 1.0
                onCanceled: searchCircle.scale = 1.0
                Behavior on scale { NumberAnimation { duration: 100 } }
            }

            RowLayout {
                id: searchModeLayout
                anchors.left: searchIcon.right
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                spacing: 12
                opacity: 0
                visible: opacity > 0

                TextField {
                    id: mobileSearchField
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    placeholderText: "Поиск..."
                    placeholderTextColor: palette.placeholderText
                    color: palette.windowText
                    font.pixelSize: 15
                    background: null
                    inputMethodHints: Qt.ImhNoPredictiveText

                    onTextChanged: root.searchTextChanged(text)

                    onAccepted: {
                        Qt.inputMethod.hide()
                    }
                }
            }
        }

        MultiEffect {
            source: searchCircle
            anchors.fill: searchCircle
            shadowEnabled: true
            shadowColor: Qt.rgba(0, 0, 0, 0.12)
            shadowBlur: 0.3
            z: -1
        }
    }

    Item {
        id: bottomSpacer
        anchors.bottom: parent.bottom
        width: parent.width
        height: 5
    }
}