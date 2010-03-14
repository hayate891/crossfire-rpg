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

package com.realtime.crossfire.jxclient.gui.gauge;

import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.geom.RectangularShape;
import java.awt.image.BufferedImage;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * A {@link GUIGauge} which displays the current value as a text string on top
 * of the gauge.
 * @author Andreas Kirschbaum
 */
public class GUITextGauge extends GUIGauge {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The text color.
     */
    @NotNull
    private final Color color;

    /**
     * The text font.
     */
    @NotNull
    private final Font font;

    /**
     * The label text.
     */
    @NotNull
    private String labelText = "";

    /**
     * Creates a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param name the name of this element
     * @param x the x-coordinate for drawing this element to screen
     * @param y the y-coordinate for drawing this element to screen
     * @param w the width for drawing this element to screen
     * @param h the height for drawing this element to screen
     * @param pictureFull the image for positive values
     * @param pictureNegative the image for negative values
     * @param pictureEmpty the image for an empty gauge
     * @param orientation the gauge's orientation
     * @param tooltipPrefix the prefix for generating a tooltip
     * @param color the text color
     * @param font the text font
     */
    public GUITextGauge(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, final int x, final int y, final int w, final int h, @NotNull final BufferedImage pictureFull, @Nullable final BufferedImage pictureNegative, @NotNull final BufferedImage pictureEmpty, @NotNull final Orientation orientation, @Nullable final String tooltipPrefix, @NotNull final Color color, @NotNull final Font font) {
        super(tooltipManager, elementListener, name, x, y, w, h, pictureFull, pictureNegative, pictureEmpty, orientation, tooltipPrefix);
        this.color = color;
        this.font = font;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void dispose() {
        super.dispose();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void render(@NotNull final Graphics2D g) {
        super.render(g);

        if (font == null) {
            return;
        }

        g.setBackground(new Color(0, 0, 0, 0.0f));
        g.setColor(color);
        g.setFont(font);
        final String text = labelText;
        final RectangularShape rect = font.getStringBounds(text, g.getFontRenderContext());
        g.drawString(text, (int)Math.round((getWidth()-rect.getWidth())/2), (int)Math.round((getHeight()-rect.getMaxY()-rect.getMinY()))/2);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setValues(final int curValue, final int minValue, final int maxValue, @NotNull final String labelText, @NotNull final String tooltipText) {
        super.setValues(curValue, minValue, maxValue, labelText, tooltipText);
        this.labelText = labelText;
        setChanged();
    }

}
