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

package com.realtime.crossfire.jxclient.skin.io;

import com.realtime.crossfire.jxclient.commands.Commands;
import com.realtime.crossfire.jxclient.commands.Macros;
import com.realtime.crossfire.jxclient.faces.FacesManager;
import com.realtime.crossfire.jxclient.faces.FacesProvider;
import com.realtime.crossfire.jxclient.faces.FacesProviderFactory;
import com.realtime.crossfire.jxclient.gui.GUIPicture;
import com.realtime.crossfire.jxclient.gui.button.ButtonImages;
import com.realtime.crossfire.jxclient.gui.button.GUIButton;
import com.realtime.crossfire.jxclient.gui.commands.CommandCallback;
import com.realtime.crossfire.jxclient.gui.commands.CommandCheckBoxOption;
import com.realtime.crossfire.jxclient.gui.commands.CommandList;
import com.realtime.crossfire.jxclient.gui.commands.CommandListType;
import com.realtime.crossfire.jxclient.gui.gauge.GUIDupGauge;
import com.realtime.crossfire.jxclient.gui.gauge.GUIDupTextGauge;
import com.realtime.crossfire.jxclient.gui.gauge.GUIGauge;
import com.realtime.crossfire.jxclient.gui.gauge.GUITextGauge;
import com.realtime.crossfire.jxclient.gui.gauge.GaugeUpdater;
import com.realtime.crossfire.jxclient.gui.gauge.Orientation;
import com.realtime.crossfire.jxclient.gui.gui.ActivatableGUIElement;
import com.realtime.crossfire.jxclient.gui.gui.GUIElement;
import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.GUIScrollBar;
import com.realtime.crossfire.jxclient.gui.gui.Gui;
import com.realtime.crossfire.jxclient.gui.gui.GuiFactory;
import com.realtime.crossfire.jxclient.gui.gui.JXCWindowRenderer;
import com.realtime.crossfire.jxclient.gui.gui.RendererGuiState;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.gui.item.GUIItemFloor;
import com.realtime.crossfire.jxclient.gui.item.GUIItemFloorFactory;
import com.realtime.crossfire.jxclient.gui.item.GUIItemInventory;
import com.realtime.crossfire.jxclient.gui.item.GUIItemInventoryFactory;
import com.realtime.crossfire.jxclient.gui.item.GUIItemItemFactory;
import com.realtime.crossfire.jxclient.gui.item.GUIItemShortcut;
import com.realtime.crossfire.jxclient.gui.item.GUIItemSpelllist;
import com.realtime.crossfire.jxclient.gui.item.ItemPainter;
import com.realtime.crossfire.jxclient.gui.keybindings.InvalidKeyBindingException;
import com.realtime.crossfire.jxclient.gui.keybindings.KeyBindings;
import com.realtime.crossfire.jxclient.gui.label.AbstractLabel;
import com.realtime.crossfire.jxclient.gui.label.Alignment;
import com.realtime.crossfire.jxclient.gui.label.GUIHTMLLabel;
import com.realtime.crossfire.jxclient.gui.label.GUILabelMessage;
import com.realtime.crossfire.jxclient.gui.label.GUILabelQuery;
import com.realtime.crossfire.jxclient.gui.label.GUILabelStats;
import com.realtime.crossfire.jxclient.gui.label.GUIMultiLineLabel;
import com.realtime.crossfire.jxclient.gui.label.GUIOneLineLabel;
import com.realtime.crossfire.jxclient.gui.label.GUISpellLabel;
import com.realtime.crossfire.jxclient.gui.label.Type;
import com.realtime.crossfire.jxclient.gui.list.GUIItemList;
import com.realtime.crossfire.jxclient.gui.list.GUIMetaElementList;
import com.realtime.crossfire.jxclient.gui.log.Fonts;
import com.realtime.crossfire.jxclient.gui.log.GUILabelLog;
import com.realtime.crossfire.jxclient.gui.log.GUIMessageLog;
import com.realtime.crossfire.jxclient.gui.log.MessageBufferUpdater;
import com.realtime.crossfire.jxclient.gui.map.GUIMagicMap;
import com.realtime.crossfire.jxclient.gui.map.GUIMap;
import com.realtime.crossfire.jxclient.gui.scrollable.GUIScrollable2;
import com.realtime.crossfire.jxclient.gui.textinput.GUICommandText;
import com.realtime.crossfire.jxclient.gui.textinput.GUIQueryText;
import com.realtime.crossfire.jxclient.gui.textinput.GUIText;
import com.realtime.crossfire.jxclient.gui.textinput.GUITextField;
import com.realtime.crossfire.jxclient.guistate.GuiStateManager;
import com.realtime.crossfire.jxclient.items.FloorView;
import com.realtime.crossfire.jxclient.items.ItemSet;
import com.realtime.crossfire.jxclient.items.ItemView;
import com.realtime.crossfire.jxclient.mapupdater.CfMapUpdater;
import com.realtime.crossfire.jxclient.metaserver.MetaserverModel;
import com.realtime.crossfire.jxclient.queue.CommandQueue;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.server.crossfire.MessageTypes;
import com.realtime.crossfire.jxclient.server.socket.UnknownCommandException;
import com.realtime.crossfire.jxclient.settings.options.CheckBoxOption;
import com.realtime.crossfire.jxclient.settings.options.OptionManager;
import com.realtime.crossfire.jxclient.shortcuts.Shortcuts;
import com.realtime.crossfire.jxclient.skills.Skill;
import com.realtime.crossfire.jxclient.skills.SkillSet;
import com.realtime.crossfire.jxclient.skin.events.ConnectionStateSkinEvent;
import com.realtime.crossfire.jxclient.skin.events.CrossfireMagicmapSkinEvent;
import com.realtime.crossfire.jxclient.skin.events.MapscrollSkinEvent;
import com.realtime.crossfire.jxclient.skin.events.SkillAddedSkinEvent;
import com.realtime.crossfire.jxclient.skin.events.SkillRemovedSkinEvent;
import com.realtime.crossfire.jxclient.skin.factory.CheckBoxFactory;
import com.realtime.crossfire.jxclient.skin.factory.DialogFactory;
import com.realtime.crossfire.jxclient.skin.factory.TextButtonFactory;
import com.realtime.crossfire.jxclient.skin.skin.DefaultJXCSkin;
import com.realtime.crossfire.jxclient.skin.skin.Dialogs;
import com.realtime.crossfire.jxclient.skin.skin.JXCSkin;
import com.realtime.crossfire.jxclient.skin.skin.JXCSkinCache;
import com.realtime.crossfire.jxclient.skin.skin.JXCSkinException;
import com.realtime.crossfire.jxclient.skin.source.JXCSkinSource;
import com.realtime.crossfire.jxclient.spells.CurrentSpellManager;
import com.realtime.crossfire.jxclient.spells.SpellsManager;
import com.realtime.crossfire.jxclient.stats.ExperienceTable;
import com.realtime.crossfire.jxclient.stats.Stats;
import com.realtime.crossfire.jxclient.util.NumberParser;
import com.realtime.crossfire.jxclient.util.Resolution;
import com.realtime.crossfire.jxclient.util.ResourceUtils;
import com.realtime.crossfire.jxclient.util.StringUtils;
import com.realtime.crossfire.jxclient.util.UnterminatedTokenException;
import java.awt.Color;
import java.awt.Font;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Parser for loading {@link JXCSkin} instances from {@link JXCSkinSource}s.
 * @author Andreas Kirschbaum
 */
public class JXCSkinLoader {

    /**
     * The resource for "Click here for next group of items" buttons.
     */
    @NotNull
    private static final String NEXT_GROUP_FACE = "resource/next_group.png";

    /**
     * The resource for "Click here for previous group of items" buttons.
     */
    @NotNull
    private static final String PREV_GROUP_FACE = "resource/prev_group.png";

    /**
     * The {@link ItemSet} instance to use.
     */
    @NotNull
    private final ItemSet itemSet;

    /**
     * The inventory {@link ItemView} to use.
     */
    @NotNull
    private final ItemView inventoryView;

    /**
     * The {@link FloorView} to use.
     */
    @NotNull
    private final FloorView floorView;

    /**
     * The {@link SpellsManager} instance to use.
     */
    @NotNull
    private final SpellsManager spellsManager;

    /**
     * The {@link FacesManager} instance to use.
     */
    @NotNull
    private final FacesManager facesManager;

    /**
     * The {@link FacesProviderFactory} instance for creating faces provider
     * instances.
     */
    @NotNull
    private final FacesProviderFactory facesProviderFactory;

    /**
     * The {@link Stats} instance to use.
     */
    @NotNull
    private final Stats stats;

    /**
     * The {@link CfMapUpdater} instance to use.
     */
    @NotNull
    private final CfMapUpdater mapUpdater;

    /**
     * The default key bindings.
     */
    @NotNull
    private final KeyBindings defaultKeyBindings;

    /**
     * The {@link OptionManager} instance to use.
     */
    @NotNull
    private final OptionManager optionManager;

    /**
     * The {@link ExperienceTable} to use.
     */
    @NotNull
    private final ExperienceTable experienceTable;

    /**
     * The {@link GaugeUpdaterParser} for parsing gauge specifications.
     */
    @NotNull
    private GaugeUpdaterParser gaugeUpdaterParser;

