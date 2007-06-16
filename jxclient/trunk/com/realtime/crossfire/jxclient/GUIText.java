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

import java.awt.Color;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.awt.Transparency;
import java.io.IOException;
import javax.imageio.ImageIO;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class GUIText extends GUIElement implements KeyListener
{
    private BufferedImage mybackground_active;
    private BufferedImage mybackground_inactive;
    protected Font myfont;
    protected String mytext;
    public GUIText
            (String nn, int nx, int ny, int nw, int nh, String picactive,
                   String picinactive, Font nf, String txt)  throws IOException
    {
        super(nn, nx, ny, nw, nh);
        mybackground_active =
            ImageIO.read(this.getClass().getClassLoader().getResource(picactive));
        mybackground_inactive =
            ImageIO.read(this.getClass().getClassLoader().getResource(picinactive));
        myfont = nf;
        mytext = txt;
        createBuffer();
        render();
    }
    public void setText(String nt)
    {
        mytext = nt;
        render();
    }
    public String getText()
    {
        return mytext;
    }
    protected void render()
    {
        synchronized(mybuffer)
        {
            createBuffer();

            Graphics2D g = mybuffer.createGraphics();
            if (active)
            {
                g.drawImage(mybackground_active, 0, 0, null);
                g.setColor(Color.WHITE);
            }
            else
            {
                g.drawImage(mybackground_inactive, 0, 0, null);
                g.setColor(Color.GRAY);
            }
            g.setFont(myfont);
            g.drawString(mytext, 4, myfont.getSize()+2);
            g.dispose();
        }
        setChanged();
    }
    public void mouseClicked(MouseEvent e)
    {
        int b = e.getButton();
        switch(b)
        {
            case MouseEvent.BUTTON1:
                active = true;
                render();
                break;
            case MouseEvent.BUTTON2:
                break;
            case MouseEvent.BUTTON3:
                break;
        }
    }
    public void setActive(boolean act)
    {
        active = act;
        render();
    }
    public void keyPressed(KeyEvent e)
    {
        switch (e.getKeyCode())
        {
            case KeyEvent.VK_BACK_SPACE:
                if (mytext.length()>0)
                {
                    mytext = mytext.substring(0,mytext.length()-1);
                    render();
                }
                break;
            case KeyEvent.VK_DELETE:
                if (mytext.length()>0)
                {
                    mytext = "";
                    render();
                }
                break;
            case KeyEvent.VK_SHIFT:
                break;
            case KeyEvent.VK_ENTER:
                setActive(false);
                break;
            default:
                char chr = e.getKeyChar();
                mytext = mytext+chr;
                render();
                break;
        }
    }
    public void keyReleased(KeyEvent e)
    {
    }
    public void keyTyped(KeyEvent e)
    {

    }

    /** {@inheritDoc} */
    protected void createBuffer()
    {
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        final GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        mybuffer = gconf.createCompatibleImage(w, h, Transparency.TRANSLUCENT);
        setChanged();
    }
}
