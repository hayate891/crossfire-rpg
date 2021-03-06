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

package com.realtime.crossfire.jxclient.sound;

import java.io.IOException;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.Line;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.UnsupportedAudioFileException;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Allocates new sound clips.
 * @author Andreas Kirschbaum
 */
public class ClipCache {

    /**
     * Allocate a new clip.
     * @param name An optional prefix for the action name.
     * @param action The action name of the clip to allocate.
     * @return The new clip, or <code>null</code> if an error occurs.
     */
    @Nullable
    public static DataLine allocateClip(@Nullable final String name, @NotNull final String action) {
        return newClip(name, action);
    }

    /**
     * Deallocate a clip.
     * @param clip The clip to deallocate.
     */
    public static void freeClip(@NotNull final Line clip) {
        clip.close();
    }

    /**
     * Allocate a new clip.
     * @param name An optional prefix for the action name.
     * @param action The action name of the clip to allocate.
     * @return The new clip, or <code>null</code> if an error occurs.
     */
    @Nullable
    private static DataLine newClip(@Nullable final String name, @NotNull final String action) {
        try {
            final AudioInputStream audioInputStream = AudioSystem.getAudioInputStream(AudioFileLoader.getInputStream(name, action));
            try {
                final Clip clip = AudioSystem.getClip();
                clip.open(audioInputStream);
                return clip;
            } finally {
                audioInputStream.close();
            }
        } catch (final UnsupportedAudioFileException ex) {
            System.err.println("sound "+name+": "+ex.getMessage());
            return null;
        } catch (final LineUnavailableException ex) {
            System.err.println("sound "+name+": "+ex.getMessage());
            return null;
        } catch (final IOException ex) {
            System.err.println("sound "+name+": "+ex.getMessage());
            return null;
        }
    }

}
