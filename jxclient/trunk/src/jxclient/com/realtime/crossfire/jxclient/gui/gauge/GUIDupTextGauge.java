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
import com.realtime.crossfire.jxclient.skin.skin.Extent;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.geom.RectangularShape;
import java.awt.image.BufferedImage;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * A {@link GUIDupGauge} which displays the current value as a text string on
 * top of the gauge.
 * @author Andreas Kirschbaum
 */
public class GUIDupTextGauge extends GUIDupGauge {

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
     * @param extent the extent of this element
     * @param pictureFullDiv the top image
     * @param pictureFullMod the bottom image
     * @param pictureEmpty the image for an empty gauge
     * @param orientationDiv the gauge's orientation
     * @param orientationMod the gauge's orientation
     * @param tooltipPrefix the prefix for generating a tooltip
     * @param color the text color
     * @param font the text font
     */
    public GUIDupTextGauge(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, @NotNull final Extent extent, @NotNull final BufferedImage pictureFullDiv, final BufferedImage pictureFullMod, @NotNull final BufferedImage pictureEmpty, @NotNull final Orientation orientationDiv, @NotNull final Orientation orientationMod, @Nullable final String tooltipPrefix, @NotNull final Color color, @NotNull final Font font) {
        super(tooltipManager, elementListener, name, extent, pictureFullDiv, pictureFullMod, pictureEmpty, orientationDiv, orientationMod, tooltipPrefix);
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
    protected void render(@NotNull final Graphics2D g2) {
        super.render(g2);

        if (font == null) {
            return;
        }

        g2.setBackground(new Color(0, 0, 0, 0.0f));
        g2.setColor(color);
        g2.setFont(font);
        final String text = labelText;
        final RectangularShape rectangle = font.getStringBounds(text, g2.getFontRenderContext());
        g2.drawString(text, (int)Math.round((getWidth()-rectangle.getWidth())/2), (int)Math.round(getHeight()-rectangle.getMaxY()-rectangle.getMinY())/2);
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
