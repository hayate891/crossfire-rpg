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

package com.realtime.crossfire.jxclient.server.socket;

import java.nio.ByteBuffer;
import org.jetbrains.annotations.NotNull;

/**
 * Interface for listeners interested in {@link ClientSocket} related events.
 * @author Andreas Kirschbaum
 */
public interface ClientSocketListener {

    /**
     * Called when connecting to a Crossfire server.
     */
    void connecting();

    /**
     * Called after a connection to a Crossfire server has been established.
     */
    void connected();

    /**
     * Called whenever a packet is received from the Crossfire server.
     * @param packet the packet contents
     * @throws UnknownCommandException if the packet cannot be processed
     */
    void packetReceived(@NotNull ByteBuffer packet) throws UnknownCommandException;

    /**
     * Called whenever a packet has been sent to the Crossfire server.
     * @param buf the packet contents; must not be modified by client code
     * @param len the length of the packet
     */
    void packetSent(@NotNull byte[] buf, int len);

    /**
     * Called when the connection is being teared down.
     * @param reason the disconnect reason
     * @param isError whether the disconnect is unexpected
     */
    void disconnecting(@NotNull String reason, boolean isError);

    /**
     * Called after the connection has been closed.
     * @param reason the disconnect reason
     */
    void disconnected(@NotNull String reason);

}
