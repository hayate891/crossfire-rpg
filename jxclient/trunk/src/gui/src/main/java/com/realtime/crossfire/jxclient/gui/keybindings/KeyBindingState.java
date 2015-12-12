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

package com.realtime.crossfire.jxclient.gui.keybindings;

import com.realtime.crossfire.jxclient.gui.commandlist.CommandList;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Manages the state for the key binding dialog.
 * @author Andreas Kirschbaum
 */
public class KeyBindingState {

    /**
     * The {@link KeyBindings} to modify.
     */
    @Nullable
    private final KeyBindings keyBindings;

    /**
     * The {@link KeyBindings} to modify.
     */
    @Nullable
    private final KeyBindings keyBindings2;

    /**
     * The commands to bind, or {@code null} to unbind.
     */
    @Nullable
    private final CommandList commands;

    /**
     * The dialog state: 0=waiting for initial key press, 1=waiting for key
     * release.
     */
    private int state;

    /**
     * The type of key binding: -1=invalid, 0=key code ({@link #keyEvent} is
     * valid).
     */
    private int type = -1;

    /**
     * The key event. Only valid if {@code type == 0}.
     */
    private KeyEvent2 keyEvent;

    /**
     * Creates a new instance.
     * @param keyBindings the {@code KeyBindings} to modify; may be {@code null}
     * when removing bindings
     * @param keyBindings2 the {@code KeyBindings} to modify; only used when
     * removing bindings; may be {@code null} when removing bindings
     * @param commands the commands to bind, or {@code null} to unbind
     */
    public KeyBindingState(@Nullable final KeyBindings keyBindings, @Nullable final KeyBindings keyBindings2, @Nullable final CommandList commands) {
        this.keyBindings = keyBindings;
        this.keyBindings2 = keyBindings2;
        this.commands = commands;
    }

    /**
     * Records a binding by key code.
     * @param keyEvent the key that was pressed
     */
    public void keyPressed(@NotNull final KeyEvent2 keyEvent) {
        state = 1;
        type = 0;
        this.keyEvent = keyEvent;
    }

    /**
     * Records a key released event.
     * @return {@code true} if the dialog has finished, or {@code false} if the
     * dialog is still active
     */
    public boolean keyReleased() {
        if (state == 0) {
            return false;
        }

        assert type != -1;
        if (commands == null) {
            if (keyBindings != null) {
                keyBindings.deleteKeyBindingAsKeyCode(keyEvent);
            }

            if (keyBindings2 != null) {
                keyBindings2.deleteKeyBindingAsKeyCode(keyEvent);
            }
        } else {
            if (keyBindings != null) {
                keyBindings.addKeyBindingAsKeyCode(keyEvent, commands, false);
            }
        }

        return true;
    }

}
