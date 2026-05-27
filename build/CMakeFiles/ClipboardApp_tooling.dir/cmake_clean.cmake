file(REMOVE_RECURSE
  "Clipboard/ui/History.qml"
  "Clipboard/ui/Settings.qml"
  "Clipboard/ui/components/MobileBottomNav.qml"
  "Clipboard/ui/components/MyButton.qml"
  "Clipboard/ui/components/TitleBar.qml"
  "Clipboard/ui/main.qml"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/ClipboardApp_tooling.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
