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
package com.realtime.crossfire.jxclient.window;

import com.realtime.crossfire.jxclient.gui.gui.ActivatableGUIElement;
import com.realtime.crossfire.jxclient.gui.gui.Gui;
import com.realtime.crossfire.jxclient.gui.gui.GuiAutoCloseListener;
import com.realtime.crossfire.jxclient.gui.label.AbstractLabel;
import com.realtime.crossfire.jxclient.gui.log.Buffer;
import com.realtime.crossfire.jxclient.gui.log.GUIMessageLog;
import com.realtime.crossfire.jxclient.server.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.server.CrossfireUpdateMapListener;
import com.realtime.crossfire.jxclient.skin.Resolution;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.DisplayMode;
import java.awt.Graphics;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Insets;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.awt.image.BufferStrategy;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * @author Andreas Kirschbaum
 */
public class JXCWindowRenderer
{
    private final JXCWindow window;

    private final MouseTracker mouseTracker;

    /**
     * The semaphore used to synchronized map model updates and map view
     * redraws.
     */
    private final Object redrawSemaphore;

    private BufferStrategy bufferStrategy = null;

    private DisplayMode oldDisplayMode = null;

    private DisplayMode displayMode = null;

    /**
     * If set, the content of {@link #openDialogs} has changed.
     */
    private boolean openDialogsChanged = false;

    /**
     * Currently opened dialogs. The ordering is the painting order: the
     * topmost dialog is at the end.
     */
    private final CopyOnWriteArrayList<Gui> openDialogs = new CopyOnWriteArrayList<Gui>();

    /**
     * Listeners to be notified about {@link #rendererGuiState} changes.
     */
    private final CopyOnWriteArrayList<RendererGuiStateListener> rendererGuiStateListeners = new CopyOnWriteArrayList<RendererGuiStateListener>();

    /**
     * If set, {@link #currentGui} has changed.
     */
    private boolean currentGuiChanged = false;

    private Gui currentGui;

    /**
     * The tooltip to use, or <code>null</code> if no tooltips should be shown.
     */
    private AbstractLabel tooltip = null;

    /**
     * If set, force a full repaint.
     */
    private volatile boolean forcePaint = false;

    /**
     * If set, do not repaint anything. It it set while a map update is in
     * progress.
     */
    private volatile boolean inhibitPaint = false;

    /**
     * If set, at least one call to {@link #redrawGUI)} has been dropped while
     * {@link #inhibitPaint} was set.
     */
    private volatile boolean skippedPaint = false;

    /**
     * The x-offset of of the visible window.
     */
    private int offsetX = 0;

    /**
     * The y-offset of of the visible window.
     */
    private int offsetY = 0;

    /**
     * Receords whether full-screen has been activated.
     */
    private boolean isFullScreen = false;

    /**
     * The current gui state.
     */
    private RendererGuiState rendererGuiState = RendererGuiState.START;

    /**
     * The {@link GuiAutoCloseListener} used to track auto-closing dialogs.
     */
    private final GuiAutoCloseListener guiAutoCloseListener = new GuiAutoCloseListener()
    {
        /** {@inheritDoc} */
        @Override
        public void autoClosed(final Gui gui)
        {
            closeDialog(gui);
        }
    };

    /**
     * The listener to detect map model changes.
     */
    private final CrossfireUpdateMapListener crossfireUpdateMapListener = new CrossfireUpdateMapListener()
    {
        /** {@inheritDoc} */
        @Override
        public void newMap(final int mapWidth, final int mapHeight)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void mapBegin()
        {
            inhibitPaint = true;
            skippedPaint = false;
        }

        /** {@inheritDoc} */
        @Override
        public void mapClear(final int x, final int y)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void mapDarkness(final int x, final int y, final int darkness)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void mapFace(final int x, final int y, final int layer, final int faceNum)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void mapAnimation(final int x, final int y, final int layer, final int animationNum, final int animationType)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void mapAnimationSpeed(final int x, final int y, final int layer, final int animSpeed)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void scroll(final int dx, final int dy)
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void mapEnd()
        {
            if (skippedPaint)
            {
                forcePaint = true;
            }
            inhibitPaint = false;
        }

        /** {@inheritDoc} */
        @Override
        public void addAnimation(final int animation, final int flags, final int[] faces)
        {
            // ignore
        }

    };

