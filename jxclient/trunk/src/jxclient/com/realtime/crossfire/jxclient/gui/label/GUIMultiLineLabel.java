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

package com.realtime.crossfire.jxclient.gui.label;

import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.skin.skin.Extent;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.geom.RectangularShape;
import java.awt.image.BufferedImage;
import java.util.regex.Pattern;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * A {@link AbstractLabel} that renders the text as a list of plain strings. The
 * lines are separated by newline characters.
 * @author Andreas Kirschbaum
 */
public class GUIMultiLineLabel extends GUILabel {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The pattern to split the text into lines.
     */
    @NotNull
    private static final Pattern LINE_SEPARATOR_PATTERN = Pattern.compile(" *\n");

    /**
     * The text lines to draw.
     */
    @Nullable
    private String[] lines = null;

    /**
     * Create a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param name The name of this element.
     * @param extent the extent of this element
     * @param picture The background image; <code>null</code> for no
     * background.
     * @param font The font for rendering the label text.
     * @param color The font color.
     * @param backgroundColor The background color.
     * @param alignment The text alignment.
     * @param text The label text.
     */
    public GUIMultiLineLabel(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, @NotNull final Extent extent, @Nullable final BufferedImage picture, @NotNull final Font font, @NotNull final Color color, @NotNull final Color backgroundColor, @NotNull final Alignment alignment, @NotNull final String text) {
        super(tooltipManager, elementListener, name, extent, picture, font, color, backgroundColor, alignment);
        setText(text);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void textChanged() {
        lines = LINE_SEPARATOR_PATTERN.split(getText(), -1);
        super.textChanged();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void render(@NotNull final Graphics2D g2) {
        super.render(g2);

        if (lines == null || lines.length <= 0) {
            return;
        }

        final Font font = getTextFont();
        if (font == null) {
            return;
        }

        final RectangularShape rectangle = font.getStringBounds("X", g2.getFontRenderContext());
        final int lineHeight = (int)Math.ceil(rectangle.getMaxY()-rectangle.getMinY());

        int y = 0;
        for (final String line : lines) {
            y += drawLine(g2, y, lineHeight, line);
        }
    }

}
