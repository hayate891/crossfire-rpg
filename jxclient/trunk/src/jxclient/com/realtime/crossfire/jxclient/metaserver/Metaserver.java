/*
 * This file is part of JXClient, the Fullscreen Java Crossfire Client.
 *
 * JXClient is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * JXClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JXClient; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Copyright (C) 2005-2008 Yann Chachkoff.
 * Copyright (C) 2006-2010 Andreas Kirschbaum.
 */

package com.realtime.crossfire.jxclient.metaserver;

import com.realtime.crossfire.jxclient.guistate.GuiStateListener;
import com.realtime.crossfire.jxclient.guistate.GuiStateManager;
import com.realtime.crossfire.jxclient.server.socket.ClientSocketState;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Map;
import java.util.Properties;
import org.jetbrains.annotations.NotNull;

/**
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class Metaserver {

    /**
     * The time (in seconds) to forget about old metaserver entries.
     */
    private static final long EXPIRE_INTERVAL = 60L*60*24*2;

    /**
     * The metaserver URL.
     */
    @NotNull
    private static final String METASERVER_URL = "http://crossfire.real-time.com/metaserver2/meta_client.php";

    /**
     * The {@link MetaserverProcessor} used for metaserver queries.
     */
    @NotNull
    private final MetaserverProcessor metaserverProcessor = new MetaserverProcessor(this);

    /**
     * The cached metaserver entries.
     */
    @NotNull
    private final ServerCache serverCache;

    /**
     * The {@link MetaserverModel} instance to update.
     */
    @NotNull
    private final MetaserverModel metaserverModel;

    /**
     * The {@link GuiStateListener} for detecting established or dropped
     * connections.
     */
    @NotNull
    private final GuiStateListener guiStateListener = new GuiStateListener() {
        /** {@inheritDoc} */
        @Override
        public void start() {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void metaserver() {
            metaserverProcessor.query();
        }

        /** {@inheritDoc} */
        @Override
        public void preConnecting(@NotNull final String serverInfo) {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void connecting(@NotNull final String serverInfo) {
            metaserverProcessor.disable();
        }

        /** {@inheritDoc} */
        @Override
        public void connecting(@NotNull final ClientSocketState clientSocketState) {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void connected() {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void connectFailed(@NotNull final String reason) {
            // ignore
        }
    };

    /**
     * Create a new instance.
     * @param metaserverCacheFile The metaserver cache file.
     * @param metaserverModel the metaserver model instance to update
     * @param guiStateManager the gui state manager to watch
     */
    public Metaserver(@NotNull final File metaserverCacheFile, @NotNull final MetaserverModel metaserverModel, @NotNull final GuiStateManager guiStateManager) {
        serverCache = new ServerCache(metaserverCacheFile);
        this.metaserverModel = metaserverModel;
        metaserverModel.begin();
        for (final MetaserverEntry metaserverEntry : serverCache.getAll().values()) {
            metaserverModel.add(metaserverEntry);
        }
        metaserverModel.commit();
        guiStateManager.addGuiStateListener(guiStateListener);
        metaserverProcessor.query();
    }

    /**
     * Update the contents of {@link #metaserverModel}.
     */
    public void updateMetaList() {
        metaserverModel.begin();

        serverCache.expire(EXPIRE_INTERVAL*1000);
        final Map<String, MetaserverEntry> oldEntries = serverCache.getAll();

        final MetaserverEntry localhostMetaserverEntry = MetaserverEntryParser.parseEntry(ServerCache.DEFAULT_ENTRY_LOCALHOST);
        assert localhostMetaserverEntry != null;
        metaserverModel.add(localhostMetaserverEntry);
        oldEntries.remove(ServerCache.makeKey(localhostMetaserverEntry));
        serverCache.put(localhostMetaserverEntry);

        try {
            final URL url = new URL(METASERVER_URL);
            final String httpProxy = System.getenv("http_proxy");
            if (httpProxy != null && httpProxy.length() > 0) {
                if (httpProxy.regionMatches(true, 0, "http://", 0, 7)) {
                    final String[] tmp = httpProxy.substring(7).replaceAll("/.*", "").split(":", 2);
                    final String proxy = tmp[0];
                    final String port = tmp.length >= 2 ? tmp[1] : "80";
                    final Properties systemProperties = System.getProperties();
                    systemProperties.setProperty("http.proxyHost", proxy);
                    systemProperties.setProperty("http.proxyPort", port);
                } else {
                    System.err.println("Warning: unsupported http_proxy protocol: "+httpProxy);
                }
            }
            final HttpURLConnection conn = (HttpURLConnection)url.openConnection();
            try {
                conn.setRequestMethod("GET");
                conn.setUseCaches(false);
                conn.connect();
                if (conn.getResponseCode() == HttpURLConnection.HTTP_OK) {
                    final InputStream in = conn.getInputStream();
                    final InputStreamReader isr = new InputStreamReader(in, "ISO-8859-1");
                    try {
                        final BufferedReader br = new BufferedReader(isr);
                        try {
                            final MetaserverEntryParser metaserverEntryParser = new MetaserverEntryParser();
                            for (; ;) {
                                final String line = br.readLine();
                                if (line == null) {
                                    break;
                                }

                                final MetaserverEntry metaserverEntry = metaserverEntryParser.parseLine(line);
                                if (metaserverEntry != null) {
                                    metaserverModel.add(metaserverEntry);
                                    oldEntries.remove(ServerCache.makeKey(metaserverEntry));
                                    serverCache.put(metaserverEntry);
                                }
                            }
                        } finally {
                            br.close();
                        }
                    } finally {
                        isr.close();
                    }
                }
            } finally {
                conn.disconnect();
            }
        } catch (final IOException ex) {
            // ignore (but keep already parsed entries)
        }

        // add previously known entries that are not anymore present
        for (final MetaserverEntry metaserverEntry : oldEntries.values()) {
            metaserverModel.add(metaserverEntry);
        }

        metaserverModel.commit();
        serverCache.save();
    }

}
