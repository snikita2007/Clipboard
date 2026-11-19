package com.clipboard.app;

import android.app.Activity;
import android.content.Context;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;

/**
 * Link between the Java side (background service, clipboard reader) and the Qt/C++ side.
 * C++ registers nativeOnClipboardText() and then calls start().
 */
public final class ClipboardBridge {
    static final String TAG = "ClipboardSync";

    private static volatile boolean sNativeReady = false;
    private static volatile long sIgnoreUntil = 0;
    private static String sPendingText = null;
    private static String[] sPendingFiles = null;

    private ClipboardBridge() {}

    // Implemented in C++ (android_clipboard.cpp)
    private static native void nativeOnClipboardText(String text);
    private static native void nativeOnClipboardFiles(String[] paths);

    // ---- Called from C++ ----

    public static void start(Context context) {
        sNativeReady = true;
        Context app = context.getApplicationContext();
        try {
            app.startForegroundService(new Intent(app, ClipboardSyncService.class));
        } catch (Exception e) {
            Log.w(TAG, "Cannot start the sync service", e);
        }

        // Android 13+: the service notification is hidden without this permission
        if (context instanceof Activity && Build.VERSION.SDK_INT >= 33
                && context.checkSelfPermission("android.permission.POST_NOTIFICATIONS") != PackageManager.PERMISSION_GRANTED) {
            ((Activity) context).requestPermissions(new String[]{"android.permission.POST_NOTIFICATIONS"}, 7301);
        }
    }

    public static void stop(Context context) {
        sNativeReady = false;
        Context app = context.getApplicationContext();
        app.stopService(new Intent(app, ClipboardSyncService.class));
    }

    public static synchronized String takePendingText() {
        String text = sPendingText;
        sPendingText = null;
        return text;
    }

    public static synchronized void deliverPendingFiles() {
        if (sPendingFiles == null) return;
        String[] paths = sPendingFiles;
        sPendingFiles = null;
        deliverFiles(paths);
    }

    public static void setClipboardFiles(Context context, String[] paths) {
        ClipboardAttachments.put(context, paths);
    }

    public static void readClipboardFiles(Context context) {
        new android.os.Handler(android.os.Looper.getMainLooper()).post(() -> {
            try {
                ClipboardManager clipboard = context.getSystemService(ClipboardManager.class);
                ClipData clip = clipboard != null ? clipboard.getPrimaryClip() : null;
                java.util.List<android.net.Uri> uris = ClipboardAttachments.uris(clip);
                if (uris.isEmpty() || ClipboardAttachments.isOwnUri(context, uris.get(0))) return;
                ClipboardAttachments.read(context, uris, (paths, error) -> {
                    if (paths != null) deliverFiles(paths);
                    else android.widget.Toast.makeText(context, error, android.widget.Toast.LENGTH_LONG).show();
                });
            } catch (Exception e) {
                Log.w(TAG, "Cannot read clipboard files", e);
            }
        });
    }

    /** The app is about to change the clipboard itself (data from another device). */
    public static void markOwnClipboardWrite() {
        sIgnoreUntil = SystemClock.uptimeMillis() + 2000;
    }

    /** "auto", "no-logs", "no-overlay" or "manual" */
    public static String backgroundStatus(Context context) {
        boolean logs = canReadLogs(context);
        boolean overlay = Settings.canDrawOverlays(context);
        if (logs && overlay) return "auto";
        if (!logs && !overlay) return "manual";
        return logs ? "no-overlay" : "no-logs";
    }

    // ---- Called from Java ----

    static boolean isOwnWriteRecent() {
        return SystemClock.uptimeMillis() < sIgnoreUntil;
    }

    static boolean isNativeReady() {
        return sNativeReady;
    }

    static boolean canReadLogs(Context context) {
        return context.checkSelfPermission("android.permission.READ_LOGS") == PackageManager.PERMISSION_GRANTED;
    }

    /** Sends text to the other devices. Returns false if the Qt part is not running. */
    static boolean deliverText(String text) {
        if (text == null || text.isEmpty()) return false;

        if (sNativeReady) {
            try {
                nativeOnClipboardText(text);
                return true;
            } catch (UnsatisfiedLinkError e) {
                Log.w(TAG, "Qt part is not loaded", e);
            }
        }
        synchronized (ClipboardBridge.class) {
            sPendingText = text;
        }
        return false;
    }

    static boolean deliverFiles(String[] paths) {
        if (paths == null || paths.length == 0) return false;
        if (sNativeReady) {
            try {
                nativeOnClipboardFiles(paths);
                return true;
            } catch (UnsatisfiedLinkError e) {
                Log.w(TAG, "Qt part is not loaded", e);
            }
        }
        synchronized (ClipboardBridge.class) { sPendingFiles = paths; }
        return false;
    }
}
