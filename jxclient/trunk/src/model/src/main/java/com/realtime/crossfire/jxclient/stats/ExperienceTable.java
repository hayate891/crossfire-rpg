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

package com.realtime.crossfire.jxclient.stats;

import java.util.HashMap;
import java.util.Map;
import org.jetbrains.annotations.NotNull;

/**
 * Stores experience &lt;-&gt; level mappings.
 * @author Andreas Kirschbaum
 */
public class ExperienceTable {

    /**
     * Maps level to experience needed to reach the level.
     */
    @NotNull
    private final Map<Integer, Long> info = new HashMap<>();

    /**
     * The minimum level value in {@link #info}.
     */
    private int minLevel;

    /**
     * The maximum level value in {@link #info}.
     */
    private int maxLevel;

    /**
     * Forgets about all level-$&gt; mappings.
     */
    private void clear() {
        info.clear();
        minLevel = Integer.MAX_VALUE;
        maxLevel = Integer.MIN_VALUE;
    }

    /**
     * Adds a new level-&gt;experience mapping.
     * @param level the level to update
     * @param exp the experience needed to reach level {@code level}
     */
    private void add(final int level, final long exp) {
        if (level < 1) {
            return;
        }

        info.put(level, exp);
        if (minLevel > level) {
            minLevel = level;
        }
        if (maxLevel < level) {
            maxLevel = level;
        }
    }

    /**
     * Returns the experience needed for a given level.
     * @param level the level to reach
     * @return the needed experience
     */
    private long getExperience(final int level) {
        if (minLevel >= maxLevel) {
            return 0;
        }

        final Long exp = info.get(level);
        if (exp != null) {
            return exp;
        }

        if (level < minLevel) {
            return info.get(minLevel);
        }

        if (level > maxLevel) {
            return maxLevel;
        }

        for (int i = level; i < maxLevel; i++) {
            final Long tmp = info.get(i);
            if (tmp != null) {
                return tmp;
            }
        }

        throw new AssertionError();
    }

    /**
     * Returns the experience needed to reach the next level.
     * @param currentLevel the current level
     * @param currentExp the current experience
     * @return the experience to reach level {@code currentLevel+1}
     */
    public long getExperienceToNextLevel(final int currentLevel, final long currentExp) {
        final long expNextLevel = getExperience(currentLevel+1);
        return Math.max(0, expNextLevel-currentExp);
    }

    /**
     * Returns the experience fraction of the current level in percents. The
     * value starts at 0% when a new level has been gained; the next level is at
     * 100%.
     * @param currentLevel the current level
     * @param currentExp the current experience
     * @return the experience fraction
     */
    public int getPercentsToNextLevel(final int currentLevel, final long currentExp) {
        final long expThisLevel = getExperience(currentLevel);
        final long expNextLevel = getExperience(currentLevel+1);
        if (expThisLevel >= expNextLevel) {
            return 0;
        }

        if (currentExp < expThisLevel) {
            return 0;
        }
        if (currentExp >= expNextLevel) {
            return 100;
        }

        return (int)((currentExp-expThisLevel)*100/(expNextLevel-expThisLevel));
    }

    /**
     * Updates the experience table information.
     * @param expTable the new experience table
     */
    public void setExpTable(@NotNull final long[] expTable) {
        clear();
        for (int level = 1; level < expTable.length; level++) {
            add(level, expTable[level]);
        }
    }

}
