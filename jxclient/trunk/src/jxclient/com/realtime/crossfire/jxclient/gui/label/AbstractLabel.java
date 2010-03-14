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

import com.realtime.crossfire.jxclient.gui.gui.GUIElement;
import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import javax.swing.ImageIcon;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Abstract base class for all label classes. It manages the label text, and
 * renders the label's background.
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public abstract class AbstractLabel extends GUIElement {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The label contents.
     */
    @NotNull
    private String text = "";

    @Nullable
    private ImageIcon backgroundImage;

    /**
     * If set, the opaque background color. This field is ignored if {@link
     * #backgroundImage} is set.
     */
    @NotNull
    private final Color backgroundColor;

    protected AbstractLabel(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, final int x, final int y, final int w, final int h, @Nullable final BufferedImage picture, @NotNull final Color backgroundColor) {
        super(tooltipManager, elementListener, name, x, y, w, h, Transparency.TRANSLUCENT);
        backgroundImage = picture == null ? null : new ImageIcon(picture);
        this.backgroundColor = backgroundColor;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void dispose() {
        super.dispose();
    }

    public void setText(@NotNull final String text) {
        if (!this.text.equals(text)) {
            this.text = text;
            textChanged();
        }
    }

    /**
     * Will be called whenever {@link #text} has changed.
     */
    protected abstract void textChanged();

    /**
     * Return the label text.
     * @return The label text.
     */
    @NotNull
    protected String getText() {
        return text;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void render(@NotNull final Graphics2D g) {
        if (backgroundImage != null) {
            g.drawImage(backgroundImage.getImage(), 0, 0, null);
        } else {
            g.setBackground(backgroundColor);
            g.clearRect(0, 0, getWidth(), getHeight());
        }
    }

    /**
     * Set the background image.
     * @param backgroundImage The new background image.
     */
    protected void setBackgroundImage(@Nullable final ImageIcon backgroundImage) {
        this.backgroundImage = backgroundImage;
        setChanged();
    }

}
