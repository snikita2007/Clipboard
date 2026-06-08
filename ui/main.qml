import QtQuick
import QtQuick.Controls
import Qt.labs.platform as Native
import "components"

ApplicationWindow {
    id: root
    visible: Qt.platform.os === "android" 
    color: (Qt.platform.os === "android" ) ? palette.window : "transparent"

    property bool _androidThemeAutoSync: {
        if (Qt.platform.os === "android") {
            // Проверяем, какой цвет темнее. Если фон темнее текста — значит тема ТЕМНАЯ
            // palette.window.hslLightness возвращает яркость от 0.0 (черный) до 1.0 (белый)
            let isDark = palette.window.hslLightness < palette.windowText.hslLightness

            // 1. Синхронизируем базовую тему Material (1 = Dark, 0 = Light)
            windowRoot["Material.theme"] = isDark ? 1 : 0
            
            // 2. Ставим нужный тебе акцент: в темной теме — белый, в светлой — темно-серый
            windowRoot["Material.accent"] = Qt.binding(function() {
                return isDark ? "#FFFFFF" : "#333333"
            })
            return true
        }
        return false
    }

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

        interactive: Qt.platform.os === "android" && !settingsPage.mobileDetailOpen
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
            onNavigate: (page) => {
                if (page === "History.qml") {
                    bottomNav.currentTab = 0
                    root.updateWindowSize(0)
                }
            }
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
    onSoftClose: {
        bottomNav.currentTab = 0
        closeAnimation.start()
    }

    ParallelAnimation {
        id: closeAnimation
        NumberAnimation { target: root; property: "opacity"; to: 0; duration: 150; easing.type: Easing.OutCubic }
        
        // Исправлено: Оставлен один корректный обработчик завершения анимации
        onStopped: {
            mainSwipe.currentItem.resetMode()
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
        
        NumberAnimation { id: wAnim; target: root; property: "width"; duration: 250; easing.type: Easing.OutCubic }
        NumberAnimation { id: hAnim; target: root; property: "height"; duration: 250; easing.type: Easing.OutCubic }
        
        onStopped: {
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
            wAnim.to = 500;
            hAnim.to = historyPage.getRequiredHeight();
        } else if (tabIndex === 1) {
            wAnim.to = 820;
            hAnim.to = 640;
        }
        
        smoothResize.start();
    }

    onWidthChanged: {
        if (Qt.platform.os === "android" || opacity === 0) return;
        
        let rightEdge = root.x + root.width;
        let maxRight = Screen.desktopAvailableWidth - 10;
        
        if (rightEdge > maxRight) {
            root.x = maxRight - root.width;
        }

        if (root.x < 10) {
            root.x = 10;
        }
    }

    onHeightChanged: {
        if (Qt.platform.os === "android" || opacity === 0) return;
        
        let bottomEdge = root.y + root.height;
        let maxBottom = Screen.desktopAvailableHeight - 15;

        if (bottomEdge > maxBottom) {
            root.y = maxBottom - root.height;
        }

        if (root.y < 30) {
            root.y = 30; 
        }
    }
} 
