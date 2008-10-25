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

import com.realtime.crossfire.jxclient.metaserver.Metaserver;
import com.realtime.crossfire.jxclient.metaserver.MetaserverEntry;
import com.realtime.crossfire.jxclient.metaserver.MetaserverEntryListener;
import com.realtime.crossfire.jxclient.window.JXCWindow;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Transparency;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;

/**
 * Display a Crossfire server entry.
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class GUIMetaElement extends ActivatableGUIElement implements GUIScrollable
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The metaserver instance to monitor.
     */
    private final Metaserver metaserver;

    /**
     * An image to draw before the server description. May be <code>null</code>
     * to draw no image.
     */
    private final BufferedImage tcpImage;

    /**
     * The font to use.
     */
    private final Font font;

    /**
     * The format used for displaying {@link Metaserver} instances.
     */
    private final String format;

    /**
     * The format used for displaying tooltips.
     */
    private final String tooltip;

    /**
     * The default scroll index.
     */
    private final int defaultIndex;

    /**
     * The metaserver index.
     */
    private int index;

    /**
     * If set, paint the element in "selected" state.
     */
    private boolean selected = false;

    /**
     * The metaserver entry listener attached for the current {@link #index}.
     */
    private final MetaserverEntryListener metaserverEntryListener = new MetaserverEntryListener()
    {
        /** {@inheritDoc} */
        public void entryChanged()
        {
            setChanged();
            updateTooltip();
        }
    };

    /**
     * Creates a new instance.
     * @param window the <code>JXCWindow</code> this element belongs to
     * @param metaserver the metaserver instance to monitor
     * @param name the name of this element
     * @param w the width for drawing this element to screen
     * @param h the height for drawing this element to screen
     * @param tcpImage an image to draw before the server description. May be
     * <code>null</code> to draw no image
     * @param font the font to use
     * @param defaultIndex the initial metaserver index
     * @param format the format used to display metaserver instances
     * @param tooltip the format used for displaying tooltips
     */
    public GUIMetaElement(final JXCWindow window, final Metaserver metaserver, final String name, final int w, final int h, final BufferedImage tcpImage, final Font font, final int defaultIndex, final String format, final String tooltip)
    {
        super(window, name, 0, 0, w, h, Transparency.TRANSLUCENT);
        this.metaserver = metaserver;
        this.tcpImage = tcpImage;
        this.font = font;
        this.defaultIndex = defaultIndex;
        index = defaultIndex;
        this.format = format;
        this.tooltip = tooltip;
        metaserver.addMetaserverEntryListener(defaultIndex, metaserverEntryListener);
        setChanged();
        updateTooltip();
    }

    /** {@inheritDoc} */
    @Override
    protected void render(final Graphics g)
    {
        final MetaserverEntry metaEntry = metaserver.getEntry(index);
        final Graphics2D g2 = (Graphics2D)g;
        g2.setBackground(new Color(0, 0, 0, 0.0f));
        g.clearRect(0, 0, getWidth(), getHeight());
        g.setFont(font);
        g.setColor(isActive() || selected ? Color.RED : Color.GRAY);
        if (tcpImage != null)
        {
            g.drawImage(tcpImage, 0, 0, null);
        }
        g.drawString(metaEntry == null ? "" : metaEntry.format(format), tcpImage != null ? 16 : 0, font.getSize()+1);
    }

    /** {@inheritDoc} */
    @Override
    public void mouseClicked(final MouseEvent e)
    {
        super.mouseClicked(e);
        final int b = e.getButton();
        switch (b)
        {
        case MouseEvent.BUTTON1:
            setActive(true);
            setChanged();
            break;

        case MouseEvent.BUTTON2:
            break;

        case MouseEvent.BUTTON3:
            break;
        }
    }

    /** {@inheritDoc} */
    public boolean canScroll(final int distance)
    {
        if (distance < 0)
        {
            return index >= -distance;
        }
        else if (distance > 0)
        {
            return index+distance < metaserver.size();
        }
        else
        {
            return false;
        }
    }

    /** {@inheritDoc} */
    public void scroll(final int distance)
    {
        setIndex(index+distance);
    }

    /** {@inheritDoc} */
    public void resetScroll()
    {
        if (index != defaultIndex)
        {
            scroll(defaultIndex-index);
        }
    }

    /** {@inheritDoc} */
    @Override
    protected void activeChanged()
    {
        setChanged();
    }

    /**
     * Returns the index of this element.
     * @return the index
     */
    public int getIndex()
    {
        return index;
    }

    /**
     * Sets the index of this element.
     * @param index the index
     */
    public void setIndex(final int index)
    {
        if (this.index == index)
        {
            return;
        }

        metaserver.removeMetaserverEntryListener(index, metaserverEntryListener);
        this.index = index;
        metaserver.addMetaserverEntryListener(index, metaserverEntryListener);
        setChanged();
        updateTooltip();
    }

    /**
     * Updates the tooltip text.
     */
    private void updateTooltip()
    {
        final MetaserverEntry metaEntry = metaserver.getEntry(index);
        setTooltipText(metaEntry == null ? null : metaEntry.format(tooltip));
    }

    /**
     * Sets the selected state.
     * @param selected whether this element should drawn as "selected"
     */
    public void setSelected(final boolean selected)
    {
        if (this.selected == selected)
        {
            return;
        }

        this.selected = selected;
        setChanged();
    }
}
