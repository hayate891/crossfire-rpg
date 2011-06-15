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
 * Copyright (C) 2006-2011 Andreas Kirschbaum.
 */

package com.realtime.crossfire.jxclient.spells;

import com.realtime.crossfire.jxclient.guistate.GuiStateListener;
import com.realtime.crossfire.jxclient.guistate.GuiStateManager;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireSpellListener;
import com.realtime.crossfire.jxclient.server.socket.ClientSocketState;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Manages all known spells.
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class SpellsManager implements Iterable<Spell> {

    /**
     * All known spells.
     */
    @NotNull
    private final List<Spell> spells = new ArrayList<Spell>();

    /**
     * All unknown spells that have been referenced before. Maps spell name to
     * {@link Spell} instance.
     */
    @NotNull
    private final Map<String, Spell> unknownSpells = new HashMap<String, Spell>();

    /**
     * The {@link SpellsManagerListener SpellsManagerListeners} to notify about
     * changes.
     */
    @NotNull
    private final Collection<SpellsManagerListener> listeners = new ArrayList<SpellsManagerListener>();

    /**
     * A {@link Comparator} to compare {@link Spell} instances by spell path and
     * name.
     */
    @NotNull
    private final Comparator<Spell> spellNameComparator = new SpellComparator();

    /**
     * The listener to receive updates for spell information.
     */
    @NotNull
    private final CrossfireSpellListener crossfireSpellListener = new CrossfireSpellListener() {

        @Override
        public void addSpell(final int tag, final int level, final int castingTime, final int mana, final int grace, final int damage, final int skill, final int path, final int face, @NotNull final String name, @NotNull final String message) {
            SpellsManager.this.addSpell(tag, level, castingTime, mana, grace, damage, skill, path, face, name, message);
        }

        @Override
        public void deleteSpell(final int tag) {
            SpellsManager.this.deleteSpell(tag);
        }

        @Override
        public void updateSpell(final int flags, final int tag, final int mana, final int grace, final int damage) {
            SpellsManager.this.updateSpell(flags, tag, mana, grace, damage);
        }

    };

    /**
     * The {@link GuiStateListener} for detecting established or dropped
     * connections.
     */
    @NotNull
    private final GuiStateListener guiStateListener = new GuiStateListener() {

        @Override
        public void start() {
            // ignore
        }

        @Override
        public void metaserver() {
            // ignore
        }

        @Override
        public void preConnecting(@NotNull final String serverInfo) {
            // ignore
        }

        @Override
        public void connecting(@NotNull final String serverInfo) {
            spells.clear();
        }

        @Override
        public void connecting(@NotNull final ClientSocketState clientSocketState) {
            // ignore
        }

        @Override
        public void connected() {
            // ignore
        }

        @Override
        public void connectFailed(@NotNull final String reason) {
            // ignore
        }

    };

    /**
     * Create a new instance.
     * @param crossfireServerConnection the connection to listen on
     * @param guiStateManager the gui state manager to watch
     */
    public SpellsManager(@NotNull final CrossfireServerConnection crossfireServerConnection, @NotNull final GuiStateManager guiStateManager) {
        crossfireServerConnection.addCrossfireSpellListener(crossfireSpellListener);
        guiStateManager.addGuiStateListener(guiStateListener);
    }

    /**
     * Adds a {@link SpellsManagerListener} to notify about changes.
     * @param listener the listener to add
     */
    public void addCrossfireSpellChangedListener(@NotNull final SpellsManagerListener listener) {
        listeners.add(listener);
    }

    /**
     * Removes a {@link SpellsManagerListener} to notify about changes.
     * @param listener the listener to remove
     */
    public void removeCrossfireSpellChangedListener(@NotNull final SpellsManagerListener listener) {
        listeners.remove(listener);
    }

    /**
     * Adds a new spell. Re-uses entries from {@link #unknownSpells} if
     * possible.
     * @param tag the spell's tag
     * @param level the spell's level
     * @param castingTime the spell's casting time
     * @param mana the spell's mana cost
     * @param grace the spell's grace cost
     * @param damage the spell's damage
     * @param skill the spell's skill
     * @param path the spell's path
     * @param faceNum the spell's face number
     * @param spellName the spell's name
     * @param message the spells' description
     */
    private void addSpell(final int tag, final int level, final int castingTime, final int mana, final int grace, final int damage, final int skill, final int path, final int faceNum, @NotNull final String spellName, @NotNull final String message) {
        final Spell key = new Spell(spellName);
        key.setParameters(faceNum, tag, message, level, castingTime, mana, grace, damage, skill, path); // set spell path which is used in the comparator

        int index = Collections.binarySearch(spells, key, spellNameComparator);
        final Spell spell;
        if (index < 0) {
            final Spell existingSpell = unknownSpells.remove(spellName);
            if (existingSpell != null) {
                spell = existingSpell;
                spell.setParameters(faceNum, tag, message, level, castingTime, mana, grace, damage, skill, path);
            } else {
                spell = key;
            }
            index = -index-1;
            spells.add(index, spell);
        } else {
            spell = spells.get(index);
            spell.setParameters(faceNum, tag, message, level, castingTime, mana, grace, damage, skill, path);
        }

        for (final SpellsManagerListener listener : listeners) {
            listener.spellAdded(index);
        }
    }

    /**
     * Updates spell information.
     * @param flags specifies which fields to update
     * @param tag the spell's tag
     * @param mana the spell's new mana cost
     * @param grace the spell's new grace cost
     * @param damage the spell's new damage
     */
    private void updateSpell(final int flags, final int tag, final int mana, final int grace, final int damage) {
        for (final Spell spell : spells) {
            if (spell.getTag() == tag) {
                spell.updateParameters((flags&CrossfireSpellListener.UPD_SP_MANA) != 0, mana, (flags&CrossfireSpellListener.UPD_SP_GRACE) != 0, grace, (flags&CrossfireSpellListener.UPD_SP_DAMAGE) != 0, damage);
                break;
            }
        }
    }

    /**
     * Deletes a spell.
     * @param tag the spell's tag
     */
    private void deleteSpell(final int tag) {
        int index = 0;
        for (final Spell spell : spells) {
            if (spell.getTag() == tag) {
                deleteSpellByIndex(index);
                break;
            }
            index++;
        }
    }

    /**
     * Deletes a spell by index into {@link #spells}.
     * @param index the index to delete
     */
    private void deleteSpellByIndex(final int index) {
        final Spell spell = spells.remove(index);
        unknownSpells.put(spell.getName(), spell);

        for (final SpellsManagerListener listener : listeners) {
            listener.spellRemoved(index);
        }

        spell.setUnknown(true);
    }

    /**
     * Returns a {@link Spell} instance by spell name. Creates a new instance if
     * the spell is unknown.
     * @param spellName the spell name to find
     * @return the spell instance
     */
    @NotNull
    public Spell getSpell(@NotNull final String spellName) {
        for (final Spell spell : spells) {
            if (spell.getName().equals(spellName)) {
                return spell;
            }
        }

        final Spell spell = new Spell(spellName);
        spell.setUnknown(true);
        unknownSpells.put(spell.getName(), spell);
        return spell;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Iterator<Spell> iterator() {
        return Collections.unmodifiableList(spells).iterator();
    }

    /**
     * Returns the number of known spells.
     * @return the number of spells
     */
    public int getSpells() {
        return spells.size();
    }

    /**
     * Returns a {@link Spell} instance by index.
     * @param index the index
     * @return the spell or <code>null</code> if the index is invalid
     */
    @Nullable
    public Spell getSpell(final int index) {
        return 0 <= index && index < spells.size() ? spells.get(index) : null;
    }

}
