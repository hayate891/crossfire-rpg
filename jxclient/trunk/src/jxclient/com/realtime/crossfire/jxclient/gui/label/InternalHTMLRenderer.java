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
package com.realtime.crossfire.jxclient.gui.label;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.util.Stack;
import javax.swing.text.MutableAttributeSet;
import javax.swing.text.html.HTML;
import javax.swing.text.html.HTMLEditorKit;

/**
 * @author Lauwenmark
 */
public class InternalHTMLRenderer extends HTMLEditorKit.ParserCallback
{
    private final Stack<Font> fonts = new Stack<Font>();

    private final Stack<Color> colors = new Stack<Color>();

    private final Graphics gc;

    private int x = 0;

    private int y = 0;

    private final int origx;

    private final int borderSize;

    public InternalHTMLRenderer(final Font font, final Color color, final Graphics gc, final int x, final int y, final int borderSize)
    {
        fonts.push(font);
        colors.push(color);
        this.gc = gc;
        this.x = x;
        this.y = y;
        origx = x;
        this.borderSize = borderSize;
    }

    @Override
    public void handleText(final char[] data, final int pos)
    {
        gc.setFont(fonts.peek());
        gc.setColor(colors.peek());
        final FontMetrics m = gc.getFontMetrics();
        final String str = new String(data);
        final int w = m.stringWidth(str);
        gc.drawString(str, x+borderSize, y+borderSize);
        x += w;
    }

    @Override
    public void handleStartTag(final HTML.Tag t, final MutableAttributeSet a, final int pos)
    {
        if (t.equals(HTML.Tag.A))
        {
            fonts.push(fonts.peek());
            colors.push(Color.YELLOW);
            //y += defaultfont.getSize()+1;
        }
        else if (t.equals(HTML.Tag.B))
        {
            fonts.push(fonts.peek().deriveFont(Font.BOLD));
            colors.push(colors.peek());
        }
        else if (t.equals(HTML.Tag.I))
        {
            fonts.push(fonts.peek().deriveFont(Font.ITALIC));
            colors.push(colors.peek());
        }
        else if (t.equals(HTML.Tag.LI))
        {
            fonts.push(fonts.peek());
            colors.push(colors.peek());
            gc.setFont(fonts.peek());
            gc.setColor(colors.peek());
            final FontMetrics m = gc.getFontMetrics();
            x = origx;
            y += fonts.peek().getSize()+1;
            final String str = " - ";
            final int w = m.stringWidth(str);
            gc.drawString(str, x+borderSize, y+borderSize);
            x += w;
        }
        else
        {
            fonts.push(fonts.peek());
            colors.push(colors.peek());
        }
    }

    @Override
    public void handleSimpleTag(final HTML.Tag t, final MutableAttributeSet a, final int pos)
    {
        if (t.equals(HTML.Tag.BR))
        {
            y += fonts.peek().getSize()+1;
            x = origx;
        }
    }

    @Override
    public void handleEndTag(final HTML.Tag t, final int pos)
    {
        fonts.pop();
        colors.pop();
    }
}
