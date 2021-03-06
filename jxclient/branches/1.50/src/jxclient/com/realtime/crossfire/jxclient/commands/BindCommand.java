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

import com.realtime.crossfire.jxclient.gui.commands.CommandCallback;
import com.realtime.crossfire.jxclient.gui.commands.CommandList;
import com.realtime.crossfire.jxclient.gui.commands.CommandListType;
import com.realtime.crossfire.jxclient.gui.commands.GUICommandFactory;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.util.StringUtils;
import org.jetbrains.annotations.NotNull;

/**
 * Implements the "bind" command. It associates a key with a command.
 * @author Andreas Kirschbaum
 */
public class BindCommand extends AbstractCommand {

    /**
     * The commands instance for executing commands.
     */
    @NotNull
    private final Commands commands;

    /**
     * The {@link CommandCallback} to use.
     */
    @NotNull
    private final CommandCallback commandCallback;

    /**
     * The {@link Macros} instance to use.
     */
    @NotNull
    private final Macros macros;

    /**
     * Creates a new instance.
     * @param crossfireServerConnection the connection instance
     * @param commands the commands instance for executing commands
     * @param commandCallback the command callback to use
     * @param macros the macros instance to use
     */
    public BindCommand(@NotNull final CrossfireServerConnection crossfireServerConnection, @NotNull final Commands commands, @NotNull final CommandCallback commandCallback, @NotNull final Macros macros) {
        super(crossfireServerConnection);
        this.commands = commands;
        this.commandCallback = commandCallback;
        this.macros = macros;
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
        final String commandList;
        final boolean perCharacterBinding;
        if (args.equals("-c")) {
            perCharacterBinding = true;
            commandList = "";
        } else if (args.startsWith("-c ")) {
            perCharacterBinding = true;
            commandList = StringUtils.trimLeading(args.substring(3));
        } else {
            perCharacterBinding = false;
            commandList = args;
        }

        if (commandList.length() == 0) {
            drawInfoError("Which command do you want to bind?");
            return;
        }

        final CommandList commandList2 = new CommandList(CommandListType.AND);
        commandList2.add(GUICommandFactory.createCommand(commandList, commandCallback, commands, macros));
        if (!commandCallback.createKeyBinding(perCharacterBinding, commandList2)) {
            drawInfoError("Cannot use bind -c since no character is logged in.");
            return;
        }
    }

}
