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

package com.realtime.crossfire.jxclient.gui.gauge;

/**
 * Implements an {@link Orientation} which grows west to east.
 * @author Andreas Kirschbaum
 */
public class OrientationWE extends AbstractOrientation
{
    /**
     * Creates a new instance.
     */
    public OrientationWE()
    {
        recalc();
    }

    /** {@inheritDoc} */
    @Override
    protected void recalc()
    {
        x = 0;
        if (isNegativeImage())
        {
            w = calc(min-cur, max-min, width);
        }
        else
        {
            w = calc(cur-min, max-min, width);
        }

        y = 0;
        h = height;
    }
}
