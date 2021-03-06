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

package com.realtime.crossfire.jxclient.skin.factory;

import com.realtime.crossfire.jxclient.gui.GUIDialogTitle;
import com.realtime.crossfire.jxclient.gui.GUIPicture;
import com.realtime.crossfire.jxclient.gui.gui.GUIElement;
import com.realtime.crossfire.jxclient.gui.gui.GUIElementListener;
import com.realtime.crossfire.jxclient.gui.gui.JXCWindowRenderer;
import com.realtime.crossfire.jxclient.gui.gui.TooltipManager;
import com.realtime.crossfire.jxclient.gui.label.Alignment;
import com.realtime.crossfire.jxclient.gui.label.GUIOneLineLabel;
import com.realtime.crossfire.jxclient.skin.skin.Expression;
import com.realtime.crossfire.jxclient.skin.skin.Extent;
import java.awt.Color;
import java.awt.Font;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Collection;
import org.jetbrains.annotations.NotNull;

/**
 * A factory class to create "textbutton" instances.
 * @author Andreas Kirschbaum
 */
public class DialogFactory {

    /**
     * The north-west frame picture.
     */
    @NotNull
    private final BufferedImage frameNW;

    /**
     * The north frame picture.
     */
    @NotNull
    private final BufferedImage frameN;

    /**
     * The north-east frame picture.
     */
    @NotNull
    private final BufferedImage frameNE;

    /**
     * The west frame picture.
     */
    @NotNull
    private final BufferedImage frameW;

    /**
     * The center frame picture.
     */
    @NotNull
    private final BufferedImage frameC;

    /**
     * The east frame picture.
     */
    @NotNull
    private final BufferedImage frameE;

    /**
     * The south-west frame picture.
     */
    @NotNull
    private final BufferedImage frameSW;

    /**
     * The south frame picture.
     */
    @NotNull
    private final BufferedImage frameS;

    /**
     * The south-east frame picture.
     */
    @NotNull
    private final BufferedImage frameSE;

    /**
     * The size of the north border in pixels.
     */
    private final int sizeN;

    /**
     * The size of the south border in pixels.
     */
    private final int sizeS;

    /**
     * The size of the west border in pixels.
     */
    private final int sizeW;

    /**
     * The size of the east border in pixels.
     */
    private final int sizeE;

    /**
     * The width of the center area in pixels.
     */
    private final int contentWidth;

    /**
     * The height of the center area in pixels.
     */
    private final int contentHeight;

    /**
     * The font for the dialog title.
     */
    private final Font titleFont;

    /**
     * The color for the dialog title.
     */
    private final Color titleColor;

    /**
     * The background color for the dialog title.
     */
    private final Color titleBackgroundColor;

    /**
     * y The alpha value for the dialog background except for the title.
     */
    private final float alpha;

    /**
     * Create a new instance. The border images must have matching sizes.
     * @param frameNW The north-west frame picture.
     * @param frameN The north frame picture.
     * @param frameNE The north-east frame picture.
     * @param frameW The west frame picture.
     * @param frameC The center frame picture.
     * @param frameE The east frame picture.
     * @param frameSW The south-west frame picture.
     * @param frameS The south frame picture.
     * @param frameSE The south-east frame picture.
     * @param titleFont The font for the dialog title.
     * @param titleColor The color for the dialog title.
     * @param titleBackgroundColor The background color for the dialog title.
     * @param alpha The alpha value for the dialog background except for the
     * title.
     */
    public DialogFactory(@NotNull final BufferedImage frameNW, @NotNull final BufferedImage frameN, @NotNull final BufferedImage frameNE, @NotNull final BufferedImage frameW, @NotNull final BufferedImage frameC, @NotNull final BufferedImage frameE, @NotNull final BufferedImage frameSW, @NotNull final BufferedImage frameS, @NotNull final BufferedImage frameSE, @NotNull final Font titleFont, @NotNull final Color titleColor, @NotNull final Color titleBackgroundColor, final float alpha) {
        this.frameNW = frameNW;
        this.frameN = frameN;
        this.frameNE = frameNE;
        this.frameW = frameW;
        this.frameC = frameC;
        this.frameE = frameE;
        this.frameSW = frameSW;
        this.frameS = frameS;
        this.frameSE = frameSE;
        sizeN = frameN.getHeight(null);
        sizeS = frameS.getHeight(null);
        sizeW = frameW.getWidth(null);
        sizeE = frameE.getWidth(null);
        if (frameNW.getHeight(null) != sizeN) {
            throw new IllegalArgumentException("height of NW ("+frameNW.getHeight(null)+") does not match height of N ("+sizeN+")");
        }
        if (frameNE.getHeight(null) != sizeN) {
            throw new IllegalArgumentException("height of NE ("+frameNE.getHeight(null)+") does not match height of N ("+sizeN+")");
        }
        if (frameSW.getHeight(null) != sizeS) {
            throw new IllegalArgumentException("height of SW ("+frameSW.getHeight(null)+") does not match height of N ("+sizeS+")");
        }
        if (frameSE.getHeight(null) != sizeS) {
            throw new IllegalArgumentException("height of SE ("+frameSE.getHeight(null)+") does not match height of N ("+sizeS+")");
        }
        if (frameNW.getWidth(null) != sizeW) {
            throw new IllegalArgumentException("width of NW ("+frameNW.getWidth(null)+") does not match width of W");
        }
        if (frameNE.getWidth(null) != sizeE) {
            throw new IllegalArgumentException("width of NE ("+frameNE.getWidth(null)+") does not match width of E");
        }
        if (frameSW.getWidth(null) != sizeW) {
            throw new IllegalArgumentException("width of SW ("+frameSW.getWidth(null)+") does not match width of W");
        }
        if (frameSE.getWidth(null) != sizeE) {
            throw new IllegalArgumentException("width of SE ("+frameSE.getWidth(null)+") does not match width of E");
        }
        contentWidth = frameC.getWidth(null);
        contentHeight = frameC.getHeight(null);
        if (frameN.getWidth(null) != contentWidth) {
            throw new IllegalArgumentException("width of N ("+frameN.getWidth(null)+") does not match width of C ("+contentWidth+")");
        }
        if (frameS.getWidth(null) != contentWidth) {
            throw new IllegalArgumentException("width of S ("+frameS.getWidth(null)+") does not match width of C ("+contentWidth+")");
        }
        if (frameW.getHeight(null) != contentHeight) {
            throw new IllegalArgumentException("width of W ("+frameW.getHeight(null)+") does not match height of C ("+contentHeight+")");
        }
        if (frameE.getHeight(null) != contentHeight) {
            throw new IllegalArgumentException("width of E ("+frameE.getHeight(null)+") does not match height of C ("+contentHeight+")");
        }
        this.titleFont = titleFont;
        this.titleColor = titleColor;
        this.titleBackgroundColor = new Color(titleBackgroundColor.getRed(), titleBackgroundColor.getGreen(), titleBackgroundColor.getBlue(), (int)(255*alpha));
        this.alpha = alpha;
    }

