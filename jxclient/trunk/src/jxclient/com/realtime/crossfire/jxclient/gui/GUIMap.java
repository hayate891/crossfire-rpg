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
package com.realtime.crossfire.jxclient.gui;

import com.realtime.crossfire.jxclient.faces.Face;
import com.realtime.crossfire.jxclient.faces.Faces;
import com.realtime.crossfire.jxclient.JXCWindow;
import com.realtime.crossfire.jxclient.map.CfMap;
import com.realtime.crossfire.jxclient.map.CfMapSquare;
import com.realtime.crossfire.jxclient.mapupdater.CfMapUpdater;
import com.realtime.crossfire.jxclient.mapupdater.CrossfireCommandMapEvent;
import com.realtime.crossfire.jxclient.mapupdater.CrossfireCommandMapscrollEvent;
import com.realtime.crossfire.jxclient.mapupdater.CrossfireCommandNewmapEvent;
import com.realtime.crossfire.jxclient.mapupdater.CrossfireMapListener;
import com.realtime.crossfire.jxclient.mapupdater.CrossfireMapscrollListener;
import com.realtime.crossfire.jxclient.mapupdater.CrossfireNewmapListener;
import com.realtime.crossfire.jxclient.server.CrossfireServerConnection;
import java.awt.Color;
import java.awt.event.MouseEvent;
import java.awt.Graphics2D;
import java.awt.Graphics;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Transparency;
import java.io.IOException;
import javax.swing.ImageIcon;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class GUIMap extends GUIElement
{
    /**
     * The color to use for overlaying fog-of-war tiles.
     */
    public static final Color fogOfWarColor = new Color(0, 0, 0.5F, 0.5F);

    /**
     * The minimum darkness alpha value; it is used for completely black tiles.
     * The maximum is 0.0F for full bright tiles.
     */
    private static final float MAX_DARKNESS_ALPHA = 0.7F;

    private final ImageIcon myblacktile;

    private final boolean use_big_images;

    private final int mysquaresize;

    /**
     * Cache to lookup darkness overlay colors. Maps darkness value to overlay
     * color. Not yet allocated entries are set to <code>null</code>.
     */
    private static final Color[] darknessColors = new Color[256];

    /**
     * The {@link CrossfireMapListener} registered to receive map updates.
     */
    private final CrossfireMapListener crossfireMapListener = new CrossfireMapListener()
    {
        /** {@inheritDoc} */
        public void commandMapReceived(final CrossfireCommandMapEvent evt)
        {
            synchronized(mybuffer)
            {
                final Graphics2D g = mybuffer.createGraphics();
                try
                {
                    for (int y = 0; y < CrossfireServerConnection.MAP_HEIGHT; y++)
                    {
                        for (int x = 0; x < CrossfireServerConnection.MAP_WIDTH; x++)
                        {
                            redrawSquare(g, CfMapUpdater.getMap(), x, y);
                        }
                    }
                }
                finally
                {
                    g.dispose();
                }
            }
            setChanged();
        }
    };

    /**
     * The {@link CrossfireNewmapListener} registered to receive newmap
     * commands.
     */
    private final CrossfireNewmapListener crossfireNewmapListener = new CrossfireNewmapListener()
    {
        /** {@inheritDoc} */
        public void commandNewmapReceived(final CrossfireCommandNewmapEvent evt)
        {
            synchronized(mybuffer)
            {
                render();
            }
            setChanged();
        }
    };

    /**
     * The {@link CrossfireMapscrollListener} registered to receive map_scroll
     * commands.
     */
    private final CrossfireMapscrollListener crossfireMapscrollListener = new CrossfireMapscrollListener()
    {
        /** {@inheritDoc} */
        public void commandMapscrollReceived(final CrossfireCommandMapscrollEvent evt)
        {
            synchronized(mybuffer)
            {
                final int dx = -evt.getDX();
                final int dy = -evt.getDY();
                if (Math.abs(dx) >= CrossfireServerConnection.MAP_WIDTH || Math.abs(dy) >= CrossfireServerConnection.MAP_HEIGHT)
                {
                    render();
                    return;
                }

                final int x;
                final int w;
                if (dx < 0)
                {
                    x = -dx;
                    w = CrossfireServerConnection.MAP_WIDTH+dx;
                }
                else
                {
                    x = 0;
                    w = CrossfireServerConnection.MAP_WIDTH-dx;
                }
                final int y;
                final int h;
                if (dy < 0)
                {
                    y = -dy;
                    h = CrossfireServerConnection.MAP_HEIGHT+dy;
                }
                else
                {
                    y = 0;
                    h = CrossfireServerConnection.MAP_HEIGHT-dy;
                }

                final Graphics2D g = mybuffer.createGraphics();
                try
                {
                    g.copyArea(x*mysquaresize, y*mysquaresize, w*mysquaresize, h*mysquaresize, dx*mysquaresize, dy*mysquaresize);

                    for (int yy = 0; yy < y; yy++)
                    {
                        for (int xx = 0; xx < CrossfireServerConnection.MAP_WIDTH; xx++)
                        {
                            redrawSquare(g, CfMapUpdater.getMap(), xx, yy);
                        }
                    }

                    for (int yy = y+h; yy < CrossfireServerConnection.MAP_HEIGHT; yy++)
                    {
                        for (int xx = 0; xx < CrossfireServerConnection.MAP_WIDTH; xx++)
                        {
                            redrawSquare(g, CfMapUpdater.getMap(), xx, yy);
                        }
                    }

                    for (int yy = y; yy < y+h; yy++)
                    {
                        for (int xx = 0; xx < x; xx++)
                        {
                            redrawSquare(g, CfMapUpdater.getMap(), xx, yy);
                        }

                        for (int xx = x+w; xx < CrossfireServerConnection.MAP_WIDTH; xx++)
                        {
                            redrawSquare(g, CfMapUpdater.getMap(), xx, yy);
                        }
                    }
                }
                finally
                {
                    g.dispose();
                }
            }
            setChanged();
        }
    };

    /**
     * Create a new instance.
     *
     * @param jxcWindow The <code>JXCWindow</code> this element belongs to.
     *
     * @param name The name of this element.
     *
     * @param x The x-coordinate for drawing this element to screen.
     *
     * @param y The y-coordinate for drawing this element to screen.
     *
     * @param w The width for drawing this element to screen.
     *
     * @param h The height for drawing this element to screen.
     *
     * @param tileSize The size of one tile in pixels.
     *
     * @throws IOException If an I/O error occurs.
     */
    public GUIMap(final JXCWindow jxcWindow, final String name, final int tileSize, final int x, final int y, final int w, final int h) throws IOException
    {
        super(jxcWindow, name, x, y, w, h);
        if (tileSize == 32)
        {
            use_big_images = false;
        }
        else if (tileSize == 64)
        {
            use_big_images = true;
        }
        else
        {
            throw new IOException("invalid tile size "+tileSize);
        }

        mysquaresize = tileSize;
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        final GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        myblacktile = new ImageIcon(gconf.createCompatibleImage(mysquaresize, mysquaresize, Transparency.OPAQUE));
        if (w != CrossfireServerConnection.MAP_WIDTH*mysquaresize) throw new IOException("w="+w+"!="+CrossfireServerConnection.MAP_WIDTH*mysquaresize);
        if (h != CrossfireServerConnection.MAP_HEIGHT*mysquaresize) throw new IOException("h="+h+"!="+CrossfireServerConnection.MAP_HEIGHT*mysquaresize);

        createBuffer();
        CfMapUpdater.addCrossfireMapListener(crossfireMapListener);
        CfMapUpdater.addCrossfireNewmapListener(crossfireNewmapListener);
        CfMapUpdater.addCrossfireMapscrollListener(crossfireMapscrollListener);
    }

    public GUIMap(final JXCWindow jxcWindow, final String name, final int x, final int y, final int w, final int h, final boolean big)
    {
        super(jxcWindow, name, x, y, w, h);
        use_big_images = big;

        if (big)
        {
            mysquaresize = Faces.SQUARE_SIZE;
        }
        else
        {
            mysquaresize = 32;
        }
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        final GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        myblacktile = new ImageIcon(gconf.createCompatibleImage(mysquaresize, mysquaresize, Transparency.OPAQUE));
        if (w != CrossfireServerConnection.MAP_WIDTH*mysquaresize) throw new IllegalArgumentException();
        if (h != CrossfireServerConnection.MAP_HEIGHT*mysquaresize) throw new IllegalArgumentException();

        createBuffer();
    }

    /** {@inheritDoc} */
    @Override protected void render(final Graphics2D g)
    {
        super.render(g);
        for (int x = 0; x < CrossfireServerConnection.MAP_WIDTH; x++)
        {
            for (int y = 0; y < CrossfireServerConnection.MAP_HEIGHT; y++)
            {
                redrawSquare(g, CfMapUpdater.getMap(), x, y);
            }
        }
    }

    /**
     * Redraw one square completely black.
     *
     * @param g The graphics to draw into.
     *
     * @param x The x-coordinate of the square to clear.
     *
     * @param y The y-coordinate of the square to clear.
     */
    private void cleanSquare(final Graphics g, final int x, final int y)
    {
        g.drawImage(myblacktile.getImage(), x*mysquaresize, y*mysquaresize, null);
    }

    /**
     * Redraw one square if it has been changed. If it is unchanged ({@link
     * CfMapSquare#dirty} is unset), nothing is drawn.
     *
     * @param g The graphics to draw into.
     *
     * @param map The map to redraw.
     *
     * @param x The x-coordinate of the map tile to redraw.
     *
     * @param y The y-coordinate of the map tile to redraw.
     */
    private void redrawSquare(final Graphics g, final CfMap map, final int x, final int y)
    {
        if (!map.resetDirty(x, y))
        {
            return;
        }

        cleanSquare(g, x, y);
        for (int layer = 0; layer < CrossfireServerConnection.NUM_LAYERS; layer++)
        {
            redrawSquare(g, map, x, y, layer);
        }
        if (map.isFogOfWar(x, y))
        {
            g.setColor(fogOfWarColor);
            g.fillRect(x*mysquaresize, y*mysquaresize, mysquaresize, mysquaresize);
        }
        final int darkness = map.getDarkness(x, y);
        if (darkness < 255)
        {
            g.setColor(getDarknessColor(darkness));
            g.fillRect(x*mysquaresize, y*mysquaresize, mysquaresize, mysquaresize);
        }
    }

    /**
     * Redraw one layer of a square.
     *
     * @param g The graphics to draw into.
     *
     * @param map The map to redraw.
     *
     * @param x The x coordinate of the square to redraw.
     *
     * @param y The y coordinate of the square to redraw.
     *
     * @param layer The layer to redraw.
     */
    private void redrawSquare(final Graphics g, final CfMap map, final int x, final int y, final int layer)
    {
        final int px = x*mysquaresize;
        final int py = y*mysquaresize;

        final CfMapSquare headMapSquare = map.getHeadMapSquare(x, y, layer);
        if (headMapSquare != null)
        {
            final Face headFace = headMapSquare.getFace(layer);
            assert headFace != null; // getHeadMapSquare() would have been cleared in this case
            final ImageIcon img = headFace.getImageIcon(use_big_images);
            final int dx = headMapSquare.getX()-map.getMapSquare(x, y).getX();
            final int dy = headMapSquare.getY()-map.getMapSquare(x, y).getY();
            assert dx > 0 || dy > 0;
            final int sx = img.getIconWidth()-mysquaresize*(dx+1);
            final int sy = img.getIconHeight()-mysquaresize*(dy+1);
            g.drawImage(img.getImage(),
                px, py, px+mysquaresize, py+mysquaresize,
                sx, sy, sx+mysquaresize, sy+mysquaresize,
                null);
        }

        final Face f = map.getFace(x, y, layer);
        if (f != null)
        {
            final ImageIcon img = f.getImageIcon(use_big_images);
            final int sx = img.getIconWidth();
            final int sy = img.getIconHeight();
            g.drawImage(img.getImage(),
                px, py, px+mysquaresize, py+mysquaresize,
                sx-mysquaresize, sy-mysquaresize, sx, sy,
                null);
        }
    }

    /** {@inheritDoc} */
    @Override public void mouseClicked(final MouseEvent e)
    {
        super.mouseClicked(e);
        switch (e.getButton())
        {
        case MouseEvent.BUTTON1:
            final int dx = e.getX()/Faces.SQUARE_SIZE-CrossfireServerConnection.MAP_WIDTH/2;
            final int dy = e.getY()/Faces.SQUARE_SIZE-CrossfireServerConnection.MAP_HEIGHT/2;
            try
            {
                final JXCWindow jxcw = (JXCWindow)e.getSource();
                jxcw.getCrossfireServerConnection().sendLookat(dx, dy);
            }
            catch (final Exception ex)
            {
                ex.printStackTrace();
                System.exit(0);
            }
            break;

        case MouseEvent.BUTTON2:
        case MouseEvent.BUTTON3:
            break;
        }
    }

    /** {@inheritDoc} */
    @Override protected void createBuffer()
    {
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        final GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        mybuffer = gconf.createCompatibleImage(w, h, Transparency.OPAQUE);
        final Graphics2D g = mybuffer.createGraphics();
        try
        {
            g.setColor(Color.BLACK);
            g.fillRect(0, 0, w, h);
        }
        finally
        {
            g.dispose();
        }
        setChanged();
    }

    /**
     * Return an overlay color for a darkness value.
     *
     * @param darkness The darkness value between 0 and 255.
     *
     * @return The overlay color.
     */
    public static synchronized Color getDarknessColor(final int darkness)
    {
        if (darknessColors[darkness] == null)
        {
            final float alpha = MAX_DARKNESS_ALPHA*(255-darkness)/255F;
            darknessColors[darkness] = new Color(0, 0, 0, alpha);
        }

        return darknessColors[darkness];
    }
}
