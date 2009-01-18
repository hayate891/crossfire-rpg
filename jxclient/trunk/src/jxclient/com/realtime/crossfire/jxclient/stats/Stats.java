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
package com.realtime.crossfire.jxclient.stats;

import com.realtime.crossfire.jxclient.experience.ExperienceTable;
import com.realtime.crossfire.jxclient.server.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.server.CrossfireStatsListener;
import com.realtime.crossfire.jxclient.skills.Skill;
import com.realtime.crossfire.jxclient.skills.SkillSet;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * This is the representation of all the statistics of a player, like its speed
 * or its experience.
 *
 * <p>Constants named <code>C_STAT_xxx</code> are client-sided; constants named
 * <code>CS_STAT_xxx</code> are stats as sent by the server.
 *
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class Stats
{
    /**
     * Whether the {@link CrossfireStatsListener#CS_STAT_WEAP_SP} value
     * contains the weapon speed directly.
     */
    private boolean simpleWeaponSpeed = false;

    /**
     * The listeners to inform of stat changes.
     */
    private final List<StatsListener> statsListeners = new ArrayList<StatsListener>();

    /**
     * The {@link ExperienceTable} instance to use.
     */
    private final ExperienceTable experienceTable;

    private final int[] stats = new int[258];

    /**
     * The total experience.
     */
    private long exp = 0;

    /**
     * The experience needed to reach the next level.
     */
    private long expNextLevel = 0;

    private String range = "";

    private String title = "";

    /**
     * The active skill name.
     */
    private String activeSkill = "";

    private final CrossfireStatsListener crossfireStatsListener = new CrossfireStatsListener()
    {
        /**
         * All unhandled stat values for which an error has been printed.
         */
        private final Set<String> unhandledStats = new HashSet<String>(0);

        /** {@inheritDoc} */
        public void setSimpleWeaponSpeed(final boolean simpleWeaponSpeed)
        {
            Stats.this.setSimpleWeaponSpeed(simpleWeaponSpeed);
        }

        /** {@inheritDoc} */
        public void statInt2Received(final int stat, final short param)
        {
            switch (stat)
            {
            case CS_STAT_HP:
            case CS_STAT_MAXHP:
            case CS_STAT_SP:
            case CS_STAT_MAXSP:
            case CS_STAT_STR:
            case CS_STAT_INT:
            case CS_STAT_WIS:
            case CS_STAT_DEX:
            case CS_STAT_CON:
            case CS_STAT_CHA:
            case CS_STAT_LEVEL:
            case CS_STAT_WC:
            case CS_STAT_AC:
            case CS_STAT_DAM:
            case CS_STAT_ARMOUR:
            case CS_STAT_FOOD:
            case CS_STAT_POW:
            case CS_STAT_GRACE:
            case CS_STAT_MAXGRACE:
                setStat(stat, param);
                if (stat == CS_STAT_LEVEL)
                {
                    calcExperienceToNextLevel();
                }
                break;

            case CS_STAT_FLAGS:
                setStat(stat, param&0xFFFF);
                break;

            default:
                if (CS_STAT_RESIST_START <= stat && stat < CS_STAT_RESIST_START+RESIST_TYPES)
                {
                    setStat(stat, param);
                }
                else
                {
                    reportUnhandledStat(stat, "int2");
                }
                break;
            }
        }

        /** {@inheritDoc} */
        public void statInt4Received(final int stat, final int param)
        {
            switch (stat)
            {
            case CS_STAT_EXP:
                setExperience(param&0xFFFFFFFFL);
                break;

            case CS_STAT_SPEED:
            case CS_STAT_WEAP_SP:
            case CS_STAT_WEIGHT_LIM:
            case CS_STAT_SPELL_ATTUNE:
            case CS_STAT_SPELL_REPEL:
            case CS_STAT_SPELL_DENY:
                setStat(stat, param);
                break;

            default:
                reportUnhandledStat(stat, "int4");
                break;
            }
        }

        /** {@inheritDoc} */
        public void statInt8Received(final int stat, final long param)
        {
            switch (stat)
            {
            case CS_STAT_EXP64:
                setExperience(param);
                break;

            default:
                reportUnhandledStat(stat, "int8");
                break;
            }
        }

        /** {@inheritDoc} */
        public void statStringReceived(final int stat, final String param)
        {
            switch (stat)
            {
            case CS_STAT_RANGE:
                setRange(param);
                break;

            case CS_STAT_TITLE:
                setTitle(param);
                break;

            default:
                reportUnhandledStat(stat, "string");
                break;
            }
        }

        /** {@inheritDoc} */
        public void statSkillReceived(final int stat, final int level, final long experience)
        {
            if (CS_STAT_SKILLINFO <= stat && stat < CS_STAT_SKILLINFO+CS_NUM_SKILLS)
            {
                final Skill sk = SkillSet.getSkill(stat);
                if (sk == null)
                {
                    System.err.println("ignoring skill value for unknown skill "+stat);
                }
                else
                {
                    sk.set(level, experience);
                }
            }
            else
            {
                reportUnhandledStat(stat, "skill");
            }
        }

        /**
         * Report an unhandled stat value.
         * @param stat the stat value
         * @param type the stat type
         */
        private void reportUnhandledStat(final int stat, final String type)
        {
            if (unhandledStats.add(type+"-"+stat))
            {
                System.err.println("Warning: unhandled stat "+stat+" of type "+type);
            }
        }
    };

    /**
     * Create a new instance.
     * @param crossfireServerConnection the connection to monitor
     * @param experienceTable the experience table instance to use
     */
    public Stats(final CrossfireServerConnection crossfireServerConnection, final ExperienceTable experienceTable)
    {
        this.experienceTable = experienceTable; // XXX: should detect changed information
        crossfireServerConnection.addCrossfireStatsListener(crossfireStatsListener);
    }

    /**
     * Set whether the {@link CrossfireStatsListener#CS_STAT_WEAP_SP} value contains the weapon speed
     * directly.
     *
     * @param simpleWeaponSpeed Whether <code>CS_STAT_WEAP_SP</code> is the
     * weapon speed value.
     */
    public void setSimpleWeaponSpeed(final boolean simpleWeaponSpeed)
    {
        if (this.simpleWeaponSpeed == simpleWeaponSpeed)
        {
            return;
        }

        this.simpleWeaponSpeed = simpleWeaponSpeed;
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.simpleWeaponSpeedChanged(this.simpleWeaponSpeed);
        }
    }

    /**
     * Forget about all stats.
     */
    public void reset()
    {
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.reset();
        }
        for (int statnr = 0; statnr < stats.length; statnr++)
        {
            setStat(statnr, 0);
        }
        setExperience(0);
        setRange("");
        setTitle("");
        setActiveSkill("");
    }

    /**
     * Returns the numerical value of the given statistic.
     * @param statnr The stat identifier. See the CS_STAT constants.
     * @return The statistic value (or "score").
     */
    public int getStat(final int statnr)
    {
        return stats[statnr];
    }

    /**
     * Returns the numerical value of the given statistic.
     * @param statnr The stat identifier. See the CS_STAT constants.
     * @return The statistic value.
     */
    public double getFloatStat(final int statnr)
    {
        return (double)stats[statnr]/CrossfireStatsListener.FLOAT_MULTI;
    }

    /**
     * Sets the given statistic numerical value.
     * @param statnr The stat identifier. See the CS_STAT constants.
     * @param value The value to assign to the chosen statistic.
     */
    public void setStat(final int statnr, final int value)
    {
        if (stats[statnr] == value) {
            return;
        }

        stats[statnr] = value;
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.statChanged(statnr, stats[statnr]);
        }
    }

    /**
     * Returns the current Title.
     * @return A String representation of the Title.
     */
    public String getTitle()
    {
        return title;
    }

    /**
     * Returns the current content of the Range stat. This is basically the
     * current active skill for the player.
     * @return A String representation of the Range.
     */
    public String getRange()
    {
        return range;
    }

    /**
     * Returns the active skill name.
     *
     * @return The active skill name.
     */
    public String getActiveSkill()
    {
        return activeSkill;
    }

    /**
     * Sets the current Title.
     * @param title The new Title content.
     */
    public void setTitle(final String title)
    {
        if (this.title.equals(title))
        {
            return;
        }

        this.title = title;
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.titleChanged(this.title);
        }
    }

    /**
     * Sets the current value for the Range - this is basically the currently
     * active skill for the player.
     * @param range The new content of Range.
     */
    public void setRange(final String range)
    {
        if (this.range.equals(range))
        {
            return;
        }

        this.range = range;
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.rangeChanged(this.range);
        }
    }

    /**
     * Set the active skill name.
     *
     * @param activeSkill The active skill name.
     */
    public void setActiveSkill(final String activeSkill)
    {
        if (this.activeSkill.equals(activeSkill))
        {
            return;
        }

        this.activeSkill = activeSkill;
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.activeSkillChanged(this.activeSkill);
        }
    }

    /**
     * Returns the amount of global experience.
     * @return Amount of global experience.
     */
    public long getExperience()
    {
        return exp;
    }

    /**
     * Sets the amount of global experience.
     * @param exp The new amount of global experience.
     */
    public void setExperience(final long exp)
    {
        if (this.exp == exp)
        {
            return;
        }

        this.exp = exp;
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.experienceChanged(this.exp);
        }

        calcExperienceToNextLevel();
    }

    /**
     * Returns the experience needed to reach the next level.
     * @return the experience needed
     */
    public long getExperienceNextLevel()
    {
        return expNextLevel;
    }

    /**
     * Calculates experience needed to reach the next level.
     */
    private void calcExperienceToNextLevel()
    {
        final long newExpNextLevel = experienceTable.getExperienceToNextLevel(stats[CrossfireStatsListener.CS_STAT_LEVEL], exp);
        if (expNextLevel == newExpNextLevel)
        {
            return;
        }

        expNextLevel = newExpNextLevel;
        for (final StatsListener statsListener : statsListeners)
        {
            statsListener.experienceNextLevelChanged(expNextLevel);
        }
    }

    public void addCrossfireStatsListener(final StatsListener statsListener)
    {
        statsListeners.add(statsListener);
    }

    public void removeCrossfireStatsListener(final StatsListener statsListener)
    {
        statsListeners.remove(statsListener);
    }

    /**
     * Return the weapon speed stat.
     *
     * @return The weapon speed stat.
     */
    public double getWeaponSpeed()
    {
        final double weaponSpeed = getFloatStat(CrossfireStatsListener.CS_STAT_WEAP_SP);
        if (simpleWeaponSpeed)
        {
            return weaponSpeed;
        }

        if (weaponSpeed < 0.001)
        {
            return 0;
        }

        return getFloatStat(CrossfireStatsListener.CS_STAT_SPEED)/weaponSpeed;
    }
}
