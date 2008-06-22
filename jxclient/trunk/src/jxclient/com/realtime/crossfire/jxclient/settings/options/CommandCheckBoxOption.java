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
package com.realtime.crossfire.jxclient.settings.options;

import com.realtime.crossfire.jxclient.window.GUICommandList;

/**
 * A {@link CheckBoxOption} that executes {@link GUICommandList}s when
 * checked/unchecked.
 *
 * @author Andreas Kirschbaum
 */
public class CommandCheckBoxOption extends CheckBoxOption
{
    /**
     * The command list to execute when checked.
     */
    private final GUICommandList commandOn;

    /**
     * The command list to execute when unchecked.
     */
    private final GUICommandList commandOff;

    /**
     * Create a new instance.
     *
     * @param commandOn The command list to execute when checked.
     *
     * @param commandOff The command list to execute when unchecked.
     */
    public CommandCheckBoxOption(final GUICommandList commandOn, final GUICommandList commandOff)
    {
        this.commandOn = commandOn;
        this.commandOff = commandOff;
    }

    /** {@inheritDoc} */
    @Override
    protected void execute(final boolean checked)
    {
        if (checked)
        {
            commandOn.execute();
        }
        else
        {
            commandOff.execute();
        }
    }

    /** {@inheritDoc} */
    @Override
    public boolean isDefaultChecked()
    {
        return true;
    }
}
