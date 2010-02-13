/*
 * This file is part of JXClient, the Fullscreen Java Crossfire Client.
 *
 * JXClient is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * JXClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JXClient; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Copyright (C) 2005-2008 Yann Chachkoff.
 * Copyright (C) 2006-2010 Andreas Kirschbaum.
 */

package com.realtime.crossfire.jxclient.gui.list;

import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.gui.item.GUIItemItem;
import java.awt.event.MouseEvent;
import javax.swing.ListCellRenderer;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * A {@link GUIList} instance that displays {@link GUIItemItem} instances.
 * @author Andreas Kirschbaum
 */
public abstract class GUIItemList extends GUIList
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * Creates a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param name the name of this element
     * @param x the x-coordinate for drawing this element to screen; it is
     * relative to <code>gui</code>
     * @param y the y-coordinate for drawing this element to screen; it is
     * relative to <code>gui</code>
     * @param w the width for drawing this element to screen
     * @param h the height for drawing this element to screen
     * @param cellHeight the height of each cell
     * @param listCellRenderer the renderer for the list
     */
    protected GUIItemList(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, final int x, final int y, final int w, final int h, final int cellHeight, @NotNull final ListCellRenderer listCellRenderer)
    {
        super(tooltipManager, elementListener, name, x, y, w, h, cellHeight, listCellRenderer);
    }

    /** {@inheritDoc} */
    @Override
    public void dispose()
    {
        super.dispose();
    }

    /**
     * Returns the selected {@link GUIItemItem} instance.
     * @return the selected instance or <code>null</code> if none is selected
     */
    @Nullable
    public GUIItemItem getSelectedItem()
    {
        return (GUIItemItem)getSelectedObject();
    }

    /** {@inheritDoc} */
    @Override
    public void mouseClicked(@NotNull final MouseEvent e)
    {
        super.mouseClicked(e);
        switch (e.getButton())
        {
        case MouseEvent.BUTTON1:
            setActive(true);
            button1Clicked(e.getModifiersEx());
            break;

        case MouseEvent.BUTTON2:
            button2Clicked(e.getModifiersEx());
            break;

        case MouseEvent.BUTTON3:
            button3Clicked(e.getModifiersEx());
            break;
        }
    }

    /**
     * Called if the user has clicked the left mouse button.
     * @param modifiers the active modifiers
     */
    protected abstract void button1Clicked(final int modifiers);

    /**
     * Called if the user has clicked the middle mouse button.
     * @param modifiers the active modifiers
     */
    protected abstract void button2Clicked(final int modifiers);

    /**
     * Called if the user has clicked the right mouse button.
     * @param modifiers the active modifiers
     */
    protected abstract void button3Clicked(final int modifiers);
}
