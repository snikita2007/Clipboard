import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl

Page{
    id: historyPage
    background: null

    readonly property string currentMode: topBar.state

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        anchors.bottomMargin: 5
        spacing: 0

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
                    PropertyChanges { target: historyList; visible: false; opacity: 0 }
                    PropertyChanges { target: filterList; visible: true; opacity: 1 }
                    PropertyChanges { target: actionsList; visible: false; opacity: 0 }
                },
                State {
                    name: "menumode"
                    PropertyChanges { target: historyList; visible: false; opacity: 0 }
                    PropertyChanges { target: filterList; visible: false; opacity: 0 }
                    PropertyChanges { target: actionsList; visible: true; opacity: 1 }
                },
                State {
                    name: ""
                    PropertyChanges { target: historyList; visible: true; opacity: 1 }
                    PropertyChanges { target: filterList; visible: false; opacity: 0 }
                    PropertyChanges { target: actionsList; visible: false; opacity: 0 }
                }
            ]

            onStateChanged: {
                if (state === "") {
                    searchInput.text = ""
                }
                historyPage.triggerWindowResize();
            }

            MouseArea {
                id: searchOrBackButton

                Layout.preferredHeight: 30
                Layout.preferredWidth: 30
                Layout.leftMargin: 5
                Layout.rightMargin: -5
                
                enabled: topBar.state === "menumode"
                
                onClicked: {
                    topBar.state = ""
                }

                IconImage {
                    anchors.fill: parent
                    sourceSize.height: 30
                    sourceSize.width: 30
                    smooth: true
                    mipmap: true
                    color: (topBar.state === "menumode")  ? palette.windowText : searchInput.placeholderTextColor
                    source: (topBar.state === "menumode") 
                            ? "qrc:/ui/assets/back.svg"  
                            : "qrc:/ui/assets/search.svg"
                }
            }

            Item {
                id: searchWrapper
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                Label {
                    anchors.fill: parent
                    visible: Qt.platform.os !== "android" && topBar.state === "menumode"
                    text: "Actions"
                    color: searchInput.placeholderTextColor
                    font.pixelSize: 20
                    font.weight: Font.DemiBold 
                    padding: 0
                    leftPadding: 10
                    verticalAlignment: Text.AlignVCenter
                }

                TextField {
                    id: searchInput
                    anchors.fill: parent
                    visible: Qt.platform.os !== "android" && topBar.state !== "menumode"
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
                        searchDebounceTimer.stop()
                        
                        if (text.trim() === "") {
                            historyPage.applySearch("") 
                        } else {
                            searchDebounceTimer.searchQuery = text
                            searchDebounceTimer.start()
                        }
                    }
                }
            }
        }


        Rectangle {
            id: separator
            visible: Qt.platform.os !== "android" && (historyList.count > 1 || topBar.state === "menumode")
            Layout.fillWidth: true 
            Layout.preferredHeight: 1
            Layout.topMargin: 10
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
                topMargin: 5
                spacing: 5
                model: filteredClipList
                visible: false
                currentIndex: -1

                onCountChanged: historyPage.triggerWindowResize()

                delegate: Button {
                    id: historyDelegate
                    width: historyList.width  
                    height: 60
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
                            Layout.preferredHeight: 35
                            Layout.preferredWidth: 35
                            Layout.leftMargin: 5
                            sourceSize.height: 35
                            sourceSize.width: 35
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
                topMargin: 5
                spacing: 5
                boundsBehavior: (Qt.platform.os === "windows") ? Flickable.StopAtBounds : Flickable.DragAndOvershootBounds
                flickableDirection: Flickable.VerticalFlick
                transformOrigin: Item.Top
                currentIndex: -1

                onCountChanged: historyPage.triggerWindowResize()

                model: ListModel { id: clipList }

                delegate: Button {
                    id: delegateLoader
                    width: historyList.width  
                    height: 60
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
                            Layout.preferredHeight: 35
                            Layout.preferredWidth: 35
                            Layout.leftMargin: 5
                            sourceSize.height: 35
                            sourceSize.width: 35
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
                topMargin: 2
                clip: true
                currentIndex: -1
                boundsBehavior: (Qt.platform.os === "windows") ? Flickable.StopAtBounds : Flickable.DragAndOvershootBounds

                onCountChanged: historyPage.triggerWindowResize()

                model: ListModel {
                    ListElement { name: "Clear history"; action: "clear"; img: "qrc:/ui/assets/clear.svg"; key: "W" }
                    ListElement { name: ""; action: "pin"; img: ""; key: "P" }
                    ListElement { name: "Settings"; action: "settings"; img: "qrc:/ui/assets/settings.svg"; key: "S" }
                    ListElement { name: "Close app"; action: "quit"; img: "qrc:/ui/assets/tab.svg"; key: "Q" }
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
                        anchors.rightMargin: 15
                        spacing: 10

                        IconImage{
                            Layout.preferredWidth: 20
                            verticalAlignment: Text.AlignVCenter
                            smooth: true
                            mipmap: true
                            source: (model.action === "pin") ? (root.isPinned ? "qrc:/ui/assets/unpin.svg" : "qrc:/ui/assets/pin.svg") : model.img
                            color: palette.windowText
                        }
                        
                        Text {
                            text: (model.action === "pin") ? (root.isPinned ? "Unpin window" : "Pin window") : model.name
                            font.pixelSize: 15
                            font.weight: Font.Medium
                            color: palette.windowText
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                        }
                        RowLayout {
                            spacing: 5
                            Layout.alignment: Qt.AlignVCenter

                            Rectangle {
                                implicitWidth: ctrlText.implicitWidth + 8
                                implicitHeight: ctrlText.implicitHeight + 2
                                radius: 3
                                color: "transparent"
                                border.color: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.3)
                                border.width: 1

                                Label {
                                    text: Qt.platform.os === "osx" ? "Opt" : "Alt"
                                    anchors.centerIn: parent
                                    font.pixelSize: 12
                                    font.weight: Font.Normal
                                    color: palette.windowText
                                }
                            }

                            Rectangle {
                                implicitWidth: actionKeyText.implicitWidth + 8
                                implicitHeight: actionKeyText.implicitHeight + 2
                                radius: 3
                                color: "transparent"
                                border.color: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.3)
                                border.width: 1

                                Label {
                                    id: actionKeyText
                                    text: model.key
                                    anchors.centerIn: parent
                                    font.pixelSize: 12
                                    font.weight: Font.Normal
                                    color: palette.windowText
                                }
                            }
                        }
                    }

                    onClicked: {
                        if (model.action === "clear") {
                            backend.clearHistory()
                            clipList.clear()
                            topBar.state = ""
                        } else if (model.action === "pin") {
                            root.isPinned = !root.isPinned
                            topBar.state = ""
                        } else if (model.action === "settings") {
                            bottomNav.currentTab = 1
                        } else if (model.action === "quit") {
                            Qt.quit()
                        }
                    }
                }
            }
            Label {
                text: clipList.count === 0 ? "There is nothing yet :(" : "Nothing found ~_~"
                font.pixelSize: 18
                font.weight: Font.DemiBold 
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.centerIn: filterList
                color: palette.placeholderText
                visible: filteredClipList.count === 0 && (topBar.state === "searchmode" || Qt.platform.os === "android")
            }
        }

        Rectangle {
            id: bottomSeparator
            visible: Qt.platform.os !== "android" && historyList.count > 1 && topBar.state !== "menumode"
            Layout.fillWidth: true 
            Layout.preferredHeight: 1
            Layout.topMargin: 5
            Layout.bottomMargin: 5
            color: palette.windowText
            opacity: 0.2
        }

        RowLayout {
            visible: Qt.platform.os !== "android" && (historyList.count !== 0 || topBar.state === "searchmode")  && topBar.state !== "menumode"
            Layout.fillWidth: true
            Layout.margins: 0
            Layout.alignment: Qt.AlignBottom 

            Button {
                id: settingsButton
                hoverEnabled: true
                padding: 4
                
                background: Rectangle {
                    color: settingsButton.hovered ? Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.1) : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
                
                onClicked: {
                    bottomNav.currentTab = 1
                }

                contentItem: RowLayout {
                    spacing: 4

                    IconImage {
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        sourceSize.height: 16
                        sourceSize.width: 16
                        source: "qrc:/ui/assets/settings.svg"
                        color: palette.windowText
                    }
                    Label {
                        text: "Settings"
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        elide: Label.ElideRight
                        color: palette.windowText
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: actionsButton
                hoverEnabled: true
                padding: 4

                background: Rectangle {
                    color: actionsButton.hovered ? Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.1) : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                onClicked: {
                    if (topBar.state === "menumode") {
                        topBar.state = "" 
                    } else {
                        topBar.state = "menumode" 
                    }
                }

                contentItem: RowLayout {
                    spacing: 6

                    Label {
                        text: "Actions"
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        color: palette.windowText
                    }

                    RowLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter

                        Rectangle {
                            implicitWidth: ctrlText.implicitWidth + 8
                            implicitHeight: ctrlText.implicitHeight + 2
                            radius: 3
                            color: "transparent"
                            border.color: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.3)
                            border.width: 1

                            Label {
                                id: ctrlText
                                text: Qt.platform.os === "osx" ? "Opt" : "Alt"
                                anchors.centerIn: parent
                                font.pixelSize: 12
                                font.weight: Font.Normal
                                color: palette.windowText
                            }
                        }

                        Rectangle {
                            implicitWidth: aText.implicitWidth + 8
                            implicitHeight: aText.implicitHeight + 2
                            radius: 3
                            color: "transparent"
                            border.color: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.3)
                            border.width: 1

                            Label {
                                id: aText
                                text: "A"
                                anchors.centerIn: parent
                                font.pixelSize: 12
                                font.weight: Font.Normal
                                color: palette.windowText
                            }
                        }
                    }
                }
            }
        }
    }

    function focusSearchInput() {
        if (searchInput) {
            searchInput.forceActiveFocus();
        }
    }

    function getRequiredHeight() {
        let basePadding = 50;
        let extraPadding = 45;
        let currentList;
        let itemHeight = 65;

        if (clipList.count === 1){
            extraPadding = 25;
        }

        if (topBar.state === "menumode") {
            currentList = actionsList;
            itemHeight = 55; 
            extraPadding = 10;
        } else if (topBar.state === "searchmode") {
            currentList = filterList;
        } else {
            currentList = historyList;
        }
        
        if (!currentList || currentList.count === 0) {
            return (topBar.state === "searchmode") ? 200 : basePadding; 
        }
        
        let calculatedContentHeight = currentList.count * itemHeight;
        let targetH = calculatedContentHeight + basePadding + extraPadding;

        return Math.min(targetH, 350);
    }

    function triggerWindowResize() {
        if (typeof root.updateWindowSize === "function") {
            root.updateWindowSize(0); 
        }
    }

    function applySearch(query) {
        var cleanQuery = query.trim().toLowerCase()
        filteredClipList.clear()

        if (cleanQuery === "") {
            if (topBar.state !== "") {
                topBar.state = "" 
            } 
        } else {
            topBar.state = "searchmode" 

            for (let i = 0; i < clipList.count; i++) {
                let item = clipList.get(i)
                if (item && item.name && item.name.toLowerCase().includes(cleanQuery)) {
                    filteredClipList.append(item)
                }
            }
        }
        triggerWindowResize();
    }

    function resetMode() {
        topBar.state = ""
    }

    Timer {
        id: searchDebounceTimer
        interval: 200
        repeat: false

        property string searchQuery: "" 
        
        onTriggered: {
            historyPage.applySearch(searchQuery)
        }
    }
    Shortcut { 
        sequence: "Alt + A"
        onActivated: {
            if (topBar.state === "menumode") {
                topBar.state = ""
            } else {
                topBar.state = "menumode"
            }
        }
    }
    Shortcut { 
        sequence: "Alt + W"
        onActivated: {
            backend.clearHistory()
            clipList.clear()
            topBar.state = ""
        }
    }
    Shortcut { 
        sequence: "Alt + S"
        onActivated: {
            if (bottomNav.currentTab === 1) {
                bottomNav.currentTab = 0
            } else {
                bottomNav.currentTab = 1
            }
        }
    }
    Shortcut { 
        sequence: "Alt + P"
        onActivated: {
            root.isPinned = !root.isPinned
        }
    }
    Shortcut { 
        sequence: "Alt + Q"
        onActivated: {
            Qt.quit()
        }
    }
}