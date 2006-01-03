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
package com.realtime.crossfire.jxclient;
import  com.realtime.crossfire.jxclient.*;

import java.net.*;
import java.util.*;
import java.io.*;
import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.image.*;
//import com.sixlegs.png.*;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class ItemsList
{
    private static java.util.List<CfItem>         items   = new ArrayList<CfItem>();
    private static Hashtable<String,CfItem>       myitems = new Hashtable<String,CfItem>();
    private static CfPlayer                       myplayer= null;
    private static java.util.List<Spell>          myspells= new ArrayList<Spell>();

    private static java.util.List<CrossfireDelitemListener> mylisteners_delitem =
            new ArrayList<CrossfireDelitemListener>();
    private static java.util.List<CrossfireItem1Listener> mylisteners_item1 =
            new ArrayList<CrossfireItem1Listener>();
    private static java.util.List<CrossfireItem2Listener> mylisteners_item2 =
            new ArrayList<CrossfireItem2Listener>();
    private static java.util.List<CrossfireUpditemListener> mylisteners_upditem =
            new ArrayList<CrossfireUpditemListener>();
    private static java.util.List<CrossfireDelinvListener> mylisteners_delinv =
            new ArrayList<CrossfireDelinvListener>();

    static
    {
        try
        {
            initSpells();
        }
        catch (Exception e)
        {
            e.printStackTrace();
            System.exit(0);
        }
    }
    public static java.util.List getCrossfireDelitemListeners()
    {
        return mylisteners_delitem;
    }
    public static java.util.List getCrossfireItem1Listeners()
    {
        return mylisteners_item1;
    }
    public static java.util.List getCrossfireItem2Listeners()
    {
        return mylisteners_item2;
    }
    public static java.util.List getCrossfireUpditemListeners()
    {
        return mylisteners_upditem;
    }
    public static java.util.List getCrossfireDelinvListeners()
    {
        return mylisteners_delinv;
    }
    public static java.util.List getItems(int location)
    {
        java.util.List<CfItem> l = new ArrayList<CfItem>();
        Iterator it = items.iterator();
        while (it.hasNext())
        {
            CfItem item = (CfItem)it.next();
            if (item.getLocation() == location)
                l.add(item);
        }
        return l;
    }
    public static CfPlayer getPlayer()
    {
        return myplayer;
    }
    public static void setPlayer(CfPlayer pl)
    {
        myplayer = pl;
    }
    public static void removeItem(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        int pos = 0;
        CfItem kickme = null;

        while (pos<len)
        {
            int tokill   = dis.readInt();
            pos+=4;
            myitems.remove(String.valueOf(tokill));
            Iterator it = items.iterator();
            while (it.hasNext())
            {
                CfItem item = (CfItem)it.next();
                if (item.getTag() == tokill)
                {
                    kickme = item;
                    break;
                }
            }
            if (kickme != null)
            {
                myitems.remove(kickme);
                items.remove(kickme);
            }
        }
    }
    public static void cleanInventory(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        byte buf[] = new byte[len];

        dis.readFully(buf);
        int tokill = Integer.parseInt(new String(buf));
        java.util.List killed = new ArrayList();

        CfItem item;
        Iterator it = items.iterator();
        while (it.hasNext())
        {
            item = (CfItem)it.next();
            if (item.getLocation() == tokill)
                killed.add(item);
        }
        it = killed.iterator();
        while (it.hasNext())
        {
            Object o = it.next();
            myitems.remove(o);
            items.remove(o);
        }
    }
    public static void addItems2(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        int pos = 0;
        int location   = dis.readInt();
        pos+=4;
        while (pos<len)
        {
            int tag        = dis.readInt();
            int flags      = dis.readInt();
            int weight     = dis.readInt();
            int faceid     = dis.readInt();
            int namelength = dis.readUnsignedByte();
            pos+=17;
            byte buf[] = new byte[namelength];
            dis.readFully(buf);
            String name = new String(buf);
            String[] names = name.split("\0",2);
            pos+=namelength;
            int anim = dis.readUnsignedShort();
            int animspeed = dis.readUnsignedByte();
            int nrof = dis.readInt();
            int type = dis.readUnsignedShort();
            pos+=9;
            Faces.ensureFaceExists(faceid);
            CfItem item = new CfItem(location, tag, flags, weight, Faces.getFace(faceid),
                                     names[0],names[1], nrof, type);
            if (myitems.containsKey(String.valueOf(tag)))
            {
                items.remove(myitems.get(String.valueOf(tag)));
                myitems.remove(String.valueOf(tag));
            }
            myitems.put(String.valueOf(tag), item);
            items.add(item);
            CrossfireCommandItem2Event evt = new CrossfireCommandItem2Event(new Object(),item);
            Iterator it = mylisteners_item2.iterator();
            while (it.hasNext())
            {
                ((CrossfireItem2Listener)it.next()).CommandItem2Received(evt);
            }
        }
    }
    public static void addItems(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        int pos = 0;
        int location   = dis.readInt();
        pos+=4;
        while (pos<len)
        {
            int tag        = dis.readInt();
            int flags      = dis.readInt();
            int weight     = dis.readInt();
            int faceid     = dis.readInt();
            int namelength = dis.readUnsignedByte();
            pos+=17;
            byte buf[] = new byte[namelength];
            dis.readFully(buf);
            String name = new String(buf);
            String[] names = name.split("\0",2);
            pos+=namelength;
            int anim = dis.readUnsignedShort();
            int animspeed = dis.readUnsignedByte();
            int nrof = dis.readInt();
            pos+=7;
            Faces.ensureFaceExists(faceid);
            CfItem item = new CfItem(location, tag, flags, weight, Faces.getFace(faceid),
                                     names[0],names[1], nrof);
            if (myitems.containsKey(String.valueOf(tag)))
            {
                items.remove(myitems.get(String.valueOf(tag)));
                myitems.remove(String.valueOf(tag));
            }
            myitems.put(String.valueOf(tag), item);
            items.add(item);
            CrossfireCommandItem1Event evt = new CrossfireCommandItem1Event(new Object(),item);
            Iterator it = mylisteners_item1.iterator();
            while (it.hasNext())
            {
                ((CrossfireItem1Listener)it.next()).CommandItem1Received(evt);
            }
        }
    }
    public static void updateItem(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        int pos = 0;

        int flags = dis.readUnsignedByte();
        int tag = dis.readInt();
        CfItem item = myitems.get(String.valueOf(tag));
        if ((myplayer != null)&&(myplayer.getTag()==tag))
            item = myplayer;

        if ((flags & CfItem.UPD_FLAGS)!=0)
        {
            int obfl = dis.readInt();
            item.setFlags(obfl);
            pos+=4;
        }
        if ((flags & CfItem.UPD_WEIGHT)!=0)
        {
            int obw = dis.readInt();
            item.setWeight(obw);
            pos+=4;
        }
        if ((flags & CfItem.UPD_FACE)!=0)
        {
            int obface = dis.readInt();
            item.setFace(Faces.getFace(obface));
            pos+=4;
        }
        if ((flags & CfItem.UPD_NAME)!=0)
        {
            int namelength = dis.readUnsignedByte();
            byte buf[] = new byte[namelength];
            dis.readFully(buf);
            String name = new String(buf);
            String[] names = name.split("\0",2);
            item.setName(names[0],names[1]);
            pos += namelength;
        }
        if ((flags & CfItem.UPD_ANIM)!=0)
        {
            int anim = dis.readShort();
            //Unused for now
            pos+=2;
        }
        if ((flags & CfItem.UPD_ANIMSPEED)!=0)
        {
            int animspeed = dis.readByte();
            //Unused for now
            pos+=1;
        }
        if ((flags & CfItem.UPD_NROF)!=0)
        {
            int nrof = dis.readInt();
            item.setNrOf(nrof);
            pos+=4;
        }
        CrossfireCommandUpditemEvent evt = new CrossfireCommandUpditemEvent(new Object(),item);
        Iterator it = mylisteners_upditem.iterator();
        while (it.hasNext())
        {
            ((CrossfireUpditemListener)it.next()).CommandUpditemReceived(evt);
        }
    }
    public static void createPlayer(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        int pos = 0;
        pos+=4;
        int tag        = dis.readInt();
        int weight     = dis.readInt();
        int faceid     = dis.readInt();
        int namelength = dis.readUnsignedByte();
        pos+=17;
        byte buf[] = new byte[namelength];
        dis.readFully(buf);
        String name = new String(buf);
        pos+=namelength;
        Faces.ensureFaceExists(faceid);
        myplayer = new CfPlayer(tag, weight, Faces.getFace(faceid),name);
    }
    public static java.util.List<Spell> getSpellList()
    {
        return myspells;
    }
    private static void initSpells() throws IOException
    {
        myspells.add(new Spell("default.theme/pictures/spells/si_001.png",   "Small Lightning",
                              "small lightning"));
        myspells.add(new Spell("default.theme/pictures/spells/si_002.png",   "Large Lightning",
                              "large lightning"));
        myspells.add(new Spell("default.theme/pictures/spells/si_003.png",   "Create Bomb",
                              "create bomb"));
        myspells.add(new Spell("default.theme/pictures/spells/si_004.png",   "Comet",
                              "comet"));
        myspells.add(new Spell("default.theme/pictures/spells/si_005.png",   "Small Snowstorm",
                              "small snowstorm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_006.png",   "Medium Snowstorm",
                              "medium snowstorm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_007.png",   "Large Snowstorm",
                              "large snowstorm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_008.png",   "Small Fireball",
                              "small fireball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_009.png",   "Medium Fireball",
                              "medium fireball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_010.png",  "Large Fireball",
                              "large fireball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_011.png",  "Create Earth Wall",
                              "create earth wall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_013.png",  "Call Holy Servant",
                              "call holy servant"));
        myspells.add(new Spell("default.theme/pictures/spells/si_014.png",  "Summon Avatar",
                              "summon avatar"));
        myspells.add(new Spell("default.theme/pictures/spells/si_015.png",  "Cause Rabies",
                              "cause rabies"));
        myspells.add(new Spell("default.theme/pictures/spells/si_016.png",  "Create Food",
                              "create food"));
        myspells.add(new Spell("default.theme/pictures/spells/si_017.png",  "Magic Bullet",
                              "magic bullet"));
        myspells.add(new Spell("default.theme/pictures/spells/si_018.png",  "Burning Hands",
                              "burning hands"));
        myspells.add(new Spell("default.theme/pictures/spells/si_019.png",  "Dragon Breath",
                              "dragon breath"));
        myspells.add(new Spell("default.theme/pictures/spells/si_020.png",  "Firebolt",
                              "firebolt"));
        myspells.add(new Spell("default.theme/pictures/spells/si_021.png",  "Rune of Fire",
                              "rune of fire"));
        myspells.add(new Spell("default.theme/pictures/spells/si_022.png",  "Steambolt",
                              "steambolt"));
        myspells.add(new Spell("default.theme/pictures/spells/si_023.png",  "Hellfire",
                              "hellfire"));
        myspells.add(new Spell("default.theme/pictures/spells/si_024.png",  "Icestorm",
                              "icestorm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_025.png",  "Large Icestorm",
                              "large icestorm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_026.png",  "Frostbolt",
                              "frostbolt"));
        myspells.add(new Spell("default.theme/pictures/spells/si_027.png",  "Rune of Frost",
                              "rune of frost"));
        myspells.add(new Spell("default.theme/pictures/spells/si_028.png",  "Ball Lightning",
                              "ball lightning"));
        myspells.add(new Spell("default.theme/pictures/spells/si_029.png",  "Rune of Shocking",
                              "rune of shocking"));
        myspells.add(new Spell("default.theme/pictures/spells/si_030.png",  "Forked Lightning",
                              "forked lightning"));
        myspells.add(new Spell("default.theme/pictures/spells/si_031.png",  "Poison Cloud",
                              "poison cloud"));
        myspells.add(new Spell("default.theme/pictures/spells/si_032.png",  "Large Bullet",
                              "large bullet"));
        myspells.add(new Spell("default.theme/pictures/spells/si_033.png",  "Bullet Swarm",
                              "bullet swarm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_034.png",  "Bullet Storm",
                              "bullet storm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_035.png",  "Small Speedball",
                              "small speedball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_036.png",  "Large Speedball",
                              "large speedball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_037.png",  "Magic Missile",
                              "magic missile"));
        myspells.add(new Spell("default.theme/pictures/spells/si_038.png",  "Summon Golem",
                              "summon golem"));
        myspells.add(new Spell("default.theme/pictures/spells/si_039.png",  "Summon Fire Elemental"
                              ,"summon fire elemental"));
        myspells.add(new Spell("default.theme/pictures/spells/si_040.png",
                     "Summon Earth Elemental", "summon earth elemental"));
        myspells.add(new Spell("default.theme/pictures/spells/si_041.png",
                     "Summon Water Elemental", "summon water elemental"));
        myspells.add(new Spell("default.theme/pictures/spells/si_042.png",
                     "Summon Air Elemental", "summon air elemental"));
        myspells.add(new Spell("default.theme/pictures/spells/si_043.png",  "Summon Pet Monster",
                              "summon pet monster"));
        myspells.add(new Spell("default.theme/pictures/spells/si_044.png",  "Mystic Fist",
                              "mystic fist"));
        myspells.add(new Spell("default.theme/pictures/spells/si_045.png",
                     "Summon Evil Monster", "summon evil monster"));
        myspells.add(new Spell("default.theme/pictures/spells/si_046.png",
                     "Summon Cult Monsters", "summon cult monsters"));
        myspells.add(new Spell("default.theme/pictures/spells/si_047.png",  "Insect Plague",
                              "insect plague"));
        myspells.add(new Spell("default.theme/pictures/spells/si_048.png",  "Dancing Sword",
                              "dancing sword"));
        myspells.add(new Spell("default.theme/pictures/spells/si_049.png",  "Build Director",
                              "build director"));
        myspells.add(new Spell("default.theme/pictures/spells/si_050.png",  "Create Fire Wall",
                              "create fire wall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_051.png",  "Create Frost Wall",
                              "create frost wall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_052.png",  "Build Bullet Wall",
                              "build bullet wall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_053.png",
                     "Build Lightning Wall", "build lightning wall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_054.png",
                     "Build Fireball Wall", "build fireball wall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_055.png",
                     "Create Pool of Chaos", "create pool of chaos"));
        myspells.add(new Spell("default.theme/pictures/spells/si_056.png",  "Magic Rune",
                              "magic rune"));
        myspells.add(new Spell("default.theme/pictures/spells/si_057.png",  "Create Missile",
                              "create missile"));
        myspells.add(new Spell("default.theme/pictures/spells/si_058.png",  "Summon Fog",
                              "summon fog"));
        myspells.add(new Spell("default.theme/pictures/spells/si_059.png",  "Wall of Thorns",
                              "wall of thorns"));
        myspells.add(new Spell("default.theme/pictures/spells/si_060.png",  "Staff to Snake",
                              "staff to snake"));
        myspells.add(new Spell("default.theme/pictures/spells/si_061.png",  "Spiderweb",
                              "spiderweb"));
        myspells.add(new Spell("default.theme/pictures/spells/si_062.png",  "Armour",
                              "armour"));
        myspells.add(new Spell("default.theme/pictures/spells/si_063.png",  "Strength",
                              "strength"));
        myspells.add(new Spell("default.theme/pictures/spells/si_064.png",  "Dexterity",
                              "dexterity"));
        myspells.add(new Spell("default.theme/pictures/spells/si_065.png",  "Constitution",
                              "constitution"));
        myspells.add(new Spell("default.theme/pictures/spells/si_066.png",  "Charisma",
                              "charisma"));
        myspells.add(new Spell("default.theme/pictures/spells/si_067.png",  "Heroism",
                              "heroism"));
        myspells.add(new Spell("default.theme/pictures/spells/si_068.png",  "Haste",
                              "haste"));
        myspells.add(new Spell("default.theme/pictures/spells/si_069.png",  "Ironwood Skin",
                              "ironwood skin"));
        myspells.add(new Spell("default.theme/pictures/spells/si_070.png",  "Wrathful Eye",
                              "wrathful eye"));
        myspells.add(new Spell("default.theme/pictures/spells/si_071.png",  "Minor Healing",
                              "minor healing"));
        myspells.add(new Spell("default.theme/pictures/spells/si_072.png",  "Medium Healing",
                              "medium healing"));
        myspells.add(new Spell("default.theme/pictures/spells/si_073.png",  "Major Healing",
                              "major healing"));
        myspells.add(new Spell("default.theme/pictures/spells/si_074.png",  "Heal",
                              "heal"));
        myspells.add(new Spell("default.theme/pictures/spells/si_075.png",  "Restoration",
                              "restoration"));
        myspells.add(new Spell("default.theme/pictures/spells/si_076.png",
                     "Regenerate Spellpoints", "regenerate spellpoints"));
        myspells.add(new Spell("default.theme/pictures/spells/si_077.png",  "Counterwall",
                              "counterwall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_078.png",  "Cure Poison",
                              "cure poison"));
        myspells.add(new Spell("default.theme/pictures/spells/si_079.png",  "Cure Confusion",
                              "cure confusion"));
        myspells.add(new Spell("default.theme/pictures/spells/si_080.png",  "Cure Blindness",
                              "cure blindness"));
        myspells.add(new Spell("default.theme/pictures/spells/si_081.png",  "Cure Disease",
                              "cure disease"));
        myspells.add(new Spell("default.theme/pictures/spells/si_082.png",  "Remove Curse",
                              "remove curse"));
        myspells.add(new Spell("default.theme/pictures/spells/si_083.png",  "Remove Damnation",
                              "remove damnation"));
        myspells.add(new Spell("default.theme/pictures/spells/si_084.png",  "Raise Dead",
                              "raise dead"));
        myspells.add(new Spell("default.theme/pictures/spells/si_085.png",  "Resurrection",
                              "resurrection"));
        myspells.add(new Spell("default.theme/pictures/spells/si_086.png",  "Reincarnation",
                              "reincarnation"));
        myspells.add(new Spell("default.theme/pictures/spells/si_087.png",  "Cancellation",
                              "cancellation"));
        myspells.add(new Spell("default.theme/pictures/spells/si_088.png",  "Counterspell",
                              "counterspell"));
        myspells.add(new Spell("default.theme/pictures/spells/si_089.png",  "Antimagic Rune",
                              "antimagic rune"));
        myspells.add(new Spell("default.theme/pictures/spells/si_090.png",  "Disarm",
                              "disarm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_091.png",  "Bless",
                              "bless"));
        myspells.add(new Spell("default.theme/pictures/spells/si_092.png",  "Curse",
                              "curse"));
        myspells.add(new Spell("default.theme/pictures/spells/si_093.png",  "Regeneration",
                              "regeneration"));
        myspells.add(new Spell("default.theme/pictures/spells/si_094.png",  "Holy Possession",
                              "holy possession"));
        myspells.add(new Spell("default.theme/pictures/spells/si_095.png",  "Consecrate",
                              "consecrate"));
        myspells.add(new Spell("default.theme/pictures/spells/si_096.png",  "Magic Mapping",
                              "magic mapping"));
        myspells.add(new Spell("default.theme/pictures/spells/si_097.png",  "Probe",
                              "probe"));
        myspells.add(new Spell("default.theme/pictures/spells/si_098.png",  "Identify",
                              "identify"));
        myspells.add(new Spell("default.theme/pictures/spells/si_099.png",  "Detect Magic",
                              "detect magic"));
        myspells.add(new Spell("default.theme/pictures/spells/si_100.png", "Detect Monster",
                              "detect monster"));
        myspells.add(new Spell("default.theme/pictures/spells/si_101.png", "Xray",
                              "xray"));
        myspells.add(new Spell("default.theme/pictures/spells/si_102.png", "Dark Vision",
                              "dark vision"));
        myspells.add(new Spell("default.theme/pictures/spells/si_103.png", "Perceive Self",
                              "perceive self"));
        myspells.add(new Spell("default.theme/pictures/spells/si_104.png", "Detect Evil",
                              "detect evil"));
        myspells.add(new Spell("default.theme/pictures/spells/si_105.png", "Detect Curse",
                              "detect curse"));
        myspells.add(new Spell("default.theme/pictures/spells/si_106.png", "Show Invisible",
                              "show invisible"));
        myspells.add(new Spell("default.theme/pictures/spells/si_107.png", "Animate Weapon",
                              "animate weapon"));
        myspells.add(new Spell("default.theme/pictures/spells/si_108.png", "Fear",
                              "fear"));
        myspells.add(new Spell("default.theme/pictures/spells/si_109.png", "Confusion",
                              "confusion"));
        myspells.add(new Spell("default.theme/pictures/spells/si_110.png", "Mass Confusion",
                              "mass confusion"));
        myspells.add(new Spell("default.theme/pictures/spells/si_111.png", "Charm Monsters",
                              "charm monsters"));
        myspells.add(new Spell("default.theme/pictures/spells/si_112.png", "Dimension Door",
                              "dimension door"));
        myspells.add(new Spell("default.theme/pictures/spells/si_113.png", "Faery Fire",
                              "faery fire"));
        myspells.add(new Spell("default.theme/pictures/spells/si_114.png", "Pacify",
                              "pacify"));
        myspells.add(new Spell("default.theme/pictures/spells/si_115.png", "Command Undead",
                              "command undead"));
        myspells.add(new Spell("default.theme/pictures/spells/si_116.png", "Conflict",
                              "conflict"));
        myspells.add(new Spell("default.theme/pictures/spells/si_117.png", "Word of Recall",
                              "word of recall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_118.png", "Light",
                              "light"));
        myspells.add(new Spell("default.theme/pictures/spells/si_119.png", "Darkness",
                              "darkness"));
        myspells.add(new Spell("default.theme/pictures/spells/si_120.png", "Nightfall",
                              "nightfall"));
        myspells.add(new Spell("default.theme/pictures/spells/si_121.png", "Daylight",
                              "daylight"));
        myspells.add(new Spell("default.theme/pictures/spells/si_122.png", "Sunspear",
                              "sunspear"));
        myspells.add(new Spell("default.theme/pictures/spells/si_123.png", "Rune of Blasting",
                              "rune of blasting"));
        myspells.add(new Spell("default.theme/pictures/spells/si_124.png", "Rune of Death",
                              "rune of death"));
        myspells.add(new Spell("default.theme/pictures/spells/si_125.png", "Flaming Aura",
                              "flaming aura"));
        myspells.add(new Spell("default.theme/pictures/spells/si_126.png", "Vitriol",
                              "vitriol"));
        myspells.add(new Spell("default.theme/pictures/spells/si_127.png", "Vitriol Splash",
                              "vitriol splash"));
        myspells.add(new Spell("default.theme/pictures/spells/si_128.png", "Face of Death",
                              "face of death"));
        myspells.add(new Spell("default.theme/pictures/spells/si_129.png", "Finger of Death",
                              "finger of death"));
        myspells.add(new Spell("default.theme/pictures/spells/si_130.png", "Turn Undead",
                              "turn undead"));
        myspells.add(new Spell("default.theme/pictures/spells/si_131.png", "Holy Word",
                              "holy word"));
        myspells.add(new Spell("default.theme/pictures/spells/si_132.png", "Banishment",
                              "banishment"));
        myspells.add(new Spell("default.theme/pictures/spells/si_133.png", "Holy Orb",
                              "holy orb"));
        myspells.add(new Spell("default.theme/pictures/spells/si_134.png", "Holy Wrath",
                              "holy wrath"));
        myspells.add(new Spell("default.theme/pictures/spells/si_135.png", "Wonder",
                              "wonder"));
        myspells.add(new Spell("default.theme/pictures/spells/si_136.png", "Polymorph",
                              "polymorph"));
        myspells.add(new Spell("default.theme/pictures/spells/si_137.png", "Alchemy",
                              "alchemy"));
        myspells.add(new Spell("default.theme/pictures/spells/si_138.png", "Charging",
                              "charging"));
        myspells.add(new Spell("default.theme/pictures/spells/si_139.png", "Rune of Magic Drain",
                              "rune of magic drain"));
        myspells.add(new Spell("default.theme/pictures/spells/si_140.png",
                     "Rune of Transferrence", "rune of transferrence"));
        myspells.add(new Spell("default.theme/pictures/spells/si_141.png", "Magic Drain",
                              "magic drain"));
        myspells.add(new Spell("default.theme/pictures/spells/si_142.png", "Transferrence",
                              "transferrence"));
        myspells.add(new Spell("default.theme/pictures/spells/si_143.png", "Mana Blast",
                              "mana blast"));
        myspells.add(new Spell("default.theme/pictures/spells/si_144.png", "Small Manaball",
                              "small manaball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_145.png", "Medium Manaball",
                              "medium manaball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_146.png", "Large Manaball",
                              "large manaball"));
        myspells.add(new Spell("default.theme/pictures/spells/si_147.png", "Mana Bolt",
                              "mana bolt"));
        myspells.add(new Spell("default.theme/pictures/spells/si_148.png", "Cause Light Wounds",
                              "cause light wounds"));
        myspells.add(new Spell("default.theme/pictures/spells/si_149.png", "Cause Medium Wounds",
                              "cause medium wounds"));
        myspells.add(new Spell("default.theme/pictures/spells/si_150.png",
                     "Cause Serious Wounds", "cause serious wounds"));
        myspells.add(new Spell("default.theme/pictures/spells/si_151.png", "Cause Many Wounds",
                              "cause many wounds"));
        myspells.add(new Spell("default.theme/pictures/spells/si_152.png",
                     "Cause Critical Wounds", "cause critical wounds"));
        myspells.add(new Spell("default.theme/pictures/spells/si_153.png", "Cause Cold",
                              "cause cold"));
        myspells.add(new Spell("default.theme/pictures/spells/si_154.png", "Cause Leprosy",
                              "cause leprosy"));
        myspells.add(new Spell("default.theme/pictures/spells/si_155.png", "Cause Smallpox",
                              "cause smallpox"));
        myspells.add(new Spell("default.theme/pictures/spells/si_156.png", "Cause White Death",
                              "cause white death"));
        myspells.add(new Spell("default.theme/pictures/spells/si_157.png", "Cause Anthrax",
                              "cause anthrax"));
        myspells.add(new Spell("default.theme/pictures/spells/si_158.png", "Cause Typhoid",
                              "cause typhoid"));
        myspells.add(new Spell("default.theme/pictures/spells/si_159.png", "Cause Flu",
                              "cause flu"));
        myspells.add(new Spell("default.theme/pictures/spells/si_160.png", "Cause Red Death",
                              "cause raise death"));
        myspells.add(new Spell("default.theme/pictures/spells/si_161.png", "Poison Fog",
                              "poison fog"));
        myspells.add(new Spell("default.theme/pictures/spells/si_162.png", "Rage",
                              "rage"));
        myspells.add(new Spell("default.theme/pictures/spells/si_163.png", "Divine Shock",
                              "divine shock"));
        myspells.add(new Spell("default.theme/pictures/spells/si_164.png", "Retributive Strike",
                              "retributive strike"));
        myspells.add(new Spell("default.theme/pictures/spells/si_165.png",
                     "Protection from Cold", "protection from cold"));
        myspells.add(new Spell("default.theme/pictures/spells/si_166.png",
                     "Protection from Electricity", "protection from electricity"));
        myspells.add(new Spell("default.theme/pictures/spells/si_167.png",
                     "Protection from Fire", "protection from fire"));
        myspells.add(new Spell("default.theme/pictures/spells/si_168.png",
                     "Protection from Poison", "protection from poison"));
        myspells.add(new Spell("default.theme/pictures/spells/si_169.png",
                     "Protection from Slow", "protection from slow"));
        myspells.add(new Spell("default.theme/pictures/spells/si_170.png",
                     "Protection from Paralysis", "protection from paralysis"));
        myspells.add(new Spell("default.theme/pictures/spells/si_171.png",
                     "Protection from Draining", "protection from draining"));
        myspells.add(new Spell("default.theme/pictures/spells/si_172.png",
                     "Protection from Magic", "protection from magic"));
        myspells.add(new Spell("default.theme/pictures/spells/si_173.png",
                     "Protection from Attack", "protection from attack"));
        myspells.add(new Spell("default.theme/pictures/spells/si_174.png",
                     "Protection from Confusion", "protection from confusion"));
        myspells.add(new Spell("default.theme/pictures/spells/si_175.png",
                     "Protection from Cancellation", "protection from cancellation"));
        myspells.add(new Spell("default.theme/pictures/spells/si_176.png",
                     "Protection from Depletion", "protection from depletion"));
        myspells.add(new Spell("default.theme/pictures/spells/si_177.png", "Defense",
                              "defense"));
        myspells.add(new Spell("default.theme/pictures/spells/si_178.png", "Sanctuary",
                              "sanctuary"));
        myspells.add(new Spell("default.theme/pictures/spells/si_179.png", "Peace",
                              "peace"));
        myspells.add(new Spell("default.theme/pictures/spells/si_180.png", "Paralyze",
                              "paralyze"));
        myspells.add(new Spell("default.theme/pictures/spells/si_181.png", "Destruction",
                              "destruction"));
        myspells.add(new Spell("default.theme/pictures/spells/si_182.png", "Invisible",
                              "invisible"));
        myspells.add(new Spell("default.theme/pictures/spells/si_183.png",
                     "Improved Invisibility", "improved invisibility"));
        myspells.add(new Spell("default.theme/pictures/spells/si_184.png", "Earth to Dust",
                              "earth to dust"));
        myspells.add(new Spell("default.theme/pictures/spells/si_185.png", "Levitate",
                              "levitate"));
        myspells.add(new Spell("default.theme/pictures/spells/si_186.png", "Slow",
                              "slow"));
        myspells.add(new Spell("default.theme/pictures/spells/si_187.png", "Aggravation",
                              "aggravation"));
        myspells.add(new Spell("default.theme/pictures/spells/si_188.png", "Color Spray",
                              "color spray"));
        myspells.add(new Spell("default.theme/pictures/spells/si_189.png", "Shockwave",
                              "shockwave"));
        myspells.add(new Spell("default.theme/pictures/spells/si_190.png", "Marking Rune",
                              "marking rune"));
        myspells.add(new Spell("default.theme/pictures/spells/si_191.png", "Invisible to Undead",
                              "invisible to undead"));
        myspells.add(new Spell("default.theme/pictures/spells/si_192.png", "Cause Black Death",
                              "cause black death"));
        myspells.add(new Spell("default.theme/pictures/spells/si_193.png", "Windstorm",
                              "windstorm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_194.png", "Meteor Swarm",
                              "meteor swarm"));
        myspells.add(new Spell("default.theme/pictures/spells/si_195.png", "Town Portal",
                              "town portal"));
        myspells.add(new Spell("default.theme/pictures/spells/si_196.png", "Missile Swarm",
                              "missile swarm"));
    }
}