    /**
     * Create a new dialog.
     * @param tooltipManager the tooltip manager to update
     * @param windowRenderer the window renderer the dialog belongs to
     * @param elementListener the element listener to notify
     * @param name The base name of the dialog's gui elements.
     * @param w The width of the dialog, including the frames.
     * @param h The height of the dialog, including the frames.
     * @param title The dialog's title, or an empty string for no title.
     * @return The gui elements comprising the new dialog.
     */
    @NotNull
    public Iterable<GUIElement> newDialog(@NotNull final TooltipManager tooltipManager, @NotNull final JXCWindowRenderer windowRenderer, @NotNull final GUIElementListener elementListener, @NotNull final String name, @NotNull final Expression w, @NotNull final Expression h, @NotNull final String title) {
        final int titleHeight = title.length() > 0 ? 18 : 0;
        final Collection<GUIElement> result = new ArrayList<GUIElement>();
        final Expression zeroExpression = new Expression(0, 0, 0);
        final Expression sizeNExpression = new Expression(sizeN, 0, 0);
        final Expression sizeSExpression = new Expression(sizeS, 0, 0);
        final Expression sizeWExpression = new Expression(sizeW, 0, 0);
        final Expression sizeEExpression = new Expression(sizeE, 0, 0);
        final Expression titleHeightExpression = new Expression(titleHeight, 0, 0);
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_nw", new Extent(0, 0, sizeW, sizeN), frameNW, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_n", new Extent(sizeWExpression, zeroExpression, w.addConstant(-sizeW-sizeE), sizeNExpression), frameN, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_ne", new Extent(w.addConstant(-sizeE), zeroExpression, sizeEExpression, sizeNExpression), frameNE, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_w", new Extent(zeroExpression, sizeNExpression, sizeWExpression, h.addConstant(-sizeN-sizeS)), frameW, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_c", new Extent(sizeWExpression, sizeNExpression.addConstant(titleHeight), w.addConstant(-sizeW-sizeE), h.addConstant(-sizeN-sizeS-titleHeight)), frameC, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_e", new Extent(w.addConstant(-sizeE), sizeNExpression, sizeEExpression, h.addConstant(-sizeN-sizeS)), frameE, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_sw", new Extent(zeroExpression, h.addConstant(-sizeS), sizeWExpression, sizeSExpression), frameSW, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_s", new Extent(sizeWExpression, h.addConstant(-sizeS), w.addConstant(-sizeW-sizeE), sizeSExpression), frameS, alpha));
        result.add(new GUIPicture(tooltipManager, elementListener, name+"_se", new Extent(w.addConstant(-sizeE), h.addConstant(-sizeS), sizeEExpression, sizeSExpression), frameSE, alpha));
        if (titleHeight > 0) {
            result.add(new GUIDialogTitle(tooltipManager, windowRenderer, elementListener, name+"_t", new Extent(sizeWExpression, sizeNExpression, w.addConstant(-sizeW-sizeE), titleHeightExpression), frameC, alpha));
            if (!title.equals("_")) {
                final GUIElement titleLabel = new GUIOneLineLabel(tooltipManager, elementListener, name+"_title", new Extent(sizeWExpression, sizeNExpression, w.addConstant(-sizeW-sizeE), titleHeightExpression), null, titleFont, titleColor, titleBackgroundColor, Alignment.LEFT, " "+title);
                result.add(titleLabel);
                titleLabel.setIgnore();
            }
        }
        return result;
    }

}
