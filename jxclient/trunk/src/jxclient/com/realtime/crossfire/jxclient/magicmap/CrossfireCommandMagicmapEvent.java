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
package com.realtime.crossfire.jxclient.magicmap;

import java.util.EventObject;

/**
 *
 * @version 1.0
 * @author Lauwenmark
 * @since 1.0
 */
public class CrossfireCommandMagicmapEvent extends EventObject
{
    /** The serial version UID. */
    private static final long serialVersionUID = 1;

    private final int mywidth, myheight;

    private final int mypx, mypy;

    private final byte[] mydata;

    public CrossfireCommandMagicmapEvent(Object src, int w, int h, int px, int py, byte[] d)
    {
        super(src);
        mywidth = w;
        myheight = h;
        mypx = px;
        mypy = py;
        mydata = d;
    }

    public int getWidth()
    {
        return mywidth;
    }

    public int getHeight()
    {
        return myheight;
    }

    public int getPX()
    {
        return mypx;
    }

    public int getPY()
    {
        return mypy;
    }

    public byte[] getData()
    {
        return mydata;
    }
}