    /**
     * The {@link SkillSet} instance to use.
     */
    @NotNull
    private final SkillSet skillSet;

    /**
     * All defined fonts.
     */
    @NotNull
    private final JXCSkinCache<Font> definedFonts = new JXCSkinCache<Font>("font");

    /**
     * The text button factory. Set to <code>null</code> until defined.
     */
    @Nullable
    private TextButtonFactory textButtonFactory = null;

    /**
     * The dialog factory. Set to <code>null</code> until defined.
     */
    @Nullable
    private DialogFactory dialogFactory = null;

    /**
     * The checkbox factory. Set to <code>null</code> until defined.
     */
    @Nullable
    private CheckBoxFactory checkBoxFactory = null;

    /**
     * The {@link ItemPainter} for default item parameters. Set to
     * <code>null</code> until defined.
     */
    @Nullable
    private ItemPainter defaultItemPainter = null;

    /**
     * The {@link ExpressionParser} for parsing integer constant expressions.
     */
    @NotNull
    private ExpressionParser expressionParser;

    /**
     * The {@link CommandParser} for parsing commands.
     */
    @NotNull
    private CommandParser commandParser;

    /**
     * The {@link ImageParser} for parsing image specifications.
     */
    @NotNull
    private ImageParser imageParser;

    /**
     * The {@link FontParser} for parsing font specifications.
     */
    @NotNull
    private FontParser fontParser;

    /**
     * The {@link GuiElementParser} for parsing gui element specifications.
     */
    @NotNull
    private GuiElementParser guiElementParser;

    /**
     * The defined {@link GUIElement}s.
     */
    @NotNull
    private final JXCSkinCache<GUIElement> definedGUIElements = new JXCSkinCache<GUIElement>("gui element");

    /**
     * The {@link JXCSkin} being loaded.
     */
    @NotNull
    private DefaultJXCSkin skin;

    /**
     * Creates a new instance.
     * @param itemSet the item set instance to use
     * @param inventoryView the inventory item view to use
     * @param floorView the floor view to use
     * @param spellsManager the spells manager instance to use
     * @param facesManager the faces manager instance to use
     * @param stats the stats instance to use
     * @param mapUpdater the map updater instance to use
     * @param defaultKeyBindings the default key bindings
     * @param optionManager the option manager to use
     * @param experienceTable the experience table to use
     * @param skillSet the skill set to use
     */
    public JXCSkinLoader(@NotNull final ItemSet itemSet, @NotNull final ItemView inventoryView, @NotNull final FloorView floorView, @NotNull final SpellsManager spellsManager, @NotNull final FacesManager facesManager, @NotNull final Stats stats, @NotNull final CfMapUpdater mapUpdater, @NotNull final KeyBindings defaultKeyBindings, @NotNull final OptionManager optionManager, @NotNull final ExperienceTable experienceTable, @NotNull final SkillSet skillSet) {
        this.itemSet = itemSet;
        this.inventoryView = inventoryView;
        this.floorView = floorView;
        this.spellsManager = spellsManager;
        this.facesManager = facesManager;
        facesProviderFactory = new FacesProviderFactory(facesManager);
        this.stats = stats;
        this.mapUpdater = mapUpdater;
        this.defaultKeyBindings = defaultKeyBindings;
        this.optionManager = optionManager;
        this.experienceTable = experienceTable;
        this.skillSet = skillSet;
    }

    /**
     * Loads the skin from its external representation.
     * @param skinSource the source to load from
     * @param crossfireServerConnection the server connection to attach to
     * @param guiStateManager the gui state manager instance
     * @param tooltipManager the tooltip manager to update
     * @param windowRenderer the window renderer to add to
     * @param elementListener the element listener to notify
     * @param metaserverModel the metaserver mode to use
     * @param commandQueue the command queue to use
     * @param resolution the preferred screen resolution
     * @param shortcuts the shortcuts to use
     * @param commands the commands instance to use
     * @param currentSpellManager the current spell manager to use
     * @param commandCallback the command callback to use
     * @param macros the macros instance to use
     * @param guiFactory the gui factory for creating gui elements
     * @return the loaded skin
     * @throws JXCSkinException if the skin cannot be loaded
     */
    @NotNull
    public JXCSkin load(@NotNull final JXCSkinSource skinSource, @NotNull final CrossfireServerConnection crossfireServerConnection, @NotNull final GuiStateManager guiStateManager, @NotNull final TooltipManager tooltipManager, @NotNull final JXCWindowRenderer windowRenderer, @NotNull final GUIElementListener elementListener, @NotNull final MetaserverModel metaserverModel, @NotNull final CommandQueue commandQueue, @NotNull final Resolution resolution, @NotNull final Shortcuts shortcuts, @NotNull final Commands commands, @NotNull final CurrentSpellManager currentSpellManager, @NotNull final CommandCallback commandCallback, @NotNull final Macros macros, @NotNull final GuiFactory guiFactory) throws JXCSkinException {
        imageParser = new ImageParser(skinSource);
        fontParser = new FontParser(skinSource);
        final Resolution selectedResolution = resolution;

        final Image nextGroupFace;
        try {
            nextGroupFace = ResourceUtils.loadImage(NEXT_GROUP_FACE).getImage();
        } catch (final IOException ex) {
            throw new JXCSkinException(ex.getMessage());
        }
        final Image prevGroupFace;
        try {
            prevGroupFace = ResourceUtils.loadImage(PREV_GROUP_FACE).getImage();
        } catch (final IOException ex) {
            throw new JXCSkinException(ex.getMessage());
        }

        expressionParser = new ExpressionParser(selectedResolution);
        final Dialogs dialogs = new Dialogs(guiFactory);
        gaugeUpdaterParser = new GaugeUpdaterParser(stats, itemSet, skillSet);
        commandParser = new CommandParser(dialogs, floorView, expressionParser, definedGUIElements);
        skin = new DefaultJXCSkin(defaultKeyBindings, optionManager, selectedResolution, dialogs);
        @Nullable JXCSkin skinToDetach = skin;
        try {
            guiElementParser = new GuiElementParser(definedGUIElements);
            imageParser.clear();
            skin.addDialog("keybind");
            skin.addDialog("query");
            skin.addDialog("book");
            skin.addDialog("main");
            skin.addDialog("meta");
            skin.addDialog("quit");
            skin.addDialog("disconnect");
            skin.addDialog("connect");
            skin.addDialog("start");
            definedFonts.clear();
            textButtonFactory = null;
            dialogFactory = null;
            checkBoxFactory = null;
            try {
                load(skinSource, "global", crossfireServerConnection, guiStateManager, tooltipManager, windowRenderer, elementListener, metaserverModel, commandQueue, null, shortcuts, commands, currentSpellManager, commandCallback, macros, nextGroupFace, prevGroupFace);
                for (; ;) {
                    final String name = skin.getDialogToLoad();
                    if (name == null) {
                        break;
                    }
                    final Gui gui = skin.getDialog(name);
                    load(skinSource, name, crossfireServerConnection, guiStateManager, tooltipManager, windowRenderer, elementListener, metaserverModel, commandQueue, gui, shortcuts, commands, currentSpellManager, commandCallback, macros, nextGroupFace, prevGroupFace);
                    gui.setStateChanged(false);
                }
            } finally {
                definedFonts.clear();
                textButtonFactory = null;
                dialogFactory = null;
                checkBoxFactory = null;
                imageParser.clear();
            }

            if (skin.getMapWidth() == 0 || skin.getMapHeight() == 0) {
                throw new JXCSkinException("Missing map command");
            }

            skinToDetach = null;
        } finally {
            if (skinToDetach != null) {
                skinToDetach.detach();
            }
        }

        if (skin.getMinResolution().getWidth() > resolution.getWidth() || skin.getMinResolution().getHeight() > resolution.getHeight()) {
            throw new JXCSkinException("resolution "+resolution+" is not supported by this skin");
        }
        if (resolution.getWidth() > skin.getMaxResolution().getWidth() || resolution.getHeight() > skin.getMaxResolution().getHeight()) {
            throw new JXCSkinException("resolution "+resolution+" is not supported by this skin");
        }

        return skin;
    }

