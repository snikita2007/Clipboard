pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.impl
import QtCore as NativeCore
import QtQuick.Effects

Page {
    id: settingsPage
    background: null

    signal navigate(string page)

    property int selectedIndex: 0
    property bool mobileDetailOpen: false
    readonly property bool isAndroid: Qt.platform.os === "android"
    readonly property string currentMode: mobileDetailOpen ? "detail" : ""
    readonly property color hoverColor: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.06)
    readonly property color pressColor: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.12)
    readonly property color borderColor: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.14)
    property alias settings: appSettings

    NativeCore.Settings {
        id: appSettings
        category: "ApplicationSettings"

        property bool saveClipboardHistory: true
        property int historyLimit: 0
        property bool launchAtStartup: false
        property int defaultAction: 0

        property bool deviceSync: false
        property int discoveryMode: 0
        property bool wifiOnly: true
        property bool requireConfirmation: true

        property int theme: 0
        property int density: 0
        property real windowOpacity: 100
        property bool useWindowEffects: true

        property bool showNotifications: true
        property bool playSound: false
        property int notificationDetail: 1
        property bool quietMode: true

        property int shortcutOpenClipboard: 0
        property int shortcutOpenActions: 0
        property bool enableEscapeToClose: true
        property bool useNumberKeys: false

        property bool ignorePasswords: false
        property int autoClear: 0
        property bool excludeImages: false
        property bool clearOnExit: true

        property int updateChannel: 0
        property bool sendDiagnostics: false
        property bool showTrayIcon: true
        property int versionIndex: 0
    }

    function applySearch(query) {
    }

    function resetMode() {
        mobileDetailOpen = false
    }

    function categoryTitle(index) {
        return settingsModel.get(index).title
    }

    function categoryDescription(index) {
        return settingsModel.get(index).description
    }

    function categoryIcon(index) {
        return settingsModel.get(index).icon
    }

    ListModel {
        id: settingsModel
        ListElement { title: "General"; description: "Startup, clipboard history, and common behavior"; icon: "qrc:/ui/assets/settings.svg" }
        ListElement { title: "Devices"; description: "Connected devices and sync preferences"; icon: "qrc:/ui/assets/bluetooth.svg" }
        ListElement { title: "Appearance"; description: "Theme, density, and window presentation"; icon: "qrc:/ui/assets/appearance.svg" }
        ListElement { title: "Notifications"; description: "Alerts for copy events and device activity"; icon: "qrc:/ui/assets/notifications.svg" }
        ListElement { title: "Hotkeys"; description: "Keyboard shortcuts for faster clipboard actions"; icon: "qrc:/ui/assets/hotkeys.svg" }
        ListElement { title: "Privacy"; description: "Sensitive data, retention, and clearing rules"; icon: "qrc:/ui/assets/privacy.svg" }
        ListElement { title: "About"; description: "Version, updates, and application information"; icon: "qrc:/ui/assets/info.svg" }
    }

    component CategoryButton: AbstractButton {
        id: categoryButton

        required property int itemIndex
        required property string title
        required property string description
        required property string iconSource
        property bool highlighted: settingsPage.selectedIndex === itemIndex

        width: parent ? parent.width : implicitWidth
        height: settingsPage.isAndroid ? 64 : 54
        padding: 0
        hoverEnabled: !settingsPage.isAndroid

        background: Rectangle {
            color: categoryButton.down ? settingsPage.pressColor
                   : (categoryButton.highlighted || categoryButton.hovered ? settingsPage.hoverColor : "transparent")
            radius: Qt.platform.os === "windows" ? 4 : 10
            Behavior on color { ColorAnimation { duration: 120 } }
        }

        contentItem: RowLayout {
            anchors.fill: parent
            anchors.leftMargin: settingsPage.isAndroid ? 14 : 10
            anchors.rightMargin: settingsPage.isAndroid ? 14 : 10
            spacing: 12

            IconImage {
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                sourceSize.width: 22
                sourceSize.height: 22
                source: categoryButton.iconSource
                color: palette.windowText
                smooth: true
                mipmap: true
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Text {
                    Layout.fillWidth: true
                    text: categoryButton.title
                    font.pixelSize: settingsPage.isAndroid ? 16 : 14
                    font.weight: Font.DemiBold
                    color: palette.windowText
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    visible: settingsPage.isAndroid
                    text: categoryButton.description
                    font.pixelSize: 12
                    color: palette.placeholderText
                    elide: Text.ElideRight
                }
            }

            IconImage {
                visible: settingsPage.isAndroid
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
                sourceSize.width: 18
                sourceSize.height: 18
                source: "qrc:/ui/assets/back.svg"
                color: palette.placeholderText
                rotation: 180
                smooth: true
                mipmap: true
            }
        }

        onClicked: {
            settingsPage.selectedIndex = itemIndex
            if (settingsPage.isAndroid) {
                settingsPage.mobileDetailOpen = true
            }
        }
    }

    component SectionTitle: Text {
        Layout.fillWidth: true
        Layout.topMargin: 8
        text: ""
        font.pixelSize: 12
        font.weight: Font.DemiBold
        color: palette.placeholderText
    }

    component SettingToggle: Item {
        id: toggleRow

        property string title: ""
        property string description: ""
        property alias checked: internalSwitch.checked

        Layout.fillWidth: true
        implicitHeight: Math.max(54, labels.implicitHeight + 18)

        RowLayout {
            anchors.fill: parent
            spacing: 12

            ColumnLayout {
                id: labels
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: toggleRow.title
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: palette.windowText
                    wrapMode: Text.WordWrap
                }

                Text {
                    Layout.fillWidth: true
                    text: toggleRow.description
                    visible: text.length > 0
                    font.pixelSize: 12
                    color: palette.placeholderText
                    wrapMode: Text.WordWrap
                }
            }

            Switch {
                id: internalSwitch
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    component SettingChoice: Item {
        id: choiceRow

        property string title: ""
        property string description: ""
        property var values: []
        property alias currentIndex: comboControl.currentIndex

        Layout.fillWidth: true
        implicitHeight: settingsPage.isAndroid ? 76 : 58

        RowLayout {
            anchors.fill: parent
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: choiceRow.title
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: choiceRow.palette.windowText
                    wrapMode: Text.WordWrap
                }

                Text {
                    Layout.fillWidth: true
                    text: choiceRow.description
                    visible: text.length > 0
                    font.pixelSize: 12
                    color: choiceRow.palette.placeholderText
                    wrapMode: Text.WordWrap
                }
            }

            ComboBox {
                id: comboControl
                Layout.preferredWidth: settingsPage.isAndroid ? 132 : 150
                Layout.alignment: Qt.AlignVCenter
                model: choiceRow.values

                palette {
                    buttonText: choiceRow.palette.windowText
                    windowText: choiceRow.palette.windowText
                    button: choiceRow.palette.window
                    window: choiceRow.palette.window
                    base: choiceRow.palette.base
                }

                Component.onCompleted: {
                    if (Qt.platform.os === "osx") {
                        comboControl.delegate = macDelegateComponent
                        popup = macPopupComponent.createObject(comboControl)
                    }
                }
            }
        }

        Component {
            id: macDelegateComponent
            ItemDelegate {
                id: itemMenu
                width: ListView.view.width
                height: 24 
                required property var modelData

                contentItem: Text {
                    text: modelData
                    color: (itemMenu.hovered || itemMenu.highlighted) ? choiceRow.palette.highlightedText : choiceRow.palette.windowText
                    font.pixelSize: 13
                    font.family: ".AppleSystemUIFont"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 8
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    radius: 4
                    color: (itemMenu.hovered || itemMenu.highlighted) ? choiceRow.palette.highlight : "transparent"
                }
            }
        }

        Component {
            id: macPopupComponent
            Popup {
                id: comboPopup
                y: - (padding + comboControl.currentIndex * 24)
                x: 0
                
                width: comboControl.width
                implicitHeight: contentItem.implicitHeight + (topPadding + bottomPadding)
                padding: 4

                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: comboControl.delegateModel
                    currentIndex: comboControl.highlightedIndex
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollIndicator.vertical: ScrollIndicator { }
                }

                background: Rectangle {
                    color: choiceRow.palette.windowText.r > 0.5 ? "#282828" : "#FFFFFF"
                    border.color: choiceRow.palette.windowText.r > 0.5 ? "#404040" : "#D2D2D2"
                    border.width: 1
                    radius: 6

                    layer.enabled: true
                    layer.effect: MultiEffect {
                        shadowEnabled: true
                        shadowColor: Qt.rgba(0, 0, 0, choiceRow.palette.windowText.r > 0.5 ? 0.4 : 0.15)
                        shadowBlur: 12
                        shadowVerticalOffset: 4
                    }
                }
            }
        }
    }

    component SettingSlider: Item {
        id: sliderRow

        property string title: ""
        property string description: ""
        property alias value: slider.value

        Layout.fillWidth: true
        implicitHeight: 76

        ColumnLayout {
            anchors.fill: parent
            spacing: 4

            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    text: sliderRow.title
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: palette.windowText
                    elide: Text.ElideRight
                }

                Text {
                    text: Math.round(slider.value) + "%"
                    font.pixelSize: 12
                    color: palette.placeholderText
                }
            }

            Slider {
                id: slider
                Layout.fillWidth: true
                from: 0
                to: 100
                value: sliderRow.value
            }

            Text {
                Layout.fillWidth: true
                text: sliderRow.description
                visible: text.length > 0
                font.pixelSize: 12
                color: palette.placeholderText
                wrapMode: Text.WordWrap
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: settingsPage.isAndroid ? 14 : 12
        spacing: settingsPage.isAndroid ? 12 : 10

        RowLayout {
            visible: !settingsPage.isAndroid
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            spacing: 8

            AbstractButton {
                id: backButton
                padding: 6
                hoverEnabled: true

                background: Rectangle {
                    color: backButton.down ? settingsPage.pressColor : (backButton.hovered ? settingsPage.hoverColor : "transparent")
                    radius: Qt.platform.os === "windows" ? 4 : 8
                }

                contentItem: RowLayout {
                    spacing: 6

                    IconImage {
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        sourceSize.width: 16
                        sourceSize.height: 16
                        source: "qrc:/ui/assets/back.svg"
                        color: palette.windowText
                    }

                    Text {
                        text: "Back"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: palette.windowText
                    }
                }

                onClicked: {
                    settingsPage.navigate("History.qml")
                }
            }

            Text {
                Layout.fillWidth: true
                text: "Settings"
                font.pixelSize: 20
                font.weight: Font.DemiBold
                color: palette.windowText
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            Item {
                Layout.preferredWidth: backButton.implicitWidth
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: settingsPage.isAndroid ? mobileSettingsComponent : desktopSettingsComponent
        }
    }

    Component {
        id: desktopSettingsComponent

        RowLayout {
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 190
                Layout.fillHeight: true
                color: "transparent"
                border.color: "transparent"
                border.width: 1
                radius: Qt.platform.os === "windows" ? 4 : 12

                ListView {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 4
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    model: settingsModel

                    delegate: CategoryButton {
                        required property int index

                        itemIndex: index
                        title: settingsPage.categoryTitle(index)
                        description: settingsPage.categoryDescription(index)
                        iconSource: settingsPage.categoryIcon(index)
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "transparent"
                border.color: "transparent"
                border.width: 1
                radius: Qt.platform.os === "windows" ? 4 : 12

                SettingsContent {
                    anchors.fill: parent
                    anchors.margins: 18
                }
            }
        }
    }

    Component {
        id: mobileSettingsComponent

        Item {
            clip: true

            focus: settingsPage.mobileDetailOpen

            Keys.onReleased: (event) => {
                if (event.key === Qt.Key_Back && settingsPage.mobileDetailOpen) {
                    settingsPage.resetMode()
                    event.accepted = true 
                }
            }

            ListView {
                id: mobileList
                anchors.fill: parent
                visible: !settingsPage.mobileDetailOpen
                spacing: 8
                clip: true
                bottomMargin: 84
                boundsBehavior: Flickable.DragAndOvershootBounds
                model: settingsModel

                header: Text {
                    width: mobileList.width
                    height: 44
                    text: "Settings"
                    font.pixelSize: 24
                    font.weight: Font.DemiBold
                    color: palette.windowText
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                delegate: CategoryButton {
                    required property int index

                    itemIndex: index
                    title: settingsPage.categoryTitle(index)
                    description: settingsPage.categoryDescription(index)
                    iconSource: settingsPage.categoryIcon(index)
                }
            }

            SettingsContent {
                visible: settingsPage.mobileDetailOpen
                anchors.fill: parent
            }
        }
    }

    component SettingsContent: ListView {
        id: contentRoot
        clip: true
        boundsBehavior: (Qt.platform.os === "windows") ? Flickable.StopAtBounds : Flickable.DragAndOvershootBounds

        ColumnLayout {
            width: contentRoot.width
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                IconImage {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    sourceSize.width: 30
                    sourceSize.height: 30
                    source: settingsPage.categoryIcon(settingsPage.selectedIndex)
                    color: palette.windowText
                    smooth: true
                    mipmap: true
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        Layout.fillWidth: true
                        text: settingsPage.categoryTitle(settingsPage.selectedIndex)
                        font.pixelSize: settingsPage.isAndroid ? 20 : 24
                        font.weight: Font.DemiBold
                        color: palette.windowText
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: settingsPage.categoryDescription(settingsPage.selectedIndex)
                        font.pixelSize: 13
                        color: palette.placeholderText
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: settingsPage.borderColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: settingsPage.selectedIndex === 0

                SectionTitle { text: "Clipboard" }
                
                SettingToggle { 
                    title: "Save clipboard history"; description: "Keep copied text and images available in History."
                    checked: appSettings.saveClipboardHistory
                    onCheckedChanged: {
                        appSettings.saveClipboardHistory = checked
                        backend.setSaveClipboardHistory(checked)
                    }
                }
                SettingChoice { 
                    title: "History limit"; description: "Maximum number of recent clipboard items."; values: ["10 items", "50 items", "100 items", "250 items"]
                    currentIndex: appSettings.historyLimit
                    onCurrentIndexChanged: {
                        appSettings.historyLimit = currentIndex
                        backend.setHistoryLimit(currentIndex)
                    }
                }
                SettingToggle { 
                    title: "Launch at startup"; description: "Open Clipboard automatically when the system starts."
                    visible: !settingsPage.isAndroid
                    checked: appSettings.launchAtStartup
                    onCheckedChanged: {
                        appSettings.launchAtStartup = checked
                        backend.setLaunchAtStartup(checked)
                    }
                }
                SettingChoice { 
                    title: "Default action"; description: "What happens when an item is selected."; values: ["Copy and close", "Copy only"]
                    currentIndex: appSettings.defaultAction
                    onCurrentIndexChanged: appSettings.defaultAction = currentIndex
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: settingsPage.selectedIndex === 1

                SectionTitle { text: "Sync" }
                
                SettingToggle { 
                    title: "Device sync"; description: "Share clipboard items with trusted devices."
                    checked: appSettings.deviceSync
                    onCheckedChanged: {
                        appSettings.deviceSync = checked
                        backend.setDeviceSynk(checked)
                    }
                }
                SettingChoice { 
                    title: "Discovery mode"; description: "How nearby devices can find this app."; values: ["Private", "Group only", "Visible"]
                    currentIndex: appSettings.discoveryMode
                    onCurrentIndexChanged: appSettings.discoveryMode = currentIndex
                }
                SettingToggle { 
                    title: "Wi-Fi only"; description: "Avoid syncing over mobile data."
                    checked: appSettings.wifiOnly
                    onCheckedChanged: appSettings.wifiOnly = checked
                }
                SettingToggle { 
                    title: "Require confirmation"; description: "Ask before accepting clipboard content from another device."
                    checked: appSettings.requireConfirmation
                    onCheckedChanged: appSettings.requireConfirmation = checked
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: settingsPage.selectedIndex === 2

                SectionTitle { text: "Interface" }
                
                SettingChoice { 
                    title: "Theme"; description: "Match the app colors to your system or choose manually."; values: ["System", "Light", "Dark"]
                    currentIndex: appSettings.theme
                    onCurrentIndexChanged: {
                        appSettings.theme = currentIndex
                        windowEffects.setTheme(currentIndex)
                    }
                }
                SettingChoice { 
                    title: "Density"; description: "Adjust spacing for history and settings rows."; values: ["Comfortable", "Compact", "Spacious"]
                    currentIndex: appSettings.density
                    onCurrentIndexChanged: appSettings.density = currentIndex
                }
                SettingToggle { 
                    title: "Use window effects"; description: "Enable native blur and material effects when supported."
                    visible: !settingsPage.isAndroid
                    checked: appSettings.useWindowEffects
                    onCheckedChanged: {
                        appSettings.useWindowEffects = checked
                        windowEffects.setUseWindowEffects(checked)
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: settingsPage.selectedIndex === 3

                SectionTitle { text: "Alerts" }
                
                SettingToggle { 
                    title: "Show notifications"; description: "Notify when Clipboard captures or syncs content."
                    checked: appSettings.showNotifications
                    onCheckedChanged: appSettings.showNotifications = checked
                }
                SettingToggle { 
                    title: "Play sound"; description: "Use a short sound for important clipboard actions."
                    checked: appSettings.playSound
                    onCheckedChanged: appSettings.playSound = checked
                }
                SettingChoice { 
                    title: "Notification detail"; description: "How much copied content is shown in notifications."; values: ["Hidden", "Title only", "Preview"]
                    currentIndex: appSettings.notificationDetail
                    onCurrentIndexChanged: appSettings.notificationDetail = currentIndex
                }
                SettingToggle { 
                    title: "Quiet mode"; description: "Mute notifications while fullscreen apps are active."
                    visible: !settingsPage.isAndroid
                    checked: appSettings.quietMode
                    onCheckedChanged: appSettings.quietMode = checked
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: settingsPage.selectedIndex === 4 && !settingsPage.isAndroid 

                SectionTitle { text: "Shortcuts" }
                
                SettingChoice { 
                    title: "Open Clipboard"; description: "Global shortcut for the floating window."; values: [Qt.platform.os === "osx" ? "Option+V" : "Alt+V", "Ctrl+Shift+V", "Disabled"]
                    currentIndex: appSettings.shortcutOpenClipboard
                    onCurrentIndexChanged: appSettings.shortcutOpenClipboard = currentIndex
                }
                SettingChoice { 
                    title: "Open actions"; description: "Shortcut for the actions menu."; values: [Qt.platform.os === "osx" ? "Option+A" : "Alt+A", "Ctrl+Shift+A", "Disabled"]
                    currentIndex: appSettings.shortcutOpenActions
                    onCurrentIndexChanged: appSettings.shortcutOpenActions = currentIndex
                }
                SettingToggle { 
                    title: "Enable Escape to close"; description: "Close popups or return from nested menus with Escape."
                    checked: appSettings.enableEscapeToClose
                    onCheckedChanged: appSettings.enableEscapeToClose = checked
                }
                SettingToggle { 
                    title: "Use number keys"; description: "Copy visible history items with number shortcuts."
                    checked: appSettings.useNumberKeys
                    onCheckedChanged: appSettings.useNumberKeys = checked
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: settingsPage.selectedIndex === 5

                SectionTitle { text: "Data" }
                
                SettingToggle { 
                    title: "Ignore passwords"; description: "Try to skip content copied from password fields."
                    checked: appSettings.ignorePasswords
                    onCheckedChanged: appSettings.ignorePasswords = checked
                }
                SettingChoice { 
                    title: "Auto-clear"; description: "Remove stored clipboard items after a period of time."; values: ["Never", "After 1 day", "After 7 days", "After 30 days"]
                    currentIndex: appSettings.autoClear
                    onCurrentIndexChanged: {
                        appSettings.autoClear = currentIndex
                        backend.setAutoClear(currentIndex)
                    }
                }
                SettingToggle { 
                    title: "Exclude images"; description: "Store text only and ignore copied images."
                    checked: appSettings.excludeImages
                    onCheckedChanged: {
                        appSettings.excludeImages = checked
                        backend.setExcludeImages(checked)
                    }
                }
                SettingToggle { 
                    title: "Clear on exit"; description: "Delete local history when the app quits."
                    checked: appSettings.clearOnExit
                    onCheckedChanged: {
                        appSettings.clearOnExit = checked
                        backend.setClearOnExit(checked)
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: settingsPage.selectedIndex === 6

                SectionTitle { text: "Application" }
                
                SettingChoice { 
                    title: "Update channel"; description: "Choose how early new versions are offered."; values: ["Stable"]
                    currentIndex: appSettings.updateChannel
                    onCurrentIndexChanged: appSettings.updateChannel = currentIndex
                }
                SettingToggle { 
                    title: "Send diagnostics"; description: "Share anonymous crash information to improve reliability."
                    checked: appSettings.sendDiagnostics
                    onCheckedChanged: appSettings.sendDiagnostics = checked
                }
                SettingToggle { 
                    title: "Show tray icon"; description: "Keep Clipboard available from the system tray."
                    visible: !settingsPage.isAndroid
                    checked: appSettings.showTrayIcon
                    onCheckedChanged: appSettings.showTrayIcon = checked
                }
                SettingChoice { 
                    title: "Version"; description: "Clipboard desktop and mobile settings UI."; values: ["1.0"]
                    currentIndex: appSettings.versionIndex
                    onCurrentIndexChanged: appSettings.versionIndex = currentIndex
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: settingsPage.isAndroid ? 96 : 8
            }
        }
    }
}

