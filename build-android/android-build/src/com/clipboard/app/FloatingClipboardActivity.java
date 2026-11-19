package com.clipboard.app;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Gravity;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

/**
 * Invisible 1x1 activity. While it has focus, Android allows reading the clipboard.
 * It reads the clipboard and resolves shared files before their temporary URI
 * permissions expire. It also accepts text and files from the Share menu.
 */
public class FloatingClipboardActivity extends Activity {
    static final String ACTION_READ_CLIPBOARD = "com.clipboard.app.READ_CLIPBOARD";
    static final String ACTION_SEND_CLIPBOARD = "com.clipboard.app.SEND_CLIPBOARD";

    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private boolean mDone;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Window window = getWindow();
        window.addFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL);
        WindowManager.LayoutParams params = window.getAttributes();
        params.width = 1;
        params.height = 1;
        params.gravity = Gravity.TOP | Gravity.START;
        params.dimAmount = 0f;
        window.setAttributes(params);

        // Never stay on screen, even if focus does not come
        mHandler.postDelayed(this::finishQuietly, 2000);

        if (isShare(getIntent())) {
            handleShare(getIntent());
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        mDone = false;
        if (isShare(intent)) {
            handleShare(intent);
        } else if (hasWindowFocus()) {
            readClipboard();
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus && !isShare(getIntent())) {
            readClipboard();
        }
    }

    private void readClipboard() {
        if (mDone) return;
        mDone = true;

        String text = null;
        try {
            ClipboardManager clipboard = getSystemService(ClipboardManager.class);
            ClipData clip = clipboard != null ? clipboard.getPrimaryClip() : null;
            if (clip != null && clip.getItemCount() > 0) {
                java.util.List<android.net.Uri> uris = ClipboardAttachments.uris(clip);
                if (!uris.isEmpty()) {
                    if (ClipboardAttachments.isOwnUri(this, uris.get(0))) finishQuietly();
                    else readFiles(uris);
                    return;
                }
                CharSequence value = clip.getItemAt(0).coerceToText(this);
                if (value != null) text = value.toString();
            }
        } catch (Exception e) {
            Log.w(ClipboardBridge.TAG, "Cannot read the clipboard", e);
        }

        boolean manual = ACTION_SEND_CLIPBOARD.equals(getIntent().getAction());
        boolean sent = ClipboardBridge.deliverText(text);
        Log.i(ClipboardBridge.TAG, "Clipboard read: " + (text == null ? "empty" : text.length() + " chars") + ", sent=" + sent);

        if (manual) {
            String message = text == null || text.isEmpty() ? "Clipboard is empty"
                    : (sent ? "Clipboard sent to your devices" : "Opening Clipboard to send it");
            Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
        }
        if (!sent && text != null && !text.isEmpty()) {
            openMainApp();
        }
        finishQuietly();
    }

    private void handleShare(Intent intent) {
        if (mDone) return;
        mDone = true;

        java.util.List<android.net.Uri> uris = new java.util.ArrayList<>();
        if (Intent.ACTION_SEND_MULTIPLE.equals(intent.getAction())) {
            java.util.ArrayList<android.net.Uri> shared = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
            if (shared != null) uris.addAll(shared);
        } else {
            android.net.Uri shared = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            if (shared != null) uris.add(shared);
        }
        if (uris.isEmpty()) uris.addAll(ClipboardAttachments.uris(intent.getClipData()));
        if (!uris.isEmpty()) {
            readFiles(uris);
            return;
        }

        CharSequence value = intent.getCharSequenceExtra(Intent.EXTRA_TEXT);
        String text = value != null ? value.toString() : null;
        boolean sent = ClipboardBridge.deliverText(text);
        if (sent) {
            Toast.makeText(getApplicationContext(), "Sent to your devices", Toast.LENGTH_SHORT).show();
        } else if (text != null && !text.isEmpty()) {
            // The Qt part is not running: start the app, it sends the text when ready
            openMainApp();
        }
        finishQuietly();
    }

    private static boolean isShare(Intent intent) {
        return Intent.ACTION_SEND.equals(intent.getAction()) || Intent.ACTION_SEND_MULTIPLE.equals(intent.getAction());
    }

    private void readFiles(java.util.List<android.net.Uri> uris) {
        // Keep the activity and its URI grants alive until the bounded copy finishes.
        mHandler.removeCallbacksAndMessages(null);
        ClipboardAttachments.read(this, uris, (paths, error) -> {
            if (paths == null) {
                Toast.makeText(getApplicationContext(), error, Toast.LENGTH_LONG).show();
            } else if (!ClipboardBridge.deliverFiles(paths)) {
                openMainApp();
            } else if (isShare(getIntent()) || ACTION_SEND_CLIPBOARD.equals(getIntent().getAction())) {
                Toast.makeText(getApplicationContext(), "Files ready for sync", Toast.LENGTH_SHORT).show();
            }
            finishQuietly();
        });
    }

    private void openMainApp() {
        Intent launch = getPackageManager().getLaunchIntentForPackage(getPackageName());
        if (launch != null) {
            launch.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(launch);
        }
    }

    private void finishQuietly() {
        mHandler.removeCallbacksAndMessages(null);
        if (!isFinishing()) {
            finish();
            overridePendingTransition(0, 0);
        }
    }
}