    /**
     * Loads a skin file and add the entries to a {@link Gui} instance.
     * @param skinSource th source to load from
     * @param dialogName the key to identify this dialog
     * @param server the server connection to monitor
     * @param guiStateManager the gui state manager instance
     * @param tooltipManager the tooltip manager to update
     * @param windowRenderer the window renderer to add to
     * @param elementListener the element listener to notify
     * @param metaserverModel the metaserver model to use
     * @param commandQueue the command queue for sending commands
     * @param gui the Gui representing the skin file
     * @param shortcuts the shortcuts instance
     * @param commands the commands instance for executing commands
     * @param currentSpellManager the current spell manager to use
     * @param commandCallback the command callback to use
     * @param macros the macros instance to use
     * @param nextGroupFace the image for "next group of items"
     * @param prevGroupFace the image for "prev group of items"
     * @throws JXCSkinException if the file cannot be loaded
     */
    private void load(@NotNull final JXCSkinSource skinSource, @NotNull final String dialogName, @NotNull final CrossfireServerConnection server, @NotNull final GuiStateManager guiStateManager, @NotNull final TooltipManager tooltipManager, @NotNull final JXCWindowRenderer windowRenderer, @NotNull final GUIElementListener elementListener, @NotNull final MetaserverModel metaserverModel, @NotNull final CommandQueue commandQueue, @Nullable final Gui gui, @NotNull final Shortcuts shortcuts, @NotNull final Commands commands, @NotNull final CurrentSpellManager currentSpellManager, @NotNull final CommandCallback commandCallback, @NotNull final Macros macros, @NotNull final Image nextGroupFace, @NotNull final Image prevGroupFace) throws JXCSkinException {
        String resourceName = dialogName+".skin";

        definedGUIElements.clear();
        try {
            InputStream inputStream;
            try {
                inputStream = skinSource.getInputStream(resourceName);
            } catch (final IOException ex) {
                resourceName = dialogName+".skin";
                inputStream = skinSource.getInputStream(resourceName);
            }
            try {
                load(skinSource, dialogName, resourceName, inputStream, server, guiStateManager, tooltipManager, windowRenderer, elementListener, metaserverModel, commandQueue, gui, shortcuts, commands, currentSpellManager, commandCallback, macros, nextGroupFace, prevGroupFace);
            } finally {
                inputStream.close();
            }
        } catch (final IOException ex) {
            throw new JXCSkinException(skinSource.getURI(resourceName)+": "+ex.getMessage());
        } catch (final JXCSkinException ex) {
            throw new JXCSkinException(skinSource.getURI(resourceName)+": "+ex.getMessage());
        } finally {
            definedGUIElements.clear();
        }
    }

    /**
     * Loads a skin file and add the entries to a {@link Gui} instance.
     * @param skinSource the source to load from
     * @param dialogName the key to identify this dialog
     * @param resourceName the name of the skin resource; used to construct
     * error messages
     * @param inputStream the input stream to load from
     * @param server the server connection to monitor
     * @param guiStateManager the gui state manager instance
     * @param tooltipManager the tooltip manager to update
     * @param windowRenderer the window renderer to add to
     * @param elementListener the element listener to notify
     * @param metaserverModel the metaserver model to use
     * @param commandQueue the command queue for sending commands
     * @param gui the Gui representing the skin file
     * @param shortcuts the shortcuts instance
     * @param commands the commands instance for executing commands
     * @param currentSpellManager the current spell manager to use
     * @param commandCallback the command callback to use
     * @param macros the macros instance to use
     * @param nextGroupFace the image for "next group of items"
     * @param prevGroupFace the image for "prev group of items"
     * @throws JXCSkinException if the file cannot be loaded
     */
    private void load(@NotNull final JXCSkinSource skinSource, @NotNull final String dialogName, @NotNull final String resourceName, @NotNull final InputStream inputStream, @NotNull final CrossfireServerConnection server, @NotNull final GuiStateManager guiStateManager, @NotNull final TooltipManager tooltipManager, @NotNull final JXCWindowRenderer windowRenderer, @NotNull final GUIElementListener elementListener, @NotNull final MetaserverModel metaserverModel, @NotNull final CommandQueue commandQueue, @Nullable final Gui gui, @NotNull final Shortcuts shortcuts, @NotNull final Commands commands, @NotNull final CurrentSpellManager currentSpellManager, @NotNull final CommandCallback commandCallback, @NotNull final Macros macros, @NotNull final Image nextGroupFace, @NotNull final Image prevGroupFace) throws JXCSkinException {
        final List<GUIElement> addedElements = new ArrayList<GUIElement>();
        boolean addedElementsContainsWildcard = false;

        try {
            final InputStreamReader isr = new InputStreamReader(inputStream, "UTF-8");
            try {
                final LineNumberReader lnr = new LineNumberReader(isr);
                try {
                    for (; ;) {
                        final String line = lnr.readLine();
                        if (line == null) {
                            break;
                        }

                        if (line.startsWith("#") || line.length() == 0) {
                            continue;
                        }

                        final String[] args;
                        try {
                            args = StringUtils.splitFields(line);
                        } catch (final UnterminatedTokenException ex) {
                            throw new JXCSkinException(ex.getMessage());
                        }
                        if (gui != null && args[0].equals("add")) {
                            if (args.length != 2) {
                                throw new IOException("syntax error");
                            }

                            if (args[1].equals("*")) {
                                addedElementsContainsWildcard = true;
                                addedElements.add(null);
                            } else {
                                addedElements.add(definedGUIElements.lookup(args[1]));
                            }
                        } else if (gui != null && args[0].equals("button")) {
                            parseButton(args, tooltipManager, elementListener, lnr);
                        } else if (gui != null && args[0].equals("checkbox")) {
                            parseCheckbox(args, tooltipManager, elementListener, lnr);
                        } else if (args[0].equals("commandlist")) {
                            parseCommandList(args, guiStateManager, lnr, commands, commandQueue, server, commandCallback, macros);
                        } else if (args[0].equals("commandlist_add")) {
                            parseCommandListAdd(args, guiStateManager, lnr, commands, commandQueue, server, commandCallback, macros);
                        } else if (gui != null && args[0].equals("command_text")) {
                            parseCommandText(args, commandCallback, tooltipManager, elementListener, commands);
                        } else if (args[0].equals("def")) {
                            parseDef(args, lnr);
                        } else if (gui != null && args[0].equals("dialog")) {
                            parseDialog(args, tooltipManager, windowRenderer, elementListener, lnr, gui, dialogName);
                        } else if (gui != null && args[0].equals("dialog_hide")) {
                            parseDialogHide(args, gui);
                        } else if (gui != null && args[0].equals("dupgauge")) {
                            parseDupGauge(args, tooltipManager, elementListener, lnr);
                        } else if (gui != null && args[0].equals("duptextgauge")) {
                            parseDupTextGauge(args, tooltipManager, elementListener, lnr);
                        } else if (args[0].equals("event")) {
                            parseEvent(args, guiStateManager, server);
                        } else if (args[0].equals("font")) {
                            parseFont(args);
                        } else if (gui != null && args[0].equals("gauge")) {
                            parseGauge(args, tooltipManager, elementListener, lnr);
                        } else if (gui != null && args[0].equals("ignore")) {
                            parseIgnore(args);
                        } else if (gui != null && args[0].equals("inventory_list")) {
                            parseList(args, true, tooltipManager, elementListener, commandQueue, server, nextGroupFace, prevGroupFace);
                        } else if (gui != null && args[0].equals("floor_list")) {
                            parseList(args, false, tooltipManager, elementListener, commandQueue, server, nextGroupFace, prevGroupFace);
                        } else if (gui != null && args[0].equals("item")) {
                            parseItem(args, tooltipManager, elementListener, commandQueue, server, shortcuts, currentSpellManager, nextGroupFace, prevGroupFace);
                        } else if (args[0].equals("key")) {
                            parseKey(args, gui, line);
                        } else if (gui != null && args[0].equals("label_html")) {
                            parseLabelHtml(args, tooltipManager, elementListener, lnr);
                        } else if (gui != null && args[0].equals("label_multi")) {
                            parseLabelMulti(args, tooltipManager, elementListener, lnr);
                        } else if (gui != null && args[0].equals("label_query")) {
                            parseLabelQuery(args, tooltipManager, elementListener, server);
                        } else if (gui != null && args[0].equals("label_message")) {
                            parseLabelMessage(args, tooltipManager, elementListener, server);
                        } else if (gui != null && args[0].equals("label_text")) {
                            parseLabelText(args, tooltipManager, elementListener, lnr);
                        } else if (gui != null && args[0].equals("label_stat")) {
                            parseLabelStat(args, tooltipManager, elementListener);
                        } else if (gui != null && args[0].equals("label_spell")) {
                            parseLabelSpell(args, tooltipManager, elementListener, currentSpellManager);
                        } else if (gui != null && args[0].equals("log_label")) {
                            parseLogLabel(args, tooltipManager, elementListener);
                        } else if (gui != null && args[0].equals("log_message")) {
                            parseLogMessage(args, tooltipManager, elementListener, server);
                        } else if (gui != null && args[0].equals("log_color")) {
                            parseLogColor(args);
                        } else if (gui != null && args[0].equals("log_filter")) {
                            parseLogFilter(args);
                        } else if (gui != null && args[0].equals("magicmap")) {
                            parseMagicmap(args, tooltipManager, elementListener, server);
                        } else if (gui != null && args[0].equals("map")) {
                            parseMap(args, tooltipManager, elementListener, server);
                        } else if (gui != null && args[0].equals("meta_list")) {
                            parseMetaList(args, tooltipManager, elementListener, metaserverModel);
                        } else if (gui != null && args[0].equals("picture")) {
                            parsePicture(args, tooltipManager, elementListener);
                        } else if (gui != null && args[0].equals("query_text")) {
                            parseQueryText(args, server, commandCallback, tooltipManager, elementListener);
                        } else if (gui != null && args[0].equals("set_forced_active")) {
                            parseSetForcedActive(args, gui);
                        } else if (gui != null && args[0].equals("set_default")) {
                            parseSetDefault(args);
                        } else if (gui != null && args[0].equals("set_invisible")) {
                            parseSetInvisible(args);
                        } else if (gui != null && args[0].equals("set_modal")) {
                            parseSetModal(args, gui);
                        } else if (gui != null && args[0].equals("set_num_look_objects")) {
                            parseSetNumLookObjects(args);
                        } else if (gui != null && args[0].equals("scrollbar")) {
                            parseScrollbar(args, tooltipManager, elementListener);
                        } else if (gui == null && args[0].equals("skin_name")) {
                            parseSkinName(args);
                        } else if (gui != null && args[0].equals("text")) {
                            parseText(args, commandCallback, tooltipManager, elementListener);
                        } else if (gui != null && args[0].equals("textbutton")) {
                            parseTextButton(args, tooltipManager, elementListener, lnr);
                        } else if (gui != null && args[0].equals("textgauge")) {
                            parseTextGauge(args, tooltipManager, elementListener, lnr);
                        } else if (args[0].equals("tooltip")) {
                            parseTooltip(args, tooltipManager, elementListener);
                        } else {
                            throw new IOException("unknown keyword '"+args[0]+"'");
                        }
                    }
                } catch (final IOException ex) {
                    throw new IOException(ex.getMessage()+" in line "+lnr.getLineNumber());
                } catch (final JXCSkinException ex) {
                    throw new IOException(ex.getMessage()+" in line "+lnr.getLineNumber());
                } catch (final IllegalArgumentException ex) {
                    final Object msg = ex.getMessage();
                    if (msg != null) {
                        throw new IOException("invalid parameter ("+ex.getMessage()+") in line "+lnr.getLineNumber());
                    } else {
                        throw new IOException("invalid parameter in line "+lnr.getLineNumber());
                    }
                } finally {
                    lnr.close();
                }
            } finally {
                isr.close();
            }
        } catch (final IOException ex) {
            throw new JXCSkinException(skinSource.getURI(resourceName)+": "+ex.getMessage());
        }

        final Iterator<GUIElement> it = definedGUIElements.iterator();
        assert gui != null || !it.hasNext();

        final Map<GUIElement, GUIElement> wildcardElements = new LinkedHashMap<GUIElement, GUIElement>();
        while (it.hasNext()) {
            final GUIElement element = it.next();
            wildcardElements.put(element, element);
        }
        for (final GUIElement element : addedElements) {
            wildcardElements.remove(element);
        }

        int i = 0;
        if (addedElementsContainsWildcard) {
            while (i < addedElements.size()) {
                final GUIElement element = addedElements.get(i);
                if (element == null) {
                    break;
                }
                gui.add(element);
                i++;
            }
            assert i < addedElements.size();
            i++;
        }

        for (final GUIElement element : wildcardElements.keySet()) {
            gui.add(element);
        }

        while (i < addedElements.size()) {
            final GUIElement element = addedElements.get(i);
            if (element != null) {
                gui.add(element);
            }
            i++;
        }
    }

