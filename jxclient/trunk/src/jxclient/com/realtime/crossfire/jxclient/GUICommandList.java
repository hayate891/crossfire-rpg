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
package com.realtime.crossfire.jxclient;

import com.realtime.crossfire.jxclient.gui.commands.ExecuteCommandCommand;
import com.realtime.crossfire.jxclient.gui.commands.GUICommand;
import java.util.ArrayList;
import java.util.List;

/**
 * A list of {@link GUICommand} instances.
 *
 * @author Andreas Kirschbaum
 */
public class GUICommandList
{
    /**
     * The command list type.
     */
    public enum Type
    {
        /** List is executed if all entries can execute. */
        AND,

        /** List is executed if any entry can execute. */
        OR,
    }

    /**
     * The command list type.
     */
    private final Type type;

    /**
     * The list of {@link GUICommand}s in execution order.
     */
    private final List<GUICommand> commands = new ArrayList<GUICommand>();

    /**
     * Create a new instance as an empty command list.
     *
     * @param type The command list type.
     */
    public GUICommandList(final Type type)
    {
        this.type = type;
    }

    /**
     * Create a new instance from a string. The passed string contains a list
     * of commands to be sent to the server. The commands are separated by ';'
     * characters.
     *
     * @param type The command list type.
     *
     * @param commands The commands.
     *
     * @param jxcWindow The window to execute the commands in.
     */
    public GUICommandList(final Type type, final String commands, final JXCWindow jxcWindow)
    {
        this(type);
        this.commands.add(new ExecuteCommandCommand(jxcWindow, commands));
    }

    /**
     * Add a command to the end of this command list.
     *
     * @param guiCommand The command to add.
     */
    public void add(final GUICommand guiCommand)
    {
        commands.add(guiCommand);
    }

    /**
     * Execute the command list by calling {@link GUICommand#execute()} for
     * each command in order.
     */
    public void execute()
    {
        switch (type)
        {
        case AND:
            for (final GUICommand command : commands)
            {
                if (!command.canExecute())
                {
                    return;
                }
            }
            break;

        case OR:
            boolean ok = false;
            for (final GUICommand command : commands)
            {
                if (command.canExecute())
                {
                    ok = true;
                    break;
                }
            }
            if (!ok)
            {
                return;
            }
            break;
        }

        for (final GUICommand command : commands)
        {
            command.execute();
        }
    }

    /**
     * Return the commands as a string.
     *
     * @return The commands as a string.
     */
    public String getCommandString()
    {
        final StringBuilder sb = new StringBuilder();
        boolean firstCommand = true;
        for (final GUICommand guiCommand : commands)
        {
            if (!(guiCommand instanceof ExecuteCommandCommand))
            {
                throw new AssertionError("Cannot encode command of type "+guiCommand.getClass().getName());
            }

            if (firstCommand)
            {
                firstCommand = false;
            }
            else
            {
                sb.append(';');
            }
            sb.append(((ExecuteCommandCommand)guiCommand).getCommand());
        }
        return sb.toString();
    }
}
