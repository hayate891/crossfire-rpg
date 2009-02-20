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

package com.realtime.crossfire.jxclient.gui.item;

import com.realtime.crossfire.jxclient.faces.Face;
import com.realtime.crossfire.jxclient.faces.FacesManager;
import com.realtime.crossfire.jxclient.faces.FacesManagerListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.shortcuts.Shortcut;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutCommand;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutListener;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutSpell;
import com.realtime.crossfire.jxclient.shortcuts.Shortcuts;
import com.realtime.crossfire.jxclient.shortcuts.ShortcutsListener;
import com.realtime.crossfire.jxclient.spells.CurrentSpellManager;
import com.realtime.crossfire.jxclient.spells.Spell;
import com.realtime.crossfire.jxclient.window.JXCWindowRenderer;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;

public class GUIItemShortcut extends GUIItem
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The background color of this item.
     */
    private static final Color BACKGROUND_COLOR = new Color(0, 0, 0, 0.0f);

    /**
     * The default tooltip text for empty slots.
     */
    private static final String DEFAULT_TOOLTIP_TEXT = "(empty)";

    private final Shortcuts shortcuts;

    /**
     * The {@link FacesManager} instance for looking up faces.
     */
    private final FacesManager facesManager;

    private final Color cursedColor;

    private final BufferedImage cursedImage;

    private final Color appliedColor;

    private final BufferedImage appliedImage;

    private final Font font;

    private final int index;

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
    private Shortcut shortcut = null;

    private final ShortcutsListener shortcutsListener = new ShortcutsListener()
    {
        /** {@inheritDoc} */
        @Override
        public void shortcutAdded(final int index, final Shortcut shortcut)
        {
            if (index == GUIItemShortcut.this.index)
            {
                setShortcut(shortcut);
            }
        }

        /** {@inheritDoc} */
        @Override
        public void shortcutRemoved(final int index, final Shortcut shortcut)
        {
            if (index == GUIItemShortcut.this.index)
            {
                setShortcut(null);
            }
        }
    };

    private final ShortcutListener shortcutListener = new ShortcutListener()
    {
        /** {@inheritDoc} */
        @Override
        public void shortcutModified()
        {
            setChanged();
            updateTooltipText();
        }
    };

    /**
     * The {@link FacesManagerListener} registered to detect updated faces.
     */
    private final FacesManagerListener facesManagerListener = new FacesManagerListener()
    {
        /** {@inheritDoc} */
        @Override
        public void faceUpdated(final Face face)
        {
            if (shortcut != null && shortcut instanceof ShortcutSpell && face.getFaceNum() == ((ShortcutSpell)shortcut).getSpell().getFaceNum())
            {
                setChanged();
            }
        }
    };

    public GUIItemShortcut(final TooltipManager tooltipManager, final JXCWindowRenderer windowRenderer, final String name, final int x, final int y, final int w, final int h, final Color cursedColor, final BufferedImage cursedImage, final Color appliedColor, final BufferedImage appliedImage, final int index, final FacesManager facesManager, final Shortcuts shortcuts, final Font font, final CurrentSpellManager currentSpellManager)
    {
        super(tooltipManager, windowRenderer, name, x, y, w, h);
        this.shortcuts = shortcuts;
        this.facesManager = facesManager;
        this.cursedColor = cursedColor;
        this.cursedImage = cursedImage;
        this.appliedColor = appliedColor;
        this.appliedImage = appliedImage;
        this.font = font;
        this.index = index;
        this.currentSpellManager = currentSpellManager;
        this.shortcuts.addShortcutsListener(shortcutsListener);
        this.w = w;
        this.h = h;
        this.facesManager.addFacesManagerListener(facesManagerListener);
        updateTooltipText();
    }

    /** {@inheritDoc} */
    @Override
    public void dispose()
    {
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
    private void setShortcut(final Shortcut shortcut)
    {
        if (this.shortcut == shortcut)
        {
            return;
        }

        if (this.shortcut != null)
        {
            this.shortcut.removeShortcutListener(shortcutListener);
        }
        this.shortcut = shortcut;
        if (this.shortcut != null)
        {
            this.shortcut.addShortcutListener(shortcutListener);
        }
        setChanged();

        updateTooltipText();
    }

    /* {@inheritDoc} */
    @Override
    public void button1Clicked(final int modifiers)
    {
        final Shortcut shortcut = shortcuts.getShortcut(index);
        if (shortcut != null)
        {
            shortcut.execute();
        }
    }

    /* {@inheritDoc} */
    @Override
    public void button2Clicked(final int modifiers)
    {
        final Shortcut shortcut = shortcuts.getShortcut(index);
        if (shortcut != null)
        {
            if (shortcut instanceof ShortcutSpell)
            {
                final ShortcutSpell shortcutSpell = (ShortcutSpell)shortcut;
                shortcutSpell.setCast(!shortcutSpell.isCast());
            }
        }
    }

    /* {@inheritDoc} */
    @Override
    public void button3Clicked(final int modifiers)
    {
        final Spell spell = currentSpellManager.getCurrentSpell();
        if (spell == null)
        {
           return;
        }

        shortcuts.setSpellShortcut(index, spell, true);
    }

    /* {@inheritDoc} */
    @Override
    protected void render(final Graphics g)
    {
        final Graphics2D g2 = (Graphics2D)g;
        g2.setBackground(BACKGROUND_COLOR);
        g.clearRect(0, 0, getWidth(), getHeight());

        final Shortcut shortcut = shortcuts.getShortcut(index);
        if (shortcut == null)
        {
            return;
        }

        if (shortcut instanceof ShortcutSpell)
        {
            final ShortcutSpell shortcutSpell = (ShortcutSpell)shortcut;
            final Color color = shortcutSpell.isCast() ? cursedColor : appliedColor;
            if (color != null)
            {
                g.setColor(color);
                g.fillRect(0, 0, w, h);
            }
            g.drawImage(facesManager.getOriginalImageIcon(shortcutSpell.getSpell().getFaceNum()).getImage(), 0, 0, null);
            final BufferedImage image = shortcutSpell.isCast() ? cursedImage : appliedImage;
            if (image != null)
            {
                g.drawImage(image, 0, 0, null);
            }
        }
        else if (shortcut instanceof ShortcutCommand)
        {
            // XXX: todo
        }
        else
        {
            throw new AssertionError();
        }
        g.setFont(font);
        g.setColor(Color.YELLOW);
        g.drawString("F"+(index+1), 1, 1+font.getSize()); // XXX: define in skin
    }

    /** {@inheritDoc} */
    @Override
    public boolean canScroll(final int distance)
    {
        return false;
    }

    /** {@inheritDoc} */
    @Override
    public void scroll(final int distance)
    {
    }

    /** {@inheritDoc} */
    @Override
    public void resetScroll()
    {
    }

    /**
     * Updates the tooltip text to reflect current settings.
     */
    private void updateTooltipText()
    {
        setTooltipText(shortcut == null ? DEFAULT_TOOLTIP_TEXT : shortcut.getTooltipText());
    }
}