    /**
     * Creates a new instance.
     * @param window the associated window
     * @param mouseTracker the mouse tracker instance
     * @param redrawSemaphore the semaphore used to synchronized map model
     * updates and map view redraws
     * @param crossfireServerConnection the server connection to monitor
     */
    public JXCWindowRenderer(final JXCWindow window, final MouseTracker mouseTracker, final Object redrawSemaphore, final CrossfireServerConnection crossfireServerConnection)
    {
        this.window = window;
        this.mouseTracker = mouseTracker;
        this.redrawSemaphore = redrawSemaphore;
        crossfireServerConnection.addCrossfireUpdateMapListener(crossfireUpdateMapListener);
    }

    /**
     * Repaint the window.
     */
    public void repaint()
    {
        forcePaint = true;
    }

    public void initRendering(final Resolution resolution, final boolean fullScreen)
    {
        displayMode = new DisplayMode(resolution.getWidth(), resolution.getHeight(), DisplayMode.BIT_DEPTH_MULTI, DisplayMode.REFRESH_RATE_UNKNOWN);
        isFullScreen = false;
        oldDisplayMode = null;

        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        if (fullScreen && gd.isFullScreenSupported())
        {
            window.setUndecorated(true);
            gd.setFullScreenWindow(window);
            isFullScreen = true;
            final DisplayMode currentDisplayMode = gd.getDisplayMode();
            if (currentDisplayMode.getWidth() == displayMode.getWidth() && currentDisplayMode.getHeight() == displayMode.getHeight())
            {
                // full-screen mode, no display mode change
            }
            else
            {
                if (!gd.isDisplayChangeSupported())
                {
                    isFullScreen = false;
                    gd.setFullScreenWindow(null);
//                    window.setUndecorated(false); // XXX: cannot be called anymore
                    // windowed mode
                }
                else
                {
                    boolean ok = true;
                    try
                    {
                        gd.setDisplayMode(displayMode);
                    }
                    catch (final IllegalArgumentException ex)
                    {
                        ok = false;
                    }
                    if (ok)
                    {
                        oldDisplayMode = currentDisplayMode;
                        // full-screen mode, display mode change
                    }
                    else
                    {
                        isFullScreen = false;
                        gd.setFullScreenWindow(null);
//                        window.setUndecorated(false); // XXX: cannot be called anymore
                        // windowed mode
                    }
                }
            }
        }
        else
        {
            // windowed mode
        }

        if (!isFullScreen)
        {
            if (fullScreen)
            {
                System.out.println("Warning ! True full-screen support is not available.");
            }

            final Dimension size = new Dimension(displayMode.getWidth(), displayMode.getHeight());
            window.getRootPane().setPreferredSize(size);
            window.pack();
            window.setResizable(false);
            window.setVisible(true);
            window.setLocationRelativeTo(null);
        }
        window.createBufferStrategy(2);
        bufferStrategy = window.getBufferStrategy();

        final Insets insets = window.getInsets();
        offsetX = insets.left;
        offsetY = insets.top;
    }

    public void endRendering()
    {
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        if (oldDisplayMode != null)
        {
            gd.setDisplayMode(oldDisplayMode);
            oldDisplayMode = null;
        }
        if (isFullScreen)
        {
            isFullScreen = false;
            gd.setFullScreenWindow(null);
        }
    }

