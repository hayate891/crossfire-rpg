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
package com.realtime.crossfire.jxclient.gui.list;

import com.realtime.crossfire.jxclient.gui.GUIMetaElement;
import com.realtime.crossfire.jxclient.gui.TooltipManager;
import com.realtime.crossfire.jxclient.gui.label.AbstractLabel;
import com.realtime.crossfire.jxclient.gui.textinput.GUIText;
import com.realtime.crossfire.jxclient.metaserver.Metaserver;
import com.realtime.crossfire.jxclient.metaserver.MetaserverEntry;
import com.realtime.crossfire.jxclient.metaserver.MetaserverEntryListener;
import com.realtime.crossfire.jxclient.metaserver.MetaserverListener;
import com.realtime.crossfire.jxclient.metaserver.MetaserverModel;
import com.realtime.crossfire.jxclient.window.JXCWindowRenderer;
import java.awt.Font;
import java.awt.image.BufferedImage;

/**
 * A {@link GUIList} that tracks a {@link Metaserver} instance.
 * @author Andreas Kirschbaum
 */
public class GUIMetaElementList extends GUIList
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The metaserver model to track.
     */
    private final MetaserverModel metaserverModel;

    /**
     * The {@link TooltipManager} to update.
     */
    private final TooltipManager tooltipManager;

    /**
     * The {@link JXCWindowRenderer} to use.
     */
    private final JXCWindowRenderer windowRenderer;

    /**
     * The name of this element.
     */
    private final String name;

    /**
     * The tcp image for drawing list entries.
     */
    private final BufferedImage tcpImage;

    /**
     * The font for drawing list entries.
     */
    private final Font font;

    /**
     * The format for drawing list entries.
     */
    private final String format;

    /**
     * The tooltip format for drawing list entries.
     */
    private final String tooltip;

    /**
     * The hostname input field to update; may be <code>null</code>.
     */
    private final GUIText hostname;

    /**
     * The comment field to update; may be <code>null</code>.
     */
    private final AbstractLabel comment;

    /**
     * The {@link MetaserverListener} attached to {@link #metaserverModel}. It
     * detects added or removed entries and updates the list accordingly.
     */
    private final MetaserverListener metaserverListener = new MetaserverListener()
    {
        /** {@inheritDoc} */
        @Override
        public void numberOfEntriesChanged()
        {
            rebuildList();
        }
    };

    /**
     * The {@link MetaserverEntryListener} attached to all tracked metaserver
     * entries. It detects changed contents and updates the list accordingly.
     */
    private final MetaserverEntryListener metaserverEntryListener = new MetaserverEntryListener()
    {
        /** {@inheritDoc} */
        @Override
        public void entryChanged()
        {
            setChanged();
        }
    };

    /**
     * Creates a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param windowRenderer the window renderer to notify
     * @param name the name of this element
     * @param x the x-coordinate for drawing this element to screen; it is
     * relative to <code>gui</code>
     * @param y the y-coordinate for drawing this element to screen; it is
     * relative to <code>gui</code>
     * @param w the width for drawing this element to screen
     * @param h the height for drawing this element to screen
     * @param cellHeight the height of each cell
     * @param metaserverModel the metaserver model to track
     * @param tcpImage the tcp image for drawing list entries
     * @param font the font for drawing list entries
     * @param format the format for drawing list entries
     * @param tooltip the tooltip format for drawing list entries
     * @param hostname the hostname input field to update; may be
     * <code>null</code>
     * @param comment the comment field to update; may be <code>null</code>
     */
    public GUIMetaElementList(final TooltipManager tooltipManager, final JXCWindowRenderer windowRenderer, final String name, final int x, final int y, final int w, final int h, final int cellHeight, final MetaserverModel metaserverModel, final BufferedImage tcpImage, final Font font, final String format, final String tooltip, final GUIText hostname, final AbstractLabel comment)
    {
        super(tooltipManager, windowRenderer, name, x, y, w, h, cellHeight, new MetaElementCellRenderer(new GUIMetaElement(tooltipManager, windowRenderer, metaserverModel, name+"_template", w, cellHeight, tcpImage, font, 0, format, tooltip)));
        this.metaserverModel = metaserverModel;
        this.tooltipManager = tooltipManager;
        this.windowRenderer = windowRenderer;
        this.name = name;
        this.tcpImage = tcpImage;
        this.font = font;
        this.format = format;
        this.tooltip = tooltip;
        this.hostname = hostname;
        this.comment = comment;
        this.metaserverModel.addMetaserverListener(metaserverListener);
        rebuildList();
    }

    /** {@inheritDoc} */
    @Override
    public void dispose()
    {
        super.dispose();
        for (int i = 0; i < metaserverModel.size(); i++)
        {
            metaserverModel.removeMetaserverEntryListener(i, metaserverEntryListener);
        }
        metaserverModel.removeMetaserverListener(metaserverListener);
    }

    /**
     * Rebuild the list cells.
     */
    private void rebuildList()
    {
        synchronized (getTreeLock())
        {
            final int newSize = metaserverModel.size();
            final int oldSize = resizeElements(newSize);
            if (oldSize < newSize)
            {
                for (int i = oldSize; i < newSize; i++)
                {
                    final GUIMetaElement metaElement = new GUIMetaElement(tooltipManager, windowRenderer, metaserverModel, name+i, 1, 1, tcpImage, font, i, format, tooltip);
                    addElement(metaElement);
                    metaserverModel.addMetaserverEntryListener(i, metaserverEntryListener);
                }
            }
            else
            {
                for (int i = newSize; i < oldSize; i++)
                {
                    metaserverModel.removeMetaserverEntryListener(i, metaserverEntryListener);
                }
            }
        }
        setChanged();
    }

    /** {@inheritDoc} */
    @Override
    protected void activeChanged()
    {
    }

    /** {@inheritDoc} */
    @Override
    protected void selectionChanged(final int selectedIndex)
    {
        if (selectedIndex == -1)
        {
            // do not update hostname
            if (comment != null)
            {
                comment.setText("");
            }
        }
        else
        {
            final MetaserverEntry metaEntry = metaserverModel.getEntry(selectedIndex);
            if (hostname != null)
            {
                hostname.setText(metaEntry != null ? metaEntry.getHostname() : "");
            }
            if (comment != null)
            {
                comment.setText(metaEntry != null ? metaEntry.getComment() : "");
            }
        }
    }

    /**
     * Updates the tooltip text.
     * @param index the index to check
     */
    @Override
    protected void updateTooltip(final int index)
    {
        final MetaserverEntry metaEntry = metaserverModel.getEntry(index);
        setTooltipText(metaEntry == null ? null : metaEntry.format(tooltip));
    }

    /**
     * Select an entry by server name.
     * @param serverName the server name
     */
    public void setSelectedHostname(final String serverName)
    {
        final int index = metaserverModel.getServerIndex(serverName);
        setSelectedIndex(index);
        if (index == -1 && hostname != null)
        {
            hostname.setText(serverName);
        }
    }
}
