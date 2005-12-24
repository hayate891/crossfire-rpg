package com.realtime.crossfire.jxclient;
import com.realtime.crossfire.jxclient.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.image.*;
import java.io.*;

public class Map
{
    private static Faces myfaces;
    private static java.util.List<CrossfireMap1Listener> mylisteners_map1 =
            new ArrayList<CrossfireMap1Listener>();
    private static java.util.List<CrossfireNewmapListener> mylisteners_newmap =
            new ArrayList<CrossfireNewmapListener>();
    private static java.util.List<CrossfireMapscrollListener> mylisteners_mapscroll =
            new ArrayList<CrossfireMapscrollListener>();
    private static java.util.List<CrossfireMagicmapListener> mylisteners_magicmap =
            new ArrayList<CrossfireMagicmapListener>();

    private static MapSquare[][] map = new MapSquare[ServerConnection.MAP_WIDTH+20]
            [ServerConnection.MAP_HEIGHT+20];

    public static java.util.List getCrossfireMap1Listeners()
    {
        return mylisteners_map1;
    }
    public static java.util.List getCrossfireNewmapListeners()
    {
        return mylisteners_newmap;
    }
    public static java.util.List getCrossfireMapscrollListeners()
    {
        return mylisteners_mapscroll;
    }
    public static java.util.List getCrossfireMagicmapListeners()
    {
        return mylisteners_magicmap;
    }

