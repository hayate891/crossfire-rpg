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
 * Copyright (C) 2006-2011 Andreas Kirschbaum.
 */

package com.realtime.crossfire.jxclient.gui.commands;

import com.realtime.crossfire.jxclient.commands.Commands;
import com.realtime.crossfire.jxclient.commands.Macros;
import com.realtime.crossfire.jxclient.util.StringUtils;
import java.util.regex.Pattern;
import org.jetbrains.annotations.NotNull;

/**
 * Factory for creating {@link GUICommand} instances from string
 * representation.
 * @author Andreas Kirschbaum
 */
public class GUICommandFactory {

    /**
     * Pattern matching lines that need a {@link #TRAILING_ESCAPE} appended.
     */
    @NotNull
    private static final Pattern PATTERN_ENCODE = Pattern.compile(".*[- \t]$");

    /**
     * Character appended to lines ending with whitespace.
     */
    @NotNull
    private static final String TRAILING_ESCAPE = "-";

    /**
     * Private constructor to prevent instantiation.
     */
    private GUICommandFactory() {
    }

    /**
     * Create a new {@link GUICommand} instance from string representation.
     * @param encodedCommandString the command string representation
     * @param commandCallback the command callback to use
     * @param commands the commands instance to use
     * @param macros the macros instance to use
     * @return the new command instance
     */
    public static GUICommand createCommandDecode(@NotNull final String encodedCommandString, @NotNull final CommandCallback commandCallback, @NotNull final Commands commands, @NotNull final Macros macros) {
        return createCommand(decode(encodedCommandString), commandCallback, commands, macros);
    }

    /**
     * Create a new {@link GUICommand} instance from string representation.
     * @param commandString the command string representation
     * @param commandCallback the command callback to use
     * @param commands the commands instance to use
     * @param macros the macros instance to use
     * @return the new command instance
     */
    @NotNull
    public static GUICommand createCommand(@NotNull final String commandString, @NotNull final CommandCallback commandCallback, @NotNull final Commands commands, @NotNull final Macros macros) {
        if (commandString.equals("-e")) {
            return new ActivateCommandInputCommand("", commandCallback, macros);
        } else if (commandString.startsWith("-e ")) {
            return new ActivateCommandInputCommand(StringUtils.trimLeading(commandString.substring(3)), commandCallback, macros);
        } else {
            return new ExecuteCommandCommand(commands, commandString, macros);
        }
    }

    /**
     * Encodes a key binding if necessary.
     * @param command the key binding
     * @return the encoded key binding
     */
    @NotNull
    public static String encode(@NotNull final String command) {
        return PATTERN_ENCODE.matcher(command).matches() ? command+TRAILING_ESCAPE : command;
    }

    /**
     * Decodes a key binding if necessary.
     * @param command the key binding
     * @return the decoded key binding
     */
    @NotNull
    private static String decode(@NotNull final String command) {
        return command.endsWith(TRAILING_ESCAPE) ? command.substring(0, command.length()-TRAILING_ESCAPE.length()) : command;
    }

}
