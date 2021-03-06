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

import org.jetbrains.annotations.NotNull;

/**
 * An entry in the cache. It consists of a {@link MetaserverEntry} and a
 * timestamp of last update.
 * @author Andreas Kirschbaum
 */
public class Info {

    /**
     * The metaserver entry.
     */
    @NotNull
    private final MetaserverEntry metaserverEntry;

    /**
     * The timestamp of last update.
     */
    private final long timestamp;

    /**
     * Creates a new instance. Sets the timestamp to "now".
     * @param metaserverEntry the metaserver entry
     */
    public Info(@NotNull final MetaserverEntry metaserverEntry) {
        this(metaserverEntry, System.currentTimeMillis());
    }

    /**
     * Creates a new instance.
     * @param metaserverEntry the metaserver entry
     * @param timestamp the timestamp
     */
    public Info(@NotNull final MetaserverEntry metaserverEntry, final long timestamp) {
        this.metaserverEntry = metaserverEntry;
        this.timestamp = timestamp;
    }

    /**
     * Returns the metaserver entry.
     * @return the metaserver entry
     */
    @NotNull
    public MetaserverEntry getMetaserverEntry() {
        return metaserverEntry;
    }

    /**
     * Returns the timestamp.
     * @return the timestamp
     */
    public long getTimestamp() {
        return timestamp;
    }

}
