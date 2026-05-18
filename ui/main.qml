import QtQuick
import QtQuick.Controls
import Qt.labs.platform as Native
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Controls.impl
import Clipboard
import "components"

ApplicationWindow {
    id: root
    visible: false
    color: (Qt.platform.os === "android" ) ? palette.window : "transparent"
    width: 500; height: 400;
    maximumWidth: 500 
    maximumHeight: 400
    minimumHeight: 400
    minimumWidth: 500

    flags: Qt.Window | Qt.CustomizeWindowHint | Qt.WindowStaysOnTopHint

    Native.SystemTrayIcon {
    id: trayIcon
        visible: true
        icon.source: "qrc:/ui/components/app.png" 
        tooltip: "Clipboard"

        menu: Native.Menu {
            Native.MenuItem {
                text: "Объектов %1/50".arg(clipList.count)
            }

            Native.MenuSeparator { }
            
            Native.MenuItem {
                text: "Show/hide"
                onTriggered: {
                    root.visible ? root.softClose() : root.softOpen()
                }
            }

            Native.MenuItem {
                text: "Clear"
                onTriggered: {
                    backend.clearHistory()
                    clipList.clear()
                }
            }

            Native.MenuItem {
                text: "Settings"
                onTriggered: {
                    settingsWindow.active = true
                }
            }

            Native.MenuItem {
                text: "Exit"
                onTriggered: Qt.quit()
            }
        }

        onActivated: function(reason) {
            if (reason === Native.SystemTrayIcon.Trigger && Qt.platform.os !== "osx") {
                root.visible ? root.softClose() : root.softOpen()
            }
        }
    }

    onActiveChanged: {
        if (!active) {
            root.softClose()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: (Qt.platform.os === "osx") ? 15 : 10
        anchors.topMargin: (Qt.platform.os === "osx") ? 15 : 5
        spacing: 10
        
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
                        historyList.opacity: 0
                        filterList.visible: true
                    }
                }
            ]

            IconImage {
                Layout.preferredHeight: 30
                Layout.preferredWidth: 30
                Layout.leftMargin: 5
                Layout.rightMargin: -5
                sourceSize.height: 30
                sourceSize.width: 30
                smooth: true
                mipmap: true
                source: "qrc:/ui/components/search.svg"
                color: searchInput.placeholderTextColor
            }

            Item {
                id: searchWrapper
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                TextField {
                    id: searchInput
                    anchors.fill: parent
                    visible: true
                    placeholderText: "Clipboard"
                    color: palette.windowText
                    background: null
                    focus: true
                    font.pixelSize: 20
                    font.weight: Font.DemiBold 

                    onTextChanged: {
                        filteredClipList.clear()
                        if (text.length > 0) {
                            topBar.state = "searchmode"
                            for (let i = 0; i < clipList.count; i++) {
                                let item = clipList.get(i)
                                if (item.name.toLowerCase().indexOf(text.toLowerCase()) !== -1) {
                                    filteredClipList.append(item)
                                }
                            }
                        }
                        else{
                            topBar.state = ""
                        }
                    }
                }
            }
        }

        Rectangle {
            id: separator
            Layout.fillWidth: true 
            Layout.preferredHeight: 1
            Layout.topMargin: 0
            Layout.bottomMargin: 0 
            color: palette.windowText
            opacity: 0.2
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

            ListModel {
                id: filteredClipList
            }

            ListView {
                id: filterList
                anchors.fill: parent
                spacing: 5
                model: filteredClipList
                visible: false

                delegate: MyButton {
                    width: historyList.width  
                    height: 70
                    clip: true
                    btnColor: "transparent"
                    btnRadius: (Qt.platform.os === "windows") ? 4 : 12
                    btnAlignH: Qt.AlignLeft
                    btnText: ""
                    RowLayout {
                        anchors.fill: parent
                        spacing: 10
                        IconImage {
                            Layout.preferredHeight: 40
                            Layout.preferredWidth: 40
                            Layout.leftMargin: 5
                            sourceSize.height: 160
                            source: (model.type === "image") ? model.preview : "qrc:/ui/components/text.svg"
                            color: (model.type === "text") ? palette.windowText : "transparent"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            anchors.margins: 15
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 2  
                            Text {
                                Layout.fillWidth: true
                                text: model.name
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                                color: palette.windowText
                                elide: Text.ElideRight
                            }
                            Text {
                                text: `Type ${model.type} • Copyed ${model.time}`
                                font.pixelSize: 12
                                color: palette.placeholderText
                                font.weight: Font.DemiBold
                            }
                        }
                        Item {
                            Layout.preferredWidth: 40
                        }
                    }
                    onClicked: {
                        let manualMap = { "type": model.type, "data": model.data }
                        backend.copyToClip(manualMap)
                        root.softClose()
                    }
                }
            }

            ListView {
                id: historyList
                anchors.fill: parent
                clip: true
                topMargin: 0
                spacing: 0
                boundsBehavior: (Qt.platform.os === "windows") ? Flickable.StopAtBounds : Flickable.DragAndOvershootBounds
                flickableDirection: Flickable.VerticalFlick
                transformOrigin: Item.Top

                model: ListModel { id: clipList }

                delegate: MyButton {
                    id: delegateLoader
                    width: historyList.width  
                    height: 70
                    clip: true
                    btnColor: "transparent"
                    btnRadius: (Qt.platform.os === "windows") ? 4 : 12
                    btnAlignH: Qt.AlignLeft
                    btnText: ""
                    RowLayout {
                        anchors.fill: parent
                        spacing: 10
                        IconImage {
                            Layout.preferredHeight: 40
                            Layout.preferredWidth: 40
                            Layout.leftMargin: 5
                            sourceSize.height: 40
                            sourceSize.width: 40
                            smooth: true
                            mipmap: true
                            source: (model.type === "image") ? model.preview : "qrc:/ui/components/text.svg"
                            color: (model.type === "text") ? palette.windowText : "transparent"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            anchors.margins: 15
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 2  
                            Text {
                                Layout.fillWidth: true
                                text: model.name
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                                color: palette.windowText
                                elide: Text.ElideRight
                            }
                            Text {
                                text: `Type ${model.type} • Copyed ${model.time}`
                                font.pixelSize: 12
                                color: palette.placeholderText
                                font.weight: Font.DemiBold
                            }
                        }
                        Item {
                            Layout.preferredWidth: 40
                        }
                    }
                    onClicked: {
                        let manualMap = { "type": model.type, "data": model.data }
                        backend.copyToClip(manualMap)
                        root.softClose()
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
                font.pixelSize: 14
                font.weight: Font.DemiBold 
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.centerIn: historyList
                visible: clipList.count === 0 && historyList.visible === true
            }
            Button {
                text: "Update"
                visible: Qt.platform.os === "android"
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                onClicked: {
                    backend.onClipboardChanged()
                }
            }
        }
    }

    Loader {
        id: settingsWindow
        active: false
        source: "Settings.qml"
        onLoaded: {
            windowEffects.applyEffects(item)
            item.visible = true
            item.raise()
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
    Shortcut {
        sequence: "Escape" 
        onActivated: root.softClose()
    }

    signal softClose()
    onSoftClose: closeAnimation.start()
    ParallelAnimation {
        id: closeAnimation
        NumberAnimation { target: root; property: "opacity"; to: 0; duration: 150; easing.type: Easing.OutCubic }
        onFinished: {
            searchInput.clear()
            root.hide()
        }
    }

    signal softOpen()
    onSoftOpen : {
        let cursorPoint = backend.getGlobalMousePos(); 
        let targetX = cursorPoint.x - root.width / 2
        let targetY = cursorPoint.y + 30
        if (targetX + root.width > Screen.desktopAvailableWidth) 
            targetX = Screen.desktopAvailableWidth - root.width - 10
        if (targetX < 0) targetX = 10
        
        if (targetY + root.height > Screen.desktopAvailableHeight)
            targetY = cursorPoint.y - root.height - 15
        root.x = targetX
        root.y = targetY
        
        openAnimation.start()
    }
    ParallelAnimation{
        id: openAnimation
        NumberAnimation {target: root; property: "opacity"; to: 1; duration: 150; easing.type: Easing.InCubic }
        onStarted:{
            if (Qt.platform.os === "windows"){
                root.showMinimized();
                root.showNormal();
            }
            else {
                root.show()
            }
            root.raise()
            root.requestActivate()
        }
    }
}