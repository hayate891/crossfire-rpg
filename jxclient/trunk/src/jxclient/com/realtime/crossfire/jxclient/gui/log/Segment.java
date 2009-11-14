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

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * One segment of a {@link Line} which should be displayed without changing
 * text attributes.
 *
 * @author Andreas Kirschbaum
 */
public class Segment
{
    /**
     * The text to display.
     */
    @NotNull
    private final String text;

    /**
     * Whether bold face is enabled.
     */
    private final boolean bold;

    /**
     * Whether italic face is enabled.
     */
    private final boolean italic;

    /**
     * Whether underlining is enabled.
     */
    private final boolean underline;

    /**
     * The font to use.
     */
    @NotNull
    private final FontID font;

    /**
     * The color to use. <code>null</code> means "default color".
     */
    @Nullable
    private final Color color;

    /**
     * The x-coordinate to display the segment. Set to <code>-1</code> if
     * unknown.
     */
    private int x = -1;

    /**
     * The y-coordinate to display the segment. Set to <code>-1</code> if
     * unknown.
     */
    private int y = -1;

    /**
     * The width of the segment if displayed. Set to <code>-1</code> if
     * unknown.
     */
    private int width = -1;

    /**
     * The distance of the underline to the base line. Set to <code>0</code> if
     * unknown.
     */
    private int underlineOffset = 0;

    /**
     * Create a new segment.
     *
     * @param text The text to display.
     *
     * @param bold Whether bold face is enabled.
     *
     * @param italic Whether italic face is enabled.
     *
     * @param underline Whether underlining is enabled.
     *
     * @param font The font to use.
     *
     * @param color The color to use; <code>null</code> means "default color".
     */
    public Segment(@NotNull final String text, final boolean bold, final boolean italic, final boolean underline, @NotNull final FontID font, @Nullable final Color color)
    {
        this.text = text;
        this.bold = bold;
        this.italic = italic;
        this.underline = underline;
        this.font = font;
        this.color = color;
    }

    /**
     * Return the text to display.
     *
     * @return The text to display.
     */
    @NotNull
    public String getText()
    {
        return text;
    }

    /**
     * Return whether bold face is enabled.
     *
     * @return Whether bold face is enabled.
     */
    public boolean isBold()
    {
        return bold;
    }

    /**
     * Return whether italic face is enabled.
     *
     * @return Whether italic face is enabled.
     */
    public boolean isItalic()
    {
        return italic;
    }

    /**
     * Return whether underlining is enabled.
     *
     * @return Whether underlining is enabled.
     */
    public boolean isUnderline()
    {
        return underline;
    }

    /**
     * Return the font to use.
     *
     * @return The font to use.
     */
    @NotNull
    public FontID getFontID()
    {
        return font;
    }

    /**
     * Return the color to use.
     *
     * @return The color to use; <code>null</code> means "default color".
     */
    @Nullable
    public Color getColor()
    {
        return color;
    }

    /**
     * Return the x-coordinate to display the segment. Returns <code>-1</code>
     * until {@link #setX(int)} was called
     *
     * @return The x-coordinate.
     */
    public int getX()
    {
        return x;
    }

    /**
     * Set the x-coordinate to display the segment.
     *
     * @param x The x-coordinate.
     */
    public void setX(final int x)
    {
        this.x = x;
    }

    /**
     * Return the y-coordinate to display the segment. Returns <code>-1</code>
     * until {@link #setY(int)} was called
     *
     * @return The y-coordinate.
     */
    public int getY()
    {
        return y;
    }

    /**
     * Set the y-coordinate to display the segment.
     *
     * @param y The y-coordinate.
     */
    public void setY(final int y)
    {
        this.y = y;
    }

    /**
     * Return the width of the segment. Returns <code>-1</code> until {@link
     * #setWidth(int)} was called
     *
     * @return The width.
     */
    public int getWidth()
    {
        return width;
    }

    /**
     * Set the width to display the segment.
     *
     * @param width The width.
     */
    public void setWidth(final int width)
    {
        this.width = width;
    }

    /**
     * Return the distance of the underline to the base line. Returns
     * <code>0</code> until {@link #setUnderlineOffset(int)} was called.
     *
     * @return The distance.
     */
    public int getUnderlineOffset()
    {
        return underlineOffset;
    }

    /**
     * Set the distance of the underline to the base line.
     *
     * @param underlineOffset The distance.
     */
    public void setUnderlineOffset(final int underlineOffset)
    {
        this.underlineOffset = underlineOffset;
    }

    /**
     * Return the {@link Font} to use for a given {@link Segment}.
     *
     * @param fonts The <code>Fonts</code> instance to use.
     *
     * @return The font.
     */
    @NotNull
    public Font getFont(@NotNull final Fonts fonts)
    {
        switch (font)
        {
        case PRINT:
            return fonts.getFontPrint();

        case FIXED:
            return bold ? fonts.getFontFixedBold() : fonts.getFontFixed();

        case ARCANE:
            return fonts.getFontArcane();

        case HAND:
            return fonts.getFontPrint();

        case STRANGE:
            return fonts.getFontPrint();
        }

        throw new AssertionError();
    }

    /**
     * Draws this segment to a {@link Graphics} instance.
     * @param g the graphics to draw to
     * @param y the y-coordinate to draw to
     * @param fonts the fonts to use
     */
    public void draw(@NotNull final Graphics g, final int y, @NotNull final Fonts fonts)
    {
        g.setColor(color);
        g.setFont(getFont(fonts));
        g.drawString(text, x, y+this.y);
        if (underline)
        {
            g.drawLine(x, y+this.y+underlineOffset, x+width-1, y+this.y+underlineOffset);
        }
    }
}
