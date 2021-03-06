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

package com.realtime.crossfire.jxclient.commands;

import com.realtime.crossfire.jxclient.gui.gui.JXCWindowRenderer;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.settings.Filenames;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;
import org.jetbrains.annotations.NotNull;

/**
 * Implements the "screenshot" command. It write the current window contents
 * into a .png file.
 * @author Andreas Kirschbaum
 */
public class ScreenshotCommand extends AbstractCommand {

    /**
     * The number of auto-created screenshot filenames. If more than this number
     * of screenshots are created, old files will be recycled.
     */
    private static final int SCREENSHOT_FILENAMES = 10;

    /**
     * A number for creating screenshot file names. It is incremented for each
     * scrrenshot.
     */
    private static int screenshotId = 0;

    /**
     * The renderer to use.
     */
    @NotNull
    private final JXCWindowRenderer windowRenderer;

    /**
     * Creates a new instance.
     * @param windowRenderer the renderer to use
     * @param crossfireServerConnection the connection instance
     */
    public ScreenshotCommand(@NotNull final JXCWindowRenderer windowRenderer, @NotNull final CrossfireServerConnection crossfireServerConnection) {
        super(crossfireServerConnection);
        this.windowRenderer = windowRenderer;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean allArguments() {
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void execute(@NotNull final String args) {
        final File file;
        if (args.length() == 0) {
            try {
                file = Filenames.getSettingsFile("screenshot"+screenshotId+".png");
            } catch (final IOException ex) {
                drawInfoError("Failed to create screenshot filename: "+ex.getMessage());
                return;
            }
            screenshotId = (screenshotId+1)%SCREENSHOT_FILENAMES;
        } else {
            file = new File(args);
        }

        final BufferedImage image = new BufferedImage(windowRenderer.getWindowWidth(), windowRenderer.getWindowHeight(), BufferedImage.TYPE_INT_RGB);
        final Graphics grfx = image.createGraphics();
        try {
            grfx.setColor(Color.black);
            grfx.fillRect(0, 0, windowRenderer.getWindowWidth(), windowRenderer.getWindowHeight());
            windowRenderer.redraw(grfx);
        } finally {
            grfx.dispose();
        }
        try {
            ImageIO.write(image, "png", file);
        } catch (final IOException ex) {
            drawInfoError("Cannot write screenshot "+file.getPath()+": "+ex.getMessage());
            return;
        } catch (final NullPointerException ex) // ImageIO.write() crashes if the destination cannot be written to
        {
            drawInfoError("Cannot write screenshot "+file.getPath());
            return;
        }

        drawInfo("Saved screenshot to "+file.getPath());
    }

}
