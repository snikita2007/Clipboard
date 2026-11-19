package com.clipboard.app;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ClipboardManager;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.IBinder;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Keeps the app alive in the background (foreground service with a notification),
 * so data from other devices still arrives.
 *
 * Android 10+ does not let background apps read the clipboard. When an app is denied,
 * the system writes "Denied/Denying clipboard access to <package>" to the log.
 * If the app was given READ_LOGS (over adb), this service watches the log and opens
 * an invisible activity, which may read the clipboard because it has focus.
 */
public class ClipboardSyncService extends Service {
    private static final String TAG = ClipboardBridge.TAG;
    private static final String CHANNEL_ID = "clipboard_sync";
    private static final int NOTIFICATION_ID = 7302;

    private ClipboardManager mClipboard;
    // Android calls this listener only while the app has focus. In the background it
    // denies the call and logs it, which is exactly what the log watcher needs.
    private final ClipboardManager.OnPrimaryClipChangedListener mClipListener = () -> { };

    private WifiManager.MulticastLock mMulticastLock;
    private volatile boolean mRunning;
    private volatile Process mLogProcess;
    private Thread mLogThread;
    private long mLastLaunch;
    private long mWatchStarted;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        createChannel();
        try {
            if (Build.VERSION.SDK_INT >= 29) {
                startForeground(NOTIFICATION_ID, buildNotification(), ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
            } else {
                startForeground(NOTIFICATION_ID, buildNotification());
            }
        } catch (Exception e) {
            Log.w(TAG, "Cannot start foreground service", e);
            stopSelf();
            return;
        }

        mClipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        if (mClipboard != null) {
            mClipboard.addPrimaryClipChangedListener(mClipListener);
        }

        // Some phones drop broadcast packets (device search) when the screen is off
        WifiManager wifi = (WifiManager) getApplicationContext().getSystemService(WIFI_SERVICE);
        if (wifi != null) {
            try {
                mMulticastLock = wifi.createMulticastLock("ClipboardSync");
                mMulticastLock.setReferenceCounted(false);
                mMulticastLock.acquire();
            } catch (Exception e) {
                Log.w(TAG, "Cannot take multicast lock", e);
            }
        }

        startLogWatcher();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // Without the Qt part the service is useless, so do not restart it alone
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        mRunning = false;
        Process process = mLogProcess;
        if (process != null) process.destroy();
        if (mLogThread != null) mLogThread.interrupt();
        if (mClipboard != null) mClipboard.removePrimaryClipChangedListener(mClipListener);
        if (mMulticastLock != null && mMulticastLock.isHeld()) mMulticastLock.release();
        super.onDestroy();
    }

    private void startLogWatcher() {
        if (!ClipboardBridge.canReadLogs(this)) {
            Log.i(TAG, "READ_LOGS is not granted: automatic sending is off");
            return;
        }

        mRunning = true;
        // AOSP writes "Denied clipboard access to <pkg>", MIUI "Denying clipboard access to <pkg>"
        final String marker = "clipboard access to " + getPackageName();

        mLogThread = new Thread(() -> {
            int attempts = 0;
            while (mRunning && attempts < 5) {
                attempts++;
                try {
                    mWatchStarted = SystemClock.uptimeMillis();
                    Process process = Runtime.getRuntime().exec(new String[]{
                            "logcat", "-T", "1", "-v", "brief", "ClipboardService:V", "*:S"});
                    mLogProcess = process;
                    BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
                    String line;
                    while (mRunning && (line = reader.readLine()) != null) {
                        if (line.contains(marker)) {
                            onClipboardDenied();
                        }
                    }
                    process.destroy();
                } catch (IOException e) {
                    Log.w(TAG, "Cannot read the system log", e);
                }
                if (!mRunning) return;
                try {
                    Thread.sleep(3000);
                } catch (InterruptedException e) {
                    return;
                }
            }
        }, "ClipboardLogWatcher");
        mLogThread.setDaemon(true);
        mLogThread.start();
        Log.i(TAG, "Watching the system log for clipboard changes");
    }

    private void onClipboardDenied() {
        long now = SystemClock.uptimeMillis();
        // "-T 1" also prints one old line at start
        if (now - mWatchStarted < 1000) return;
        // The app changed the clipboard itself (data from another device)
        if (ClipboardBridge.isOwnWriteRecent()) return;

        synchronized (this) {
            // Several listeners are denied for one change: react once
            if (now - mLastLaunch < 800) return;
            mLastLaunch = now;
        }

        Intent intent = new Intent(this, FloatingClipboardActivity.class);
        intent.setAction(FloatingClipboardActivity.ACTION_READ_CLIPBOARD);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_NO_ANIMATION
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        try {
            startActivity(intent);
        } catch (Exception e) {
            Log.w(TAG, "Cannot open the clipboard reader", e);
        }
    }

    private void createChannel() {
        NotificationManager manager = getSystemService(NotificationManager.class);
        if (manager == null) return;
        NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID, "Clipboard sync", NotificationManager.IMPORTANCE_LOW);
        channel.setDescription("Shown while clipboard sync runs in the background");
        channel.setShowBadge(false);
        manager.createNotificationChannel(channel);
    }

    private Notification buildNotification() {
        int flags = PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT;

        Intent open = getPackageManager().getLaunchIntentForPackage(getPackageName());
        PendingIntent openIntent = open != null ? PendingIntent.getActivity(this, 0, open, flags) : null;

        Intent send = new Intent(this, FloatingClipboardActivity.class)
                .setAction(FloatingClipboardActivity.ACTION_SEND_CLIPBOARD)
                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_ANIMATION);
        PendingIntent sendIntent = PendingIntent.getActivity(this, 1, send, flags);

        boolean auto = ClipboardBridge.canReadLogs(this) && Settings.canDrawOverlays(this);
        String text = auto
                ? "Copied text is sent to your devices automatically"
                : "Copy text, then tap \"Send clipboard\"";

        int icon = getResources().getIdentifier("ic_stat_clipboard", "drawable", getPackageName());
        if (icon == 0) icon = getApplicationInfo().icon;

        Notification.Builder builder = new Notification.Builder(this, CHANNEL_ID)
                .setSmallIcon(icon)
                .setContentTitle("Clipboard sync is on")
                .setContentText(text)
                .setOngoing(true)
                .setShowWhen(false)
                .addAction(new Notification.Action.Builder(null, "Send clipboard", sendIntent).build());
        if (openIntent != null) builder.setContentIntent(openIntent);
        return builder.build();
    }
}
