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

package com.realtime.crossfire.jxclient.skin.skin;

import com.realtime.crossfire.jxclient.gui.commandlist.GUICommandFactory;
import com.realtime.crossfire.jxclient.gui.gui.Gui;
import com.realtime.crossfire.jxclient.gui.keybindings.KeyBindings;
import javax.swing.GroupLayout;
import org.jetbrains.annotations.NotNull;

/**
 * Factory for creating {@link Gui} instances.
 * @author Andreas Kirschbaum
 */
public class GuiFactory {

    /**
     * The {@link GUICommandFactory} for creating commands.
     */
    private final GUICommandFactory guiCommandFactory;

    /**
     * Creates a new instance.
     * @param guiCommandFactory the gui command factory for creating commands
     */
    public GuiFactory(@NotNull final GUICommandFactory guiCommandFactory) {
        this.guiCommandFactory = guiCommandFactory;
    }

    /**
     * Creates a new {@link Gui} instance.
     * @return the new gui instance
     */
    @NotNull
    public Gui newGui() {
        final Gui gui = new Gui(new KeyBindings(null, guiCommandFactory));
        gui.setLayout(new GroupLayout(gui));
        return gui;
    }

}
