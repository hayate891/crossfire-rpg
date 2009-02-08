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
package com.realtime.crossfire.jxclient.skills;

import com.realtime.crossfire.jxclient.server.CrossfireStatsListener;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * Maintain the set of skills as sent by the server.
 * @author Andreas Kirschbaum
 */
public class SkillSet
{
    /**
     * Maps stat number to skill instance. Entries may be <code>null</code> if
     * the server did not provide a mapping.
     */
    private final Skill[] numberedSkills = new Skill[CrossfireStatsListener.CS_NUM_SKILLS];

    /**
     * Maps skill name to skill instance.
     */
    private final Map<String, Skill> namedSkills = new HashMap<String, Skill>();

    /**
     * Creates a new instance.
     */
    public SkillSet()
    {
    }

    /**
     * Adds a new skill to the list of known skills.
     * @param id The numerical identifier for the new skill.
     * @param skillName The skill name.
     */
    public void addSkill(final int id, final String skillName)
    {
        final int index = id-CrossfireStatsListener.CS_STAT_SKILLINFO;
        final Skill oldSkill = numberedSkills[index];
        final Skill newSkill = getNamedSkill(skillName);
        if (oldSkill == newSkill)
        {
            return;
        }

        if (oldSkill != null)
        {
            oldSkill.set(0, 0);
        }
        numberedSkills[index] = newSkill;
    }

    /**
     * Return the skill instance for a given skill name.
     *
     * @param skillName The skill name to look up.
     *
     * @return The skill instance.
     */
    public Skill getNamedSkill(final String skillName)
    {
        final Skill oldSkill = namedSkills.get(skillName);
        if (oldSkill != null)
        {
            return oldSkill;
        }

        final Skill newSkill = new Skill(skillName);
        namedSkills.put(skillName, newSkill);
        return newSkill;
    }

    /**
     * Clears all stat info in {@link #numberedSkills}.
     */
    public void clearNumberedSkills()
    {
        for (final Skill skill : numberedSkills)
        {
            if (skill != null)
            {
                skill.set(0, 0);
            }
        }
    }

    /**
     * Forget about all skill name mappings.
     */
    public void clearSkills()
    {
        clearNumberedSkills();
        Arrays.fill(numberedSkills, null);
    }

    /**
     * Returns the given skill as a Skill object.
     * @param id The numerical skill identifier.
     * @return The Skill object matching the given identifier; may be
     * <code>null</code> for undefined skills.
     */
    public Skill getSkill(final int id)
    {
        return numberedSkills[id-CrossfireStatsListener.CS_STAT_SKILLINFO];
    }
}
