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

package com.realtime.crossfire.jxclient.faces;

import java.util.HashMap;
import java.util.Map;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * A factory for creating {@link FacesProvider} instances.
 * @author Andreas Kirschbaum
 */
public class FacesProviderFactory {

    /**
     * The defined {@link FacesProvider}s. Maps face size in pixels to faces
     * provider instance.
     */
    @NotNull
    private final Map<Integer, FacesProvider> facesProviders = new HashMap<Integer, FacesProvider>();

    /**
     * Creates a new instance.
     * @param facesManager the faces manager to quey
     */
    public FacesProviderFactory(@NotNull final FacesManager facesManager) {
        add(new MagicMapFacesProvider(facesManager));
        add(new OriginalFacesProvider(facesManager));
        add(new ScaledFacesProvider(facesManager));
    }

    /**
     * Adds a {@link FacesProvider} to {@link #facesProviders}.
     * @param facesProvider the faces provider to add
     */
    private void add(@NotNull final FacesProvider facesProvider) {
        final int size = facesProvider.getSize();
        if (facesProviders.containsKey(size)) {
            throw new IllegalArgumentException();
        }
        facesProviders.put(size, facesProvider);
    }

    /**
     * Returns the {@link FacesProvider} for a given face size.
     * @param size the face size in pixels
     * @return the faces providers
     */
    @Nullable
    public FacesProvider getFacesProvider(final int size) {
        return facesProviders.get(size);
    }

}
