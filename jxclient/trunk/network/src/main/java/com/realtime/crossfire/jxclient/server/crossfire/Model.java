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

package com.realtime.crossfire.jxclient.server.crossfire;

import com.realtime.crossfire.jxclient.character.ClassRaceInfo;
import com.realtime.crossfire.jxclient.character.StartingMap;
import com.realtime.crossfire.jxclient.faces.AskfaceFaceQueue;
import com.realtime.crossfire.jxclient.faces.FaceCache;
import com.realtime.crossfire.jxclient.faces.FacesManager;
import com.realtime.crossfire.jxclient.faces.SmoothFaces;
import com.realtime.crossfire.jxclient.guistate.GuiStateManager;
import com.realtime.crossfire.jxclient.items.ItemSet;
import com.realtime.crossfire.jxclient.items.ItemsManager;
import com.realtime.crossfire.jxclient.knowledge.KnowledgeManager;
import com.realtime.crossfire.jxclient.quests.QuestsManager;
import com.realtime.crossfire.jxclient.skills.SkillSet;
import com.realtime.crossfire.jxclient.spells.SpellsManager;
import com.realtime.crossfire.jxclient.stats.ExperienceTable;
import com.realtime.crossfire.jxclient.stats.Stats;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.jetbrains.annotations.NotNull;

/**
 * Combines all model classes that are updated.
 * @author Andreas Kirschbaum
 */
public class Model {

    @NotNull
    private final GuiStateManager guiStateManager = new GuiStateManager();

    @NotNull
    private final SkillSet skillSet = new SkillSet(guiStateManager);

    @NotNull
    private final ExperienceTable experienceTable = new ExperienceTable();

    @NotNull
    private final Stats stats = new Stats(experienceTable, skillSet, guiStateManager);

    @NotNull
    private final SpellsManager spellsManager = new SpellsManager(guiStateManager, skillSet, stats);

    @NotNull
    private final QuestsManager questsManager = new QuestsManager(guiStateManager);

    @NotNull
    private final KnowledgeManager knowledgeManager = new KnowledgeManager(guiStateManager);

    @NotNull
    private final FaceCache faceCache = new FaceCache();

    @NotNull
    private final SmoothFaces smoothFaces = new SmoothFaces();

    @NotNull
    private ItemsManager itemsManager;

    @NotNull
    private final ItemSet itemSet = new ItemSet();

    @NotNull
    private AskfaceFaceQueue askfaceFaceQueue;

    /**
     * The {@link StartingMap} entries available for character creation.
     */
    @NotNull
    private final List<StartingMap> startingMaps = new ArrayList<StartingMap>();

    /**
     * The races available for character creation.
     */
    @NotNull
    private final List<String> raceList = new ArrayList<String>();

    /**
     * The classes available for character creation.
     */
    @NotNull
    private final List<String> classList = new ArrayList<String>();

    /**
     * The defined races for character creation.
     */
    @NotNull
    private final Map<String, ClassRaceInfo> raceInfo = new HashMap<String, ClassRaceInfo>();

    /**
     * The defined classes for character creation.
     */
    @NotNull
    private final Map<String, ClassRaceInfo> classInfo = new HashMap<String, ClassRaceInfo>();

    @Deprecated
    public void setItemsManager(@NotNull final FacesManager facesManager) {
        itemsManager = new ItemsManager(facesManager, stats, skillSet, guiStateManager, itemSet);
    }

    @Deprecated
    public void setAskfaceFaceQueue(@NotNull final AskfaceFaceQueue askfaceFaceQueue) {
        this.askfaceFaceQueue = askfaceFaceQueue;
    }

    @NotNull
    public SkillSet getSkillSet() {
        return skillSet;
    }

    @NotNull
    public Stats getStats() {
        return stats;
    }

    @NotNull
    public ExperienceTable getExperienceTable() {
        return experienceTable;
    }

    @NotNull
    public GuiStateManager getGuiStateManager() {
        return guiStateManager;
    }

    @NotNull
    public SpellsManager getSpellsManager() {
        return spellsManager;
    }

    @NotNull
    public QuestsManager getQuestsManager() {
        return questsManager;
    }

    @NotNull
    public KnowledgeManager getKnowledgeManager() {
        return knowledgeManager;
    }

    @NotNull
    public FaceCache getFaceCache() {
        return faceCache;
    }

    @NotNull
    public SmoothFaces getSmoothFaces() {
        return smoothFaces;
    }

    @NotNull
    public ItemsManager getItemsManager() {
        return itemsManager;
    }

    @NotNull
    public ItemSet getItemSet() {
        return itemSet;
    }

    @NotNull
    public AskfaceFaceQueue getAskfaceFaceQueue() {
        return askfaceFaceQueue;
    }

    /**
     * Sets the {@link StartingMap} entries available for character creation.
     * @param startingMaps the starting map entries
     */
    public void setStartingMaps(@NotNull final Collection<StartingMap> startingMaps) {
        this.startingMaps.clear();
        this.startingMaps.addAll(startingMaps);
    }

    /**
     * Returns all defined {@link StartingMap} entries available for character
     * creation.
     * @return the starting map entries
     */
    @NotNull
    public List<StartingMap> getStartingMaps() {
        return Collections.unmodifiableList(startingMaps);
    }

    /**
     * Sets the races available for character creation.
     * @param raceList the races
     */
    public void setRaceList(@NotNull final String[] raceList) {
        this.raceList.clear();
        this.raceList.addAll(Arrays.asList(raceList));
    }

    /**
     * Returns all defined races available for character creation.
     * @return the races
     */
    @NotNull
    public List<String> getRaceList() {
        return Collections.unmodifiableList(raceList);
    }

    /**
     * Sets the classes available for character creation.
     * @param classList the classes
     */
    public void setClassList(@NotNull final String[] classList) {
        this.classList.clear();
        this.classList.addAll(Arrays.asList(classList));
    }

    /**
     * Returns all defined classes available for character creation.
     * @return the classes
     */
    @NotNull
    public List<String> getClassesList() {
        return Collections.unmodifiableList(classList);
    }

    /**
     * Sets or updates a {@link ClassRaceInfo}.
     * @param classRaceInfo the race info to set
     */
    public void addRaceInfo(@NotNull final ClassRaceInfo classRaceInfo) {
        this.raceInfo.put(classRaceInfo.getArchName(), classRaceInfo);
    }

    /**
     * Returns a {@link ClassRaceInfo} by race name.
     * @param race the race name
     * @return the race info or <code>null</code> if no race info is defined
     */
    @NotNull
    public ClassRaceInfo getRaceInfo(@NotNull final String race) {
        return raceInfo.get(race);
    }

    /**
     * Sets or updates a {@link ClassRaceInfo class info}.
     * @param classInfo the class info to set
     */
    public void addClassInfo(@NotNull final ClassRaceInfo classInfo) {
        this.classInfo.put(classInfo.getArchName(), classInfo);
    }

    /**
     * Returns a {@link ClassRaceInfo class info} by class name.
     * @param className the class name
     * @return the class race info or <code>null</code> if no such class info is
     *         defined
     */
    @NotNull
    public ClassRaceInfo getClassInfo(@NotNull final String className) {
        return classInfo.get(className);
    }

}
