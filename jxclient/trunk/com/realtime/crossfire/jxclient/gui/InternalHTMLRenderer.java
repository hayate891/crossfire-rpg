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

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.util.Stack;
import javax.swing.text.html.HTML;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.MutableAttributeSet;

/**
 * @author Lauwenmark
 */
public class InternalHTMLRenderer extends HTMLEditorKit.ParserCallback
{
    private final Stack<Font> myfonts = new Stack<Font>();

    private final Stack<Color> mycolors = new Stack<Color>();

    private final Graphics2D mygc;

    private int myx = 0;

    private int myy = 0;

    private final int myorigx;

    private final int borderSize;

    public InternalHTMLRenderer(final Font fd, final Color fdc, final Graphics2D g, final int x, final int y, final int borderSize)
    {
        myfonts.push(fd);
        mycolors.push(fdc);
        mygc = g;
        myx = x;
        myy = y;
        myorigx = myx;
        this.borderSize = borderSize;
    }

    public void handleText(final char[] data, final int pos)
    {
        mygc.setFont(myfonts.peek());
        mygc.setColor(mycolors.peek());
        final FontMetrics m = mygc.getFontMetrics();
        final String str = new String(data);
        final int w = m.stringWidth(str);
        mygc.drawString(str, myx+borderSize, myy+borderSize);
        myx += w;
    }

    public void handleStartTag(final HTML.Tag tag, final MutableAttributeSet attrSet, final int pos)
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
            final FontMetrics m = mygc.getFontMetrics();
            myx = myorigx;
            myy += myfonts.peek().getSize()+1;
            final String str = " - ";
            final int w = m.stringWidth(str);
            mygc.drawString(str, myx+borderSize, myy+borderSize);
            myx += w;
        }
        else
        {
            myfonts.push(myfonts.peek());
            mycolors.push(mycolors.peek());
        }
    }

    public void handleSimpleTag(final HTML.Tag tag, final MutableAttributeSet attrSet, final int pos)
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

    public void handleEndTag(final HTML.Tag tag, final int pos)
    {
        myfonts.pop();
        mycolors.pop();
        //System.out.println("End Tag:"+tag);
    }
}
