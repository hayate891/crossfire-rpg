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

package com.realtime.crossfire.jxclient.gui;

import com.realtime.crossfire.jxclient.faces.Face;
import com.realtime.crossfire.jxclient.faces.FacesManager;
import com.realtime.crossfire.jxclient.gui.gui.GUIElement;
import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.map.CfMap;
import com.realtime.crossfire.jxclient.map.CfMapSquare;
import com.realtime.crossfire.jxclient.mapupdater.CfMapUpdater;
import com.realtime.crossfire.jxclient.mapupdater.MapListener;
import com.realtime.crossfire.jxclient.mapupdater.MapscrollListener;
import com.realtime.crossfire.jxclient.mapupdater.NewmapListener;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireMagicmapListener;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireMap2Command;
import com.realtime.crossfire.jxclient.server.crossfire.CrossfireServerConnection;
import com.realtime.crossfire.jxclient.server.crossfire.MapSizeListener;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Transparency;
import java.util.Set;
import javax.swing.ImageIcon;
import org.jetbrains.annotations.NotNull;

/**
 * Displays magic map results. Fallback for unknown tiles is the normal map
 * contents.
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public class GUIMagicMap extends GUIElement {

    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The {@link CfMapUpdater} instance to use.
     */
    @NotNull
    private final CfMapUpdater mapUpdater;

    /**
     * The {@link FacesManager} instance for looking up faces.
     */
    @NotNull
    private final FacesManager facesManager;

    /**
     * The {@link CrossfireServerConnection} to monitor.
     */
    @NotNull
    private final CrossfireServerConnection crossfireServerConnection;

    /**
     * The map width in tiles.
     */
    private int mapWidth;

    /**
     * The map height in tiles.
     */
    private int mapHeight;

    /**
     * The size of one tile.
     */
    private final int tileSize;

    /**
     * The x offset of the tile representing the player.
     */
    private final int playerX;

    /**
     * The y offset of the tile representing the player.
     */
    private final int playerY;

    /**
     * The x offset for the visible map area.
     */
    private int offsetX;

    /**
     * The y offset for the visible map area.
     */
    private int offsetY;

    /**
     * The colors for displaying magic map data.
     */
    @NotNull
    private static final Color[] tileColors = {
        Color.BLACK,
        Color.WHITE,
        Color.BLUE,
        Color.RED,
        Color.GREEN,
        Color.YELLOW,
        Color.PINK,
        Color.GRAY,
        Color.ORANGE,
        Color.CYAN,
        Color.MAGENTA,
        Color.DARK_GRAY,
        Color.DARK_GRAY,
        Color.DARK_GRAY,
        Color.DARK_GRAY,
        Color.DARK_GRAY,
    };

    static {
        assert CrossfireMagicmapListener.FACE_COLOR_MASK+1 == tileColors.length;
    }

    /**
     * The {@link CrossfireMagicmapListener} registered to receive mapgicmap
     * commands.
     */
    @NotNull
    private final CrossfireMagicmapListener crossfireMagicmapListener = new CrossfireMagicmapListener() {
        /** {@inheritDoc} */
        @Override
        public void commandMagicmapReceived(final int width, final int height, final int px, final int py, @NotNull final byte[] data, final int pos) {
            int datapos = pos;
            synchronized (bufferedImageSync) {
                final Graphics g = createBufferGraphics();
                try {
                    final int offsetX = playerX-px*tileSize;
                    final int offsetY = playerY-py*tileSize;
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            if (data[datapos] != 0) {
                                g.setColor(tileColors[data[datapos]&FACE_COLOR_MASK]);
                                final int sx = offsetX+x*tileSize;
                                final int sy = offsetY+y*tileSize;
                                g.fillRect(sx, sy, tileSize, tileSize);
                            }
                            datapos++;
                        }
                    }
                    markPlayer(g);
                } finally {
                    g.dispose();
                }
            }
            setChanged();
        }
    };

    /**
     * The {@link MapscrollListener} used to track player position changes into
     * the magic map.
     */
    @NotNull
    private final MapscrollListener mapscrollListener = new MapscrollListener() {
        /** {@inheritDoc} */
        @Override
        public void mapScrolled(final int dx, final int dy) {
            synchronized (bufferedImageSync) {
                final Graphics2D g = createBufferGraphics();
                try {
                    final CfMap map = mapUpdater.getMap();
                    final int dxPixels = dx*tileSize;
                    final int dyPixels = dy*tileSize;
                    if (Math.abs(dxPixels) >= getWidth() || Math.abs(dyPixels) >= getHeight()) {
                        redrawTiles(g, map, 0, 0, getWidth()/tileSize, getHeight()/tileSize);
                    } else {
                        g.copyArea(dxPixels <= 0 ? 0 : dxPixels, dyPixels <= 0 ? 0 : dyPixels, dxPixels == 0 ? getWidth() : getWidth()-Math.abs(dxPixels), dyPixels == 0 ? getHeight() : getHeight()-Math.abs(dyPixels), -dxPixels, -dyPixels);
                        g.setColor(Color.BLACK);
                        if (dxPixels < 0) {
                            redrawTiles(g, map, 0, 0, -dxPixels/tileSize, getHeight()/tileSize);
                        } else if (dxPixels > 0) {
                            redrawTiles(g, map, getWidth()/tileSize-dxPixels/tileSize, 0, getWidth()/tileSize, getHeight()/tileSize);
                        }
                        if (dyPixels < 0) {
                            redrawTiles(g, map, 0, 0, getWidth()/tileSize, -dyPixels/tileSize);
                        } else if (dyPixels > 0) {
                            redrawTiles(g, map, 0, getHeight()/tileSize-dyPixels/tileSize, getWidth()/tileSize, getHeight()/tileSize);
                        }
                    }
                    redrawSquare(g, map, (mapWidth-1)/2-dx, (mapHeight-1)/2-dy);
                    markPlayer(g);
                } finally {
                    g.dispose();
                }
            }
            setChanged();
        }
    };

    /**
     * The {@link MapListener} registered to receive map updates.
     */
    @NotNull
    private final MapListener mapListener = new MapListener() {
        /** {@inheritDoc} */
        @Override
        public void mapChanged(@NotNull final CfMap map, @NotNull final Set<CfMapSquare> changedSquares) {
            final int x0 = map.getOffsetX();
            final int y0 = map.getOffsetY();
            synchronized (bufferedImageSync) {
                final Graphics g = createBufferGraphics();
                try {
                    for (final CfMapSquare mapSquare : changedSquares) {
                        final int x = mapSquare.getX()+x0;
                        final int y = mapSquare.getY()+y0;
                        redrawSquare(g, map, x, y);
                    }
                    markPlayer(g);
                } finally {
                    g.dispose();
                }
            }
            setChanged();
        }
    };

    /**
     * The {@link NewmapListener} registered to receive newmap commands.
     */
    @NotNull
    private final NewmapListener newmapListener = new NewmapListener() {
        /** {@inheritDoc} */
        @Override
        public void commandNewmapReceived() {
            synchronized (bufferedImageSync) {
                final Graphics g = createBufferGraphics();
                try {
                    g.setColor(Color.BLACK);
                    g.fillRect(0, 0, getWidth(), getHeight());
                    g.setColor(GUIMap.FOG_OF_WAR_COLOR);
                    g.fillRect(0, 0, getWidth(), getHeight());
                    markPlayer(g);
                } finally {
                    g.dispose();
                }
            }
            setChanged();
        }
    };

    /**
     * The {@link MapSizeListener} registered to receive changes of the map view
     * size.
     */
    @NotNull
    private final MapSizeListener mapSizeListener = new MapSizeListener() {
        /** {@inheritDoc} */
        @Override
        public void mapSizeChanged(final int mapWidth, final int mapHeight) {
            GUIMagicMap.this.mapWidth = mapWidth;
            GUIMagicMap.this.mapHeight = mapHeight;
            offsetX = playerX-((mapWidth-1)/2)*tileSize;
            offsetY = playerY-((mapHeight-1)/2)*tileSize;
            synchronized (bufferedImageSync) {
                final Graphics g = createBufferGraphics();
                try {
                    g.setColor(Color.BLACK);
                    g.fillRect(0, 0, getWidth(), getHeight());
                    redrawTiles(g, mapUpdater.getMap(), 0, 0, getWidth()/tileSize, getHeight()/tileSize);
                } finally {
                    g.dispose();
                }
            }
        }
    };

    /**
     * Creates a new instance.
     * @param tooltipManager the tooltip manager to update
     * @param elementListener the element listener to notify
     * @param name the name of this element
     * @param tileSize the size of one tile in pixels
     * @param x the x-coordinate for drawing this element to screen
     * @param y the y-coordinate for drawing this element to screen
     * @param w the width for drawing this element to screen
     * @param h the height for drawing this element to screen
     * @param crossfireServerConnection the server connection to monitor
     * @param mapUpdater the map updater instance to use
     * @param facesManager the faces manager instance to use
     */
    public GUIMagicMap(@NotNull final TooltipManager tooltipManager, @NotNull final GUIElementListener elementListener, @NotNull final String name, final int tileSize, final int x, final int y, final int w, final int h, @NotNull final CrossfireServerConnection crossfireServerConnection, @NotNull final CfMapUpdater mapUpdater, @NotNull final FacesManager facesManager) {
        super(tooltipManager, elementListener, name, x, y, w, h, Transparency.TRANSLUCENT);
        if (w <= 0 || h <= 0) {
            throw new IllegalArgumentException("area must be non-empty");
        }
        if (w%tileSize != 0) {
            throw new IllegalArgumentException("width is not a multiple of "+tileSize);
        }
        if (h%tileSize != 0) {
            throw new IllegalArgumentException("height is not a multiple of "+tileSize);
        }
        if ((w/tileSize)%2 != 1) {
            throw new IllegalArgumentException("width is not an odd number of tiles");
        }
        if ((h/tileSize)%2 != 1) {
            throw new IllegalArgumentException("height is not an odd number of tiles");
        }
        this.mapUpdater = mapUpdater;
        this.facesManager = facesManager;
        this.crossfireServerConnection = crossfireServerConnection;
        this.tileSize = tileSize;
        playerX = w/2-tileSize/2;
        playerY = h/2-tileSize/2;

        this.crossfireServerConnection.addMapSizeListener(mapSizeListener);
        mapSizeListener.mapSizeChanged(crossfireServerConnection.getMapWidth(), crossfireServerConnection.getMapHeight());

        this.crossfireServerConnection.addCrossfireMagicmapListener(crossfireMagicmapListener);
        this.mapUpdater.addCrossfireNewmapListener(newmapListener);
        this.mapUpdater.addCrossfireMapscrollListener(mapscrollListener);
        this.mapUpdater.addCrossfireMapListener(mapListener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void dispose() {
        super.dispose();
        crossfireServerConnection.removeMapSizeListener(mapSizeListener);
        crossfireServerConnection.removeCrossfireMagicmapListener(crossfireMagicmapListener);
        mapUpdater.removeCrossfireNewmapListener(newmapListener);
        mapUpdater.removeCrossfireMapscrollListener(mapscrollListener);
        mapUpdater.removeCrossfireMapListener(mapListener);
    }

    /**
     * Redraws a rectangular area of tiles.
     * @param g the graphics to draw into
     * @param map the map to draw
     * @param x0 the left edge to redraw (inclusive)
     * @param y0 the top edge to redraw (inclusive)
     * @param x1 the right edge to redraw (exclusive)
     * @param y1 the bottom edge to redraw (exclusive)
     */
    private void redrawTiles(@NotNull final Graphics g, @NotNull final CfMap map, final int x0, final int y0, final int x1, final int y1) {
        for (int x = x0; x < x1; x++) {
            for (int y = y0; y < y1; y++) {
                redrawSquare(g, map, x-offsetX/tileSize, y-offsetY/tileSize);
            }
        }
    }

    /**
     * Redraws one square completely black.
     * @param g the graphics to draw into
     * @param x the x-coordinate of the square to clear
     * @param y the y-coordinate of the square to clear
     */
    private void cleanSquare(@NotNull final Graphics g, final int x, final int y) {
        g.setColor(Color.BLACK);
        g.fillRect(x*tileSize+offsetX, y*tileSize+offsetY, tileSize, tileSize);
    }

    /**
     * Redraws one square.
     * @param g the graphics to draw into
     * @param map the map to draw
     * @param x the x-coordinate of the map tile to redraw
     * @param y the y-coordinate of the map tile to redraw
     */
    private void redrawSquare(@NotNull final Graphics g, @NotNull final CfMap map, final int x, final int y) {
        cleanSquare(g, x, y);
        for (int layer = 0; layer < CrossfireMap2Command.NUM_LAYERS; layer++) {
            redrawSquare(g, map, x, y, layer);
        }
        if (map.isFogOfWar(x, y) || x < 0 || y < 0 || x >= mapWidth || y >= mapHeight) {
            g.setColor(GUIMap.FOG_OF_WAR_COLOR);
            g.fillRect(x*tileSize+offsetX, y*tileSize+offsetY, tileSize, tileSize);
        }
        final int darkness = map.getDarkness(x, y);
        if (darkness < 255) {
            g.setColor(GUIMap.getDarknessColor(darkness));
            g.fillRect(x*tileSize+offsetX, y*tileSize+offsetY, tileSize, tileSize);
        }
    }

    /**
     * Redraws one layer of a square.
     * @param g the graphics to draw into
     * @param map the map to draw
     * @param x the x coordinate of the square to redraw
     * @param y the y coordinate of the square to redraw
     * @param layer the layer to redraw
     */
    private void redrawSquare(@NotNull final Graphics g, @NotNull final CfMap map, final int x, final int y, final int layer) {
        final int px = x*tileSize+offsetX;
        final int py = y*tileSize+offsetY;

        final CfMapSquare headMapSquare = map.getHeadMapSquare(x, y, layer);
        if (headMapSquare != null) {
            final Face headFace = headMapSquare.getFace(layer);
            assert headFace != null; // getHeadMapSquare() would have been cleared in this case
            final ImageIcon img = facesManager.getMagicMapImageIcon(headFace.getFaceNum());
            final int dx = headMapSquare.getX()-map.getMapSquare(x, y).getX();
            final int dy = headMapSquare.getY()-map.getMapSquare(x, y).getY();
            assert dx > 0 || dy > 0;
            final int sx = img.getIconWidth()-tileSize*(dx+1);
            final int sy = img.getIconHeight()-tileSize*(dy+1);
            g.drawImage(img.getImage(), px, py, px+tileSize, py+tileSize, sx, sy, sx+tileSize, sy+tileSize, null);
        }

        final Face face = map.getFace(x, y, layer);
        if (face != null) {
            final ImageIcon img = facesManager.getMagicMapImageIcon(face.getFaceNum());
            final int sx = img.getIconWidth();
            final int sy = img.getIconHeight();
            g.drawImage(img.getImage(), px, py, px+tileSize, py+tileSize, sx-tileSize, sy-tileSize, sx, sy, null);
        }
    }

    /**
     * Paints the player location.
     * @param g the graphics to paint to
     */
    private void markPlayer(@NotNull final Graphics g) {
        g.setColor(Color.RED);
        g.fillRect(playerX, playerY, tileSize, tileSize);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void render(@NotNull final Graphics g) {
    }

}
