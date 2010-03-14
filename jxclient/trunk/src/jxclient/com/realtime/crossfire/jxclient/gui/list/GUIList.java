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

import com.realtime.crossfire.jxclient.gui.gui.ActivatableGUIElement;
import com.realtime.crossfire.jxclient.gui.gui.GUIElement;
import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.gui.item.GUIItemItem;
import com.realtime.crossfire.jxclient.gui.scrollable.GUIScrollable;
import com.realtime.crossfire.jxclient.skin.skin.Extent;
import java.awt.Adjustable;
import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Transparency;
import java.awt.event.MouseEvent;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.JViewport;
import javax.swing.ListCellRenderer;
import javax.swing.ListSelectionModel;
import javax.swing.ScrollPaneConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import org.jetbrains.annotations.NotNull;

/**
 * A {@link GUIElement} that displays a list of entries.
 * @author Andreas Kirschbaum
 */
public abstract class GUIList extends ActivatableGUIElement implements GUIScrollable {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The height of a list cell in pixels.
     */
    private final int cellHeight;

    /**
     * The list model of {@link #list}.
     */
    @NotNull
    private final DefaultListModel model = new DefaultListModel();

    /**
     * The list used to display the cells.
     */
    @NotNull
    private final JList list = new JList(model);

    /**
     * The viewport used by {@link #scrollPane}.
     */
    @NotNull
    private final GUIListViewport viewport = new GUIListViewport();

    /**
     * The scroll pane instance used to display the list.
     */
    @NotNull
    private final JScrollPane scrollPane;

    /**
     * The {@link ListSelectionListener} attached to {@link #list}.
     */
    @NotNull
    private final ListSelectionListener listSelectionListener = new ListSelectionListener() {
        /** {@inheritDoc} */
        @Override
        public void valueChanged(@NotNull final ListSelectionEvent e) {
            selectionChanged();
        }
    };

