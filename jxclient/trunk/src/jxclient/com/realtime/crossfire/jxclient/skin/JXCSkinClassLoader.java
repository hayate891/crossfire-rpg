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
package com.realtime.crossfire.jxclient.skin;

import com.realtime.crossfire.jxclient.faces.FacesManager;
import com.realtime.crossfire.jxclient.gui.keybindings.KeyBindings;
import com.realtime.crossfire.jxclient.items.ItemsManager;
import com.realtime.crossfire.jxclient.mapupdater.CfMapUpdater;
import com.realtime.crossfire.jxclient.spells.SpellsManager;
import com.realtime.crossfire.jxclient.stats.Stats;
import java.io.IOException;
import java.io.InputStream;

/**
 * A {@link JXCSkinLoader} that loads via the class loader.
 *
 * @author Andreas Kirschbaum
 */
public class JXCSkinClassLoader extends JXCSkinLoader
{
    /**
     * The base resource name to prepend to all resource names.
     */
    private final String baseName;

    /**
     * Create a new instance.
     *
     * @param baseName The base resource name to prepend to all resource names.
     *
     * @throws JXCSkinException if the skin cannot be loaded
     */
    public JXCSkinClassLoader(final ItemsManager itemsManager, final SpellsManager spellsManager, final FacesManager facesManager, final Stats stats, final CfMapUpdater mapUpdater, final String baseName, final KeyBindings defaultKeyBindings) throws JXCSkinException
    {
        super(itemsManager, spellsManager, facesManager, stats, mapUpdater, defaultKeyBindings);
        if (baseName == null) throw new IllegalArgumentException();
        this.baseName = baseName;
        checkAccess();
    }

    /** {@inheritDoc} */
    @Override protected InputStream getInputStream(final String name) throws IOException
    {
        final InputStream inputStream = getClassLoader().getResourceAsStream(baseName+"/"+name);
        if (inputStream == null)
        {
            throw new IOException("resource not found");
        }
        return inputStream;
    }

    /** {@inheritDoc} */
    @Override protected String getURI(final String name)
    {
        return "resource:"+baseName+"/"+name;
    }

    /**
     * Return the {@link ClassLoader} to use.
     *
     * @return The class loader.
     */
    private ClassLoader getClassLoader()
    {
        final ClassLoader classLoader = getClass().getClassLoader();
        if (classLoader != null)
        {
            return classLoader;
        }

        final ClassLoader systemClassLoader = ClassLoader.getSystemClassLoader();
        if (systemClassLoader != null)
        {
            return systemClassLoader;
        }

        throw new InternalError("cannot find class loader");
    }
}
