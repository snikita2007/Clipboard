import QtQuick
import QtQuick.Controls
import Qt.labs.platform as Native
import "components"

ApplicationWindow {
    id: root
    visible: Qt.platform.os === "android" 
    color: (Qt.platform.os === "android" ) ? palette.window : "transparent"

    width: Screen.width
    height: Screen.height
    minimumWidth: width
    minimumHeight: height 
    maximumWidth: width 
    maximumHeight: height
    background: null

    flags: Qt.Window | Qt.CustomizeWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    property bool isPinned: false

    Native.SystemTrayIcon {
        id: trayIcon
        visible: true
        icon.source: "qrc:/ui/assets/app.png"
        tooltip: "Clipboard"
        menu: Native.Menu {
            Native.MenuItem { text: "Show/hide"; onTriggered: root.visible ? root.softClose() : root.softOpen() }
            Native.MenuItem { text: "Clear"; onTriggered: { backend.clearHistory() } }
            Native.MenuItem { text: "Settings"; onTriggered: bottomNav.currentTab = 1 }
            Native.MenuItem { text: "Exit"; onTriggered: Qt.quit() }
        }
        onActivated: function(reason) {
            if (reason === Native.SystemTrayIcon.Trigger && Qt.platform.os !== "osx") {
                root.visible ? root.softClose() : root.softOpen()
            }
        }
    }

    onActiveChanged: { 
        if (!active && !isPinned) {
            root.softClose() 
        }
    }

    SwipeView {
        id: mainSwipe
        anchors.fill: parent
        currentIndex: bottomNav.currentTab

        interactive: Qt.platform.os === "android"
        clip: true

        Component.onCompleted: {
            if (Qt.platform.os !== "android") {
                if (mainSwipe.contentItem) {
                    mainSwipe.contentItem.highlightMoveDuration = 0
                }
            }
        }

        History {
            id: historyPage
        }

        Settings {
            id: settingsPage
        }
    
        onCurrentIndexChanged: {
            bottomNav.currentTab = currentIndex
            root.updateWindowSize(currentIndex)
        }
    }

    MobileBottomNav {
        id: bottomNav
        anchors.bottom: parent.bottom
        width: parent.width

        onNavigate: (page) => {
            if (page === "History.qml") {
                bottomNav.currentTab = 0
            } else if (page === "Settings.qml") {
                bottomNav.currentTab = 1
            }
            root.updateWindowSize(bottomNav.currentTab)
        }

        onSearchTextChanged: (text) => {
            historyPage.applySearch(text)
        }
    }

    Shortcut { 
        sequence: "Escape"
        onActivated: {
            if (mainSwipe.currentItem.currentMode === "") {
                root.softClose() 
            } else {
                mainSwipe.currentItem.resetMode()
            }
        }
    }

    signal softClose()
    onSoftClose: closeAnimation.start()
    ParallelAnimation {
        id: closeAnimation
        NumberAnimation { target: root; property: "opacity"; to: 0; duration: 150; easing.type: Easing.OutCubic }
        onFinished: {
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
        
        mainSwipe.currentItem.resetMode()
        historyPage.triggerWindowResize();
        openAnimation.start()
    }
    ParallelAnimation{
        id: openAnimation
        NumberAnimation {target: root; property: "opacity"; to: 1; duration: 150; easing.type: Easing.InCubic }
        onStarted:{
            root.show()
            root.raise()
            root.requestActivate()
            historyPage.focusSearchInput()
        }
    }

    ParallelAnimation {
        id: smoothResize
        
        NumberAnimation { target: root; property: "width"; duration: 250; easing.type: Easing.OutCubic }
        NumberAnimation { target: root; property: "height"; duration: 250; easing.type: Easing.OutCubic }
        
        onFinished: {
            root.minimumWidth = root.width
            root.maximumWidth = root.width
            root.minimumHeight = root.height
            root.maximumHeight = root.height
        }
    }

    function updateWindowSize(tabIndex) {
        if (Qt.platform.os === "android") return;
        smoothResize.stop();
        
        root.minimumWidth = 0;
        root.maximumWidth = 9999;
        root.minimumHeight = 0;
        root.maximumHeight = 9999;

        if (tabIndex === 0) {
            smoothResize.animations[0].to = 500;
            smoothResize.animations[1].to = historyPage.getRequiredHeight();
        } else if (tabIndex === 1) {
            smoothResize.animations[0].to = 600;
            smoothResize.animations[1].to = 550;
        }
        
        smoothResize.start();
    }
}