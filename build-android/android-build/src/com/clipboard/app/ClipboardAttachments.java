package com.clipboard.app;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.provider.OpenableColumns;
import android.util.Log;
import android.webkit.MimeTypeMap;
import androidx.core.content.FileProvider;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/** Resolves temporary content URI grants while the receiving activity is alive. */
final class ClipboardAttachments {
    private static final int MAX_BYTES = 5 * 1024 * 1024;
    private static final int MAX_FILES = 32;
    private static final ExecutorService WORKER = Executors.newSingleThreadExecutor();
    private static boolean sReading;

    interface Result { void complete(String[] paths, String error); }

    static List<Uri> uris(ClipData clip) {
        List<Uri> result = new ArrayList<>();
        if (clip != null) {
            for (int i = 0; i < clip.getItemCount(); ++i) {
                Uri uri = clip.getItemAt(i).getUri();
                if (uri != null) result.add(uri);
            }
        }
        return result;
    }

    static boolean isOwnUri(Context context, Uri uri) {
        return (context.getPackageName() + ".qtprovider").equals(uri.getAuthority());
    }

    static synchronized void read(Context context, List<Uri> uris, Result callback) {
        if (sReading) {
            callback.complete(null, "A clipboard file is already being read. Try again shortly.");
            return;
        }
        if (uris.isEmpty() || uris.size() > MAX_FILES) {
            callback.complete(null, "Select between 1 and 32 files.");
            return;
        }
        sReading = true;
        WORKER.execute(() -> {
            String[] paths = null;
            String error = null;
            try {
                ArrayList<String> files = new ArrayList<>();
                int total = 0;
                for (Uri uri : uris) {
                    if (!"content".equals(uri.getScheme()) && !"file".equals(uri.getScheme()))
                        throw new Exception("This clipboard item is not a local file.");
                    String name = "attachment";
                    if ("content".equals(uri.getScheme())) {
                        try (Cursor cursor = context.getContentResolver().query(uri,
                                new String[]{OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE}, null, null, null)) {
                            if (cursor != null && cursor.moveToFirst()) {
                                int nameColumn = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                                if (nameColumn >= 0 && !cursor.isNull(nameColumn)) name = cursor.getString(nameColumn);
                                int sizeColumn = cursor.getColumnIndex(OpenableColumns.SIZE);
                                if (sizeColumn >= 0 && !cursor.isNull(sizeColumn) && cursor.getLong(sizeColumn) > MAX_BYTES - total)
                                    throw new Exception("Photos and files must be at most 5 MB in total.");
                            }
                        }
                    } else {
                        name = new File(uri.getPath()).getName();
                    }
                    name = new File(name.replace('\\', '/')).getName().replaceAll("[\\p{Cntrl}<>:\"/\\\\|?*]", "_");
                    if (name.length() > 80) name = name.substring(name.length() - 80);
                    if (name.isEmpty() || name.equals(".") || name.equals("..")) name = "attachment";
                    String mime = context.getContentResolver().getType(uri);
                    if (mime != null && mime.startsWith("image/") && !name.contains(".")) {
                        String extension = MimeTypeMap.getSingleton().getExtensionFromMimeType(mime);
                        if (extension != null) name += "." + extension;
                    }
                    ByteArrayOutputStream bytes = new ByteArrayOutputStream();
                    try (InputStream input = context.getContentResolver().openInputStream(uri)) {
                        if (input == null) throw new Exception("Cannot open the copied file.");
                        byte[] buffer = new byte[16384];
                        int count;
                        while ((count = input.read(buffer)) != -1) {
                            total += count;
                            if (total > MAX_BYTES) throw new Exception("Photos and files must be at most 5 MB in total.");
                            bytes.write(buffer, 0, count);
                        }
                    }
                    byte[] content = bytes.toByteArray();
                    byte[] digest = MessageDigest.getInstance("SHA-256").digest(content);
                    StringBuilder hex = new StringBuilder();
                    for (byte value : digest) hex.append(String.format(java.util.Locale.ROOT, "%02x", value & 255));
                    File directory = new File(context.getCacheDir(), "clipboard-history/import_" + hex);
                    if (!directory.isDirectory() && !directory.mkdirs()) throw new Exception("Cannot save the copied file.");
                    File file = new File(directory, name);
                    if (!file.isFile() || file.length() != content.length) {
                        File temporary = File.createTempFile("import", ".tmp", directory);
                        try {
                            try (FileOutputStream output = new FileOutputStream(temporary)) { output.write(content); }
                            if (!temporary.renameTo(file)) throw new Exception("Cannot save the copied file.");
                        } finally {
                            temporary.delete();
                        }
                    }
                    files.add(file.getAbsolutePath());
                }
                paths = files.toArray(new String[0]);
            } catch (Exception e) {
                Log.w(ClipboardBridge.TAG, "Cannot read clipboard files", e);
                error = e.getMessage() == null ? "Cannot read the copied file." : e.getMessage();
            }
            synchronized (ClipboardAttachments.class) { sReading = false; }
            final String[] result = paths;
            final String failure = error;
            new Handler(Looper.getMainLooper()).post(() -> callback.complete(result, failure));
        });
    }

    static void put(Context context, String[] paths) {
        new Handler(Looper.getMainLooper()).post(() -> {
            try {
                ClipData clip = null;
                for (String path : paths) {
                    Uri uri = FileProvider.getUriForFile(context, context.getPackageName() + ".qtprovider", new File(path));
                    if (clip == null) clip = ClipData.newUri(context.getContentResolver(), "Clipboard files", uri);
                    else clip.addItem(context.getContentResolver(), new ClipData.Item(uri));
                }
                ClipboardManager clipboard = context.getSystemService(ClipboardManager.class);
                if (clipboard != null && clip != null) {
                    ClipboardBridge.markOwnClipboardWrite();
                    clipboard.setPrimaryClip(clip);
                }
            } catch (Exception e) {
                Log.w(ClipboardBridge.TAG, "Cannot copy received files", e);
                android.widget.Toast.makeText(context, "Cannot copy the received file. It is available in History.",
                        android.widget.Toast.LENGTH_LONG).show();
            }
        });
    }
}
