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

import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.window.GUICommandList;
import com.realtime.crossfire.jxclient.window.GuiManager;
import com.realtime.crossfire.jxclient.window.JXCWindowRenderer;
import java.awt.Color;
import java.awt.Font;
import java.awt.image.BufferedImage;

/**
 * A text input field which executes a {@link GUICommandList} when ENTER is
 * pressed.
 *
 * @author Andreas Kirschbaum
 */
public class GUITextField extends GUIText
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    private final GUICommandList commandList;

    public GUITextField(final GuiManager guiManager, final TooltipManager tooltipManager, final JXCWindowRenderer windowRenderer, final String name, final int x, final int y, final int w, final int h, final BufferedImage activePicture, final BufferedImage inactivePicture, final Font font, final Color inactiveColor, final Color activeColor, final int margin, final String text, final GUICommandList commandList, final boolean ignoreUpDown)
    {
        super(guiManager, tooltipManager, windowRenderer, name, x, y, w, h, activePicture, inactivePicture, font, inactiveColor, activeColor, margin, text, ignoreUpDown);
        if (commandList == null) throw new IllegalArgumentException();
        this.commandList = commandList;
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
        commandList.execute();
    }
}
