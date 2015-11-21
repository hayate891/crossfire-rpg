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
 * Copyright (C) 2006-2011 Andreas Kirschbaum.
 */

package com.realtime.crossfire.jxclient.metaserver;

import com.realtime.crossfire.jxclient.util.NumberParser;
import java.io.IOException;
import java.util.regex.Pattern;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Parser for response lines of metaserver response lines.
 * @author Andreas Kirschbaum
 */
public class MetaserverEntryParser {

    /**
     * The default server version if none specified.
     */
    @NotNull
    private static final String UNKNOWN_VERSION = "?";

    /**
     * The default for arch base, map base, and code base if none specified.
     */
    @NotNull
    private static final String DEFAULT_BASE = "not specified";

    /**
     * The pattern to split a metaserver response line into fields.
     */
    @NotNull
    private static final Pattern FIELD_SEPARATOR_PATTERN = Pattern.compile("\\|");

    /**
     * Whether response parsing is withing a server entry.
     */
    private boolean inSection;

    /**
     * The "update seconds" value for the current server entry.
     */
    private int updateSeconds;

    /**
     * The "hostname" value for the current server entry.
     */
    @Nullable
    private String hostname;

    /**
     * The "players" value for the current server entry.
     */
    private int players;

    /**
     * The "server version" value for the current server entry.
     */
    @NotNull
    private String version = UNKNOWN_VERSION;

    /**
     * The "comment" value for the current server entry. html_command is
     * preferred over text_comment.
     */
    @NotNull
    private String comment = "";

    /**
     * The "bytes in" value for the current server entry.
     */
    private long bytesIn;

    /**
     * The "bytes out" value for the current server entry.
     */
    private long bytesOut;

    /**
     * The "uptime seconds" value for the current server entry.
     */
    private int uptimeSeconds;

    /**
     * The "archetype base" value for the current server entry.
     */
    @NotNull
    private String archBase = DEFAULT_BASE;

    /**
     * The "map base" value for the current server entry.
     */
    @NotNull
    private String mapBase = DEFAULT_BASE;

    /**
     * The "code base" value for the current server entry.
     */
    @NotNull
    private String codeBase = DEFAULT_BASE;

    /**
     * Creates a new instance.
     */
    public MetaserverEntryParser() {
        clear();
    }

    /**
     * Parses a metaserver response line.
     * @param entry the response line to parse
     * @return the metaserver entry or {@code null} if the line is invalid
     */
    @Nullable
    public static MetaserverEntry parseEntry(@NotNull final CharSequence entry) {
        final String[] entries = FIELD_SEPARATOR_PATTERN.split(entry, -1);
        if (entries.length != 11) {
            return null;
        }

        final int updateSeconds;
        final String hostname;
        final int players;
        final String version;
        final String comment;
        final long bytesIn;
        final long bytesOut;
        final int uptimeSeconds;
        final String archBase;
        final String mapBase;
        final String codeBase;
        try {
            updateSeconds = Integer.parseInt(entries[0]);
            hostname = entries[1];
            players = Integer.parseInt(entries[2]);
            version = entries[3];
            comment = entries[4];
            bytesIn = Long.parseLong(entries[5]);
            bytesOut = Long.parseLong(entries[6]);
            uptimeSeconds = Integer.parseInt(entries[7]);
            archBase = entries[8];
            codeBase = entries[9];
            mapBase = entries[10];
        } catch (final NumberFormatException ignored) {
            return null;
        }

        return new MetaserverEntry(updateSeconds, hostname, players, version, comment, bytesIn, bytesOut, uptimeSeconds, archBase, codeBase, mapBase);
    }

