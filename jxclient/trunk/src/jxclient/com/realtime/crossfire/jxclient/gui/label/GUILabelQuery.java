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

package com.realtime.crossfire.jxclient.gui.label;

import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireQueryListener;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import java.awt.Color;
import java.awt.Font;
import org.jetbrains.annotations.NotNull;

/**
 * A {@link GUIHTMLLabel} that displays the last received "query" command.
 * @author Andreas Kirschbaum
 */
public class GUILabelQuery extends GUIMultiLineLabel {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The {@link CrossfireServerConnection} to monitor.
     */
    @NotNull
    private final CrossfireServerConnection crossfireServerConnection;

    /**
     * The {@link CrossfireQueryListener} registered to receive query commands.
     */
    @NotNull
    private final CrossfireQueryListener crossfireQueryListener = new CrossfireQueryListener() {
        /** {@inheritDoc} */
        @Override
        public void commandQueryReceived(@NotNull final String prompt, final int queryType) {
            setText(prompt);
        }
    };

    /**
     * Create a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param name The name of this element.
     * @param x The x-coordinate for drawing this element to screen.
     * @param y The y-coordinate for drawing this element to screen.
     * @param w The width for drawing this element to screen.
     * @param h The height for drawing this element to screen.
     * @param crossfireServerConnection the connection instance
     * @param font The font to use.
     * @param color The color to use.
     * @param backgroundColor The background color.
     */
    public GUILabelQuery(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, final int x, final int y, final int w, final int h, @NotNull final CrossfireServerConnection crossfireServerConnection, @NotNull final Font font, @NotNull final Color color, @NotNull final Color backgroundColor) {
        super(tooltipManager, elementListener, name, x, y, w, h, null, font, color, backgroundColor, Alignment.LEFT, "");
        this.crossfireServerConnection = crossfireServerConnection;
        this.crossfireServerConnection.addCrossfireQueryListener(crossfireQueryListener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void dispose() {
        super.dispose();
        crossfireServerConnection.removeCrossfireQueryListener(crossfireQueryListener);
    }

}
