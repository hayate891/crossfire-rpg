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

package com.realtime.crossfire.jxclient.faces;

import javax.swing.ImageIcon;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Interface for {@link ImageIcon} caching classes.
 * @author Andreas Kirschbaum
 */
public interface ImageCache {

    /**
     * Retrieves an image from the cache.
     * @param face the face to retrieve
     * @return the image icon, or {@code null} if the cache does not contain
     * the image
     */
    @Nullable
    ImageIcon load(@NotNull Face face);

    /**
     * Stores an {@link ImageIcon} into the cache.
     * @param face the face to save
     * @param imageIcon the image icon to store
     */
    void save(@NotNull Face face, @NotNull ImageIcon imageIcon);

}
