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
package com.realtime.crossfire.jxclient.spells;

import com.realtime.crossfire.jxclient.faces.Faces;
import com.realtime.crossfire.jxclient.server.CrossfireServerConnection;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Manages all known spells.
 *
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class SpellsManager
{
    /**
     * The {@link Faces} instance.
     */
    private final Faces faces;

    /**
     * All known spells.
     */
    private final List<Spell> spells = new ArrayList<Spell>();

    private final List<CrossfireSpellChangedListener> listeners = new ArrayList<CrossfireSpellChangedListener>();

    /**
     * A {@link Comparator} to compare {@link Spell} instances by spell path
     * and name.
     */
    private final Comparator<Spell> spellNameComparator = new Comparator<Spell>()
    {
        /** {@inheritDoc} */
        public int compare(final Spell spell1, final Spell spell2)
        {
            final int path1 = spell1.getPath();
            final int path2 = spell2.getPath();
            if (path1 < path2) return -1;
            if (path1 > path2) return +1;
            return String.CASE_INSENSITIVE_ORDER.compare(spell1.getName(), spell2.getName());
        }
    };

    /**
     * Create a new instance.
     * @param faces the <code>Faces</code> instance
     */
    public SpellsManager(final Faces faces)
    {
        this.faces = faces;
        initSpells();
    }

    public void reset()
    {
        spells.clear();
    }

    public void addCrossfireSpellChangedListener(final CrossfireSpellChangedListener listener)
    {
        listeners.add(listener);
    }

    public void removeCrossfireSpellChangedListener(final CrossfireSpellChangedListener listener)
    {
        listeners.remove(listener);
    }

    public List<Spell> getSpellList()
    {
        return spells;
    }

    private void initSpells()
    {
        for (int i = spells.size()-1; i >= 0; i--)
        {
            final Spell spell = spells.remove(i);
            for (final CrossfireSpellChangedListener listener : listeners)
            {
                listener.spellRemoved(spell, i);
            }
        }
    }

    public void addSpell(final int tag, final int level, final int castingTime, final int mana, final int grace, final int damage, final int skill, final int path, final int faceNum, final String spellName, final String message)
    {
        faces.askface(faceNum);

        final Spell spell = new Spell(faces.getFace(faceNum), tag, spellName, message);
        spell.setLevel(level);
        spell.setCastingTime(castingTime);
        spell.setMana(mana);
        spell.setGrace(grace);
        spell.setDamage(damage);
        spell.setSkill(skill);
        addSpell(spell);
    }

    private void addSpell(final Spell spell)
    {
        final int index = Collections.binarySearch(spells, spell, spellNameComparator);
        if (index < 0)
        {
            spells.add(-index-1, spell);
        }
        else
        {
            spells.set(index, spell);
        }

        for (final CrossfireSpellChangedListener listener : listeners)
        {
            listener.spellAdded(spell, index);
        }
    }

    public void updateSpell(final int flags, final int tag, final int mana, final int grace, final int damage)
    {
        int index = 0;
        for (final Spell spell : spells)
        {
            if (spell.getTag() == tag)
            {
                if ((flags&CrossfireServerConnection.UPD_SP_MANA) != 0)
                {
                    spell.setMana(mana);
                }

                if ((flags&CrossfireServerConnection.UPD_SP_GRACE) != 0)
                {
                    spell.setGrace(mana);
                }

                if ((flags&CrossfireServerConnection.UPD_SP_DAMAGE) != 0)
                {
                    spell.setDamage(mana);
                }

                for (final CrossfireSpellChangedListener listener : listeners)
                {
                    listener.spellModified(spell, index);
                }
                break;
            }
            index++;
        }
    }

    public void deleteSpell(final int tag)
    {
        int index = 0;
        for (final Spell spell : spells)
        {
            if (spell.getTag() == tag)
            {
                spells.remove(index);

                for (final CrossfireSpellChangedListener listener : listeners)
                {
                    listener.spellRemoved(spell, index);
                }
                break;
            }
            index++;
        }
    }

    /**
     * Find a spell by name.
     *
     * @param spellName The spell name to find.
     *
     * @return The spell, or <code>null</code> if the spell name is undefined.
     */
    public Spell getSpell(final String spellName)
    {
        for (final Spell spell : spells)
        {
            if (spell.getInternalName().equals(spellName))
            {
                return spell;
            }
        }

        return null;
    }
}
