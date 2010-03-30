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

import com.realtime.crossfire.jxclient.faces.Face;
import com.realtime.crossfire.jxclient.faces.FacesManager;
import com.realtime.crossfire.jxclient.faces.FacesManagerListener;
import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.shortcuts.Shortcut;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutCommand;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutListener;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutSpell;
import com.realtime.crossfire.jxclient.shortcuts.Shortcuts;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutsListener;
import com.realtime.crossfire.jxclient.skin.skin.Extent;
import com.realtime.crossfire.jxclient.spells.CurrentSpellManager;
import com.realtime.crossfire.jxclient.spells.Spell;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Image;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Displays a shortcut command.
 * @author Andreas Kirschbaum
 */
public class GUIItemShortcut extends GUIItem {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The background color of this item.
     */
    @NotNull
    private static final Color BACKGROUND_COLOR = new Color(0, 0, 0, 0.0f);

    /**
     * The default tooltip text for empty slots.
     */
    @NotNull
    private static final String DEFAULT_TOOLTIP_TEXT = "(empty)";

    /**
     * The {@link Shortcuts} instance for looking up {@link Shortcut}s.
     */
    @NotNull
    private final Shortcuts shortcuts;

    /**
     * The {@link FacesManager} instance for looking up faces.
     */
    @NotNull
    private final FacesManager facesManager;

    /**
     * The background color for shortcuts that /cast a spell.
     */
    @Nullable
    private final Color castColor;

    /**
     * The overlay image for shortcuts that /cast a spell.
     */
    @Nullable
    private final Image castImage;

    /**
     * The background color for shortcuts that /invoke a spell.
     */
    @Nullable
    private final Color invokeColor;

    /**
     * The overlay image for shortcuts that /invoke a spell.
     */
    @Nullable
    private final Image invokeImage;

    /**
     * The {@link Font} for displaying the key that activates the shortcut.
     */
    @NotNull
    private final Font font;

    /**
     * The shortcut index.
     */
    private final int index;

    /**
     * The {@link CurrentSpellManager} for tracking the active spell.
     */
    @NotNull
    private final CurrentSpellManager currentSpellManager;

    /**
     * The item's width in pixel.
     */
    private final int w;

    /**
     * The item's height in pixel.
     */
    private final int h;

    /**
     * The currently monitored {@link Shortcut} instance. Set to
     * <code>null</code> if not active.
     */
    @Nullable
    private Shortcut shortcut = null;

    /**
     * The {@link ShortcutsListener} attached to {@link #shortcuts}.
     */
    @NotNull
    private final ShortcutsListener shortcutsListener = new ShortcutsListener() {
        /** {@inheritDoc} */
        @Override
        public void shortcutAdded(final int index, @NotNull final Shortcut shortcut) {
            if (index == GUIItemShortcut.this.index) {
                setShortcut(shortcut);
            }
        }

        /** {@inheritDoc} */
        @Override
        public void shortcutRemoved(final int index, @NotNull final Shortcut shortcut) {
            if (index == GUIItemShortcut.this.index) {
                setShortcut(null);
            }
        }
    };

    /**
     * The {@link ShortcutListener} attached to {@link #shortcut}.
     */
    @NotNull
    private final ShortcutListener shortcutListener = new ShortcutListener() {
        /** {@inheritDoc} */
        @Override
        public void shortcutModified() {
            setChanged();
            updateTooltipText();
        }
    };

    /**
     * The {@link FacesManagerListener} registered to detect updated faces.
     */
    @NotNull
    private final FacesManagerListener facesManagerListener = new FacesManagerListener() {
        /** {@inheritDoc} */
        @Override
        public void faceUpdated(@NotNull final Face face) {
            if (shortcut != null && shortcut instanceof ShortcutSpell && face.getFaceNum() == ((ShortcutSpell)shortcut).getSpell().getFaceNum()) {
                setChanged();
            }
        }
    };

