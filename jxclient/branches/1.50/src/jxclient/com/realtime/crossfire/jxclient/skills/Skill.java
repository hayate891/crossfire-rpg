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

package com.realtime.crossfire.jxclient.skills;

import java.util.ArrayList;
import java.util.Collection;
import org.jetbrains.annotations.NotNull;

/**
 * One skill of the character.
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class Skill {

    /**
     * The listeners to inform of changes.
     */
    @NotNull
    private final Collection<SkillListener> listeners = new ArrayList<SkillListener>();

    /**
     * The skill name.
     */
    @NotNull
    private final String name;

    /**
     * The skill experience.
     */
    private long experience = 0;

    /**
     * The skill level.
     */
    private int level = 0;

    /**
     * Create a new instance.
     * @param name The skill name.
     */
    public Skill(@NotNull final String name) {
        this.name = name;
    }

    /**
     * Update the skill attributes.
     * @param level The new skill level.
     * @param experience The new skill experience.
     */
    public void set(final int level, final long experience) {
        if (this.level == level && this.experience == experience) {
            return;
        }

        final boolean oldKnown = isKnown();
        this.level = level;
        this.experience = experience;
        fireEvents(oldKnown);
    }

    /**
     * Return the skill experience.
     * @return The skill experience.
     */
    public long getExperience() {
        return experience;
    }

    /**
     * Return the skill level.
     * @return The skill level.
     */
    public int getLevel() {
        return level;
    }

    /**
     * Return whether the skill is known.
     * @return Whether the skill is known.
     */
    private boolean isKnown() {
        return experience != 0 || level != 0;
    }

    /**
     * {@inheritDoc}
     */
    @NotNull
    @Override
    public String toString() {
        return name;
    }

    /**
     * Notify all listeners about changes.
     * @param oldKnown Whether the skill was known before the update.
     */
    private void fireEvents(final boolean oldKnown) {
        final boolean newKnown = isKnown();

        if (!oldKnown) {
            assert newKnown;
            fireAddSkill();
        } else if (!newKnown) {
            fireDelSkill();
        } else {
            fireUpdSkill();
        }
    }

    /**
     * Notify all listeners about a gained skill.
     */
    private void fireAddSkill() {
        for (final SkillListener listener : listeners) {
            listener.gainedSkill();
        }
    }

    /**
     * Notify all listeners about a lost attribute.
     */
    private void fireDelSkill() {
        for (final SkillListener listener : listeners) {
            listener.lostSkill();
        }
    }

    /**
     * Notify all listeners about an updated attribute.
     */
    private void fireUpdSkill() {
        for (final SkillListener listener : listeners) {
            listener.changedSkill();
        }
    }

    /**
     * Add a listener to be notified of changes.
     * @param listener The listener to add.
     */
    public void addSkillListener(@NotNull final SkillListener listener) {
        listeners.add(listener);
    }

    /**
     * Remove a listener to be notified of changes.
     * @param listener The listener to remove.
     */
    public void removeSkillListener(@NotNull final SkillListener listener) {
        listeners.remove(listener);
    }

}
