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
package com.realtime.crossfire.jxclient.skin.events;

import com.realtime.crossfire.jxclient.skills.Skill;
import com.realtime.crossfire.jxclient.skills.SkillListener;
import com.realtime.crossfire.jxclient.window.GUICommandList;

/**
 * A {@link SkinEvent} that executes a {@link GUICommandList} whenever a new
 * skill has been gained.
 * @author Andreas Kirschbaum
 */
public class SkillAddedSkinEvent implements SkinEvent
{
    /**
     * The {@link GUICommandList} to execute.
     */
    private final GUICommandList commandList;

    /**
     * The {@link Skill} to monitor.
     */
    private final Skill skill;

    /**
     * The {@link SkillListener} attached to {@link #skill}.
     */
    private final SkillListener skillListener = new SkillListener()
    {
        /** {@inheritDoc} */
        @Override
        public void gainedSkill()
        {
            commandList.execute();
        }

        /** {@inheritDoc} */
        @Override
        public void lostSkill()
        {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void changedSkill()
        {
            // ignore
        }
    };

    /**
     * Creates a new instance.
     * @param commandList the command list to execute.
     * @param skill the skill to monitor
     */
    public SkillAddedSkinEvent(final GUICommandList commandList, final Skill skill)
    {
        this.commandList = commandList;
        this.skill = skill;
        skill.addSkillListener(skillListener);
    }

    /** {@inheritDoc} */
    @Override
    public void dispose()
    {
        skill.removeSkillListener(skillListener);
    }
}