    static
    {
        for (int x=0;x<ServerConnection.MAP_WIDTH+20;x++)
        {
            for (int y=0;y<ServerConnection.MAP_HEIGHT+20;y++)
            {
                map[x][y] = new MapSquare(x,y);
            }
        }
    }
    public static void magicmap(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        byte buf[] = new byte[len];

        System.out.println("**************** MAGIC MAPPING ********************");
        dis.readFully(buf);

        String str = new String(buf);
        String packs[] = str.split(" ",5);
        CrossfireCommandMagicmapEvent evt = new
                CrossfireCommandMagicmapEvent(new Object(),
                                              Integer.parseInt(packs[0]),
                                              Integer.parseInt(packs[1]),
                                              Integer.parseInt(packs[2]),
                                              Integer.parseInt(packs[3]),
                                              packs[4].getBytes());
        Iterator it = mylisteners_magicmap.iterator();
        while (it.hasNext())
        {
            ((CrossfireMagicmapListener)it.next()).CommandMagicmapReceived(evt);
        }
    }
    public static void newMap(DataInputStream dis) throws IOException
    {
        long stime = System.nanoTime();

        for (int x=0;x<ServerConnection.MAP_WIDTH+20;x++)
        {
            for (int y=0;y<ServerConnection.MAP_HEIGHT+20;y++)
            {
                map[x][y].clear();
            }
        }
        ServerConnection.writePacket("mapredraw");
        CrossfireCommandNewmapEvent evt = new CrossfireCommandNewmapEvent(new Object());
        Iterator it = mylisteners_newmap.iterator();
        while (it.hasNext())
        {
            ((CrossfireNewmapListener)it.next()).CommandNewmapReceived(evt);
        }
        long etime = System.nanoTime();
        System.out.println("Free Memory before Newmap GC:"+
                Runtime.getRuntime().freeMemory()/1024+" KB");
        System.gc();
        long egtime = System.nanoTime();
        System.out.println("Free Memory after Newmap GC:"+
                Runtime.getRuntime().freeMemory()/1024 + " KB");
        System.out.println("Cleaning complete, Cleaning time:"+(etime-stime)/1000000+"ms, GC:"+
                (egtime-etime)/1000000+"ms.");
    }
    public static void scroll(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        byte buf[] = new byte[len];

        dis.readFully(buf);
        String[] datas = (new String(buf)).split(" ",2);
        int dx = Integer.parseInt(datas[0]);
        int dy = Integer.parseInt(datas[1]);
        System.out.println("--------------------------------------------------");
        int mx = ServerConnection.MAP_WIDTH+20;
        int my = ServerConnection.MAP_HEIGHT+20;

        if (dx >= 0)
        {
            if (dy >= 0)
            {
                /*for(int i=dx;i<mx;i++)
                {
                    for(int j=dy;j<my;j++)
                    {
                        map[i][j].copy(map[i-dx][j-dy]);
                        map[i][j].clear();
                    }
                }*/
                for(int i=dx;i<mx;i++)
                {
                    for(int j=dy;j<my;j++)
                    {
                        map[i-dx][j-dy] = map[i][j];
                        map[i-dx][j-dy].setPos(i-dx, j-dy);
                        map[i][j] = null;
                    }
                }
                for(int i=0; i<mx; i++)
                {
                    for (int j=0; j<my; j++)
                    {
                        if (map[i][j] == null)
                            map[i][j] = new MapSquare(i,j);
                        map[i][j].dirty();
                    }
                }
            }
            else
            {
                /*for(int i=dx;i<mx;i++)
                {
                    for(int j=my+dy-1;j>=0;j--)
                    {
                        map[i][j].copy(map[i-dx][j-dy]);
                        map[i][j].clear();
                    }
                }*/
                for(int i=dx;i<mx;i++)
                {
                    for(int j=my+dy-1;j>=0;j--)
                    {
                        map[i-dx][j-dy] = map[i][j];
                        map[i-dx][j-dy].setPos(i-dx, j-dy);
                        map[i][j] = null;
                    }
                }
                for(int i=0; i<mx; i++)
                {
                    for (int j=0; j<my; j++)
                    {
                        if (map[i][j] == null)
                            map[i][j] = new MapSquare(i,j);
                        map[i][j].dirty();
                    }
                }
            }
        }
        else
        {
            if (dy >= 0)
            {
                /*for(int i=mx+dx-1;i>=0;i--)
                {
                    for(int j=dy;j<my;j++)
                    {
                        map[i][j].copy(map[i-dx][j-dy]);
                        map[i][j].clear();
                    }
                }
                */
                for(int i=mx+dx-1;i>=0;i--)
                {
                    for(int j=dy;j<my;j++)
                    {
                        map[i-dx][j-dy] = map[i][j];
                        map[i-dx][j-dy].setPos(i-dx, j-dy);
                        map[i][j] = null;
                    }
                }
                for(int i=0; i<mx; i++)
                {
                    for (int j=0; j<my; j++)
                    {
                        if (map[i][j] == null)
                            map[i][j] = new MapSquare(i,j);
                        map[i][j].dirty();
                    }
                }
            }
            else
            {
                /*for(int i=mx+dx-1;i>=0;i--)
                {
                    for(int j=my+dy-1;j>=0;j--)
                    {
                        map[i][j].copy(map[i-dx][j-dy]);
                        map[i][j].clear();
                    }
                }
                */
                for(int i=mx+dx-1;i>=0;i--)
                {
                    for(int j=my+dy-1;j>=0;j--)
                    {
                        map[i-dx][j-dy] = map[i][j];
                        map[i-dx][j-dy].setPos(i-dx, j-dy);
                        map[i][j] = null;
                    }
                }
                for(int i=0; i<mx; i++)
                {
                    for (int j=0; j<my; j++)
                    {
                        if (map[i][j] == null)
                            map[i][j] = new MapSquare(i,j);
                        map[i][j].dirty();
                    }
                }
            }
        }

        CrossfireCommandMapscrollEvent evt = new CrossfireCommandMapscrollEvent(
                new Object(), dx, dy);

        Iterator it = mylisteners_mapscroll.iterator();
        while (it.hasNext())
        {
            ((CrossfireMapscrollListener)it.next()).CommandMapscrollReceived(evt);
        }
    }
    public static void changeSquare(int x, int y, int z, int darkness, Face face)
    {
        map[x][y].setDarkness(darkness);
        map[x][y].setFace(face, z);
        map[x][y].dirty();
    }
    public static void map1(DataInputStream dis) throws IOException
    {
        int len = dis.available();
        int pos = 0;
        java.util.List<MapSquare> l = new LinkedList<MapSquare>();
        boolean bottom = true;
        int[] faces = new int[ServerConnection.NUM_LAYERS];
        while (pos<len)
        {
            int coord    = dis.readUnsignedShort();
            int x        = 10+(coord >> 10) & 0x3f;
            int y        = 10+(coord >> 4) & 0x3f;
            int isclear  = coord & 0xf;
            int isdark   = coord & 0x8;
            int mask     = coord & 0x7;
            int darkness = -1;
            pos+=2;
            /*System.out.println("------------------------------------------");
            System.out.println("Packet map received");
            System.out.println("X:"+x+" Y:"+y);
            System.out.println("Clear:"+isclear+" Dark:"+isdark);*/
            if (isclear == 0)
            {
                continue;
            }
            if (isdark != 0)
            {
                darkness = dis.readUnsignedByte();
                //System.out.println("Darkness:"+darkness+ "("+x+";"+y+")");
                pos++;
            }
            for (int layer=ServerConnection.NUM_LAYERS-1; layer>=0; layer--)
            {
                if ((mask & (1<<layer))!=0)
                {
                    faces[(ServerConnection.NUM_LAYERS-1)-layer] = dis.readUnsignedShort();
                    Faces.ensureFaceExists(faces[(ServerConnection.NUM_LAYERS-1)-layer]);
                    Face ff = Faces.getFace(faces[(ServerConnection.NUM_LAYERS-1)-layer]);
                    /*if (ff != null)
                    System.out.println("Layer face :"+ff.getName());*/
                    pos+=2;
                }
                else
                {
                    faces[(ServerConnection.NUM_LAYERS-1)-layer] = -1;
                    //System.out.println("Empty face on the square");
                }
            }
            for (int layer=0; layer<ServerConnection.NUM_LAYERS; layer++)
            {
                if(faces[layer]<0)
                {
                    continue;
                }
                Faces.ensureFaceExists(faces[layer]);
                Face f = Faces.getFace(faces[layer]);
                changeSquare(x,y,layer,darkness,f);
                l.add(map[x][y]);
            }
            CrossfireCommandMap1Event evt = new CrossfireCommandMap1Event(new Object(),l);
            Iterator it = mylisteners_map1.iterator();
            while (it.hasNext())
            {
                ((CrossfireMap1Listener)it.next()).CommandMap1Received(evt);
            }
        }
    }
    public static MapSquare[][] getMap()
    {
        return map;
    }
    public static void invalidate()
    {
        for(int y=0;y<ServerConnection.MAP_HEIGHT+20;y++)
        {
            for (int x=0;x<ServerConnection.MAP_WIDTH+20;x++)
            {
                map[x][y].dirty();
            }
        }
        CrossfireCommandNewmapEvent evt = new CrossfireCommandNewmapEvent(new Object());
        Iterator it = mylisteners_newmap.iterator();
        while (it.hasNext())
        {
            ((CrossfireNewmapListener)it.next()).CommandNewmapReceived(evt);
        }
    }
    public static void updateFace(int pixnum)
    {
        java.util.List<MapSquare> l = new LinkedList<MapSquare>();

        //System.out.println("Face update: "+pixnum);
        for(int y=0;y<ServerConnection.MAP_HEIGHT+20;y++)
        {
            for (int x=0;x<ServerConnection.MAP_WIDTH+20;x++)
            {
                for (int z=0;z<ServerConnection.NUM_LAYERS;z++)
                {
                    if (map[x][y].getFace(z)!=null)
                    if (map[x][y].getFace(z).getID()==pixnum)
                    {
                        map[x][y].dirty();
                        //l.add(new MapSquare(x,y,z,0,myfaces.getFace(pixnum)));
                    }
                }
            }
        }
        CrossfireCommandMap1Event evt = new CrossfireCommandMap1Event(new Object(),l);
        Iterator it = mylisteners_map1.iterator();
        while (it.hasNext())
        {
            ((CrossfireMap1Listener)it.next()).CommandMap1Received(evt);
        }
    }

}
