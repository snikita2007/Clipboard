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

                    onTextChanged: historyPage.applySearch(text)
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

            Label {
                text: "Clipboard is empty"
                font.pixelSize: 14
                font.weight: Font.DemiBold 
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.centerIn: historyList
                visible: clipList.count === 0 && historyList.visible === true
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
}