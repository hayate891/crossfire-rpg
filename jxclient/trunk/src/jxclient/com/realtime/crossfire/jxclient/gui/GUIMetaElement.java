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
import com.realtime.crossfire.jxclient.Metaserver;
import com.realtime.crossfire.jxclient.MetaserverEntry;
import java.awt.Color;
import java.awt.event.MouseEvent;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.awt.Transparency;
import java.util.List;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class GUIMetaElement extends GUIElement implements GUIScrollable
{
    private final BufferedImage mypicture_tcp;

    private final BufferedImage mypicture_udp;

    private final Font myfont;

    private final GUIText mytext;

    private final AbstractLabel mylabel;

    private int myindex;

    public GUIMetaElement(final JXCWindow jxcWindow, final String nn, final int nx, final int ny, final int nw, final int nh, final BufferedImage pic_tcp, final BufferedImage pic_udp, final Font nf, final GUIText txt, final AbstractLabel comment, final int meta_id)
    {
        super(jxcWindow, nn, nx, ny, nw, nh);
        mypicture_tcp = pic_tcp;
        mypicture_udp = pic_udp;
        myfont = nf;
        mytext = txt;
        mylabel = comment;
        myindex = meta_id;
        createBuffer();
        render();
    }

    protected void render()
    {
        final List<MetaserverEntry> l = Metaserver.query();
        if (myindex < 0 || myindex >= l.size())
        {
            return;
        }

        final MetaserverEntry mentry = l.get(myindex);
        final Graphics2D g = mybuffer.createGraphics();
        g.setFont(myfont);
        g.setColor(active ? Color.RED : Color.GRAY);
        if (mypicture_tcp != null)
        {
            g.drawImage(mypicture_tcp, 0, 0, null);
        }
        g.drawString("P:"+mentry.getNrPlayers()+" L:"+mentry.getPing()+" - "+mentry.getHost()+" - "+mentry.getComment(), 16, myfont.getSize()+1);
        g.dispose();
        if (mylabel != null && active)
        {
            mylabel.setText(mentry.getComment());
        }
        if (mytext != null && active)
        {
            mytext.setText(mentry.getHost());
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
        if (active && !act)
        {
            if (mylabel != null)
            {
                mylabel.setText("");
            }
        }
        active = act;
        render();
    }

    /** {@inheritDoc} */
    public boolean canScrollUp()
    {
        return myindex > 0;
    }

    public void scrollUp()
    {
        myindex--;
        render();
    }

    /** {@inheritDoc} */
    public boolean canScrollDown()
    {
        return myindex+1 < Metaserver.query().size();
    }

    public void scrollDown()
    {
        myindex++;
        render();
    }

    public int getIndex()
    {
        return myindex;
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