    /**
     * Parses a "button" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseButton(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length != 10 && args.length < 14) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage upImage = imageParser.getImage(args[6]);
        final BufferedImage downImage = imageParser.getImage(args[7]);
        final boolean autoRepeat = NumberParser.parseBoolean(args[8]);
        final CommandList commandList = skin.getCommandList(args[9]);
        @Nullable final String label;
        @Nullable final Font font;
        @Nullable final Color color;
        final int textX;
        final int textY;
        if (args.length == 10) {
            label = null;
            font = null;
            color = null;
            textX = 0;
            textY = 0;
        } else {
            assert args.length >= 14;
            font = definedFonts.lookup(args[10]);
            color = ParseUtils.parseColor(args[11]);
            textX = expressionParser.parseInt(args[12]);
            textY = expressionParser.parseInt(args[13]);
            label = ParseUtils.parseText(args, 14, lnr);
        }
        insertGuiElement(new GUIButton(tooltipManager, elementListener, name, x, y, w, h, upImage, downImage, label, font, color, textX, textY, autoRepeat, commandList));
    }

    /**
     * Parses a "checkbox" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseCheckbox(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 7) {
            throw new IOException("syntax error");
        }

        if (checkBoxFactory == null) {
            throw new IOException("missing 'def checkbox' command");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final CheckBoxOption option = ParseUtils.parseCheckBoxOption(args[6], optionManager);
        final String text = ParseUtils.parseText(args, 7, lnr);
        assert checkBoxFactory != null;
        insertGuiElement(checkBoxFactory.newCheckBox(tooltipManager, elementListener, name, x, y, w, h, option, text));
    }

    /**
     * Parses a "commandlist" command.
     * @param args the command arguments
     * @param guiStateManager the gui state manager instance
     * @param lnr the line number reader for reading more lines
     * @param commands the commands to add to
     * @param commandQueue the command queue to use
     * @param server the server to use
     * @param commandCallback the command callback to use
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseCommandList(@NotNull final String[] args, @NotNull final GuiStateManager guiStateManager, @NotNull final LineNumberReader lnr, @NotNull final Commands commands, @NotNull final CommandQueue commandQueue, @NotNull final CrossfireServerConnection server, @NotNull final CommandCallback commandCallback, @NotNull final Macros macros) throws IOException, JXCSkinException {
        if (args.length != 3 && args.length < 5) {
            throw new IOException("syntax error");
        }

        final String commandListName = args[1];
        final CommandListType commandListType = NumberParser.parseEnum(CommandListType.class, args[2], "type");
        skin.addCommandList(commandListName, commandListType);
        if (args.length >= 5) {
            final GUIElement element = args[3].equals("null") ? null : definedGUIElements.lookup(args[3]);
            addCommand(commandListName, args, 5, element, args[4], guiStateManager, commands, lnr, commandQueue, server, commandCallback, macros);
        }
    }

    /**
     * Parses a "commandlist_add" command.
     * @param args the command arguments
     * @param guiStateManager the gui state manager instance
     * @param lnr the line number reader for reading more lines
     * @param commands the commands to add to
     * @param commandQueue the command queue to use
     * @param server the server to use
     * @param commandCallback the command callback to use
     * @param macros the macros instance to use
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseCommandListAdd(@NotNull final String[] args, @NotNull final GuiStateManager guiStateManager, @NotNull final LineNumberReader lnr, @NotNull final Commands commands, @NotNull final CommandQueue commandQueue, @NotNull final CrossfireServerConnection server, @NotNull final CommandCallback commandCallback, @NotNull final Macros macros) throws IOException, JXCSkinException {
        if (args.length < 4) {
            throw new IOException("syntax error");
        }

        final GUIElement element = args[2].equals("null") ? null : definedGUIElements.lookup(args[2]);
        addCommand(args[1], args, 4, element, args[3], guiStateManager, commands, lnr, commandQueue, server, commandCallback, macros);
    }

    /**
     * Parses a "command_text" command.
     * @param args the command arguments
     * @param commandCallback the command callback to use
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param commands the commands to use
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseCommandText(@NotNull final String[] args, @NotNull final CommandCallback commandCallback, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final Commands commands) throws IOException, JXCSkinException {
        if (args.length != 12) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage activeImage = imageParser.getImage(args[6]);
        final BufferedImage inactiveImage = imageParser.getImage(args[7]);
        final Font font = definedFonts.lookup(args[8]);
        final Color inactiveColor = ParseUtils.parseColor(args[9]);
        final Color activeColor = ParseUtils.parseColor(args[10]);
        final int margin = expressionParser.parseInt(args[11]);
        insertGuiElement(new GUICommandText(commandCallback, tooltipManager, elementListener, name, x, y, w, h, activeImage, inactiveImage, font, inactiveColor, activeColor, margin, "", commands, false));
    }

    /**
     * Parses a "def" command.
     * @param args the command arguments
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseDef(@NotNull final String[] args, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 2) {
            throw new IOException("syntax error");
        }

        if (args[1].equals("checkbox")) {
            if (args.length != 6) {
                throw new IOException("syntax error");
            }

            final BufferedImage checkedImage = imageParser.getImage(args[2]);
            final BufferedImage uncheckedImage = imageParser.getImage(args[3]);
            final Font font = definedFonts.lookup(args[4]);
            final Color color = ParseUtils.parseColor(args[5]);
            checkBoxFactory = new CheckBoxFactory(checkedImage, uncheckedImage, font, color);
        } else if (args[1].equals("checkbox_option")) {
            if (args.length < 5) {
                throw new IOException("syntax error");
            }

            final String optionName = args[2];
            final CommandList commandOn = skin.getCommandList(args[3]);
            final CommandList commandOff = skin.getCommandList(args[4]);
            final String documentation = ParseUtils.parseText(args, 5, lnr);
            skin.addOption(optionName, documentation, new CommandCheckBoxOption(commandOn, commandOff));
        } else if (args[1].equals("dialog")) {
            if (args.length != 7) {
                throw new IOException("syntax error");
            }

            final String frame = args[2];
            final BufferedImage frameNW = imageParser.getImage(frame+"_nw");
            final BufferedImage frameN = imageParser.getImage(frame+"_n");
            final BufferedImage frameNE = imageParser.getImage(frame+"_ne");
            final BufferedImage frameW = imageParser.getImage(frame+"_w");
            final BufferedImage frameC = imageParser.getImage(frame+"_c");
            final BufferedImage frameE = imageParser.getImage(frame+"_e");
            final BufferedImage frameSW = imageParser.getImage(frame+"_sw");
            final BufferedImage frameS = imageParser.getImage(frame+"_s");
            final BufferedImage frameSE = imageParser.getImage(frame+"_se");
            final Font titleFont = definedFonts.lookup(args[3]);
            final Color titleColor = ParseUtils.parseColor(args[4]);
            final Color titleBackgroundColor = ParseUtils.parseColor(args[5]);
            final float alpha = NumberParser.parseFloat(args[6]);
            if (alpha < 0 || alpha > 1F) {
                throw new IOException("invalid alpha value: "+alpha);
            }
            dialogFactory = new DialogFactory(frameNW, frameN, frameNE, frameW, frameC, frameE, frameSW, frameS, frameSE, titleFont, titleColor, titleBackgroundColor, alpha);
        } else if (args[1].equals("item")) {
            if (args.length != 12) {
                throw new IOException("syntax error");
            }

            final Color cursedColor = ParseUtils.parseColorNull(args[2]);
            final BufferedImage cursedImage = imageParser.getImage(cursedColor, args[2]);
            final Color damnedColor = ParseUtils.parseColorNull(args[3]);
            final BufferedImage damnedImage = imageParser.getImage(damnedColor, args[3]);
            final Color magicColor = ParseUtils.parseColorNull(args[4]);
            final BufferedImage magicImage = imageParser.getImage(magicColor, args[4]);
            final Color blessedColor = ParseUtils.parseColorNull(args[5]);
            final BufferedImage blessedImage = imageParser.getImage(blessedColor, args[5]);
            final Color appliedColor = ParseUtils.parseColorNull(args[6]);
            final BufferedImage appliedImage = imageParser.getImage(appliedColor, args[6]);
            final Color selectorColor = ParseUtils.parseColorNull(args[7]);
            final BufferedImage selectorImage = imageParser.getImage(selectorColor, args[7]);
            final Color lockedColor = ParseUtils.parseColorNull(args[8]);
            final BufferedImage lockedImage = imageParser.getImage(lockedColor, args[8]);
            final Color unpaidColor = ParseUtils.parseColorNull(args[9]);
            final BufferedImage unpaidImage = imageParser.getImage(unpaidColor, args[9]);
            final Font font = definedFonts.lookup(args[10]);
            final Color nrofColor = ParseUtils.parseColor(args[11]);
            defaultItemPainter = new ItemPainter(cursedImage, damnedImage, magicImage, blessedImage, appliedImage, selectorImage, lockedImage, unpaidImage, cursedColor, damnedColor, magicColor, blessedColor, appliedColor, selectorColor, lockedColor, unpaidColor, font, nrofColor, 1, 1);
        } else if (args[1].equals("textbutton")) {
            if (args.length != 6) {
                throw new IOException("syntax error");
            }

            final String up = args[2];
            final String down = args[3];
            final Font font = definedFonts.lookup(args[4]);
            final Color color = ParseUtils.parseColor(args[5]);
            final ButtonImages upImages = new ButtonImages(imageParser.getImage(up+"_w"), imageParser.getImage(up+"_c"), imageParser.getImage(up+"_e"));
            final ButtonImages downImages = new ButtonImages(imageParser.getImage(down+"_w"), imageParser.getImage(down+"_c"), imageParser.getImage(down+"_e"));
            textButtonFactory = new TextButtonFactory(upImages, downImages, font, color);
        } else {
            throw new IOException("unknown type '"+args[1]+"'");
        }
    }

    /**
     * Parses a "dialog" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param windowRenderer the window renderer the dialog belongs to
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @param gui the gui instance to add to
     * @param dialogName the dialog name
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseDialog(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final JXCWindowRenderer windowRenderer, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr, @NotNull final Gui gui, @NotNull final String dialogName) throws IOException, JXCSkinException {
        if (args.length < 7) {
            throw new IOException("syntax error");
        }

        if (dialogFactory == null) {
            throw new IOException("missing 'def dialog' command");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final boolean saveDialog = NumberParser.parseBoolean(args[6]);
        final String title = ParseUtils.parseText(args, 7, lnr);
        assert dialogFactory != null;
        for (final GUIElement element : dialogFactory.newDialog(tooltipManager, windowRenderer, elementListener, name, w, h, title)) {
            insertGuiElement(element);
        }
        if (saveDialog) {
            gui.setName(dialogName);
        }
        gui.setSize(w, h);
        gui.setPosition(x, y);
    }

    /**
     * Parses a "dialog_hide" command.
     * @param args the command arguments
     * @param gui the gui instance to use
     * @throws IOException if the command cannot be parsed
     */
    private static void parseDialogHide(@NotNull final String[] args, @NotNull final Gui gui) throws IOException {
        if (args.length < 2) {
            throw new IOException("syntax error");
        }

        for (int i = 1; i < args.length; i++) {
            gui.hideInState(NumberParser.parseEnum(RendererGuiState.class, args[i], "gui state"));
        }
    }

