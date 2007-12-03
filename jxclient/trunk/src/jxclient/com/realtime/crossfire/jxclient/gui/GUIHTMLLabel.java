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

import com.realtime.crossfire.jxclient.JXCWindow;
import java.awt.Color;
import java.awt.font.FontRenderContext;
import java.awt.Font;
import java.awt.geom.Rectangle2D;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.awt.Transparency;
import java.io.Reader;
import java.io.StringReader;
import java.util.regex.Pattern;
import javax.swing.ImageIcon;
import javax.swing.text.html.parser.ParserDelegator;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class GUIHTMLLabel extends AbstractLabel
{
    /**
     * Size of border around text in auto-resize mode.
     */
    public static final int AUTO_BORDER_SIZE = 2;

    /**
     * The pattern used to split a string into lines.
     */
    private static final Pattern patternLineBreak = Pattern.compile("<br>");

    private final Font myfont;

    private final Color mycolor;

    /**
     * If set, auto-resize this element to the extent of {@link #mycaption}.
     */
    private boolean autoResize = false;

    private void commonInit()
    {
        createBuffer();
    }

    public GUIHTMLLabel(final JXCWindow jxcWindow, final String name, final int x, final int y, final int w, final int h, final BufferedImage picture, final Font font, final Color color, final String text)
    {
        super(jxcWindow, name, x, y, w, h, picture, text);
        myfont = font;
        mycolor = color;
        render();
    }

    /** {@inheritDoc} */
    @Override protected void textChanged()
    {
        autoResize();
        render();
    }

    /**
     * Enable or disable auto-resizing. If enabled, the gui element's size
     * changes to the displayed text's size.
     *
     * @param autoResize If set, enable auto-resizing; if unset, disable
     * auto-resizing.
     */
    public void setAutoResize(final boolean autoResize)
    {
        if (this.autoResize != autoResize)
        {
            this.autoResize = autoResize;
            autoResize();
        }
    }

    protected void render()
    {
        if (myfont == null)
        {
            return;
        }

        super.render();
        try
        {
            final Graphics2D g = mybuffer.createGraphics();
            g.setFont(myfont);
            g.setColor(mycolor);

            final Reader reader = new StringReader(getText());
            try
            {
                new ParserDelegator().parse(reader, new InternalHTMLRenderer(myfont, mycolor, g, 0, myfont.getSize(), autoResize ? AUTO_BORDER_SIZE : 0), false);
            }
            catch (final Exception e)
            {
                e.printStackTrace();
            }
            g.dispose();
        }
        catch (final Exception e)
        {
            e.printStackTrace();
        }
        setChanged();
    }

    /**
     * If auto-resizing is enabled, calculate the new width and height.
     */
    private void autoResize()
    {
        if (!autoResize)
        {
            return;
        }

        final Graphics2D g = mybuffer.createGraphics();
        try
        {
            final FontRenderContext context = g.getFontRenderContext();
            int width = 0;
            int height = 0;
            for (final String str : patternLineBreak.split(getText(), -1))
            {
                final Rectangle2D size = myfont.getStringBounds(str, context);
                width = Math.max(width, (int)size.getWidth());
                height += (int)size.getHeight();
            }
            setSize(Math.max(1, width+2*AUTO_BORDER_SIZE), Math.max(1, height+2*AUTO_BORDER_SIZE));
        }
        finally
        {
            g.dispose();
        }
    }
}
