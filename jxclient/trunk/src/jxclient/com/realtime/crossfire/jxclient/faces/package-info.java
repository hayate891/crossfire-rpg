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
 * Copyright (C) 2006-2011 Andreas Kirschbaum.
 */

/**
 * Manages image information ("faces") needed to display the map view, items,
 * and spell icons. The main class is {@link
 * com.realtime.crossfire.jxclient.faces.FacesManager}; it delegates to a
 * {@link com.realtime.crossfire.jxclient.faces.FaceCache} instance for caching
 * the face information. The faces are stored in {@link
 * java.lang.ref.SoftReference SoftReferences} so they can be reclaimed by the
 * garbage collector.
 * <p>If a face is not available, an "unknown" (question mark) face is returned
 * and the face is requested through a {@link
 * com.realtime.crossfire.jxclient.faces.FacesQueue} instance.
 */

package com.realtime.crossfire.jxclient.faces;