    /**
     * Parses a "dupgauge" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseDupGauge(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 12) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage positiveDivImage = imageParser.getImage(args[6]);
        final BufferedImage positiveModImage = imageParser.getImage(args[7]);
        final BufferedImage emptyImage = args[8].equals("null") ? null : imageParser.getImage(args[8]);
        final GaugeUpdater gaugeUpdater = newGaugeUpdater(args[9]);
        final Orientation orientationDiv = ParseUtils.parseOrientation(args[10]);
        final Orientation orientationMod = ParseUtils.parseOrientation(args[11]);
        final String tooltipPrefix = ParseUtils.parseText(args, 12, lnr);
        final GUIDupGauge element = new GUIDupGauge(tooltipManager, elementListener, name, x, y, w, h, positiveDivImage, positiveModImage, emptyImage, orientationDiv, orientationMod, tooltipPrefix.length() > 0 ? tooltipPrefix : null);
        insertGuiElement(element);
        gaugeUpdater.setGauge(element);
    }

    /**
     * Parses a "duptextgauge" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseDupTextGauge(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 14) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage positiveDivImage = imageParser.getImage(args[6]);
        final BufferedImage positiveModImage = imageParser.getImage(args[7]);
        final BufferedImage emptyImage = imageParser.getImage(args[8]);
        final GaugeUpdater gaugeUpdater = newGaugeUpdater(args[9]);
        final Orientation orientationDiv = ParseUtils.parseOrientation(args[10]);
        final Orientation orientationMod = ParseUtils.parseOrientation(args[11]);
        final Color color = ParseUtils.parseColor(args[12]);
        final Font font = definedFonts.lookup(args[13]);
        final String tooltipPrefix = ParseUtils.parseText(args, 14, lnr);
        final GUIDupTextGauge element = new GUIDupTextGauge(tooltipManager, elementListener, name, x, y, w, h, positiveDivImage, positiveModImage, emptyImage, orientationDiv, orientationMod, tooltipPrefix.length() > 0 ? tooltipPrefix : null, color, font);
        insertGuiElement(element);
        gaugeUpdater.setGauge(element);
    }

    /**
     * Parses an "event" command.
     * @param args the command arguments
     * @param guiStateManager the gui state manager instance
     * @param server the server to monitor
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseEvent(@NotNull final String[] args, @NotNull final GuiStateManager guiStateManager, @NotNull final CrossfireServerConnection server) throws IOException, JXCSkinException {
        if (args.length < 2) {
            throw new IOException("syntax error");
        }

        final String type = args[1];
        if (type.equals("connect")) {
            if (args.length != 3) {
                throw new IOException("syntax error");
            }

            final CommandList commandList = skin.getCommandList(args[2]);
            skin.addSkinEvent(new ConnectionStateSkinEvent(commandList, guiStateManager));
        } else if (type.equals("init")) {
            if (args.length != 3) {
                throw new IOException("syntax error");
            }

            skin.addInitEvent(skin.getCommandList(args[2]));
        } else if (type.equals("magicmap")) {
            if (args.length != 3) {
                throw new IOException("syntax error");
            }

            final CommandList commandList = skin.getCommandList(args[2]);
            skin.addSkinEvent(new CrossfireMagicmapSkinEvent(commandList, server));
        } else if (type.equals("mapscroll")) {
            if (args.length != 3) {
                throw new IOException("syntax error");
            }

            final CommandList commandList = skin.getCommandList(args[2]);
            skin.addSkinEvent(new MapscrollSkinEvent(commandList, mapUpdater));
        } else if (type.equals("skill")) {
            if (args.length != 5) {
                throw new IOException("syntax error");
            }

            final String subtype = args[2];
            final Skill skill = skillSet.getNamedSkill(args[3].replaceAll("_", " "));
            final CommandList commandList = skin.getCommandList(args[4]);
            if (subtype.equals("add")) {
                skin.addSkinEvent(new SkillAddedSkinEvent(commandList, skill));
            } else if (subtype.equals("del")) {
                skin.addSkinEvent(new SkillRemovedSkinEvent(commandList, skill));
            } else {
                throw new IOException("undefined event sub-type: "+subtype);
            }
        } else {
            throw new IOException("undefined event type: "+type);
        }
    }

    /**
     * Parses a "font" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseFont(@NotNull final String[] args) throws IOException, JXCSkinException {
        if (args.length != 4) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final Font fontNormal = fontParser.getFont(args[2]);
        final Font font = fontNormal.deriveFont(NumberParser.parseFloat(args[3]));
        definedFonts.insert(name, font);
    }

    /**
     * Parses a "gauge" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseGauge(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 11) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage positiveImage = args[6].equals("null") ? null : imageParser.getImage(args[6]);
        final BufferedImage negativeImage = args[7].equals("null") ? null : imageParser.getImage(args[7]);
        final BufferedImage emptyImage = args[8].equals("null") ? null : imageParser.getImage(args[8]);
        final GaugeUpdater gaugeUpdater = newGaugeUpdater(args[9]);
        final Orientation orientation = ParseUtils.parseOrientation(args[10]);
        final String tooltipPrefix = ParseUtils.parseText(args, 11, lnr);
        final GUIGauge element = new GUIGauge(tooltipManager, elementListener, name, x, y, w, h, positiveImage, negativeImage, emptyImage, orientation, tooltipPrefix.length() > 0 ? tooltipPrefix : null);
        insertGuiElement(element);
        gaugeUpdater.setGauge(element);
    }

    /**
     * Parses an "ignore" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseIgnore(@NotNull final String[] args) throws IOException, JXCSkinException {
        if (args.length != 2) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        definedGUIElements.lookup(name).setIgnore();
    }

    /**
     * Parses an "inventory_list" or a "floor_list" command.
     * @param args the command arguments
     * @param inventoryList <code>true</code> for "inventory_list" command,
     * <code>false</code> for "floor_list" command
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param commandQueue the command queue to use
     * @param server the server to use
     * @param nextGroupFace the image for "next group of items"
     * @param prevGroupFace the image for "prev group of items"
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseList(@NotNull final String[] args, final boolean inventoryList, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CommandQueue commandQueue, @NotNull final CrossfireServerConnection server, @NotNull final Image nextGroupFace, @NotNull final Image prevGroupFace) throws IOException, JXCSkinException {
        if (args.length != 8) {
            throw new IOException("syntax error");
        }

        if (defaultItemPainter == null) {
            throw new IOException("cannot use '"+(inventoryList ? "inventory_list" : "floor_list")+"' without 'def item' command");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final int cellHeight = expressionParser.parseInt(args[6]);
        final AbstractLabel selectedItem = args[7].equals("null") ? null : guiElementParser.lookupLabelElement(args[7]);

        assert defaultItemPainter != null;
        final ItemPainter itemPainter = defaultItemPainter.newItemPainter(cellHeight, cellHeight);
        final GUIItemItemFactory itemFactory;
        if (inventoryList) {
            itemFactory = new GUIItemInventoryFactory(tooltipManager, elementListener, commandQueue, name, itemPainter, server, facesManager, floorView, inventoryView);
        } else {
            itemFactory = new GUIItemFloorFactory(tooltipManager, elementListener, commandQueue, name, itemPainter, server, facesManager, floorView, itemSet, nextGroupFace, prevGroupFace);
        }
        final GUIElement element = new GUIItemList(tooltipManager, elementListener, commandQueue, name, x, y, w, h, cellHeight, server, inventoryList ? inventoryView : floorView, selectedItem, itemFactory);
        insertGuiElement(element);

        if (!inventoryList) {
            skin.setNumLookObjects(Math.max(h/cellHeight, 1)*Math.max(w/cellHeight, 1));
        }
    }

    /**
     * Parses an "item" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param commandQueue the command queue to use
     * @param server the server to use
     * @param shortcuts the shortcuts to use
     * @param currentSpellManager the current spell manager to use
     * @param nextGroupFace the image for "next group of items"
     * @param prevGroupFace the image for "prev group of items"
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseItem(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CommandQueue commandQueue, @NotNull final CrossfireServerConnection server, @NotNull final Shortcuts shortcuts, @NotNull final CurrentSpellManager currentSpellManager, @NotNull final Image nextGroupFace, @NotNull final Image prevGroupFace) throws IOException, JXCSkinException {
        if (args.length < 8) {
            throw new IOException("syntax error");
        }

        final String type = args[1];
        final String name = args[2];
        final int x = expressionParser.parseInt(args[3]);
        final int y = expressionParser.parseInt(args[4]);
        final int w = expressionParser.parseInt(args[5]);
        final int h = expressionParser.parseInt(args[6]);
        final int index = expressionParser.parseInt(args[7]);
        final GUIElement element;
        if (type.equals("floor")) {
            if (args.length != 8) {
                throw new IOException("syntax error");
            }

            if (defaultItemPainter == null) {
                throw new IOException("cannot use 'item floor' without 'def item' command");
            }

            final ItemPainter itemPainter = defaultItemPainter.newItemPainter(w, h);
            element = new GUIItemFloor(tooltipManager, elementListener, commandQueue, name, x, y, w, h, itemPainter, index, server, floorView, itemSet, facesManager, nextGroupFace, prevGroupFace);
        } else if (type.equals("inventory")) {
            if (args.length != 8) {
                throw new IOException("syntax error");
            }

            if (defaultItemPainter == null) {
                throw new IOException("cannot use 'item floor' without 'def item' command");
            }

            final ItemPainter itemPainter = defaultItemPainter.newItemPainter(w, h);
            element = new GUIItemInventory(tooltipManager, elementListener, commandQueue, name, x, y, w, h, itemPainter, index, server, facesManager, floorView, inventoryView);
        } else if (type.equals("shortcut")) {
            if (args.length != 11) {
                throw new IOException("syntax error");
            }

            final Color cursedColor = ParseUtils.parseColorNull(args[8]);
            final BufferedImage cursedImage = imageParser.getImage(cursedColor, args[8]);
            final Color appliedColor = ParseUtils.parseColorNull(args[9]);
            final BufferedImage appliedImage = imageParser.getImage(appliedColor, args[9]);
            final Font font = definedFonts.lookup(args[10]);
            element = new GUIItemShortcut(tooltipManager, elementListener, name, x, y, w, h, cursedColor, cursedImage, appliedColor, appliedImage, index, facesManager, shortcuts, font, currentSpellManager);
        } else if (type.equals("spelllist")) {
            if (args.length != 9) {
                throw new IOException("syntax error");
            }

            final Color selectorColor = ParseUtils.parseColorNull(args[8]);
            final BufferedImage selectorImage = imageParser.getImage(selectorColor, args[8]);
            element = new GUIItemSpelllist(tooltipManager, elementListener, commandQueue, name, x, y, w, h, selectorColor, selectorImage, index, facesManager, spellsManager, currentSpellManager);
        } else {
            throw new IOException("undefined item type: "+type);
        }
        insertGuiElement(element);
    }

    /**
     * Parses a "key" command.
     * @param args the command arguments
     * @param gui the gui to add to
     * @param line the unparsed command line
     * @throws IOException if the command cannot be parsed
     */
    private void parseKey(@NotNull final String[] args, @Nullable final Gui gui, @NotNull final String line) throws IOException {
        if (args.length < 2) {
            throw new IOException("syntax error");
        }

        final KeyBindings keyBindings = gui != null ? gui.getKeyBindings() : skin.getDefaultKeyBindings();
        try {
            keyBindings.parseKeyBinding(line.substring(4).trim(), true);
        } catch (final InvalidKeyBindingException ex) {
            throw new IOException("invalid key binding: "+ex.getMessage());
        }
    }

