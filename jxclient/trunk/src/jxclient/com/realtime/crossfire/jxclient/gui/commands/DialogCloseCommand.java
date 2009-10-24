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
package com.realtime.crossfire.jxclient.gui.commands;

import com.realtime.crossfire.jxclient.gui.gui.Gui;
import com.realtime.crossfire.jxclient.window.GuiManager;
import org.jetbrains.annotations.NotNull;

/**
 * A {@link GUICommand} which closes a {@link Gui}.
 * @author Andreas Kirschbaum
 */
public class DialogCloseCommand implements GUICommand
{
    /**
     * The {@link GuiManager} to use.
     */
    @NotNull
    private final GuiManager guiManager;

    /**
     * The dialog to close.
     */
    @NotNull
    private final Gui dialog;

    /**
     * Creates a new instance.
     * @param guiManager the gui manager to use
     * @param dialog the dialog to close
     */
    public DialogCloseCommand(@NotNull final GuiManager guiManager, @NotNull final Gui dialog)
    {
        this.guiManager = guiManager;
        this.dialog = dialog;
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
        guiManager.getWindowRenderer().closeDialog(dialog);
    }
}
