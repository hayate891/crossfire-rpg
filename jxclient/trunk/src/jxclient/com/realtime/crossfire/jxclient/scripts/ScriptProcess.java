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
package com.realtime.crossfire.jxclient.scripts;

import com.realtime.crossfire.jxclient.server.CommandQueue;
import com.realtime.crossfire.jxclient.server.CrossfireScriptMonitorListener;
import com.realtime.crossfire.jxclient.server.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.server.CrossfireStatsListener;
import com.realtime.crossfire.jxclient.skills.SkillSet;
import com.realtime.crossfire.jxclient.stats.Stats;
import com.realtime.crossfire.jxclient.window.JXCWindow;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

/**
 * An external command executed as a client-sided script.
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class ScriptProcess extends Thread implements Comparable<ScriptProcess>
{
    /**
     * The script ID identifying this script instance.
     */
    private final int scriptId;

    /**
     * The script command including arguments.
     */
    private final String filename;

    /**
     * The associated {@link JXCWindow} instance.
     */
    private final JXCWindow window;

    /**
     * The {@link CommandQueue} for sending commands.
     */
    private final CommandQueue commandQueue;

    /**
     * The connection instance.
     */
    private final CrossfireServerConnection crossfireServerConnection;

    /**
     * The {@link Stats} instance to watch.
     */
    private final Stats stats;

    /**
     * The {@link Process} instance for the executed child process.
     */
    private final Process proc;

    /**
     * The {@link InputStream} of {@link #proc}.
     */
    private final InputStream in;

    /**
     * The {@link OutputStreamWriter} associated with {@link #proc}.
     */
    private final OutputStreamWriter osw;

    /**
     * The {@link ScriptProcessListener}s to notify.
     */
    private final List<ScriptProcessListener> scriptProcessListeners = new ArrayList<ScriptProcessListener>(1);

    /**
     * The {@link CrossfireScriptMonitorListener} attached to {@link
     * #crossfireServerConnection} to track commands sent to the server.
     */
    private final CrossfireScriptMonitorListener crossfireScriptMonitorListener = new CrossfireScriptMonitorListener()
    {
        /** {@inheritDoc} */
        public void commandSent(final byte[] packet, final int length)
        {
            final String cmd;
            try
            {
                cmd = new String(packet, 0, length, "ISO-8859-1");
            }
            catch (final UnsupportedEncodingException ex)
            {
                throw new AssertionError(); // will never happen: every JVM must implement ISO-8859-1
            }
            ScriptProcess.this.commandSent(cmd);
        }
    };

    /**
     * Creates a new instance.
     * @param scriptId the script ID identifying the new script
     * @param filename the command including arguments to execute
     * @param window the associated window instance
     * @param commandQueue the command queue for sending commands
     * @param crossfireServerConnection the server connection
     * @param stats the stats instance to watch
     * @throws IOException if the script cannot be created
     */
    public ScriptProcess(final int scriptId, final String filename, final JXCWindow window, final CommandQueue commandQueue, final CrossfireServerConnection crossfireServerConnection, final Stats stats) throws IOException
    {
        this.scriptId = scriptId;
        this.filename = filename;
        this.window = window;
        this.commandQueue = commandQueue;
        this.crossfireServerConnection = crossfireServerConnection;
        this.stats = stats;
        final Runtime rt = Runtime.getRuntime();
        proc = rt.exec(filename);
        in = proc.getInputStream();
        osw = new OutputStreamWriter(proc.getOutputStream());
    }

    /**
     * Returns the script ID identifying this script instance.
     * @return the script ID
     */
    public int getScriptId()
    {
        return scriptId;
    }

    /**
     * Returns the script's filename.
     * @return the script's filename
     */
    public String getFilename()
    {
        return filename;
    }

    /** {@inheritDoc} */
    @Override
    public void run()
    {
        String result = "unexpected";
        try
        {
            try
            {
                final InputStreamReader isr = new InputStreamReader(in);
                try
                {
                    final BufferedReader br = new BufferedReader(isr);
                    try
                    {
                        for (;;)
                        {
                            final String line = br.readLine();
                            if (line == null)
                            {
                                break;
                            }

                            runScriptCommand(line);
                        }
                    }
                    finally
                    {
                        br.close();
                    }
                }
                finally
                {
                    isr.close();
                }
                try
                {
                    final int exitStatus = proc.waitFor();
                    result = exitStatus == 0 ? null : "exit "+exitStatus;
                }
                catch (final InterruptedException ex)
                {
                    result = ex.getMessage();
                }
            }
            catch (final IOException ex)
            {
                result = ex.getMessage();
            }
            crossfireServerConnection.getScriptMonitorListeners().removeScriptMonitor(crossfireScriptMonitorListener);
        }
        finally
        {
            for(final ScriptProcessListener scriptProcessListener : scriptProcessListeners)
            {
                scriptProcessListener.scriptTerminated(result);
            }
        }
    }

    /**
     * Sends a message to the script process.
     * @param cmd the message to send
     */
    public void commandSent(final String cmd)
    {
        try
        {
            osw.write(cmd+"\n");
            osw.flush();
        }
        catch (final IOException e)
        {
            e.printStackTrace();
        }
    }

    /** {@inheritDoc} */
    @Override
    public String toString()
    {
        return scriptId+" "+filename;
    }

    /**
     * Processes a "watch" command from the script process.
     * @param cmdline the command arguments
     */
    private static void cmdWatch(final String cmdline)
    {
        final String parms = cmdline.substring(6);
        System.out.println(" - Watch   :"+parms);
    }

    /**
     * Processes an "unwatch" command from the script process.
     * @param cmdline the command arguments
     */
    private static void cmdUnwatch(final String cmdline)
    {
        final String parms = cmdline.substring(8);
        System.out.println(" - Unwatch :"+parms);
    }

    /**
     * Processes a "request" command from the script process.
     * @param cmdline the command arguments
     */
    private void cmdRequest(final String cmdline)
    {
        final String parms = cmdline.substring(8);
        System.out.println(" - Request :"+parms);

        if (parms.equals("range"))
        {
            commandSent(stats.getRange());
        }
        else if (parms.equals("stat stats"))
        {
            commandSent(stats.getStat(CrossfireStatsListener.CS_STAT_STR)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_CON)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_DEX)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_INT)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_WIS)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_POW)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_CHA));
        }
        else if (parms.equals("stat cmbt"))
        {
            commandSent(stats.getStat(CrossfireStatsListener.CS_STAT_WC)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_AC)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_DAM)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_SPEED)+","+
                        stats.getStat(CrossfireStatsListener.CS_STAT_WEAP_SP));
        }
        else if (parms.equals("stat hp"))
        {
            commandSent(stats.getStat(CrossfireStatsListener.CS_STAT_HP)+","+
                    stats.getStat(CrossfireStatsListener.CS_STAT_MAXHP)+","+
                    stats.getStat(CrossfireStatsListener.CS_STAT_SP)+","+
                    stats.getStat(CrossfireStatsListener.CS_STAT_MAXSP)+","+
                    stats.getStat(CrossfireStatsListener.CS_STAT_GRACE)+","+
                    stats.getStat(CrossfireStatsListener.CS_STAT_MAXGRACE)+","+
                    stats.getStat(CrossfireStatsListener.CS_STAT_FOOD));
        }
        else if (parms.equals("stat xp"))
        {
            final StringBuilder sb = new StringBuilder();
            sb.append(stats.getStat(CrossfireStatsListener.CS_STAT_LEVEL));
            sb.append(',').append(stats.getExperience());
            for (int i = CrossfireStatsListener.CS_STAT_SKILLINFO; i < CrossfireStatsListener.CS_STAT_SKILLINFO+CrossfireStatsListener.CS_NUM_SKILLS; i++)
            {
                if (SkillSet.getSkill(i) != null)
                {
                    sb.append(',').append(SkillSet.getSkill(i).getLevel());
                    sb.append(',').append(SkillSet.getSkill(i).getExperience());
                }
            }
            commandSent(sb.toString());
        }
        else if (parms.equals("stat resists"))
        {
            final StringBuilder sb = new StringBuilder();
            for (int i = CrossfireStatsListener.CS_STAT_RESIST_START; i <= CrossfireStatsListener.CS_STAT_RESIST_END; i++)
            {
                sb.append(stats.getStat(i));
                if (i < CrossfireStatsListener.CS_STAT_RESIST_END)
                {
                    sb.append(',');
                }
            }
            commandSent(sb.toString());
        }
        else if (parms.equals("weight"))
        {
            //mmm... I've lost the location of the weight...
            //commandSent(stats.getStat(CS_STAT_MAXWEIGHT)+","+stats.getStat(CS_STAT_WEIGHT));
        }
        else if (parms.equals("flags"))
        {
            commandSent((window.checkFire() ? "1" : "0")+","+(commandQueue.checkRun() ? "1" : "0"));
        }
        else if (parms.equals("items inv"))
        {
        }
        else if (parms.equals("items actv"))
        {
        }
        else if (parms.equals("items on"))
        {
        }
        else if (parms.equals("items cont"))
        {
        }
        else if (parms.equals("map pos"))
        {
        }
        else if (parms.equals("map near"))
        {
        }
        else if (parms.equals("map all"))
        {
        }
        else if (parms.startsWith("map "))
        {
        }
        else if (parms.startsWith("stat "))
        {
        }
    }

    /**
     * Processes a line received from the script process.
     * @param cmdline the line
     */
    private void runScriptCommand(final String cmdline)
    {
        if (cmdline.startsWith("watch "))
        {
            cmdWatch(cmdline);
        }
        else if (cmdline.startsWith("unwatch "))
        {
            cmdUnwatch(cmdline);
        }
        else if (cmdline.startsWith("request "))
        {
            cmdRequest(cmdline);
        }
        else if (cmdline.startsWith("issue mark "))
        {
            final String parms = cmdline.substring(11);
            System.out.println(" - Issue M :"+parms);
        }
        else if (cmdline.startsWith("issue lock "))
        {
            final String parms = cmdline.substring(11);
            System.out.println(" - Issue L :"+parms);
        }
        else if (cmdline.startsWith("issue "))
        {
            final String parms = cmdline.substring(6);
            final String[] pps = parms.split(" ", 3);
            for (int i = 0; i < Integer.parseInt(pps[0]); i++)
            {
                commandQueue.sendNcom(pps[1].equals("1"), 0, pps[2]);
            }
        }
        else if (cmdline.startsWith("draw "))
        {
            final String parms = cmdline.substring(5);
            final String[] pps = parms.split(" ", 2);
            crossfireServerConnection.drawInfo(pps[1], Integer.parseInt(pps[0]));
        }
        else if (cmdline.startsWith("monitor"))
        {
            crossfireServerConnection.getScriptMonitorListeners().addScriptMonitor(crossfireScriptMonitorListener);
        }
        else if (cmdline.startsWith("unmonitor"))
        {
            crossfireServerConnection.getScriptMonitorListeners().removeScriptMonitor(crossfireScriptMonitorListener);
        }
    }

    /**
     * Adds a {@link ScriptProcessListener} to be notified.
     * @param scriptProcessListener the listener to add
     */
    public void addScriptProcessListener(final ScriptProcessListener scriptProcessListener)
    {
        scriptProcessListeners.add(scriptProcessListener);
    }

    /**
     * Kills the script process. Does nothing if the process is not running.
     */
    public void killScript()
    {
        proc.destroy();
    }

    /** {@inheritDoc} */
    @Override
    public int compareTo(final ScriptProcess o)
    {
        if(scriptId < o.scriptId)
        {
            return -1;
        }
        else if(scriptId > o.scriptId)
        {
            return +1;
        }
        else
        {
            return 0;
        }
    }
}
