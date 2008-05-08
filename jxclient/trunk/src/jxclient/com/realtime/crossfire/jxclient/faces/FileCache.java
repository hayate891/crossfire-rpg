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

import com.realtime.crossfire.jxclient.util.Filenames;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;

/**
 * Implements a disk based cache for image files.
 *
 * @author Andreas Kirschbaum
 */
public class FileCache
{

    /**
     * The directory where the images are saved.
     */
    private final File cacheDir;

    /**
     * Create a new instance.
     *
     * @param cacheDir The directory where the images are saved.
     */
    public FileCache(final File cacheDir)
    {
        this.cacheDir = cacheDir;
        cacheDir.mkdirs();
    }

    /**
     * Retrieve an image from the cache.
     *
     * @param name The image name to retrieve.
     *
     * @param checksum The checksum to retrieve.
     *
     * @return The image icon, or <code>null</code> if the cache does not
     * contain the image.
     */
    public ImageIcon load(final String name, final int checksum)
    {
        final File file = getImageFileName(name, checksum);
        final long len = file.length();
        if (len >= 0x10000 || len <= 0)
        {
            return null;
        }
        final byte[] data = new byte[(int)len];
        try
        {
            final FileInputStream fis = new FileInputStream(file);
            try
            {
                if (fis.read(data) != data.length)
                {
                    return null;
                }
            }
            finally
            {
                fis.close();
            }
        }
        catch (final IOException ex)
        {
            return null;
        }
        final ImageIcon imageIcon = new ImageIcon(data); // cannot use ImageIcon(String) since this caches "file not found"
        return imageIcon.getIconWidth() <= 0 && imageIcon.getIconHeight() <= 0 ? null : imageIcon;
    }

    /**
     * Store an {@link ImageIcon} into the cache.
     *
     * @param name The image name to save.
     *
     * @param checksum The checksum to retrieve.
     *
     * @param imageIcon The image icon to store.
     */
    public void save(final String name, final int checksum, final ImageIcon imageIcon)
    {
        saveImageIcon(getImageFileName(name, checksum), imageIcon);
    }

    /**
     * Save an {@link ImageIcon} to a file.
     *
     * @param outputFile the file to save to
     *
     * @param imageIcon the image to save
     */
    public static void saveImageIcon(final File outputFile, final ImageIcon imageIcon)
    {
        final BufferedImage bufferedImage = new BufferedImage(imageIcon.getIconWidth(), imageIcon.getIconHeight(), BufferedImage.TYPE_INT_ARGB);
        imageIcon.paintIcon(null, bufferedImage.getGraphics(), 0, 0);
        try
        {
            ImageIO.write(bufferedImage, "png", outputFile);
        }
        catch (final IOException ex)
        {
            System.err.println("Cannot write cache file "+outputFile+": "+ex.getMessage());
        }
    }

    /**
     * Calculate a hashed image name to be used as a file name.
     *
     * @param name The image name to hash.
     *
     * @param checksum The checksum to hash.
     *
     * @return the hashed image name.
     */
    private File getImageFileName(final String name, final int checksum)
    {
        final String quotedName = Filenames.quoteName(name);
        final String dirName = quotedName.substring(0, Math.min(2, quotedName.length()));
        final File dir = new File(new File(cacheDir, dirName), quotedName);
        if (!dir.exists() && !dir.mkdirs())
        {
            System.err.println("Cannot create directory: "+dir);
        }
        return new File(dir, Integer.toString(checksum));
    }
}
