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
import com.realtime.crossfire.jxclient.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.prefs.*;
import java.io.*;
/**
 * This is the entry point for JXClient. Note that this class doesn't do much
 * by itself - most of the work in done in JXCWindow or ServerConnection.
 * @see com.realtime.crossfire.jxclient.JXCWindow
 * @see com.realtime.crossfire.jxclient.ServerConnection
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class jxclient
{
    /**
     * The program entry point.
     * @since 1.0
     */
    public static void main(String args[])
    {
        System.out.println("JXClient - Crossfire Java Client");
        System.out.println("(C)2005 by Lauwenmark.");
        System.out.println("This software is placed under the GPL License");
        jxclient game = new jxclient(args);
    }

    /**
     * The constructor of the class. This is where the main window is created.
     * Initialization of a JXCWindow is the only task performed here.
     * @since 1.0
     */
    public jxclient(String args[])
    {
        try
        {
            Object p = Class.forName("com.sixlegs.png.PngImage");
        }
        catch (Exception e)
        {
            System.out.println("Sixlegs PNG Library not correctly installed, aborting");
            System.exit(0);
        }
        try
        {
            Preferences prefs = Preferences.userRoot();
            String str_width  = prefs.get("width", "1024");
            String str_height = prefs.get("height", "768");
            String str_bpp    = prefs.get("bpp", "-1");
            String str_freq   = prefs.get("frequency", "0");
            String str_skin   = prefs.get("skin", "com.realtime.crossfire.jxclient.JXCSkinPrelude");
            boolean mkdir_result = (new File("cache")).mkdirs();
            for(int i=0; i<args.length; i++)
            {
                if ((args[i].equals("-W"))&&(i+1<args.length))
                {
                    str_width = args[i+1];
                    i++;
                }
                else if ((args[i].equals("-H"))&&(i+1<args.length))
                {
                    str_height = args[i+1];
                    i++;
                }
                else if ((args[i].equals("-B"))&&(i+1<args.length))
                {
                    str_bpp = args[i+1];
                    i++;
                }
                else if ((args[i].equals("-F"))&&(i+1<args.length))
                {
                    str_freq = args[i+1];
                    i++;
                }
                else if ((args[i].equals("-S"))&&(i+1<args.length))
                {
                    str_skin = args[i+1];
                    i++;
                }
                else if ((args[i].equals("-opengl")))
                {
                    System.setProperty("sun.java2d.opengl", "True");
                }
                else
                {
                    System.out.println("");
                    System.out.println("Available options:");
                    System.out.println(" -W <size> : Width of the screen, in pixels;");
                    System.out.println(" -H <size> : Height of the screen, in pixels;");
                    System.out.println(" -B <bpp>  : Bit per pixels, or (-1) for multibpp mode;");
                    System.out.println(" -F <freq> : Refresh frequency of the screen in Hz (0:guess);");
                    System.out.println(" -S <skin> : Class containing the skin to use.");
                    System.out.println(" -opengl   : Enable the OpenGL rendering pipeline.");
                    System.exit(1);
                }
            }
            prefs.put("width",      str_width);
            prefs.put("height",     str_height);
            prefs.put("bpp",        str_bpp);
            prefs.put("frequency",  str_freq);
            prefs.put("skin",       str_skin);

            JXCWindow jxwin = new JXCWindow();
            jxwin.init(Integer.parseInt(str_width), Integer.parseInt(str_height),
                      Integer.parseInt(str_bpp), Integer.parseInt(str_freq), str_skin);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