    /**
     * Parses a "label_html" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLabelHtml(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 8) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final Font font = definedFonts.lookup(args[6]);
        final Color color = ParseUtils.parseColor(args[7]);
        final String text = ParseUtils.parseText(args, 8, lnr);
        insertGuiElement(new GUIHTMLLabel(tooltipManager, elementListener, name, x, y, w, h, null, font, color, new Color(0, 0, 0, 0F), text));
    }

    /**
     * Parses a "label_multi" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLabelMulti(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 8) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final Font font = definedFonts.lookup(args[6]);
        final Color color = ParseUtils.parseColor(args[7]);
        final String text = ParseUtils.parseText(args, 8, lnr);
        insertGuiElement(new GUIMultiLineLabel(tooltipManager, elementListener, name, x, y, w, h, null, font, color, new Color(0, 0, 0, 0F), Alignment.LEFT, text));
    }

    /**
     * Parses a "label_query" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param server the server instance to monitor
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLabelQuery(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CrossfireServerConnection server) throws IOException, JXCSkinException {
        if (args.length != 8) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final Font font = definedFonts.lookup(args[6]);
        final Color color = ParseUtils.parseColor(args[7]);
        final GUIElement element = new GUILabelQuery(tooltipManager, elementListener, name, x, y, w, h, server, font, color, new Color(0, 0, 0, 0F));
        insertGuiElement(element);
    }

    /**
     * Parses a "label_message" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param server the server instance to monitor
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLabelMessage(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CrossfireServerConnection server) throws IOException, JXCSkinException {
        if (args.length != 8) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final Font font = definedFonts.lookup(args[6]);
        final Color color = ParseUtils.parseColor(args[7]);
        final GUIElement element = new GUILabelMessage(tooltipManager, elementListener, name, x, y, w, h, server, font, color, new Color(0, 0, 0, 0F));
        insertGuiElement(element);
    }

    /**
     * Parses a "label_text" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLabelText(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 8) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final Font font = definedFonts.lookup(args[6]);
        final Color color = ParseUtils.parseColor(args[7]);
        final String text = ParseUtils.parseText(args, 8, lnr);
        insertGuiElement(new GUIOneLineLabel(tooltipManager, elementListener, name, x, y, w, h, null, font, color, new Color(0, 0, 0, 0F), Alignment.LEFT, text));
    }

    /**
     * Parses a "label_stat" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLabelStat(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener) throws IOException, JXCSkinException {
        if (args.length != 10) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final Font font = definedFonts.lookup(args[6]);
        final Color color = ParseUtils.parseColor(args[7]);
        final int stat = ParseUtils.parseStat(args[8]);
        final Alignment alignment = NumberParser.parseEnum(Alignment.class, args[9], "text alignment");
        final GUIElement element = new GUILabelStats(tooltipManager, elementListener, name, x, y, w, h, font, color, new Color(0, 0, 0, 0F), stat, alignment, stats);
        insertGuiElement(element);
    }

    /**
     * Parses a "label_spell" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param currentSpellManager the current spell manager to use
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLabelSpell(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CurrentSpellManager currentSpellManager) throws IOException, JXCSkinException {
        if (args.length != 8) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final Font font = definedFonts.lookup(args[6]);
        final Type type = NumberParser.parseEnum(Type.class, args[7], "label type");
        final GUIElement element = new GUISpellLabel(tooltipManager, elementListener, name, x, y, w, h, null, facesManager, font, type, currentSpellManager);
        insertGuiElement(element);
    }

    /**
     * Parses a "log_label" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLogLabel(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener) throws IOException, JXCSkinException {
        if (args.length != 12) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage emptyImage = imageParser.getImage(args[6]);
        final Font fontPrint = definedFonts.lookup(args[7]);
        final Font fontFixed = definedFonts.lookup(args[8]);
        final Font fontFixedBold = definedFonts.lookup(args[9]);
        final Font fontArcane = definedFonts.lookup(args[10]);
        final Color defaultColor = ParseUtils.parseColor(args[11]);
        final Fonts fonts = new Fonts(fontPrint, fontFixed, fontFixedBold, fontArcane);
        final GUIElement element = new GUILabelLog(tooltipManager, elementListener, name, x, y, w, h, emptyImage, fonts, defaultColor);
        insertGuiElement(element);
    }

    /**
     * Parses a "log_message" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param server the server to use
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLogMessage(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CrossfireServerConnection server) throws IOException, JXCSkinException {
        if (args.length != 12) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage emptyImage = imageParser.getImage(args[6]);
        final Font fontPrint = definedFonts.lookup(args[7]);
        final Font fontFixed = definedFonts.lookup(args[8]);
        final Font fontFixedBold = definedFonts.lookup(args[9]);
        final Font fontArcane = definedFonts.lookup(args[10]);
        final Color defaultColor = ParseUtils.parseColor(args[11]);
        final Fonts fonts = new Fonts(fontPrint, fontFixed, fontFixedBold, fontArcane);
        final GUIElement element = new GUIMessageLog(tooltipManager, elementListener, name, x, y, w, h, server, emptyImage, fonts, defaultColor);
        insertGuiElement(element);
    }

    /**
     * Parses a "log_color" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLogColor(@NotNull final String[] args) throws IOException, JXCSkinException {
        if (args.length != 4) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int index = expressionParser.parseInt(args[2]);
        final Color color = ParseUtils.parseColor(args[3]);
        final Object element = definedGUIElements.lookup(name);
        if (!(element instanceof GUIMessageLog)) {
            throw new IOException("element '"+name+"' is not of type 'log'");
        }
        if (index < 0 || index >= MessageBufferUpdater.NUM_COLORS) {
            throw new IOException("invalid color index "+index);
        }
        ((GUIMessageLog)element).setColor(index, color);
    }

    /**
     * Parses a "log_filter" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseLogFilter(@NotNull final String[] args) throws IOException, JXCSkinException {
        if (args.length < 4) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final String type = args[2];
        final boolean add;
        if (type.equals("only")) {
            add = true;
        } else if (type.equals("not")) {
            add = false;
        } else {
            throw new IOException("type '"+type+"' is invalid");
        }
        int types = 0;
        for (int i = 3; i < args.length; i++) {
            try {
                types |= 1<<MessageTypes.parseMessageType(args[i]);
            } catch (final UnknownCommandException ex) {
                throw new IOException("undefined message type '"+args[i]+"'");
            }
        }
        if (!add) {
            types = ~types;
        }
        final Object element = definedGUIElements.lookup(name);
        if (!(element instanceof GUIMessageLog)) {
            throw new IOException("element '"+name+"' is not of type 'log'");
        }
        ((GUIMessageLog)element).setTypes(types);
    }

    /**
     * Parses a "magicmap" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param server the server to monitor
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseMagicmap(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CrossfireServerConnection server) throws IOException, JXCSkinException {
        if (args.length != 6) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final FacesProvider facesProvider = facesProviderFactory.getFacesProvider(4);
        if (facesProvider == null) {
            throw new IOException("cannot create faces with size 4");
        }
        final GUIElement element = new GUIMagicMap(tooltipManager, elementListener, name, x, y, w, h, server, mapUpdater, facesProvider);
        insertGuiElement(element);
    }

    /**
     * Parses a "map" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param server the server to monitor
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseMap(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final CrossfireServerConnection server) throws IOException, JXCSkinException {
        if (args.length != 7) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int tileSize = expressionParser.parseInt(args[2]);
        final int x = expressionParser.parseInt(args[3]);
        final int y = expressionParser.parseInt(args[4]);
        final int w = expressionParser.parseInt(args[5]);
        final int h = expressionParser.parseInt(args[6]);

        if (tileSize <= 0) {
            throw new IOException("invalid tile size "+tileSize);
        }
        skin.setMapSize((w+tileSize-1)/tileSize, (h+tileSize-1)/tileSize);

        final FacesProvider facesProvider = facesProviderFactory.getFacesProvider(tileSize);
        if (facesProvider == null) {
            throw new IOException("cannot create faces with size "+tileSize);
        }
        final GUIElement element = new GUIMap(tooltipManager, elementListener, name, x, y, w, h, mapUpdater, facesProvider, server);
        insertGuiElement(element);
    }

    /**
     * Parses a "meta_list" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param metaserverModel the metaserver model to use
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseMetaList(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final MetaserverModel metaserverModel) throws IOException, JXCSkinException {
        if (args.length != 13) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final int cellHeight = expressionParser.parseInt(args[6]);
        final BufferedImage tcpImage = args[7].equals("null") ? null : imageParser.getImage(args[7]);
        final Font font = definedFonts.lookup(args[8]);
        final GUIText text = args[9].equals("null") ? null : guiElementParser.lookupTextElement(args[9]);
        final AbstractLabel label = args[10].equals("null") ? null : guiElementParser.lookupLabelElement(args[10]);
        final String format = args[11];
        final String tooltip = args[12];

        final GUIElement list = new GUIMetaElementList(tooltipManager, elementListener, name, x, y, w, h, cellHeight, metaserverModel, tcpImage, font, format, tooltip, text, label);
        insertGuiElement(list);
    }

    /**
     * Parses a "picture" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parsePicture(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener) throws IOException, JXCSkinException {
        if (args.length != 8) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage image = imageParser.getImage(args[6]);
        final float alpha = NumberParser.parseFloat(args[7]);
        if (alpha < 0 || alpha > 1F) {
            throw new IOException("invalid alpha value: "+alpha);
        }
        insertGuiElement(new GUIPicture(tooltipManager, elementListener, name, x, y, w, h, image, alpha));
    }

    /**
     * Parses a "query_text" command.
     * @param args the command arguments
     * @param server the crossfire server connectio for sending reply commands
     * @param commandCallback the command callback to use
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseQueryText(@NotNull final String[] args, @NotNull final CrossfireServerConnection server, @NotNull final CommandCallback commandCallback, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener) throws IOException, JXCSkinException {
        if (args.length != 12) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage activeImage = imageParser.getImage(args[6]);
        final BufferedImage inactiveImage = imageParser.getImage(args[7]);
        final Font font = definedFonts.lookup(args[8]);
        final Color inactiveColor = ParseUtils.parseColor(args[9]);
        final Color activeColor = ParseUtils.parseColor(args[10]);
        final int margin = expressionParser.parseInt(args[11]);
        insertGuiElement(new GUIQueryText(server, commandCallback, tooltipManager, elementListener, name, x, y, w, h, activeImage, inactiveImage, font, inactiveColor, activeColor, margin, "", false));
    }

    /**
     * Parses a "set_forced_active" command.
     * @param args the command arguments
     * @param gui the gui to modify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseSetForcedActive(@NotNull final String[] args, @NotNull final Gui gui) throws IOException, JXCSkinException {
        if (args.length != 2) {
            throw new IOException("syntax error");
        }

        final Object forcedActive = definedGUIElements.lookup(args[1]);
        if (!(forcedActive instanceof ActivatableGUIElement)) {
            throw new IOException("argument to set_forced_active must be an activatable gui element");
        }
        gui.setForcedActive((ActivatableGUIElement)forcedActive);
    }

    /**
     * Parses a "set_default" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseSetDefault(@NotNull final String[] args) throws IOException, JXCSkinException {
        if (args.length != 2) {
            throw new IOException("syntax error");
        }

        definedGUIElements.lookup(args[1]).setDefault(true);
    }

    /**
     * Parses a "set_invisible" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseSetInvisible(@NotNull final String[] args) throws IOException, JXCSkinException {
        if (args.length != 2) {
            throw new IOException("syntax error");
        }

        definedGUIElements.lookup(args[1]).setElementVisible(false);
    }

    /**
     * Parses a "set_modal" command.
     * @param args the command arguments
     * @param gui the gui to modify
     * @throws IOException if the command cannot be parsed
     */
    private static void parseSetModal(@NotNull final String[] args, @NotNull final Gui gui) throws IOException {
        if (args.length != 1) {
            throw new IOException("syntax error");
        }

        gui.setModal(true);
    }

