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
package com.realtime.crossfire.jxclient.window;

public enum ConnectionStatus
{
    /**
     * Represents the unconnected status of the client, which is the first to
     * happen during a normal gaming session.
     * @since 1.0
     */
    UNCONNECTED,

    /**
     * Represents the status of the client that is used during play.
     * @since 1.0
     */
    PLAYING,

    /**
     * Represents the status of the client that is displaying a Query dialog.
     * @since 1.0
     */
    QUERY;
}
