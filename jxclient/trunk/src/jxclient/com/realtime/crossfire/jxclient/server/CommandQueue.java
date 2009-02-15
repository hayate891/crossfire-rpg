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
package com.realtime.crossfire.jxclient.server;

import com.realtime.crossfire.jxclient.window.GuiStateListener;
import com.realtime.crossfire.jxclient.window.JXCWindow;
import java.util.LinkedList;
import java.util.List;

/**
 * Maintains the pending (ncom) commands sent to the server.
 *
 * @author Andreas Kirschbaum
 */
public class CommandQueue
{
    /**
     * Maximum number of pending commands sent to the server. Excess commands
     * will be dropped.
     */
    public static final int MAX_PENDING_COMMANDS = 10;

    /**
     * The server connection for sending ncom commands.
     */
    private final CrossfireServerConnection crossfireServerConnection;

    /**
     * Records command ids of commands sent to the server for which no comc
     * commands has been received. Note that the size may be larger than {@link
     * #MAX_PENDING_COMMANDS} due to "must send" commands.
     */
    private final List<Integer> pendingCommands = new LinkedList<Integer>();

    /**
     * The default repeat counter for ncom commands.
     */
    private int repeatCount = 0;

    /**
     * Whether a "run" command has been sent without a following "run_stop"
     * command.
     */
    private boolean isRunning = false;

    /**
     * The listener to track comc commands.
     */
    private final CrossfireComcListener crossfireComcListener = new CrossfireComcListener()
    {
        /** {@inheritDoc} */
        @Override
        public void commandComcReceived(final int packetNo, final int time)
        {
            synchronized (pendingCommands)
            {
                final int index = pendingCommands.indexOf(packetNo);
                if (index == -1)
                {
                    System.err.println("Error: got unexpected comc command #"+packetNo);
                    return;
                }
                if (index > 0)
                {
                    System.err.println("Warning: got out of order comc command #"+packetNo);
                }

                for (int i = 0; i <= index; i++)
                {
                    pendingCommands.remove(0);
                }
            }
        }
    };

    /**
     * The {@link GuiStateListener} for detecting established or dropped
     * connections.
     */
    private final GuiStateListener guiStateListener = new GuiStateListener()
    {
        /** {@inheritDoc} */
        @Override
        public void start()
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void metaserver()
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void connecting()
        {
            clear();
        }

        /** {@inheritDoc} */
        @Override
        public void connecting(final ClientSocketState clientSocketState)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void connected()
        {
            // ignore
        }
    };

    /**
     * Create a new instance.
     * @param crossfireServerConnection The server connection for sending
     * ncom commands.
     * @param window the window to attach to
     */
    public CommandQueue(final CrossfireServerConnection crossfireServerConnection, final JXCWindow window)
    {
        this.crossfireServerConnection = crossfireServerConnection;
        crossfireServerConnection.addCrossfireComcListener(crossfireComcListener);
        window.addConnectionStateListener(guiStateListener);
    }

    /**
     * Return the current repeat count and reset it to zero.
     *
     * @return The current repeat count.
     */
    private int getRepeatCount()
    {
        final int oldRepeatCount = repeatCount;
        resetRepeatCount();
        return oldRepeatCount;
    }

    /**
     * Reset the current repeat count to zero.
     */
    public void resetRepeatCount()
    {
        repeatCount = 0;
    }

    /**
     * Add a digit to the current repeat count.
     *
     * @param digit The digit (0-9) to add.
     */
    public void addToRepeatCount(final int digit)
    {
        assert 0 <= digit && digit <= 9;
        repeatCount = (10*repeatCount+digit)%100000;
    }

    /**
     * Forget about sent commands.
     */
    private void clear()
    {
        resetRepeatCount();
        synchronized (pendingCommands)
        {
            pendingCommands.clear();
            isRunning = false;
        }
    }

    /**
     * Send a "ncom" command to the server. This function uses the default
     * repeat count.
     *
     * @param mustSend If set, always send the command; if unset, drop the
     * command if the command queue is full.
     *
     * @param command The command to send.
     *
     * @see #sendNcom(boolean, int, String)
     */
    public void sendNcom(final boolean mustSend, final String command)
    {
        sendNcom(mustSend, getRepeatCount(), command);
    }

    /**
     * Send a "ncom" command to the server.
     *
     * @param mustSend If set, always send the command; if unset, drop the
     * command if the command queue is full.
     *
     * @param repeat The repeat count.
     *
     * @param command The command to send.
     *
     * @see #sendNcom(boolean, String)
     */
    public void sendNcom(final boolean mustSend, final int repeat, final String command)
    {
        synchronized (pendingCommands)
        {
            if (!mustSend && pendingCommands.size() >= MAX_PENDING_COMMANDS)
            {
                return;
            }

            final int packetNo = crossfireServerConnection.sendNcom(repeat, command);
            pendingCommands.add(packetNo);

            if (command.startsWith("run "))
            {
                isRunning = true;
            }
            else if (command.startsWith("run_stop"))
            {
                isRunning = false;
            }
        }
    }

    /**
     * Tell the server to stop running. If the character is not running, do
     * nothing.
     * @return whether running was active
     */
    public boolean stopRunning()
    {
        final boolean result = isRunning;
        if (result)
        {
            sendNcom(true, 0, "run_stop");
            assert !isRunning;
        }
        return result;
    }

    /**
     * Returns whether the character is running.
     * @return whether the character is running
     */
    public boolean checkRun()
    {
        return isRunning;
    }

    /**
     * Returns whether the character is firing.
     * @return whether the character is firing
     */
    public boolean checkFire()
    {
        return false; // XXX: implement
    }

    /**
     * Sends a "move" command to the server. The repeat count value is the
     * client's current repeat count.
     * @param to the destination location
     * @param tag the item to move
     */
    public void sendMove(final int to, final int tag)
    {
        crossfireServerConnection.sendMove(to, tag, getRepeatCount());
    }
}