    /**
     * Parses a "set_num_look_objects" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     */
    private void parseSetNumLookObjects(@NotNull final String[] args) throws IOException {
        if (args.length != 2) {
            throw new IOException("syntax error");
        }

        skin.setNumLookObjects(expressionParser.parseInt(args[1]));
    }

    /**
     * Parses a "scrollbar" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseScrollbar(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener) throws IOException, JXCSkinException {
        if (args.length != 10) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final boolean proportionalSlider = NumberParser.parseBoolean(args[6]);
        final Object element = definedGUIElements.lookup(args[7]);
        final Color colorBackground = ParseUtils.parseColor(args[8]);
        final Color colorForeground = ParseUtils.parseColor(args[9]);
        if (!(element instanceof GUIScrollable2)) {
            throw new IOException("'"+element+"' is not a scrollable element");
        }
        insertGuiElement(new GUIScrollBar(tooltipManager, elementListener, name, x, y, w, h, proportionalSlider, (GUIScrollable2)element, colorBackground, colorForeground));
    }

    /**
     * Parses a "skin_name" command.
     * @param args the command arguments
     * @throws IOException if the command cannot be parsed
     */
    private void parseSkinName(@NotNull final String[] args) throws IOException {
        if (args.length != 4) {
            throw new IOException("syntax error");
        }

        final String newSkinName = args[1];
        final Resolution minResolution = parseResolution(args[2]);
        final Resolution maxResolution = parseResolution(args[3]);
        if (!newSkinName.matches("[-a-z_0-9]+")) {
            throw new IOException("invalid skin_name: "+newSkinName);
        }
        if (minResolution.getWidth() > maxResolution.getWidth() || minResolution.getHeight() > maxResolution.getHeight()) {
            throw new IOException("minimum resolution ("+minResolution+") must not exceed maximum resolution ("+maxResolution+")");
        }

        skin.setSkinName(newSkinName, minResolution, maxResolution);
    }

