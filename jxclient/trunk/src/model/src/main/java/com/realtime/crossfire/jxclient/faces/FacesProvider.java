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
 * Interface for face providers.
 * @author Andreas Kirschbaum
 */
public interface FacesProvider {

    /**
     * Returns the size of faces in pixels.
     * @return the size in pixels
     */
    int getSize();

    /**
     * Returns the face for a face ID. This function returns immediately even if
     * the face is not loaded. A not loaded face will be updated as soon as
     * loading has finished.
     * @param faceNum the face ID
     * @param isUnknownImage returns whether the returned face is the "unknown"
     * face; ignored if {@code null}
     * @return the face, or the "unknown" face if the face is not loaded
     */
    @NotNull
    ImageIcon getImageIcon(int faceNum, @Nullable boolean[] isUnknownImage);

}
