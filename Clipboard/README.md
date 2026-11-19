# Clipboard 

Fast conectivity between multiply devices (mac, win, linux, android) with fluent design

## Windows installer

A themed, offline installer is available under `packaging/windows`. It installs
Clipboard for the current user, offers desktop/startup options, adds a Start menu
shortcut, and supports updates and standard Windows uninstallation. See the
[build and verification instructions](packaging/windows/README.md).

## Linux installer

A self-extracting installer and a portable AppImage for x86_64 are built with the
scripts under `packaging/linux`. The installer needs no administrator rights: it
installs Clipboard for the current user, adds an application menu entry, offers
login startup and a desktop shortcut, and supports updates and uninstallation.
See the [build and verification instructions](packaging/linux/README.md), which
also describe how the app behaves on X11 and Wayland.

## Bluetooth startup

Bluetooth starts automatically when Clipboard opens, including when launched at
system login. In **Settings → Devices**, **Bluetooth on startup** is enabled by
default. Disable it to restore the last explicitly selected connection type
(Wi-Fi or Bluetooth) instead.

Pair the devices in system settings, open Clipboard on both, and connect once.
The app saves the address and name of the last successful Clipboard Bluetooth
connection, including connections initiated by the other device. Older versions
did not save this information, so one connection after updating is necessary.
The saved device remains in the list even when it is not discoverable.

At startup the app requests Bluetooth permission, powers on the adapter where
supported, waits until it is ready, and reconnects directly to the saved address.
The operating system may require confirmation to enable Bluetooth. On macOS,
Bluetooth must be enabled in system settings; the app resumes when it turns on.
See [Qt's platform notes](https://doc.qt.io/qt-6.9/qbluetoothlocaldevice.html#powerOn).

An unavailable device is retried after 5, 10, 20, then 30 seconds between attempts.
Each connection attempt has a 20-second timeout. Switching to Wi-Fi stops retries.
Pressing Disconnect or choosing another device suspends local automatic attempts
until a connection succeeds, Bluetooth mode is reselected, or the app restarts.
Only successful connections replace the saved device.

## Bluetooth photos and files

Bluetooth sync supports text, copied images (including screenshots), and copied
files. Select files in your file manager and copy them as usual. A selection may
contain up to **32 files and 5 MiB (5,242,880 bytes) in total**; the same size limit
applies to a single photo and to text. Clipboard images are sent as PNG; copied
files keep their original bytes and names. Directories are not transferred.

Update Clipboard on **both devices** before sending attachments. Text retains
the previous wire format, but older versions cannot receive attachments. Large
items are skipped with an explanation in **Settings → Devices → Bluetooth**;
use Wi-Fi for larger desktop files. Transfer status also appears there. If no
device is connected, the most recently copied item waits for the connection.
An interrupted transfer never replaces the receiving clipboard with a partial
file; copy the item again after reconnecting.

Received items are immediately available in the clipboard and History. Selecting
a file or group in History copies the actual files, ready to paste into a file
manager or an app that accepts attachments. Files are kept in the app's cache;
save needed files elsewhere before clearing history. Files with the same name
and different contents are kept separately.

On Android, copy a photo/file and bring Clipboard to the foreground, use **Send
clipboard** in the notification, or choose **Share → Clipboard sync** (including
multiple files). URI attachments imported on Android have the same 5 MiB/32-file
limit. Receiving and copying files uses Android content URIs with read grants,
so other apps can paste the attachment. Availability of background clipboard
reading still depends on the existing Android permissions and system restrictions.

## Regression tests

With Qt 6 (including Qt Test) and a desktop compiler configured:

```sh
cmake -S . -B build -DCLIPBOARD_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The tests use temporary settings and do not access a Bluetooth adapter. They check
startup preferences, remembered peers, delayed adapter readiness, retry after
failure, incoming connections, cancellation, and the settings page bindings.
The transfer tests cover binary/image/empty-file round trips, the size boundary,
fragmented messages, short writes and backpressure, bounded queues, disconnects,
malformed messages, safe file names, grouped files, history copying, and echo
suppression with history disabled.

Hardware check: connect two devices once, restart Clipboard without opening
settings, and copy text in both directions. Repeat with the second app initially
closed, then open it and verify reconnection. Also check startup with Bluetooth
off, the system permission prompt, and switching to Wi-Fi during a pending retry.
Copy a screenshot, a small binary file, an empty file, and a multi-file selection
in both directions. Verify pasted bytes, Android Share and History copying, a
selection exceeding 5 MiB, and disconnect/reconnect midway through a transfer.
