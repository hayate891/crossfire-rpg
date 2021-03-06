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
 * Copyright (C) 2010 Nicolas Weeger.
 */

package com.realtime.crossfire.jxclient.gui.commands;

import com.realtime.crossfire.jxclient.gui.gui.GUIElement;
import com.realtime.crossfire.jxclient.gui.gui.Gui;
import com.realtime.crossfire.jxclient.gui.textinput.GUIText;
import org.jetbrains.annotations.NotNull;

/**
 * A {@link GUICommand} sending a character creation request.
 * @author Nicolas Weeger
 */
public class AccountCreateCharacterCommand implements GUICommand {

    /**
     * The {@link CommandCallback} to use.
     */
    @NotNull
    private final CommandCallback commandCallback;

    /**
     * The {@link GUIElement} to find the Gui containing the fields.
     */
    @NotNull
    private final GUIElement element;

    /**
     * Creates a new instance.
     * @param commandCallback what to inform of various changes.
     * @param button item to link to to find the Gui from which to get
     * information.
     */
    public AccountCreateCharacterCommand(@NotNull final CommandCallback commandCallback, @NotNull final GUIElement button) {
        this.commandCallback = commandCallback;
        element = button;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean canExecute() {
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void execute() {
        final Gui gui = element.getGui();
        if (gui == null) {
            return;
        }

        final GUIText l = gui.getFirstElement(GUIText.class, "character_login");

        if (l == null) {
            return;
        }

        final String login = l.getText();

        commandCallback.accountCreateCharacter(login);
    }

}
