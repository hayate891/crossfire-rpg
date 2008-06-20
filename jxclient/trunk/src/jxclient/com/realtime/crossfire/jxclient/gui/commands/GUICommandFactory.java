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
package com.realtime.crossfire.jxclient.gui.commands;

import com.realtime.crossfire.jxclient.commands.Commands;
import com.realtime.crossfire.jxclient.util.StringUtils;
import com.realtime.crossfire.jxclient.window.JXCWindow;

/**
 * Factory for creating {@link GUICommand} instances from string
 * representation.
 * @author Andreas Kirschbaum
 */
public class GUICommandFactory
{
    /**
     * Private constructor to prevent instantiation.
     */
    private GUICommandFactory()
    {
    }

    /**
     * Create a new {@link GUICommand} instance from string representation.
     * @param commandString the command string representation
     * @param window the window instance to use
     * @param commands the commands instance to use
     * @return the new command instance
     */
    public static GUICommand createCommand(final String commandString, final JXCWindow window, final Commands commands)
    {
        if (commandString.equals("-e"))
        {
            return new ActivateCommandInputCommand("", window);
        }
        else if (commandString.startsWith("-e "))
        {
            return new ActivateCommandInputCommand(StringUtils.trimLeading(commandString.substring(3)), window);
        }
        else
        {
            return new ExecuteCommandCommand(commands, commandString);
        }
    }
}
