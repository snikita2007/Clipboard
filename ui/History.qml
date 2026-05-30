import QtQuick
import QtQuick.Controls
import Qt.labs.platform as Native
import QtQuick.Layouts
import QtQuick.Controls.impl
import "components"

Page{
    id: historyPage
    background: null
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Label {
            visible: Qt.platform.os === "android"
            color: palette.windowText 
            text: "Clipboard"
            Layout.leftMargin: 10
            font.pixelSize: 24
            font.weight: Font.DemiBold
            elide: Label.ElideRight
            verticalAlignment: Text.AlignVCenter 
        }
        
        RowLayout {
            id: topBar
            visible: Qt.platform.os !== "android"
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
                },
                State {
                    name: "menumode"
                    PropertyChanges { 
                        historyList.opacity: 0
                        actionsList.visible: true
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
                source: "qrc:/ui/assets/search.svg"
                color: searchInput.placeholderTextColor
            }

            Item {
                id: searchWrapper
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                TextField {
                    id: searchInput
                    anchors.fill: parent
                    visible: Qt.platform.os !== "android"
                    placeholderText: "Clipboard"
                    color: palette.windowText
                    background: null
                    focus: Qt.platform.os !== "android"
                    font.pixelSize: 20
                    font.weight: Font.DemiBold 
                    padding: 0
                    leftPadding: 10
                    cursorDelegate: Item {
                        width: 2
                        height: searchInput.height 

                        Rectangle {
                            width: 2 
                            height: searchInput.font.pixelSize
                            anchors.verticalCenter: parent.verticalCenter
                            color: palette.windowText 
                            visible: searchInput.cursorVisible
                        }
                    }

                    onActiveFocusChanged: searchInput.forceActiveFocus()

                    function getActiveList() {
                        if (topBar.state === "menumode") {
                            return actionsList;
                        } else if (topBar.state === "searchmode") {
                            return filterList;
                        }
                        return historyList;
                    }

                    Keys.onPressed: (event) => {
                        let currentList = getActiveList();
                        
                        if (event.key === Qt.Key_Down) {
                            if (currentList.currentIndex < currentList.count - 1) {
                                currentList.currentIndex++;
                                currentList.positionViewAtIndex(currentList.currentIndex, ListView.Contain);
                            }
                            event.accepted = true;
                        } 
                        else if (event.key === Qt.Key_Up) {
                            if (currentList.currentIndex > 0) {
                                currentList.currentIndex--;
                                currentList.positionViewAtIndex(currentList.currentIndex, ListView.Contain);
                            }
                            event.accepted = true;
                        } 
                        else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            if (currentList.currentItem && typeof currentList.currentItem.triggerAction === "function") {
                                currentList.currentItem.triggerAction();
                            }
                            event.accepted = true;
                        }
                    }

                    onTextChanged: {
                        historyList.currentIndex = -1
                        filterList.currentIndex = 0;
                        historyPage.applySearch(text);
                    }
                }
            }
            Button {
                id: menuButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Layout.rightMargin: 5
                
                background: Rectangle {
                    color: menuButton.down ? Qt.rgba(1, 1, 1, 0.1) : (menuButton.hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent")
                    radius: 6
                }

                contentItem: IconImage {
                    anchors.centerIn: parent
                    sourceSize.height: 20
                    sourceSize.width: 20
                    source: "qrc:/ui/assets/android_settings.svg"
                    color: palette.windowText
                }

                onClicked: {
                    if (topBar.state === "menumode" ){
                        topBar.state = "" 
                    }
                    else{
                        topBar.state = "menumode" 
                    }
                }
            }
        }

        Rectangle {
            id: separator
            visible: Qt.platform.os !== "android"
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
                topMargin: 0
                spacing: 0
                model: filteredClipList
                visible: false
                currentIndex: -1

                delegate: Button {
                    id: historyDelegate
                    width: historyList.width  
                    height: 70
                    clip: true
                    highlighted: ListView.isCurrentItem
                    function triggerAction() { historyDelegate.clicked(); }

                    background: Rectangle{
                        color: historyDelegate.down ? Qt.rgba(1, 1, 1, 0.1) 
                                   : (historyDelegate.hovered || historyDelegate.highlighted ? Qt.rgba(1, 1, 1, 0.05) : "transparent")
                        radius: (Qt.platform.os === "windows") ? 4 : 12
                    }
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
                            source: (model.type === "image") ? model.preview : "qrc:/ui/assets/text.svg"
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
                currentIndex: -1

                model: ListModel { id: clipList }

                delegate: Button {
                    id: delegateLoader
                    width: historyList.width  
                    height: 70
                    clip: true

                    highlighted: ListView.isCurrentItem
                    function triggerAction() { delegateLoader.clicked(); }

                    background: Rectangle{
                        color: delegateLoader.down ? Qt.rgba(1, 1, 1, 0.1) 
                                   : (delegateLoader.hovered || delegateLoader.highlighted ? Qt.rgba(1, 1, 1, 0.05) : "transparent")
                        radius: (Qt.platform.os === "windows") ? 4 : 12
                    }
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
                            source: (model.type === "image") ? model.preview : "qrc:/ui/assets/text.svg"
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

            ListView {
                id: actionsList
                anchors.fill: parent
                visible: false
                spacing: 2
                clip: true
                currentIndex: -1

                model: ListModel {
                    ListElement { name: "Clear history"; action: "clear" }
                    ListElement { name: ""; action: "pin" }
                    ListElement { name: "Settings"; action: "settings" }
                    ListElement { name: "Close app"; action: "quit" }
                }

                delegate: Button {
                    id: actionControl
                    width: actionsList.width
                    height: 50

                    highlighted: ListView.isCurrentItem
                    function triggerAction() { actionControl.clicked(); }

                    background: Rectangle {
                         color: actionControl.down ? Qt.rgba(1, 1, 1, 0.1) 
                                  : (actionControl.hovered || actionControl.highlighted ? Qt.rgba(1, 1, 1, 0.05) : "transparent")
                        radius: (Qt.platform.os === "windows") ? 4 : 12
                        Behavior on color { ColorAnimation { duration: 80 } }
                    }

                    contentItem: RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 15
                        
                        Text {
                            text: (model.action === "pin") ? (root.isPinned ? "Unpin window" : "Pin window") : model.name
                            font.pixelSize: 15
                            font.weight: Font.Medium
                            color: palette.windowText
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    onClicked: {
                        if (model.action === "clear") {
                            backend.clearHistory()
                            topBar.state = ""
                        } else if (model.action === "pin") {
                            root.isPinned = !root.isPinned
                            topBar.state = ""
                        } else if (model.action === "settings") {
                            bottomNav.currentTab = 1
                            topBar.state = ""
                        } else if (model.action === "quit") {
                            Qt.quit()
                        }
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
                visible: clipList.count === 0 && topBar.state === ""
            }
        }
    }

    function applySearch(query) {
        var cleanQuery = query.trim().toLowerCase()
        filteredClipList.clear()

        if (cleanQuery === "") {
            filterList.visible = false
            historyList.visible = true
            topBar.state = "" 
        } else {
            historyList.visible = false
            filterList.visible = true
            topBar.state = "searchmode" 

            for (let i = 0; i < clipList.count; i++) {
                let item = clipList.get(i)
                if (item && item.name && item.name.toLowerCase().includes(cleanQuery)) {
                    filteredClipList.append(item)
                }
            }
        }
    }
    function focusSearchInput() {
        if (searchInput) {
            searchInput.forceActiveFocus();
        }
    }
}