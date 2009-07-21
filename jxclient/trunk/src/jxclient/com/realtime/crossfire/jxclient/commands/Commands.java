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
package com.realtime.crossfire.jxclient.commands;

import com.realtime.crossfire.jxclient.scripts.ScriptManager;
import com.realtime.crossfire.jxclient.server.CommandQueue;
import com.realtime.crossfire.jxclient.server.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.settings.options.OptionManager;
import com.realtime.crossfire.jxclient.util.Patterns;
import com.realtime.crossfire.jxclient.util.StringUtils;
import com.realtime.crossfire.jxclient.window.GuiManager;
import com.realtime.crossfire.jxclient.window.JXCWindow;
import com.realtime.crossfire.jxclient.window.JXCWindowRenderer;
import java.util.HashMap;
import java.util.Map;

/**
 * Parses and executes client-side commands.
 *
 * @author Andreas Kirschbaum
 */
public class Commands
{
    /**
     * The command queue for sending commands.
     */
    private final CommandQueue commandQueue;

    /**
     * Maps command name to {@link Command} instance.
     */
    private final Map<String, Command> commands = new HashMap<String, Command>();

    /**
     * Creates a new instance.
     * @param window the window to execute the commands in
     * @param windowRenderer the renderer to use
     * @param commandQueue the command queue for sending commands
     * @param crossfireServerConnection the connection instance
     * @param scriptManager the script manager instance
     * @param optionManager the option manager instance
     * @param guiManager the gui manager to use
     * @param macros the macros instance to use
     */
    public Commands(final JXCWindow window, final JXCWindowRenderer windowRenderer, final CommandQueue commandQueue, final CrossfireServerConnection crossfireServerConnection, final ScriptManager scriptManager, final OptionManager optionManager, final GuiManager guiManager, final Macros macros)
    {
        this.commandQueue = commandQueue;
        commands.put("bind", new BindCommand(window, crossfireServerConnection, this, guiManager, macros));
        commands.put("unbind", new UnbindCommand(window, crossfireServerConnection));
        commands.put("screenshot", new ScreenshotCommand(window, windowRenderer, crossfireServerConnection));
        commands.put("script", new ScriptCommand(scriptManager, crossfireServerConnection));
        commands.put("scriptkill", new ScriptkillCommand(scriptManager, crossfireServerConnection));
        commands.put("scriptkillall", new ScriptkillallCommand(scriptManager, crossfireServerConnection));
        commands.put("scripts", new ScriptsCommand(scriptManager, crossfireServerConnection));
        commands.put("scripttell", new ScripttellCommand(scriptManager, crossfireServerConnection));
        commands.put("exec", new ExecCommand(guiManager, crossfireServerConnection));
        commands.put("set", new SetCommand(crossfireServerConnection, optionManager));
        commands.put("clear", new ClearCommand(windowRenderer, crossfireServerConnection));
        commands.put("debug_colors", new DebugColorsCommand(crossfireServerConnection));
    }

    /**
     * Execute a command or a list of commands. The commands may be a client-
     * or a server-sided command.
     *
     * @param commands The commands to execute.
     */
    public void executeCommand(final String commands)
    {
        String cmds = StringUtils.trimLeading(commands);
        while (cmds.length() > 0)
        {
            final String[] cmd = cmds.split(" *; *", 2);
            if (execute(cmd[0], cmds))
            {
                break;
            }
            if (cmd.length <= 1)
            {
                break;
            }
            cmds = cmd[1];
        }
    }

    /**
     * Execute a client-side command.
     *
     * @param command The command.
     *
     * @param commandList The command and all remaining commands of the command
     * list.
     *
     * @return <code>true</code> if all remaining commands have been consumed.
     */
    private boolean execute(final String command, final String commandList)
    {
        if (command.length() <= 0)
        {
            return false;
        }

        final String[] args = Patterns.PATTERN_WHITESPACE.split(StringUtils.trimLeading(command), 2);
        final Command cmd = commands.get(args[0]);
        if (cmd == null)
        {
            commandQueue.sendNcom(false, command);
            return false;
        }

        if (!cmd.allArguments())
        {
            cmd.execute(args.length >= 2 ? args[1] : "");
            return false;
        }

        assert commandList.startsWith(command);
        final String[] argsList = Patterns.PATTERN_WHITESPACE.split(StringUtils.trimLeading(commandList), 2);
        assert argsList[0].equals(args[0]);

        cmd.execute(argsList.length >= 2 ? argsList[1] : "");
        return true;
    }
}