    public void redrawGUI()
    {
        if (inhibitPaint)
        {
            skippedPaint = true;
            return;
        }

        if (forcePaint)
        {
            forcePaint = false;
        }
        else if (!needRedraw())
        {
            return;
        }

        do
        {
            do
            {
                final Graphics g = bufferStrategy.getDrawGraphics();
                try
                {
                    g.translate(offsetX, offsetY);
                    if (bufferStrategy.contentsRestored())
                    {
                        redrawBlack(g);
                    }
                    redraw(g);
                }
                finally
                {
                    g.dispose();
                }
            }
            while (bufferStrategy.contentsLost());
            bufferStrategy.show();
        }
        while (bufferStrategy.contentsLost());
    }

    /**
     * Paints the view into the given graphics instance.
     * @param g the graphics instance to paint to
     */
    public void redraw(final Graphics g)
    {
        synchronized (redrawSemaphore)
        {
            redrawGUIBasic(g);
            redrawGUIDialog(g);
            redrawTooltip(g);
        }
    }

    public void clearGUI(final Gui gui)
    {
        setCurrentGui(gui);
        for (int ig = 0; ig < 3; ig++)
        {
            final Graphics g = bufferStrategy.getDrawGraphics();
            g.translate(offsetX, offsetY);
            redrawBlack(g);
            g.dispose();
            bufferStrategy.show();
        }
    }

    private void redrawGUIBasic(final Graphics g)
    {
        currentGuiChanged = false;
        currentGui.redraw(g);
    }

    private void redrawGUIDialog(final Graphics g)
    {
        openDialogsChanged = false;
        for (final Gui dialog : openDialogs)
        {
            if (!dialog.isHidden(rendererGuiState))
            {
                dialog.redraw(g);
            }
        }
    }

    private void redrawTooltip(final Graphics g)
    {
        if (tooltip != null)
        {
            if (tooltip.isElementVisible())
            {
                tooltip.paintComponent(g);
            }
            else
            {
                tooltip.resetChanged();
            }
        }
    }

    private void redrawBlack(final Graphics g)
    {
        g.setColor(Color.BLACK);
        g.fillRect(0, 0, window.getWindowWidth(), window.getWindowHeight());
    }

    /**
     * Open a dialog. Raises an already opened dialog.
     *
     * @param dialog The dialog to show.
     *
     * @param autoCloseOnDeactivate whether the dialog should auto-close when
     * it becomes inactive; ignored if the dialog is already open
     *
     * @return Whether the dialog was opened or raised; <code>false</code> if
     * the dialog already was opened as the topmost dialog.
     */
    public boolean openDialog(final Gui dialog, final boolean autoCloseOnDeactivate)
    {
        if (dialog == currentGui)
        {
            return false;
        }

        if (!openDialogs.isEmpty() && openDialogs.get(openDialogs.size()-1) == dialog)
        {
            return false;
        }

        dialog.setStateChanged(true);
        if (!openDialogsRemove(dialog))
        {
            dialog.activateDefaultElement();
            dialog.setGuiAutoCloseListener(autoCloseOnDeactivate ? guiAutoCloseListener : null);
        }
        openDialogsAdd(dialog);
        openDialogsChanged = true;
        return true;
    }

    /**
     * Raise an already opened dialog.
     *
     * @param dialog The dialog to show.
     */
    public void raiseDialog(final Gui dialog)
    {
        if (dialog == currentGui)
        {
            return;
        }

        if (!openDialogs.isEmpty() && openDialogs.get(openDialogs.size()-1) == dialog)
        {
            return;
        }

        if (!isDialogOpen(dialog))
        {
            return;
        }

        if (!openDialogsRemove(dialog))
        {
            assert false;
        }
        openDialogsAdd(dialog);
        openDialogsChanged = true;
    }

    /**
     * Return whether a given dialog is currently visible.
     *
     * @param dialog The dialog to check.
     *
     * @return Whether the dialog is visible.
     */
    public boolean isDialogOpen(final Gui dialog)
    {
        return openDialogs.contains(dialog);
    }

