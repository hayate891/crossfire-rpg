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

import com.realtime.crossfire.jxclient.server.crossfire.CrossfireFaceListener;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import java.util.Arrays;
import org.jetbrains.annotations.NotNull;

/**
 * A cache for {@link Face} instances.
 *
 * @author Andreas Kirschbaum
 */
public class FaceCache
{
    /**
     * The cached faces. Empty slots are set to <code>null</code>.
     */
    @NotNull
    private final Face[] faces = new Face[65536];

    /**
     * The listener to receive face commands.
     */
    @NotNull
    private final CrossfireFaceListener crossfireFaceListener = new CrossfireFaceListener()
    {
        /** {@inheritDoc} */
        @Override
        public void faceReceived(final int faceNum, final int faceSetNum, final int faceChecksum, @NotNull final String faceName)
        {
            // XXX: ignores faceSetNum
            if (faces[faceNum] != null)
            {
                System.err.println("Warning: defining duplicate face "+faceNum+" ("+faceName+")");
            }
            faces[faceNum] = new Face(faceNum, faceName, faceChecksum);
        }
    };

    /**
     * Creates a new instance.
     */
    public FaceCache()
    {
        faces[0] = new Face(0, "empty", 0);
    }

    /**
     * Creates a new instance.
     * @param crossfireServerConnection the server connection to use
     */
    public FaceCache(@NotNull final CrossfireServerConnection crossfireServerConnection)
    {
        this();
        crossfireServerConnection.addCrossfireFaceListener(crossfireFaceListener);
    }

    /**
     * Add a new face to the cache.
     *
     * @param face The face to add.
     */
    public void addFace(@NotNull final Face face)
    {
        faces[face.getFaceNum()] = face;
    }

    /**
     * Return a face by face id.
     *
     * @param faceNum The face id to look up.
     *
     * @return the face
     */
    @NotNull
    public Face getFace(final int faceNum)
    {
        final Face face = faces[faceNum];
        if (face != null)
        {
            return face;
        }

        System.err.println("Warning: accessing undefined face "+faceNum);
        faces[faceNum] = new Face(faceNum, "face#"+faceNum, 0);
        return faces[faceNum];
    }

    /**
     * Forgets about all face information. Should be called when connecting to
     * a Crossfire server.
     */
    public void reset()
    {
        Arrays.fill(faces, null);
        faces[0] = new Face(0, "empty", 0);
    }
}
