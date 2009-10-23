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
package com.realtime.crossfire.jxclient.gui.textinput;

import com.realtime.crossfire.jxclient.commands.Commands;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.window.GuiManager;
import com.realtime.crossfire.jxclient.window.JXCWindowRenderer;
import java.awt.Color;
import java.awt.Font;
import java.awt.Image;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class GUICommandText extends GUIText
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    private final Commands commands;

    public GUICommandText(final GuiManager guiManager, final TooltipManager tooltipManager, final JXCWindowRenderer windowRenderer, final String name, final int x, final int y, final int w, final int h, final Image activeImage, final Image inactiveImage, final Font font, final Color inactiveColor, final Color activeColor, final int margin, final String text, final Commands commands, final boolean ignoreUpDown)
    {
        super(guiManager, tooltipManager, windowRenderer, name, x, y, w, h, activeImage, inactiveImage, font, inactiveColor, activeColor, margin, text, ignoreUpDown);
        this.commands = commands;
    }

    /** {@inheritDoc} */
    @Override
    public void dispose()
    {
        super.dispose();
    }

    /** {@inheritDoc} */
    @Override
    protected void execute(final String command)
    {
        commands.executeCommand(command);
        setText("");
    }
}
