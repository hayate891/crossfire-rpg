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

package com.realtime.crossfire.jxclient.server.crossfire;

import java.util.EventListener;
import org.jetbrains.annotations.NotNull;

/**
 * Interface for listeners interested in "sound" commands.
 * @author Andreas Kirschbaum
 */
public interface CrossfireSoundListener extends EventListener {

    /**
     * Type for "living sound (moving, dying, ...)".
     */
    int TYPE_LIVING = 1;

    /**
     * Type for "spell casting sound."
     */
    int TYPE_SPELL = 2;

    /**
     * Type for "item sound (potion, weapon ...)."
     */
    int TYPE_ITEM = 3;

    /**
     * Type for "ground sound (door, trap opening, ...)".
     */
    int TYPE_GROUND = 4;

    /**
     * Type for "hit something".
     */
    int TYPE_HIT = 5;

    /**
     * Type for "hit by something".
     */
    int TYPE_HIT_BY = 6;

    /**
     * A sound command has been received.
     * @param x the x-coordinate relative to the player
     * @param y the y-coordinate relative to the player
     * @param num the sound number
     * @param type the sound type
     */
    void commandSoundReceived(int x, int y, int num, int type);

    /**
     * A sound2 command has been received.
     * @param x the x-coordinate relative to the player
     * @param y the y-coordinate relative to the player
     * @param dir the direction of the sound
     * @param volume the volume of the sound
     * @param type the sound type
     * @param action the action name
     * @param name the sound name
     */
    void commandSound2Received(int x, int y, int dir, int volume, int type, @NotNull String action, @NotNull String name);

}
