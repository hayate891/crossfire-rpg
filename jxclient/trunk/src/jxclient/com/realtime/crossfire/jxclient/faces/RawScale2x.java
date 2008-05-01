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
package com.realtime.crossfire.jxclient.faces;

/**
 * A simple implementation of the Scale2x algorithm for scaling raw image data.
 *
 * @author Kevin Glass
 */
public class RawScale2x
{
    /** The source image data. */
    private final int[] srcImage;

    /** The destination image data. */
    private final int[] dstImage;

    /** The width of the source image. */
    private final int width;

    /** The height of the source image. */
    private final int height;

    /**
     * Create a new scaler based on some raw data. Right now it doesn't matter
     * what order the channels in, just that its an int per pixel
     *
     * @param imageData The source image data.
     *
     * @param dataWidth The width of the source image.
     *
     * @param dataHeight The height of the source image.
     */
    public RawScale2x(final int[] imageData, final int dataWidth, final int dataHeight)
    {
        width = dataWidth;
        height = dataHeight;
        srcImage = imageData;
        dstImage = new int[imageData.length*4];
    }

    /**
     * Check if two pixels are different. Place holder for maybe some clever
     * code about tolerance checking.
     *
     * @param a The first pixel value.
     *
     * @param b The second pixel value.
     *
     * @return <code>true</code> if the pixels are different.
     */
    private static boolean different(final int a, final int b)
    {
        return a != b;
    }

    /**
     * Set a pixel in the destination image data.
     *
     * @param x The x location of the pixel to set.
     *
     * @param y The y location of the pixel to set.
     *
     * @param p The value of the pixel to set.
     */
    private void setDestPixel(final int x, final int y, final int p)
    {
        dstImage[x+(y*width*2)] = p;
    }

    /**
     * Get a pixel from the source image. This handles bonds checks and
     * resolves to edge pixels.
     *
     * @param x The x location of the pixel to retrieve.
     *
     * @param y The y location of the pixel to retrieve.
     *
     * @return The pixel value at the specified location.
     */
    private int getSourcePixel(final int x, final int y)
    {
        final int xx = Math.min(width-1, Math.max(0, x));
        final int yy = Math.min(height-1, Math.max(0, y));
        return srcImage[xx+(yy*width)];
    }

    /**
     * Process a specific pixel. This will generate 4 pixels in the destination
     * image based on the scale2x algorithm.
     *
     * @param x The x location in the source image of the pixel to process.
     *
     * @param y The y location in the source image of the pixel to process.
     */
    private void process(final int x, final int y)
    {
//        final int A = getSourcePixel(x-1, y-1);
        final int B = getSourcePixel(x, y-1);
//        final int C = getSourcePixel(x+1, y-1);
        final int D = getSourcePixel(x-1, y);
        final int E = getSourcePixel(x, y);
        final int F = getSourcePixel(x+1, y);
//        final int G = getSourcePixel(x-1, y+1);
        final int H = getSourcePixel(x, y+1);
//        final int I = getSourcePixel(x+1, y+1);
        int E0 = E;
        int E1 = E;
        int E2 = E;
        int E3 = E;

        if (different(B, H) && different(D, F))
        {
            E0 = !different(D, B) ? D : E;
            E1 = !different(B, F) ? F : E;
            E2 = !different(D, H) ? D : E;
            E3 = !different(H, F) ? F : E;
        }

        setDestPixel(x*2, y*2, E0);
        setDestPixel((x*2)+1, y*2, E1);
        setDestPixel((x*2), (y*2)+1, E2);
        setDestPixel((x*2)+1, (y*2)+1, E3);
    }

    /**
     * Get the scale image data. Note this is the method that does the work so
     * it might take some time to process.
     *
     * @return An array of pixels 4 times the size of the input array
     * containing the smoothly scaled image.
     */
    public int[] getScaledData()
    {
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                process(x, y);
            }
        }

        return dstImage;
    }
}
