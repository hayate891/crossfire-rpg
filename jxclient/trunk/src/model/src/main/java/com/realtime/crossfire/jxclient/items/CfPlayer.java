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

package com.realtime.crossfire.jxclient.items;

import com.realtime.crossfire.jxclient.faces.Face;
import org.jetbrains.annotations.NotNull;

/**
 * A {@link CfItem} that represents a character.
 * @author Lauwenmark
 */
public class CfPlayer extends CfItem {

    /**
     * Creates a new instance.
     * @param tag the character's item tag
     * @param weight the character's weight
     * @param face the character's face
     * @param name the character's name
     */
    public CfPlayer(final int tag, final int weight, @NotNull final Face face, @NotNull final String name) {
        super(0, tag, 0, weight, face, name, name, 0, 0, 1, -1);
    }

}
