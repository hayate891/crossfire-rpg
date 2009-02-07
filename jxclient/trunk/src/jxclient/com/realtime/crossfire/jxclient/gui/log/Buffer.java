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
package com.realtime.crossfire.jxclient.gui.log;

import java.awt.Font;
import java.awt.font.FontRenderContext;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

/**
 * Manages the contents of the contents of a log window. It consists of a list
 * of {@link Line}s.
 *
 * @author Andreas Kirschbaum
 */
public class Buffer
{
    /**
     * The maximum number of lines the buffer can hold.
     */
    public static final int MAX_LINES = 250;

    /**
     * The listeners to notify about changes.
     */
    private final List<BufferListener> listeners = new ArrayList<BufferListener>();

    /**
     * The {@link Fonts} instance for looking up fonts.
     */
    private final Fonts fonts;

    /**
     * The {@link FontRenderContext} to use.
     */
    private final FontRenderContext context;

    /**
     * The width to render.
     */
    private final int renderWidth;

    /**
     * The lines in display order.
     */
    private final List<Line> lines = new ArrayList<Line>();

    /**
     * The total height of all {@link #lines}.
     */
    private int totalHeight = 0;

    /**
     * Object to synchronized access to {@link #lines} and {@link
     * #totalHeight}.
     */
    private final Object sync = new Object();

    /**
     * Create a new instance.
     *
     * @param fonts The <code>Fonts</code> instance for looking up fonts.
     *
     * @param context The <code>FontRenderContext</code> to use.
     *
     * @param renderWidth The width to render.
     */
    public Buffer(final Fonts fonts, final FontRenderContext context, final int renderWidth)
    {
        this.fonts = fonts;
        this.context = context;
        this.renderWidth = renderWidth;
    }

    /**
     * Clear all lines from the buffer.
     */
    public void clear()
    {
        final List<Line> removedLines;
        synchronized (sync)
        {
            removedLines = new ArrayList<Line>(lines);
            totalHeight = 0;
            lines.clear();
        }
        for (final BufferListener listener : listeners)
        {
            listener.linesRemoved(removedLines);
        }
    }

    /**
     * Append a {@link Line} to the end of the buffer.
     *
     * @param line The line to append.
     */
    public void addLine(final Line line)
    {
        final int height = calculateHeight(line);
        line.setHeight(height);
        synchronized (sync)
        {
            totalHeight += height;
            lines.add(line);
        }

        for (final BufferListener listener : listeners)
        {
            listener.linesAdded(1);
        }
    }

    /**
     * Replace the last {@link Line} of this buffer.
     *
     * @param line The replacing line.
     */
    public void replaceLine(final Line line)
    {
        final int height = calculateHeight(line);
        line.setHeight(height);
        synchronized (sync)
        {
            totalHeight += height;
            final int lastIndex = lines.size()-1;
            totalHeight -= lines.get(lastIndex).getHeight();
            lines.set(lastIndex, line);
        }

        for (final BufferListener listener : listeners)
        {
            listener.linesReplaced(1);
        }
    }

    /**
     * Prune excess lines.
     */
    public void prune()
    {
        final List<Line> removedLines;
        synchronized (sync)
        {
            if (lines.size() <= MAX_LINES)
            {
                return;
            }

            removedLines = new ArrayList<Line>(lines.size()-MAX_LINES);
            while (lines.size() > MAX_LINES)
            {
                final Line line = lines.remove(0);
                removedLines.add(line);
                totalHeight -= line.getHeight();
            }
        }
        for (final BufferListener listener : listeners)
        {
            listener.linesRemoved(removedLines);
        }
    }

    /**
     * Return one {@link Line} by line index. The first line has the index
     * <code>0</code>.
     *
     * @param line The line index.
     *
     * @return The line.
     */
    public Line getLine(final int line)
    {
        synchronized (sync)
        {
            return lines.get(line);
        }
    }

    /**
     * Return the total height of all lines.
     *
     * @return The total height.
     */
    public int getTotalHeight()
    {
        synchronized (sync)
        {
            return totalHeight;
        }
    }

    /**
     * Return an {@link Iterator} for the lines in this buffer. The caller must
     * hold {@link #sync}'s lock.
     * @return the iterator
     */
    public Iterator<Line> iterator()
    {
        assert Thread.holdsLock(sync);
        return Collections.unmodifiableList(lines).iterator();
    }

    /**
     * Return a {@link ListIterator} for the lines in this buffer.
     *
     * @param line The initial line index of the list iterator.
     *
     * @return The list iterator.
     */
    public ListIterator<Line> listIterator(final int line)
    {
        assert Thread.holdsLock(sync);
        return Collections.unmodifiableList(lines).listIterator(line);
    }

    /**
     * Return the number of lines.
     *
     * @return The number of lines.
     */
    public int size()
    {
        synchronized (sync)
        {
            return lines.size();
        }
    }

    /**
     * Determine the height of a {@link Line} in pixels.
     *
     * @param line The line to process.
     *
     * @return The height in pixels.
     */
    private int calculateHeight(final Line line)
    {
        int height = 0;
        int x = 0;
        int minY = 0;
        int maxY = 0;
        int beginIndex = 0;
        int i = 0;
        for (final Segment segment : line)
        {
            final String text = segment.getText();
            final Font font = segment.getFont(fonts);
            final Rectangle2D rect = font.getStringBounds(text, context);
            final int width = (int)Math.round(rect.getWidth());
            if (x != 0 && x+width > renderWidth)
            {
                line.updateAttributes(beginIndex, i, height, minY, fonts, context);

                height += maxY-minY;
                x = 0;
                minY = 0;
                maxY = 0;
                beginIndex = i;
            }

            segment.setX(x);
            segment.setY(height);
            segment.setWidth(width);

            x += width;
            minY = (int)Math.min(minY, Math.round(rect.getY()));
            maxY = (int)Math.max(maxY, Math.round(rect.getY()+rect.getHeight()));

            i++;
        }

        line.updateAttributes(beginIndex, i, height, minY, fonts, context);
        height += maxY-minY;

        return Math.max(1, height);
    }

    /**
     * Add a listener to notify of changes.
     *
     * @param listener The listener.
     */
    public void addBufferListener(final BufferListener listener)
    {
        listeners.add(listener);
    }

    /**
     * Removes a listener to be notified of changes.
     * @param listener the listener
     */
    public void removeBufferListener(final BufferListener listener)
    {
        listeners.remove(listener);
    }

    /**
     * Returns the object to synchronize on when calling {@link #iterator()} or
     * {@link #listIterator(int)}.
     * @return the object
     */
    public Object getSyncObject()
    {
        return sync;
    }
}