    /**
     * Return all open dialogs in reverse painting order; the first element is
     * the top-most dialog.
     *
     * @return The open dialogs; client code must not modify this list.
     */
    public Iterable<Gui> getOpenDialogs()
    {
        return new Iterable<Gui>()
        {
            /** {@inheritDoc} */
            @Override
            public Iterator<Gui> iterator()
            {
                return new Iterator<Gui>()
                {
                    /**
                     * The backing list iterator; it returns the elements in
                     * reversed order.
                     */
                    private final ListIterator<Gui> it = openDialogs.listIterator(openDialogs.size());

                    /** {@inheritDoc} */
                    @Override
                    public boolean hasNext()
                    {
                        return it.hasPrevious();
                    }

                    /** {@inheritDoc} */
                    @Override
                    public Gui next()
                    {
                        return it.previous();
                    }

                    /** {@inheritDoc} */
                    @Override
                    public void remove()
                    {
                        throw new UnsupportedOperationException();
                    }
                };
            }
        };
    }

    public void setCurrentGui(final Gui gui)
    {
        currentGui = gui;
        currentGuiChanged = true;
    }

    public Gui getCurrentGui()
    {
        return currentGui;
    }

    /**
     * Check whether any gui element has changed and needs a redraw.
     *
     * @return whether any gui element has changed
     */
    private boolean needRedraw()
    {
        if (openDialogsChanged)
        {
            return true;
        }

        if (currentGuiChanged)
        {
            return true;
        }

        if (currentGui.needRedraw())
        {
                return true;
        }

        for (final Gui dialog : openDialogs)
        {
            if (!dialog.isHidden(rendererGuiState) && dialog.needRedraw())
            {
                return true;
            }
        }

        return tooltip != null && tooltip.isChanged();
    }

    /**
     * Return the x-offset of of the visible window.
     *
     * @return The x-offset of of the visible window.
     */
    public int getOffsetX()
    {
        return offsetX;
    }

    /**
     * Return the y-offset of of the visible window.
     *
     * @return The y-offset of of the visible window.
     */
    public int getOffsetY()
    {
        return offsetY;
    }

    /**
     * Close a dialog. Does nothing if the given dialog is not open.
     *
     * @param dialog The dialog to close.
     */
    public void closeDialog(final Gui dialog)
    {
        if (openDialogsRemove(dialog))
        {
            dialog.setStateChanged(true);
            final ActivatableGUIElement activeElement = dialog.getActiveElement();
            if (activeElement != null)
            {
                activeElement.setActive(false);
            }
            openDialogsChanged = true;
        }
    }

    /**
     * Toggle a dialog: if the dialog is not shown, show it; else hide it.
     *
     * @param dialog The dialog to toggle.
     *
     * @return Whether the dialog is shown.
     */
    public boolean toggleDialog(final Gui dialog)
    {
        if (dialog == currentGui)
        {
            return true;
        }

        openDialogsChanged = true;
        dialog.setStateChanged(true);

        if (openDialogsRemove(dialog))
        {
            final ActivatableGUIElement activeElement = dialog.getActiveElement();
            if (activeElement != null)
            {
                activeElement.setActive(false);
            }
            return false;
        }

        dialog.setGuiAutoCloseListener(null);
        openDialogsAdd(dialog);
        dialog.activateDefaultElement();
        return true;
    }

    /**
     * Set the tooltip to use, or <code>null</code> if no tooltips should be
     * shown.
     *
     * @param tooltip The tooltip to use, or <code>null</code>.
     */
    public void setTooltip(final AbstractLabel tooltip)
    {
        this.tooltip = tooltip;
    }

    /**
     * Set the current gui state.
     *
     * @param rendererGuiState The gui state.
     */
    public void setGuiState(final RendererGuiState rendererGuiState)
    {
        if (this.rendererGuiState == rendererGuiState)
        {
            return;
        }

        this.rendererGuiState = rendererGuiState;
        forcePaint = true;
        for (final RendererGuiStateListener listener : rendererGuiStateListeners)
        {
            listener.guiStateChanged(rendererGuiState);
        }
    }

