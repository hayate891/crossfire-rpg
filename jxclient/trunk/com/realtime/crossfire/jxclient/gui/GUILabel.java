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
import java.awt.FontMetrics;
import java.awt.geom.Rectangle2D;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.awt.Transparency;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.util.regex.Pattern;
import java.util.Stack;
import javax.swing.ImageIcon;
import javax.swing.text.html.HTML;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.html.parser.ParserDelegator;
import javax.swing.text.MutableAttributeSet;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class GUILabel extends GUIElement
{
    /**
     * Size of border around text in auto-resize mode.
     */
    public static final int AUTO_BORDER_SIZE = 2;

    /**
     * The pattern used to split a string into lines.
     */
    private static final Pattern patternLineBreak = Pattern.compile("\n");

    private ImageIcon mybackground = null;

    private Font myfont;

    private String mycaption = "";

    private Color mycolor = Color.WHITE;

    /**
     * If set, the opaque background color; if <code>null</code>, the
     * background is transparent. This field is ignored if {@link
     * #mybackground} is set.
     */
    private Color backgroundColor = null;

    /**
     * If set, auto-resize this element to the extent of {@link #mycaption}.
     */
    private boolean autoResize = false;

    private void commonInit(BufferedImage picture, Font nf) throws IOException
    {
        if (picture != null)
            mybackground = new ImageIcon(picture);
        else
            mybackground = null;
        myfont = nf;
        createBuffer();
    }

    public GUILabel(final JXCWindow jxcWindow, String nn, int nx, int ny, int nw, int nh, BufferedImage picture, Font nf, Color clr, String cap) throws IOException
    {
        super(jxcWindow, nn, nx, ny, nw, nh);
        commonInit(picture, nf);
        mycolor = clr;
        mycaption = cap;
        render();
    }

    public GUILabel(final JXCWindow jxcWindow, String nn, int nx, int ny, int nw, int nh, BufferedImage picture, Font nf, String cap) throws IOException
    {
        super(jxcWindow, nn, nx, ny, nw, nh);
        commonInit(picture, nf);
        mycaption = cap;
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

    public void setText(String ntxt)
    {
        if (ntxt == null) throw new IllegalArgumentException();
        if (!mycaption.equals(ntxt))
        {
            mycaption = ntxt;
            autoResize();
            render();
        }
    }

    /**
     * Return the label text.
     *
     * @return The label text.
     */
    public String getText()
    {
        return mycaption;
    }

    /**
     * Set the background color.
     *
     * @param backgroundColor The background color, or <code>null</code> for
     * transparent background.
     */
    public void setBackgroundColor(final Color backgroundColor)
    {
        if (this.backgroundColor != backgroundColor)
        {
            this.backgroundColor = backgroundColor;
            createBuffer();
        }
    }

    protected void render()
    {
        try
        {
            Graphics2D g = mybuffer.createGraphics();
            g.setBackground(new Color(0, 0, 0, 0.0f));
            g.clearRect(0, 0, w, h);
            if (mybackground != null)
            {
                g.drawImage(mybackground.getImage(), x, y, null);
            }
            else if (backgroundColor != null)
            {
                g.setBackground(backgroundColor);
                g.clearRect(0, 0, w-1, h-1);
            }
            g.setFont(myfont);
            g.setColor(mycolor);

            mycaption = mycaption.replaceAll("\n", "<br>");
            Reader reader = new StringReader(mycaption);
            try
            {
                new ParserDelegator().parse(reader, new InternalHTMLRenderer(myfont, mycolor, g, 0, myfont.getSize()), false);
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
            g.dispose();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        setChanged();
    }

    class InternalHTMLRenderer extends HTMLEditorKit.ParserCallback
    {
        private Stack<Font> myfonts = new Stack<Font>();

        private Stack<Color> mycolors = new Stack<Color>();

        private Graphics2D mygc;

        private int myx = 0;

        private int myy = 0;

        private int myorigx = 0;

        public InternalHTMLRenderer(Font fd, Color fdc, Graphics2D g, int x, int y)
        {
            myfonts.push(fd);
            mycolors.push(fdc);
            mygc = g;
            myx = x;
            myy = y;
            myorigx = myx;
        }

        public void handleText(char[] data, int pos)
        {
            mygc.setFont(myfonts.peek());
            mygc.setColor(mycolors.peek());
            FontMetrics m = mygc.getFontMetrics();
            String str = new String(data);
            int w = m.stringWidth(str);
            mygc.drawString(str, myx+(autoResize ? AUTO_BORDER_SIZE : 0), myy+(autoResize ? AUTO_BORDER_SIZE : 0));
            myx += w;
        }

        public void handleStartTag(HTML.Tag tag, MutableAttributeSet attrSet, int pos)
        {
            if (tag.equals(HTML.Tag.A))
            {
                myfonts.push(myfonts.peek());
                mycolors.push(Color.YELLOW);
                //myy += mydefaultfont.getSize()+1;
            }
            else if (tag.equals(HTML.Tag.B))
            {
                myfonts.push(myfonts.peek().deriveFont(Font.BOLD));
                mycolors.push(mycolors.peek());
            }
            else if (tag.equals(HTML.Tag.I))
            {
                myfonts.push(myfonts.peek().deriveFont(Font.ITALIC));
                mycolors.push(mycolors.peek());
            }
            else if (tag.equals(HTML.Tag.LI))
            {
                myfonts.push(myfonts.peek());
                mycolors.push(mycolors.peek());
                mygc.setFont(myfonts.peek());
                mygc.setColor(mycolors.peek());
                FontMetrics m = mygc.getFontMetrics();
                myx = myorigx;
                myy += myfonts.peek().getSize()+1;
                String str = " - ";
                int w = m.stringWidth(str);
                mygc.drawString(str, myx+(autoResize ? AUTO_BORDER_SIZE : 0), myy+(autoResize ? AUTO_BORDER_SIZE : 0));
                myx += w;
            }
            else
            {
                myfonts.push(myfonts.peek());
                mycolors.push(mycolors.peek());
            }
        }

        public void handleSimpleTag(HTML.Tag tag, MutableAttributeSet attrSet, int pos)
        {
            if (tag.equals(HTML.Tag.BR))
            {
                myy += myfonts.peek().getSize()+1;
                myx = myorigx;
            }
            /*else
            {
                System.out.println("Tag:"+tag);
            }*/
        }

        public void handleEndTag(HTML.Tag tag, int pos)
        {
            myfonts.pop();
            mycolors.pop();
            //System.out.println("End Tag:"+tag);
        }
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
            for (final String str : patternLineBreak.split(mycaption, -1))
            {
                final Rectangle2D size = myfont.getStringBounds(mycaption, context);
                width = Math.max(width, (int)size.getWidth());
                height += (int)size.getHeight();
            }
            setSize(width+2*AUTO_BORDER_SIZE, height+2*AUTO_BORDER_SIZE);
        }
        finally
        {
            g.dispose();
        }
    }

    /**
     * Set the background image.
     *
     * @param background The new background image.
     */
    protected void setBackground(final ImageIcon background)
    {
        mybackground = background;
        createBuffer();
        render();
    }

    /** {@inheritDoc} */
    protected void createBuffer()
    {
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        final GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        mybuffer = gconf.createCompatibleImage(w, h, backgroundColor == null ? Transparency.TRANSLUCENT : Transparency.OPAQUE);
        final Graphics2D g = mybuffer.createGraphics();
        if (mybackground != null)
        {
            g.drawImage(mybackground.getImage(), x, y, null);
        }
        g.dispose();
        setChanged();
    }
}
