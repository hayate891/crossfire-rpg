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

package com.realtime.crossfire.jxclient.gui.commands;

import com.realtime.crossfire.jxclient.gui.command.GUICommand;
import com.realtime.crossfire.jxclient.gui.gui.GUIElement;
import org.jetbrains.annotations.NotNull;

/**
 * A {@link GUICommand} which hides a target {@link GUIElement}.
 * @author Andreas Kirschbaum
 */
public class HideCommand implements GUICommand
{
    /**
     * The gui element to hide.
     */
    @NotNull
    private final GUIElement target;

    /**
     * Creates a new instance.
     * @param target The gui element to hide.
     */
    public HideCommand(@NotNull final GUIElement target)
    {
        this.target = target;
    }

    /** {@inheritDoc} */
    @Override
    public boolean canExecute()
    {
        return true;
    }

    /** {@inheritDoc} */
    @Override
    public void execute()
    {
        target.setElementVisible(false);
    }
}