    /**
     * Creates a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param name the name of this element
     * @param extent the extent of this element
     * @param castColor the background color for shortcuts that /cast a spell
     * @param castImage the overlay image for shortcuts that /cast a spell
     * @param invokeColor the background color for shortcuts that /invoke a
     * spell
     * @param invokeImage the overlay image for shortcuts that /invoke a spell
     * @param index the spell index
     * @param facesManager the faces manager instance for looking up faces
     * @param shortcuts the shortcuts instance for looking up shortcuts
     * @param font the font for displaying the key that activates the shortcut
     * @param currentSpellManager the current spell manager for tracking the
     * active spell
     */
    public GUIItemShortcut(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, @NotNull final Extent extent, @Nullable final Color castColor, @Nullable final Image castImage, @Nullable final Color invokeColor, @Nullable final Image invokeImage, final int index, @NotNull final FacesManager facesManager, @NotNull final Shortcuts shortcuts, @NotNull final Font font, @NotNull final CurrentSpellManager currentSpellManager) {
        super(tooltipManager, elementListener, name, extent);
        this.shortcuts = shortcuts;
        this.facesManager = facesManager;
        this.castColor = castColor;
        this.castImage = castImage;
        this.invokeColor = invokeColor;
        this.invokeImage = invokeImage;
        this.font = font;
        this.index = index;
        this.currentSpellManager = currentSpellManager;
        this.shortcuts.addShortcutsListener(shortcutsListener);
        w = extent.getConstantW();
        h = extent.getConstantH();
        this.facesManager.addFacesManagerListener(facesManagerListener);
        updateTooltipText();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void dispose() {
        super.dispose();
        facesManager.removeFacesManagerListener(facesManagerListener);
        shortcuts.removeShortcutsListener(shortcutsListener);
        setShortcut(null);
    }

    /**
     * Updates {@link #shortcut} and registers/deregisteres {@link
     * #shortcutListener}.
     * @param shortcut the new shortcut
     */
    private void setShortcut(@Nullable final Shortcut shortcut) {
        if (this.shortcut == shortcut) {
            return;
        }

        if (this.shortcut != null) {
            this.shortcut.removeShortcutListener(shortcutListener);
        }
        this.shortcut = shortcut;
        if (this.shortcut != null) {
            this.shortcut.addShortcutListener(shortcutListener);
        }
        setChanged();

        updateTooltipText();
    }

    /* {@inheritDoc} */
    @Override
    public void button1Clicked(final int modifiers) {
        if (shortcut != null) {
            shortcut.execute();
        }
    }

    /* {@inheritDoc} */
    @Override
    public void button2Clicked(final int modifiers) {
        if (shortcut != null) {
            if (shortcut instanceof ShortcutSpell) {
                final ShortcutSpell shortcutSpell = (ShortcutSpell)shortcut;
                shortcutSpell.setCast(!shortcutSpell.isCast());
            }
        }
    }

    /* {@inheritDoc} */
    @Override
    public void button3Clicked(final int modifiers) {
        final Spell spell = currentSpellManager.getCurrentSpell();
        if (spell == null) {
            return;
        }

        shortcuts.setSpellShortcut(index, spell, true);
    }

    /* {@inheritDoc} */
    @Override
    protected void render(@NotNull final Graphics2D g) {
        g.setBackground(BACKGROUND_COLOR);
        g.clearRect(0, 0, getWidth(), getHeight());

        if (shortcut == null) {
            return;
        }

        if (shortcut instanceof ShortcutSpell) {
            final ShortcutSpell shortcutSpell = (ShortcutSpell)shortcut;
            final Color color = shortcutSpell.isCast() ? castColor : invokeColor;
            if (color != null) {
                g.setColor(color);
                g.fillRect(0, 0, w, h);
            }
            g.drawImage(facesManager.getOriginalImageIcon(shortcutSpell.getSpell().getFaceNum()).getImage(), 0, 0, null);
            final Image image = shortcutSpell.isCast() ? castImage : invokeImage;
            if (image != null) {
                g.drawImage(image, 0, 0, null);
            }
        } else if (shortcut instanceof ShortcutCommand) {
            // XXX: todo
        } else {
            throw new AssertionError();
        }
        g.setFont(font);
        g.setColor(Color.YELLOW);
        g.drawString("F"+(index+1), 1, 1+font.getSize()); // XXX: define in skin
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean canScroll(final int distance) {
        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void scroll(final int distance) {
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void resetScroll() {
    }

    /**
     * Updates the tooltip text to reflect current settings.
     */
    private void updateTooltipText() {
        setTooltipText(shortcut == null ? DEFAULT_TOOLTIP_TEXT : shortcut.getTooltipText());
    }

}
