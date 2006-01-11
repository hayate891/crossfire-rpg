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
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.image.*;
import java.io.*;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class GUIMap extends GUIElement implements CrossfireMap1Listener,
                                                  CrossfireNewmapListener,
                                                  CrossfireMapscrollListener
{
    private boolean need_update = false;
    private int need_update_cnt = 0;
    private boolean new_map_happened = true;
    private BufferedImage myblacktile = null;
    private boolean use_big_images = true;
    private int mysquaresize;

    public GUIMap
            (String nn, int nx, int ny, int nw, int nh)  throws IOException
    {
        x = nx;
        y = ny;
        w = nw;
        h = nh;
        myname = nn;

        myblacktile =
            javax.imageio.ImageIO.read(this.getClass().getClassLoader().getResource("black_big.png"));

        GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        GraphicsDevice      gd = ge.getDefaultScreenDevice();
        GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        mybuffer = gconf.createCompatibleImage(nw, nh, Transparency.TRANSLUCENT);
        Graphics2D g = mybuffer.createGraphics();
        g.setColor(Color.BLACK);
        g.fillRect(0,0,mybuffer.getWidth(), mybuffer.getHeight());
        mysquaresize = ServerConnection.SQUARE_SIZE;
        g.dispose();
    }
    public GUIMap
            (String nn, int nx, int ny, int nw, int nh, boolean big)  throws IOException
    {
        x = nx;
        y = ny;
        w = nw;
        h = nh;
        myname = nn;

        use_big_images = big;

        if (big)
        {
            myblacktile =
                javax.imageio.ImageIO.read(this.getClass().getClassLoader().getResource("black_big.png"));
            mysquaresize = ServerConnection.SQUARE_SIZE;
        }
        else
        {
            myblacktile =
                javax.imageio.ImageIO.read(this.getClass().getClassLoader().getResource("black.png"));
            mysquaresize = 32;
        }

        GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        GraphicsDevice      gd = ge.getDefaultScreenDevice();
        GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        mybuffer = gconf.createCompatibleImage(nw, nh, Transparency.TRANSLUCENT);
        Graphics2D g = mybuffer.createGraphics();
        g.setColor(Color.BLACK);
        g.fillRect(0,0,mybuffer.getWidth(), mybuffer.getHeight());
        g.dispose();
    }
    public void redraw(Graphics g)
    {
        synchronized(mybuffer)
        {
            if (new_map_happened == true)
            {
                g.setColor(Color.BLACK);
                g.fillRect(x,y,w,h);
                new_map_happened = false;
            }
            if (need_update == true)
            {
                need_update_cnt--;
                if (need_update_cnt <= 0)
                    need_update = false;
                MapSquare[][] map = com.realtime.crossfire.jxclient.Map.getMap();
                for (int nz=0; nz<ServerConnection.NUM_LAYERS; nz++)
                {
                    for (int ny=10; ny<ServerConnection.MAP_HEIGHT+10; ny++)
                    {
                        for (int nx=10; nx<ServerConnection.MAP_WIDTH+10; nx++)
                        {
                            redrawSquare(g, map[nx][ny], nz);
                        }
                    }
                }
            }
        }
    }
    protected void cleanSquare(Graphics g, MapSquare square)
    {
        g.setColor(Color.BLACK);
        g.fillRect(((square.getXPos()-10)*mysquaresize),
                   ((square.getYPos()-10)*mysquaresize),
                   mysquaresize, mysquaresize);
    }
    protected void redrawSquare(Graphics g, MapSquare square, int nz)
    {
        if (square == null) //Sometimes happen. Not sure of the origin, but I think
                            //it is related to a scrolling faster than a non-cached
                            //image happening. Seems harmless to simply ignore the
                            //null square here.
        {
            System.err.println("Warning ! Null square detected");
            return;
        }
        if (square.isDirty()==false)
            return;
        if (nz == 0)
        {
            cleanSquare(g, square);
        }
        if (square.getHead(nz) == square)
        {
            Face f = square.getFace(nz);
            if (f != null)
            {
                BufferedImage img = null;

                if (use_big_images==true)
                {
                    img = f.getPicture();
                }
                else
                {
                    img = f.getOriginalPicture();
                }

                int px = (square.getXPos()-10)*mysquaresize;
                int py = (square.getYPos()-10)*mysquaresize;
                int psx = px - (img.getWidth()-mysquaresize);
                int psy = py - (img.getHeight()-mysquaresize);
                g.drawImage(img, psx, psy, img.getWidth(), img.getHeight(), null);
            }
        }
        if (nz == ServerConnection.NUM_LAYERS-1)
        {
            /*if (square.getDarkness() >= 0)
            {
                g.setColor(new Color(0,0,0,square.getDarkness()));
                g.fillRect(((square.getXPos()-10)*ServerConnection.SQUARE_SIZE),
                             ((square.getYPos()-10)*ServerConnection.SQUARE_SIZE),
                             ServerConnection.SQUARE_SIZE,
                             ServerConnection.SQUARE_SIZE);
                System.out.println("Darkness:"+square+" D:" +square.getDarkness());
            }
            else
            {
                System.out.println("Darkness:"+square+" I:" +square.getDarkness());
            }*/
            square.clean();
        }
    }
    public void CommandMapscrollReceived(CrossfireCommandMapscrollEvent evt)
    {
        synchronized(mybuffer)
        {
            need_update = true;
            need_update_cnt = 2;
        }
    }
    public void CommandNewmapReceived(CrossfireCommandNewmapEvent evt)
    {
        /*Graphics2D g = mybuffer.createGraphics();
        g.setColor(Color.BLACK);
        g.fillRect(0,0,mybuffer.getWidth(), mybuffer.getHeight());
        g.dispose();*/
        synchronized(mybuffer)
        {
            need_update = true;
            need_update_cnt = 2;
            new_map_happened = true;
        }
    }
    public void CommandMap1Received(CrossfireCommandMap1Event evt)
    {
        synchronized(mybuffer)
        {
            need_update = true;
            need_update_cnt = 2;
        }
    }
    public void refresh()
    {
        synchronized(mybuffer)
        {
            need_update = true;
            need_update_cnt = 2;
            new_map_happened = true;
        }
    }
}