    /**
     * Creates a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param name the name of this element
     * @param extent the extent of this element
     * @param cellWidth the width of each cell
     * @param cellHeight the height of each cell
     * @param listCellRenderer the renderer for the list
     */
    protected GUIList(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, @NotNull final Extent extent, final int cellWidth, final int cellHeight, @NotNull final ListCellRenderer listCellRenderer) {
        super(tooltipManager, elementListener, name, extent, Transparency.TRANSLUCENT);
        final Dimension size = new Dimension(extent.getConstantW(), extent.getConstantH());

        this.cellHeight = cellHeight;

        viewport.setView(list);
        scrollPane = new JScrollPane(null, ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER, ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        scrollPane.setViewport(viewport);
        viewport.setScrollMode(JViewport.BLIT_SCROLL_MODE);
        scrollPane.setOpaque(false);
        scrollPane.setPreferredSize(size);
        scrollPane.setMinimumSize(size);
        scrollPane.setMaximumSize(size);
        scrollPane.setSize(size);
        scrollPane.setLocation(extent.getX(), extent.getY());
        viewport.setSize(size);
        viewport.setOpaque(false);
        scrollPane.setBorder(new EmptyBorder(0, 0, 0, 0));

        list.setCellRenderer(listCellRenderer);
        list.setFixedCellWidth(cellWidth);
        list.setFixedCellHeight(cellHeight);
        list.setOpaque(false);
        list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        list.setSize(getWidth(), Integer.MAX_VALUE);
        list.addListSelectionListener(listSelectionListener);

        synchronized (bufferedImageSync) {
            final Graphics2D g = createBufferGraphics();
            try {
                render(g);
            } finally {
                g.dispose();
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void dispose() {
        super.dispose();
        list.removeListSelectionListener(listSelectionListener);
        synchronized (getTreeLock()) {
            resizeElements(0);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void render(@NotNull final Graphics2D g) {
        final Composite composite = g.getComposite();
        g.setComposite(AlphaComposite.Clear);
        g.setColor(new Color(0, 255, 0, 255));
        g.fillRect(0, 0, getWidth(), getHeight());
        g.setComposite(composite);

        synchronized (getTreeLock()) {
            scrollPane.paint(g);
            viewport.paint(g);
        }
    }

    /**
     * Returns the {@link GUIElement} for a given index.
     * @param index the index
     * @return the gui element
     */
    @NotNull
    public GUIElement getElement(final int index) {
        return (GUIElement)model.get(index);
    }

    /**
     * Adds an {@link GUIElement} to the list.
     * @param element the element to add
     */
    protected void addElement(@NotNull final GUIElement element) {
        assert Thread.holdsLock(getTreeLock());
        model.addElement(element);
        list.setSize(getWidth(), Integer.MAX_VALUE);
        viewport.update();
        if (model.getSize() == 1) {
            setSelectedIndex(0);
        }
    }

    /**
     * Changes the number of list elements. If the new element count is less
     * than the current count, excess elements are cut off. Otherwise the caller
     * has to add elements with {@link #addElement(GUIElement)}.
     * @param newSize the new element count
     * @return the number of elements to add by the caller
     */
    protected int resizeElements(final int newSize) {
        assert Thread.holdsLock(getTreeLock());
        final int index = list.getSelectedIndex();
        final int oldSize = model.getSize();
        if (newSize < oldSize) {
            for (int i = newSize; i < oldSize; i++) {
                final GUIElement element = (GUIElement)model.get(i);
                if (element instanceof GUIItemItem) {
                    element.dispose();
                }
            }
            model.removeRange(newSize, oldSize-1);
            list.setSize(getWidth(), Integer.MAX_VALUE);
            if (index >= newSize && newSize > 0) {
                setSelectedIndex(newSize-1);
            }
            setChanged();
        }
        return oldSize;
    }

    /**
     * Returns whether the selection can be moved.
     * @param diffLines the distance in lines to move
     * @param diffElements the distance in elements to move
     * @return whether moving is possible
     */
    public boolean canMoveSelection(final int diffLines, final int diffElements) {
        synchronized (getTreeLock()) {
            final int distance;
            switch (list.getLayoutOrientation()) {
            case JList.HORIZONTAL_WRAP:
                distance = (list.getWidth()/cellHeight)*diffLines+diffElements;
                break;

            default:
                distance = diffLines+diffElements;
                break;
            }
            final int index = list.getSelectedIndex();
            if (distance > 0) {
                return index == -1 || index+distance < list.getModel().getSize();
            } else if (distance < 0) {
                return index == -1 || index >= -distance;
            } else {
                return false;
            }
        }
    }

    /**
     * Moves the selection.
     * @param diffLines the distance in lines to move
     * @param diffElements the distance in elements to move
     */
    public void moveSelection(final int diffLines, final int diffElements) {
        synchronized (getTreeLock()) {
            final int distance;
            switch (list.getLayoutOrientation()) {
            case JList.HORIZONTAL_WRAP:
                distance = (list.getWidth()/cellHeight)*diffLines+diffElements;
                break;

            default:
                distance = diffLines+diffElements;
                break;
            }
            final int index = list.getSelectedIndex();
            final int newIndex;
            if (distance > 0) {
                if (index == -1) {
                    newIndex = 0;
                } else {
                    newIndex = Math.min(index+distance, list.getModel().getSize()-1);
                }
            } else if (distance < 0) {
                if (index == -1) {
                    newIndex = list.getModel().getSize()-1;
                } else {
                    newIndex = Math.max(index+distance, 0);
                }
            } else {
                if (index == -1) {
                    newIndex = 0;
                } else {
                    newIndex = index;
                }
            }
            setSelectedIndex(newIndex);
        }
    }

    /**
     * Returns whether the list can be scrolled.
     * @param distance the distance to scroll
     * @return whether scrolling is possible
     */
    @Override
    public boolean canScroll(final int distance) {
        synchronized (getTreeLock()) {
            final Adjustable scrollBar = scrollPane.getVerticalScrollBar();
            if (distance > 0) {
                return scrollBar.getValue() < scrollBar.getMaximum()-scrollBar.getVisibleAmount();
            } else if (distance < 0) {
                return scrollBar.getValue() > scrollBar.getMinimum();
            } else {
                return false;
            }
        }
    }

    /**
     * Moves the list.
     * @param distance the distance to scroll
     */
    @Override
    public void scroll(final int distance) {
        synchronized (getTreeLock()) {
            final Adjustable scrollBar = scrollPane.getVerticalScrollBar();
            final int value = scrollBar.getValue()+distance*cellHeight;
            scrollBar.setValue(value);
            final int index = list.getSelectedIndex();
            if (index != -1) {
                final int firstIndex = list.getFirstVisibleIndex();
                if (index < firstIndex) {
                    switch (list.getLayoutOrientation()) {
                    case JList.HORIZONTAL_WRAP:
                        final int columns = list.getWidth()/cellHeight;
                        setSelectedIndex(firstIndex+index%columns);
                        break;

                    default:
                        setSelectedIndex(firstIndex);
                        break;
                    }
                } else {
                    final int lastIndex = list.getLastVisibleIndex();
                    if (index > lastIndex) {
                        switch (list.getLayoutOrientation()) {
                        case JList.HORIZONTAL_WRAP:
                            final int columns = list.getWidth()/cellHeight;
                            final int newTmpColumn = lastIndex-lastIndex%columns+index%columns;
                            final int newColumn;
                            if (newTmpColumn <= lastIndex) {
                                newColumn = newTmpColumn;
                            } else if (newTmpColumn >= columns) {
                                newColumn = newTmpColumn-columns;
                            } else {
                                newColumn = lastIndex;
                            }
                            setSelectedIndex(newColumn);
                            break;

                        default:
                            setSelectedIndex(lastIndex);
                            break;
                        }
                    }
                }
            }
        }
        setChanged();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void resetScroll() {
        setSelectedIndex(0);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void mouseClicked(@NotNull final MouseEvent e) {
        doSelect(e);
        super.mouseClicked(e);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void mouseEntered(@NotNull final MouseEvent e) {
        super.mouseEntered(e);
        doTooltip(e);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void mouseExited(@NotNull final MouseEvent e) {
        super.mouseExited(e);
        doTooltip(e);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void mousePressed(@NotNull final MouseEvent e) {
        super.mouseClicked(e);
        doSelect(e);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void mouseMoved(@NotNull final MouseEvent e) {
        super.mouseMoved(e);
        doTooltip(e);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void mouseDragged(@NotNull final MouseEvent e) {
        super.mouseClicked(e);
        doSelect(e);
    }

    /**
     * Selects the list entry corresponding to a {@link MouseEvent} instance.
     * @param e the mouse event instance
     */
    private void doSelect(@NotNull final MouseEvent e) {
        synchronized (getTreeLock()) {
            setSelectedIndex(list.getFirstVisibleIndex()+list.locationToIndex(e.getPoint()));
        }
    }

    /**
     * Updates the tooltip text corresponding to a {@link MouseEvent} instance.
     * @param e the mouse event instance
     */
    private void doTooltip(@NotNull final MouseEvent e) {
        synchronized (getTreeLock()) {
            updateTooltip(list.getFirstVisibleIndex()+list.locationToIndex(e.getPoint()));
        }
    }

    /**
     * Update the selected list entry.
     * @param newIndex the new selected list entry
     */
    protected void setSelectedIndex(final int newIndex) {
        synchronized (getTreeLock()) {
            final int newIndex2 = Math.min(Math.max(newIndex, 0), list.getModel().getSize()-1);
            final int index = list.getSelectedIndex();
            if (newIndex2 == index) {
                return;
            }

            list.setSelectedIndex(newIndex2);
            if (newIndex2 >= 0) {
                list.ensureIndexIsVisible(newIndex2);
            }
        }
        setChanged();
    }

    /**
     * Called whenever the selected list entry has changed.
     */
    protected void selectionChanged() {
        synchronized (getTreeLock()) {
            selectionChanged(list.getSelectedIndex());
        }
    }

    /**
     * Called whenever the selected list entry has changed.
     * @param selectedIndex the selected list entry
     */
    protected abstract void selectionChanged(final int selectedIndex);

    /**
     * Updates the tooltip text.
     * @param index the index to use
     */
    protected abstract void updateTooltip(final int index);

    /**
     * Sets the layout orientation. See {@link JList#setLayoutOrientation(int)}
     * and {@link JList#setVisibleRowCount(int)}.
     * @param layoutOrientation the layout orientation
     * @param visibleRowCount the number of visible rows
     */
    protected void setLayoutOrientation(final int layoutOrientation, final int visibleRowCount) {
        synchronized (getTreeLock()) {
            list.setLayoutOrientation(layoutOrientation);
            list.setVisibleRowCount(visibleRowCount);
        }
    }

    /**
     * Returns the selected list object.
     * @return the selected object or <code>null</code> if none is selected
     */
    @NotNull
    protected Object getSelectedObject() {
        synchronized (getTreeLock()) {
            return list.getSelectedValue();
        }
    }

}
