//
// This file is part of JXClient, the Fullscreen Java Crossfire Client.
//
//    JXClient is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    JXClient is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with JXClient; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// JXClient is (C)2005 by Yann Chachkoff.
//
package com.realtime.crossfire.jxclient.server;

/**
 * Interface for clients processing received packets.
 * @author Andreas Kirschbaum
 */
public interface PacketListener
{
    /**
     * Called once after the connection has been established and before the
     * first {@link #processPacket(byte[], int, int)} is called.
     */
    void connected();

    /**
     * Processes a received packet. This passed buffer must not be modified
     * except for the packet range.
     * @param packet the buffer holding the packet
     * @param start the start index of the packet
     * @param end the end of the packet data
     * @throws UnknownCommandException if the packet cannot be parsed
     */
    void processPacket(byte[] packet, int start, int end) throws UnknownCommandException;
}