    /**
     * Parses a metaserver response line.
     * @param line the response line to parse
     * @return the metaserver entry, or {@code null} if the line is invalid
     * @throws IOException if the response line is invalid
     */
    @Nullable
    public MetaserverEntry parseLine(@NotNull final String line) throws IOException {
        if (inSection) {
            if (line.equals("END_SERVER_DATA")) {
                @Nullable final MetaserverEntry metaserverEntry;
                if (hostname == null) {
                    System.err.println("Warning: metaserver response missing hostname field, skipping");
                    metaserverEntry = null;
                } else {
                    metaserverEntry = new MetaserverEntry(updateSeconds, hostname, players, version, comment, bytesIn, bytesOut, uptimeSeconds, archBase, mapBase, codeBase);
                }
                clear();
                inSection = false;
                return metaserverEntry;
            } else {
                final String[] tmp = line.split("=", 2);
                if (tmp.length == 2) {
                    final String key = tmp[0];
                    final String value = tmp[1];
                    if (key.equals("hostname")) {
                        hostname = value;
                    } else if (key.equals("port")) {
                    } else if (key.equals("html_comment")) {
                        comment = value;
                    } else if (key.equals("text_comment")) {
                        if (comment.length() == 0) {
                            comment = value;
                        }
                    } else if (key.equals("archbase")) {
                        archBase = value;
                    } else if (key.equals("mapbase")) {
                        mapBase = value;
                    } else if (key.equals("codebase")) {
                        codeBase = value;
                    } else if (key.equals("num_players")) {
                        players = NumberParser.parseInt(value, 0);
                    } else if (key.equals("in_bytes")) {
                        bytesIn = NumberParser.parseLong(value, 0);
                    } else if (key.equals("out_bytes")) {
                        bytesOut = NumberParser.parseLong(value, 0);
                    } else if (key.equals("uptime")) {
                        uptimeSeconds = NumberParser.parseInt(value, 0);
                    } else if (key.equals("version")) {
                        version = value;
                    } else if (key.equals("sc_version")) {
                    } else if (key.equals("cs_version")) {
                    } else if (key.equals("last_update")) {
                        final long now = (System.currentTimeMillis()+500)/1000;
                        final long uptime = NumberParser.parseLong(value, now);
                        updateSeconds = Math.max((int)((uptime-now)/1000), 0);
                    } else if (key.equals("flags")) {
                    } else {
                        System.err.println("Ignoring unknown key: "+key);
                    }
                } else {
                    throw new IOException("syntax error: "+line);
                }
            }
        } else {
            if (line.equals("START_SERVER_DATA")) {
                inSection = true;
            } else {
                throw new IOException("syntax error: "+line);
            }
        }

        return null;
    }

    /**
     * Resets values for the current server entry. Will be called whenever
     * parsing of a new entry starts.
     */
    private void clear() {
        updateSeconds = 0;
        hostname = null;
        players = 0;
        version = UNKNOWN_VERSION;
        comment = "";
        bytesIn = 0;
        bytesOut = 0;
        uptimeSeconds = 0;
        archBase = DEFAULT_BASE;
        mapBase = DEFAULT_BASE;
        codeBase = DEFAULT_BASE;
    }

    /**
     * Formats a metaserver entry that returns the metaserver entry when parse
     * with {@link #parseEntry(CharSequence)}.
     * @param entry the metaserver entry to format
     * @return the formatted entry
     */
    @NotNull
    public static String format(@NotNull final MetaserverEntry entry) {
        return entry.getUpdateSeconds()+"|"+replace(entry.getHostname())+"|"+entry.getPlayers()+"|"+replace(entry.getVersion())+"|"+replace(entry.getComment())+"|"+entry.getBytesIn()+"|"+entry.getBytesOut()+"|"+entry.getUptimeSeconds()+"|"+replace(entry.getArchBase())+"|"+replace(entry.getCodeBase())+"|"+replace(entry.getMapBase());
    }

    /**
     * Replaces characters with may cause parsing issues.
     * @param str the string to replace
     * @return the replaced string
     */
    @NotNull
    private static String replace(@NotNull final String str) {
        return str.replaceAll("[\\|\r\n]", " ");
    }

}
