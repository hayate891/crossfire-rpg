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

/**
 * A {@link GUICommand} which executes a Crossfire command.
 * @author Andreas Kirschbaum
 */
public class ExecuteCommandCommand implements GUICommand
{
    /** The commands instance for executing the command. */
    private final Commands commands;

    /** The command to execute. */
    private final String command;

    /**
     * Creates a new instance.
     * @param commands the commands instance for executing the command
     * @param command the command to execute
     */
    public ExecuteCommandCommand(final Commands commands, final String command)
    {
        this.commands = commands;
        this.command = command;
    }

    /** {@inheritDoc} */
    @Override
    public boolean canExecute()
    {
        return true;
    }

    /** {@inheritDoc} */
    @Override
    public void execute()
    {
        commands.executeCommand(command);
    }

    /**
     * Returns the command to execute.
     * @return the command to execute
     */
    public String getCommand()
    {
        return command;
    }
}
