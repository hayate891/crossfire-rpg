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

package com.realtime.crossfire.jxclient.skin.skin;

import com.realtime.crossfire.jxclient.gui.commands.CommandList;
import com.realtime.crossfire.jxclient.gui.gui.Gui;
import com.realtime.crossfire.jxclient.gui.gui.JXCWindowRenderer;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.gui.keybindings.KeyBindings;
import com.realtime.crossfire.jxclient.util.Resolution;
import java.util.Iterator;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Defines a JXClient skin consisting of a main {@link Gui} and zero or more
 * dialog {@link Gui}s.
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public interface JXCSkin extends Iterable<Gui> {

    /**
     * Returns a short name for the skin. It is used to construct path or file
     * names.
     * @return the skin name
     */
    @NotNull
    String getSkinName();

    /**
     * Returns the resolution of this skin.
     * @return the resolution
     */
    @NotNull
    Resolution getResolution();

    /**
     * Returns the minimal resolution of this skin.
     * @return the minimal resolution
     */
    @NotNull
    Resolution getMinResolution();

    /**
     * Returns the maximal resolution of this skin.
     * @return the maximal resolution
     */
    @NotNull
    Resolution getMaxResolution();

    /**
     * Returns the map width in tiles.
     * @return the map width in tiles
     */
    int getMapWidth();

    /**
     * Returns the map height in tiles.
     * @return the map height in tiles
     */
    int getMapHeight();

    /**
     * The number of ground view objects to request from the server.
     * @return the number of ground view objects
     */
    int getNumLookObjects();

    /**
     * Returns all gui instances of this skin. The instances has no defined
     * order.
     * @return an iterator returning all gui instances
     */
    @NotNull
    @Override
    Iterator<Gui> iterator();

    /**
     * Returns the "really quit?" dialog. It is opened when the user presses
     * ESCAPE.
     * @return the dialog or <code>null</code> if the dialog does not exist
     */
    @Nullable
    Gui getDialogQuit();

    /**
     * Returns the "disconnect from server?" dialog. It is opened when the user
     * presses ESCAPE.
     * @return the dialog or <code>null</code> if the dialog does not exist
     */
    @Nullable
    Gui getDialogDisconnect();

    /**
     * Returns the "connection in progress" dialog. It is opened while the
     * connection establishment is in progress.
     * @return the dialog or <code>null</code> if the dialog does not exist
     */
    @Nullable
    Gui getDialogConnect();

    /**
     * Returns the key bindings dialog.
     * @return the dialog
     */
    @NotNull
    Gui getDialogKeyBind();

    /**
     * Returns the dialog for query text input.
     * @return the dialog
     */
    @NotNull
    Gui getDialogQuery();

    /**
     * Returns the popup dialog for readables.
     * @param booknr the book ID
     * @return the dialog
     */
    @NotNull
    Gui getDialogBook(int booknr);

    /**
     * Returns the main window.
     * @return the dialog
     */
    @NotNull
    Gui getMainInterface();

    /**
     * Returns the server selection window.
     * @return the dialog
     */
    @NotNull
    Gui getMetaInterface();

    /**
     * Returns the start window.
     * @return the dialog
     */
    @NotNull
    Gui getStartInterface();

    /**
     * Returns a dialog by name.
     * @param name the dialog's name
     * @return the dialog
     * @throws JXCSkinException if the dialog does not exist
     */
    @NotNull
    Gui getDialog(@NotNull final String name) throws JXCSkinException;

    /**
     * Returns a named command list.
     * @param name the name of the command list
     * @return the command list
     * @throws JXCSkinException if the command list does not exist
     */
    @NotNull
    CommandList getCommandList(@NotNull String name) throws JXCSkinException;

    /**
     * Returns whether the dialog state should be saved.
     * @return whether the dialog state should be saved
     */
    boolean hasChangedDialog();

    /**
     * Returns the default key bindings for this skin.
     * @return the default key bindings
     */
    @NotNull
    KeyBindings getDefaultKeyBindings();

    /**
     * Attaches this skin to a gui manager.
     * @param windowRenderer the window renderer to attach to
     * @param tooltipManager the tooltip manager to attach to
     */
    void attach(@NotNull JXCWindowRenderer windowRenderer, @NotNull TooltipManager tooltipManager);

    /**
     * Frees all allocated resources.
     */
    void detach();

}
