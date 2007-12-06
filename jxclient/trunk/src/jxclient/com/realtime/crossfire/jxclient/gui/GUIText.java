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
package com.realtime.crossfire.jxclient.gui;

import com.realtime.crossfire.jxclient.JXCWindow;
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

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public abstract class GUIText extends GUIElement implements KeyListener
{
    private final BufferedImage mybackground_active;

    private final BufferedImage mybackground_inactive;

    protected final Font myfont;

    private final Color inactiveColor;

    private final Color activeColor;

    protected String mytext;

    /**
     * If set, hide input; else show input.
     */
    private boolean hideInput = false;

    public GUIText(final JXCWindow jxcWindow, final String nn, final int nx, final int ny, final int nw, final int nh, final BufferedImage picactive, final BufferedImage picinactive, final Font nf, final Color inactiveColor, final Color activeColor, final String txt)
    {
        super(jxcWindow, nn, nx, ny, nw, nh);
        mybackground_active = picactive;
        mybackground_inactive = picinactive;
        myfont = nf;
        this.inactiveColor = inactiveColor;
        this.activeColor = activeColor;
        mytext = txt;
        createBuffer();
        render();
    }

    public void setText(final String nt)
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
                g.setColor(activeColor);
            }
            else
            {
                g.drawImage(mybackground_inactive, 0, 0, null);
                g.setColor(inactiveColor);
            }
            g.setFont(myfont);
            if (hideInput)
            {
                final String template = "********************";
                final String hiddenText = template.substring(0, Math.min(mytext.length(), template.length()));
                g.drawString(hiddenText, 4, myfont.getSize()+2);
            }
            else
            {
                g.drawString(mytext, 4, myfont.getSize()+2);
            }
            g.dispose();
        }
        setChanged();
    }

    /** {@inheritDoc} */
    @Override public void mouseClicked(final MouseEvent e)
    {
        super.mouseClicked(e);
        int b = e.getButton();
        switch (b)
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

    public void setActive(final boolean act)
    {
        active = act;
        render();
    }

    public void keyPressed(final KeyEvent e)
    {
        switch (e.getKeyCode())
        {
        case KeyEvent.VK_BACK_SPACE:
            if (mytext.length() > 0)
            {
                mytext = mytext.substring(0, mytext.length()-1);
                render();
            }
            break;

        case KeyEvent.VK_DELETE:
            if (mytext.length() > 0)
            {
                mytext = "";
                render();
            }
            break;

        case KeyEvent.VK_SHIFT:
            break;

        case KeyEvent.VK_ENTER:
            execute((JXCWindow)e.getSource(), mytext.toString());
            setActive(false);
            break;

        default:
            char chr = e.getKeyChar();
            mytext = mytext+chr;
            render();
            break;
        }
    }

    public void keyReleased(final KeyEvent e)
    {
    }

    public void keyTyped(final KeyEvent e)
    {

    }

    /**
     * Will be called to execute the entered command.
     *
     * @param jxcWindow The JXCWindow instance.
     *
     * @param command The entered command.
     */
    protected abstract void execute(final JXCWindow jxcWindow, final String command);

    /** {@inheritDoc} */
    protected void createBuffer()
    {
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        final GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        mybuffer = gconf.createCompatibleImage(w, h, Transparency.TRANSLUCENT);
        setChanged();
    }

    /**
     * Enable or disable hidden text.
     *
     * @param hideInput If set, hide input; else show input.
     */
    public void setHideInput(final boolean hideInput)
    {
        if (this.hideInput != hideInput)
        {
            this.hideInput = hideInput;
            render();
        }
    }
}