    /**
     * Parses a "text" command.
     * @param args the command arguments
     * @param commandCallback the command callback to use
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseText(@NotNull final String[] args, @NotNull final CommandCallback commandCallback, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener) throws IOException, JXCSkinException {
        if (args.length != 14) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage activeImage = imageParser.getImage(args[6]);
        final BufferedImage inactiveImage = imageParser.getImage(args[7]);
        final Font font = definedFonts.lookup(args[8]);
        final Color inactiveColor = ParseUtils.parseColor(args[9]);
        final Color activeColor = ParseUtils.parseColor(args[10]);
        final int margin = expressionParser.parseInt(args[11]);
        final CommandList commandList = skin.getCommandList(args[12]);
        final boolean ignoreUpDown = NumberParser.parseBoolean(args[13]);
        insertGuiElement(new GUITextField(commandCallback, tooltipManager, elementListener, name, x, y, w, h, activeImage, inactiveImage, font, inactiveColor, activeColor, margin, "", commandList, ignoreUpDown));
    }

    /**
     * Parses a "textbutton" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseTextButton(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 7) {
            throw new IOException("syntax error");
        }

        if (textButtonFactory == null) {
            throw new IOException("missing 'def textbutton' command");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final boolean autoRepeat = NumberParser.parseBoolean(args[6]);
        final CommandList commandList = skin.getCommandList(args[7]);
        final String text = ParseUtils.parseText(args, 8, lnr);
        assert textButtonFactory != null;
        insertGuiElement(textButtonFactory.newTextButton(tooltipManager, elementListener, name, x, y, w, h, text, autoRepeat, commandList));
    }

    /**
     * Parses a "textgauge" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param lnr the line number reader for reading more lines
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseTextGauge(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final LineNumberReader lnr) throws IOException, JXCSkinException {
        if (args.length < 13) {
            throw new IOException("syntax error");
        }

        final String name = args[1];
        final int x = expressionParser.parseInt(args[2]);
        final int y = expressionParser.parseInt(args[3]);
        final int w = expressionParser.parseInt(args[4]);
        final int h = expressionParser.parseInt(args[5]);
        final BufferedImage positiveImage = imageParser.getImage(args[6]);
        final BufferedImage negativeImage = args[7].equals("null") ? null : imageParser.getImage(args[7]);
        final BufferedImage emptyImage = imageParser.getImage(args[8]);
        final GaugeUpdater gaugeUpdater = newGaugeUpdater(args[9]);
        final Orientation orientation = ParseUtils.parseOrientation(args[10]);
        final Color color = ParseUtils.parseColor(args[11]);
        final Font font = definedFonts.lookup(args[12]);
        final String tooltipPrefix = ParseUtils.parseText(args, 13, lnr);
        final GUITextGauge element = new GUITextGauge(tooltipManager, elementListener, name, x, y, w, h, positiveImage, negativeImage, emptyImage, orientation, tooltipPrefix.length() > 0 ? tooltipPrefix : null, color, font);
        insertGuiElement(element);
        gaugeUpdater.setGauge(element);
    }

    /**
     * Parses a "tooltip" command.
     * @param args the command arguments
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @throws IOException if the command cannot be parsed
     * @throws JXCSkinException if the command cannot be parsed
     */
    private void parseTooltip(@NotNull final String[] args, @NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener) throws IOException, JXCSkinException {
        if (args.length != 2) {
            throw new IOException("syntax error");
        }

        final Font font = definedFonts.lookup(args[1]);
        final GUIHTMLLabel tooltipLabel = new GUIHTMLLabel(tooltipManager, elementListener, "tooltip", 0, 0, 1, 1, null, font, Color.BLACK, Color.WHITE, "");
        tooltipLabel.setAutoResize(true);
        skin.setTooltipLabel(tooltipLabel);
    }

    /**
     * Parses a resolution specification.
     * @param text the resolution specification
     * @return the resolution
     * @throws IOException if the resolution specification cannot be parsed
     */
    @NotNull
    private Resolution parseResolution(@NotNull final String text) throws IOException {
        final Resolution resolution = Resolution.parse(true, text);
        if (resolution == null) {
            throw new IOException("invalid resolution: "+resolution);
        }
        return resolution;
    }

    /**
     * Adds a new {@link GUIElement} to this skin.
     * @param guiElement the GUI element
     * @throws JXCSkinException if the name is not unique
     */
    private void insertGuiElement(@NotNull final GUIElement guiElement) throws JXCSkinException {
        definedGUIElements.insert(guiElement.getName(), guiElement);
        skin.insertGuiElement(guiElement);
    }

    /**
     * Parses and builds command arguments.
     * @param listName the command list name to add to
     * @param args the list of arguments
     * @param argc the start index for parsing
     * @param element the target element
     * @param command the command to parse the arguments of
     * @param guiStateManager the gui state manager instance
     * @param commands the commands instance for executing commands
     * @param lnr the source to read more parameters from
     * @param commandQueue the command queue for executing commands
     * @param crossfireServerConnection the server connection to use
     * @param commandCallback the command callback to use
     * @param macros the macros instance to use
     * @throws IOException if a syntax error occurs
     * @throws JXCSkinException if an element cannot be found
     */
    private void addCommand(@NotNull final String listName, @NotNull final String[] args, final int argc, @Nullable final GUIElement element, @NotNull final String command, @NotNull final GuiStateManager guiStateManager, @NotNull final Commands commands, @NotNull final LineNumberReader lnr, @NotNull final CommandQueue commandQueue, @NotNull final CrossfireServerConnection crossfireServerConnection, @NotNull final CommandCallback commandCallback, @NotNull final Macros macros) throws IOException, JXCSkinException {
        final CommandList commandList = skin.getCommandList(listName);
        commandList.add(commandParser.parseCommandArgs(args, argc, element, command, guiStateManager, commands, lnr, commandQueue, crossfireServerConnection, commandCallback, macros));
    }

    /**
     * Creates a new {@link GaugeUpdater} instance from a string
     * representation.
     * @param name the gauge updater value to parse
     * @return the gauge updater
     * @throws IOException if the gauge updater value does not exist
     */
    @NotNull
    private GaugeUpdater newGaugeUpdater(@NotNull final String name) throws IOException {
        final GaugeUpdater gaugeUpdater = gaugeUpdaterParser.parseGaugeUpdater(name, experienceTable);
        skin.addGaugeUpdater(gaugeUpdater);
        return gaugeUpdater;
    }

}
