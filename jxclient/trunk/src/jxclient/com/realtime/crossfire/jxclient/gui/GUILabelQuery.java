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
package com.realtime.crossfire.jxclient.gui;

import com.realtime.crossfire.jxclient.server.CrossfireQueryListener;
import com.realtime.crossfire.jxclient.server.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.window.JXCWindow;
import java.awt.Color;
import java.awt.Font;

/**
 * A {@link GUIHTMLLabel} that displays the last received "query" command.
 *
 * @author Andreas Kirschbaum
 */
public class GUILabelQuery extends GUIMultiLineLabel
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The {@link CrossfireServerConnection} to monitor.
     */
    private final CrossfireServerConnection crossfireServerConnection;

    /**
     * The {@link CrossfireQueryListener} registered to receive query commands.
     */
    private final CrossfireQueryListener crossfireQueryListener = new CrossfireQueryListener()
    {
        /** {@inheritDoc} */
        @Override
        public void commandQueryReceived(final String prompt, final int queryType)
        {
            setText(prompt);
        }
    };

    /**
     * Create a new instance.
     *
     * @param window The <code>JXCWindow</code> this element belongs to.
     *
     * @param name The name of this element.
     *
     * @param x The x-coordinate for drawing this element to screen.
     *
     * @param y The y-coordinate for drawing this element to screen.
     *
     * @param w The width for drawing this element to screen.
     *
     * @param h The height for drawing this element to screen.
     *
     * @param crossfireServerConnection the connection instance
     *
     * @param font The font to use.
     *
     * @param color The color to use.
     *
     * @param backgroundColor The background color.
     */
    public GUILabelQuery(final JXCWindow window, final String name, final int x, final int y, final int w, final int h, final CrossfireServerConnection crossfireServerConnection, final Font font, final Color color, final Color backgroundColor)
    {
        super(window, name, x, y, w, h, null, font, color, backgroundColor, Alignment.LEFT, "");
        this.crossfireServerConnection = crossfireServerConnection;
        this.crossfireServerConnection.addCrossfireQueryListener(crossfireQueryListener);
    }

    /** {@inheritDoc} */
    @Override
    public void dispose()
    {
        super.dispose();
        crossfireServerConnection.removeCrossfireQueryListener(crossfireQueryListener);
    }
}