    /**
     * Return the current gui state.
     *
     * @return The gui state.
     */
    public RendererGuiState getGuiState()
    {
        return rendererGuiState;
    }

    /**
     * Add a gui state listener to be notified about {@link #rendererGuiState} changes.
     *
     * @param listener The listener to add.
     */
    public void addGuiStateListener(final RendererGuiStateListener listener)
    {
        rendererGuiStateListeners.add(listener);
    }

    /**
     * Adds a dialog to {@link #openDialogs}. Generates mouse events if
     * necessary.
     * @param dialog the dialog
     */
    private void openDialogsAdd(final Gui dialog)
    {
        if (openDialogs.contains(dialog))
        {
            return;
        }

        final Point mouse = window.getMousePosition(true);
        if (mouse == null)
        {
            openDialogs.add(dialog);
        }
        else
        {
            mouse.x -= offsetX;
            mouse.y -= offsetY;
            if (dialog.isWithinDrawingArea(mouse.x, mouse.y))
            {
                final MouseEvent mouseEvent = new MouseEvent(window, 0, System.currentTimeMillis(), 0, mouse.x, mouse.y, 0, false);
                mouseTracker.mouseExited(mouseEvent);
                openDialogs.add(dialog);
                mouseTracker.mouseEntered(mouseEvent);
            }
            else
            {
                openDialogs.add(dialog);
            }
        }
    }

    /**
     * Removes a dialog to {@link #openDialogs}. Generates mouse events if
     * necessary.
     * @param dialog the dialog
     * @return whether the dialog was opened
     */
    private boolean openDialogsRemove(final Gui dialog)
    {
        if (!openDialogs.contains(dialog))
        {
            return false;
        }

        final Point mouse = window.getMousePosition(true);
        if (mouse == null)
        {
            openDialogs.remove(dialog);
        }
        else
        {
            mouse.x -= offsetX;
            mouse.y -= offsetY;
            if (dialog.isWithinDrawingArea(mouse.x, mouse.y))
            {
                final MouseEvent mouseEvent = new MouseEvent(window, 0, System.currentTimeMillis(), 0, mouse.x, mouse.y, 0, false);
                mouseTracker.mouseExited(mouseEvent);
                openDialogs.remove(dialog);
                mouseTracker.mouseEntered(mouseEvent);
            }
            else
            {
                openDialogs.remove(dialog);
            }
        }

        return true;
    }

    /**
     * Deactivates the command input text field. Does nothing if the command
     * input text field is not active.
     * @return  whether the command input text field has been deactivated
     */
    public boolean deactivateCommandInput()
    {
        for (final Gui dialog : getOpenDialogs())
        {
            if (!dialog.isHidden(rendererGuiState))
            {
                if (dialog.deactivateCommandInput())
                {
                    return true;
                }
                if (dialog.isModal())
                {
                    return false;
                }
            }
        }

        return currentGui.deactivateCommandInput();
    }

    /**
     * Returns the active message buffer.
     * @return the active message buffer or <code>null</code> if none is active
     */
    public Buffer getActiveMessageBuffer()
    {
        for (final Gui dialog : getOpenDialogs())
        {
            if (!dialog.isHidden(rendererGuiState))
            {
                final Buffer buffer = getActiveMessageBuffer(dialog);
                if (buffer != null)
                {
                    return buffer;
                }
                if (dialog.isModal())
                {
                    return null;
                }
            }
        }

        return getActiveMessageBuffer(currentGui);
    }

    /**
     * Returns the active message buffer for a {@link Gui} instance.
     * @param gui the gui instance
     * @return the active message buffer or <code>null</code>
     */
    private Buffer getActiveMessageBuffer(final Gui gui)
    {
        final GUIMessageLog buffer = gui.getFirstElement(GUIMessageLog.class);
        return buffer == null ? null : buffer.getBuffer();
    }
}
