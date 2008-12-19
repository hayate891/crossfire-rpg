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
package com.realtime.crossfire.jxclient.map;

import com.realtime.crossfire.jxclient.animations.Animation;
import com.realtime.crossfire.jxclient.mapupdater.CfMapUpdater;

/**
 * Animation state information.
 * @author Andreas Kirschbaum
 */
public class AnimationState
{
    /**
     * The animation to display.
     */
    private final Animation animation;

    /**
     * The animation type.
     */
    private final int type;

    /**
     * The {@link CfMapUpdater} instance to use.
     */
    private final CfMapUpdater mapUpdater;

    /**
     * The animation speed.
     */
    private int speed = 1;

    /**
     * The face was updated last in this tick number.
     */
    private int tickno = 0;

    /**
     * The face index currently shown.
     */
    private int index = 0;

    /**
     * Creates a new instance.
     * @param animation the animation to display
     * @param type the animation type
     * @param mapUpdater the instance to use
     */
    public AnimationState(final Animation animation, final int type, final CfMapUpdater mapUpdater)
    {
        this.animation = animation;
        this.type = type;
        this.mapUpdater = mapUpdater;
    }

    /**
     * Sets the animation speed.
     * @param speed the new animation speed to set
     */
    public void setSpeed(final int speed)
    {
        assert speed > 0;
        final int tmpIndex = index/this.speed;
        final int tmpDelay = Math.min(index%this.speed, speed-1);
        this.speed = speed;
        index = tmpIndex*speed+tmpDelay;
    }

    /**
     * Sets the tick number. This function does not update the displayed face.
     * @param tickno the current tick number
     */
    public void setTickno(final int tickno)
    {
        this.tickno = tickno;
    }

    /**
     * Sets the tick number and update affected faces.
     * @param tickno the tick number
     * @param location the location to update
     */
    public void updateTickno(final int tickno, final Location location)
    {
        final int oldFaceIndex = index/speed;
        final int diff = tickno-this.tickno;
        if (tickno < this.tickno)
        {
            System.err.println("Ignoring inconsistent tick value: current tick number is "+tickno+", previous tick number was "+this.tickno+".");
        }
        else
        {
            index = (index+diff)%(speed*animation.getFaces());
        }
        this.tickno = tickno;

        draw(location, oldFaceIndex);
    }

    /**
     * Updates the map face at the given location.
     * @param location the map location to update
     * @param oldFaceIndex suppress the map face update if the new face
     * index equals this value
     */
    public void draw(final Location location, final int oldFaceIndex)
    {
        final int faceIndex = index/speed;
        if (faceIndex == oldFaceIndex)
        {
            return;
        }

        final int face = animation.getFace(faceIndex);
        mapUpdater.processMapFace(location.getX(), location.getY(), location.getLayer(), face, false);
    }
}
