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

package com.realtime.crossfire.jxclient.gui.item;

import com.realtime.crossfire.jxclient.faces.FacesManager;
import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.items.AbstractManager;
import com.realtime.crossfire.jxclient.items.CfItem;
import com.realtime.crossfire.jxclient.items.ItemSet;
import com.realtime.crossfire.jxclient.items.ItemsManager;
import com.realtime.crossfire.jxclient.items.LocationListener;
import com.realtime.crossfire.jxclient.queue.CommandQueue;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import java.awt.Image;
import java.awt.event.InputEvent;
import java.util.Collection;
import java.util.List;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * A {@link GUIItem} for displaying inventory objects.
 * @author Andreas Kirschbaum
 */
public class GUIItemInventory extends GUIItemItem {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The command queue for sending commands.
     */
    @NotNull
    private final CommandQueue commandQueue;

    /**
     * The server instance.
     */
    @NotNull
    private final CrossfireServerConnection crossfireServerConnection;

    /**
     * The {@link FacesManager} instance to use.
     */
    @NotNull
    private final FacesManager facesManager;

    /**
     * The {@link ItemsManager} instance to watch.
     */
    @NotNull
    private final ItemsManager itemsManager;

    /**
     * The {@link ItemSet} to query.
     */
    @NotNull
    private final ItemSet itemSet;

    /**
     * The inventory manager instance to watch.
     */
    @NotNull
    private final AbstractManager inventoryManager;

    /**
     * The default scroll index.
     */
    private final int defaultIndex;

    /**
     * The object used for synchronization on {@link #index}.
     */
    @NotNull
    private final Object sync = new Object();

    /**
     * The inventory slot index.
     */
    private int index = -1;

    /**
     * If set, paint the element in "selected" state.
     */
    private boolean selected = false;

    /**
     * The {@link LocationListener} used to detect items added to or removed
     * from this inventory slot.
     */
    @NotNull
    private final LocationListener inventoryLocationListener = new LocationListener() {
        /** {@inheritDoc} */
        @Override
        public void locationModified(final int index, @Nullable final CfItem item) {
            synchronized (sync) {
                assert index == GUIItemInventory.this.index;
            }
            setItem(item);
        }
    };

    public GUIItemInventory(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CommandQueue commandQueue, final String name, final int x, final int y, final int w, final int h, @NotNull final ItemPainter itemPainter, final int index, @NotNull final CrossfireServerConnection crossfireServerConnection, @NotNull final FacesManager facesManager, @NotNull final ItemsManager itemsManager, @NotNull final ItemSet itemSet, @NotNull final AbstractManager inventoryManager) {
        super(tooltipManager, elementListener, name, x, y, w, h, crossfireServerConnection, itemPainter, facesManager);
        this.commandQueue = commandQueue;
        this.crossfireServerConnection = crossfireServerConnection;
        this.facesManager = facesManager;
        this.itemsManager = itemsManager;
        this.itemSet = itemSet;
        defaultIndex = index;
        this.inventoryManager = inventoryManager;
        setIndex(index);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void dispose() {
        super.dispose();
        setIndex(-1);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean canScroll(final int distance) {
        if (distance < 0) {
            synchronized (sync) {
                return index >= -distance;
            }
        } else if (distance > 0) {
            final CfItem player = itemsManager.getPlayer();
            if (player == null) {
                return false;
            }

            final Collection<CfItem> list = itemSet.getItems(player.getTag());
            synchronized (sync) {
                return index+distance < list.size();
            }
        } else {
            return false;
        }
    }

    /* {@inheritDoc} */
    @Override
    public void scroll(final int distance) {
        synchronized (sync) {
            setIndex(index+distance);
        }
        setChanged();
    }

    /* {@inheritDoc} */
    @Override
    public void resetScroll() {
        setIndex(defaultIndex);
    }

    /* {@inheritDoc} */
    @Override
    public void button1Clicked(final int modifiers) {
        final CfItem item = getItem();
        if (item == null) {
            return;
        }

        if ((modifiers&InputEvent.SHIFT_DOWN_MASK) == 0) {
            crossfireServerConnection.sendExamine(item.getTag());
        } else {
            crossfireServerConnection.sendLock(!item.isLocked(), item.getTag());
        }
    }

    /* {@inheritDoc} */
    @Override
    public void button2Clicked(final int modifiers) {
        final CfItem item = getItem();
        if (item != null) {
            if ((modifiers&InputEvent.SHIFT_DOWN_MASK) != 0) {
                crossfireServerConnection.sendMark(item.getTag());
                return;
            }
        }

        super.button2Clicked(modifiers);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setSelected(final boolean selected) {
        if (this.selected == selected) {
            return;
        }

        this.selected = selected;
        setChanged();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isSelected() {
        return selected || isActive();
    }

    /* {@inheritDoc} */
    @Override
    public void button3Clicked(final int modifiers) {
        final CfItem item = getItem();
        if (item == null) {
            return;
        }

        if (item.isLocked()) {
            crossfireServerConnection.drawInfo("This item is locked. To drop it, first unlock by SHIFT+leftclicking on it.", 3);
            return;
        }

        commandQueue.sendMove(itemsManager.getCurrentFloorManager().getCurrentFloor(), item.getTag());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getIndex() {
        synchronized (sync) {
            return index;
        }
    }

    /**
     * Set the inventory slot to display.
     * @param index the inventory slot
     */
    private void setIndex(final int index) {
        synchronized (sync) {
            if (this.index == index) {
                return;
            }

            if (this.index >= 0) {
                inventoryManager.removeLocationListener(this.index, inventoryLocationListener);
            }
            this.index = index;
            if (this.index >= 0) {
                inventoryManager.addLocationListener(this.index, inventoryLocationListener);
            }
        }

        final CfItem player = itemsManager.getPlayer();
        if (player != null) {
            final List<CfItem> list = itemSet.getItems(player.getTag());
            if (0 <= this.index && this.index < list.size()) {
                setItem(list.get(this.index));
            } else {
                setItem(null);
            }
        } else {
            setItem(null);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setIndexNoListeners(final int index) {
        synchronized (sync) {
            if (this.index == index) {
                return;
            }

            this.index = index;
        }

        final CfItem player = itemsManager.getPlayer();
        if (player != null) {
            final List<CfItem> list = itemSet.getItems(player.getTag());
            if (0 <= this.index && this.index < list.size()) {
                setItemNoListeners(list.get(this.index));
            } else {
                setItemNoListeners(null);
            }
        } else {
            setItemNoListeners(null);
        }
    }

    /**
     * {@inheritDoc}
     */
    @NotNull
    @Override
    protected Image getFace(@NotNull final CfItem item) {
        return facesManager.getOriginalImageIcon(item.getFace().getFaceNum()).getImage();
    }

}
