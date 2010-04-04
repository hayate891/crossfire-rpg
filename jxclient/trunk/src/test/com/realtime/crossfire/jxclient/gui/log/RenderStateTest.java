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

package com.realtime.crossfire.jxclient.gui.log;

import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.List;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;
import junit.textui.TestRunner;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Regression tests for class {@link RenderState}.
 * @author Andreas Kirschbaum
 */
public class RenderStateTest extends TestCase {

    /**
     * Assumed height of log window.
     */
    private static final int HEIGHT = 100;

    @Nullable
    private RenderState rs = null;

    @Nullable
    private Buffer buffer = null;

    /**
     * Create a new instance.
     * @param name the test case name
     */
    public RenderStateTest(@NotNull final String name) {
        super(name);
    }

    @NotNull
    public static Test suite() {
        return new TestSuite(RenderStateTest.class);
    }

    /**
     * Run the regression tests.
     * @param args The command line arguments (ignored).
     */
    public static void main(@NotNull final String[] args) {
        TestRunner.run(suite());
    }

    public void test1() {
        final Parser parser = new Parser();

        for (int i = 0; i < HEIGHT+10; i++) {
            assert buffer != null;
            parser.parse("xxx"+i, null, buffer);
        }
        checkState(10, 0, 10);

        // scroll to valid positions
        assert buffer != null;
        assert rs != null;
        rs.scrollTo(buffer, 0);
        checkState(0, 0, 0);
        assert buffer != null;
        assert rs != null;
        rs.scrollTo(buffer, 5);
        checkState(5, 0, 5);
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, 10);
        checkState(10, 0, 10);

        // scroll to invalid positions
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, -1);
        checkState(0, 0, 0);
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, -10);
        checkState(0, 0, 0);

        // scroll to invalid positions
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, 11);
        checkState(10, 0, 10);
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, 21);
        checkState(10, 0, 10);
    }

    public void test2() {
        final Parser parser = new Parser();

        checkState(0, 0, 0);
        assert buffer != null;
        parser.parse("xxx1", null, buffer);
        checkState(0, 0, 0);
        assert buffer != null;
        parser.parse("xxx2", null, buffer);
        checkState(0, 0, 0);

        // add lines to completely fill visible area
        for (int i = 2; i < HEIGHT; i++) {
            assert buffer != null;
            parser.parse("xxx3"+i, null, buffer);
        }
        checkState(0, 0, 0);

        // add one more line ==> buffer sticks at bottom
        assert buffer != null;
        parser.parse("xxx4", null, buffer);
        checkState(1, 0, 1);

        // add one more line ==> buffer sticks at bottom
        assert buffer != null;
        parser.parse("xxx5", null, buffer);
        checkState(2, 0, 2);

        // scroll up one line
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, 1);
        checkState(1, 0, 1);

        // add one more line ==> buffer sticks at scroll position
        assert buffer != null;
        parser.parse("xxx6", null, buffer);
        checkState(1, 0, 1);

        // scroll back to bottom
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, 3);
        checkState(3, 0, 3);

        // add one more line ==> buffer sticks at bottom
        assert buffer != null;
        parser.parse("xxx7", null, buffer);
        checkState(4, 0, 4);

        // completely fill buffer
        for (int i = HEIGHT+4; i < Buffer.MAX_LINES; i++) {
            assert buffer != null;
            parser.parse("xxx8"+i, null, buffer);
        }
        checkState(Buffer.MAX_LINES-HEIGHT, 0, Buffer.MAX_LINES-HEIGHT);

        // add one more line ==> buffer sticks at bottom
        assert buffer != null;
        parser.parse("xxx9", null, buffer);
        checkState(Buffer.MAX_LINES-HEIGHT, 0, Buffer.MAX_LINES-HEIGHT);

        // scroll one line up
        assert rs != null;
        assert buffer != null;
        rs.scrollTo(buffer, Buffer.MAX_LINES-HEIGHT-1);
        checkState(Buffer.MAX_LINES-HEIGHT-1, 0, Buffer.MAX_LINES-HEIGHT-1);

        // fill more lines ==> scroll position sticks
        for (int i = 0; i < Buffer.MAX_LINES-HEIGHT-2; i++) {
            assert buffer != null;
            parser.parse("xxx0"+i, null, buffer);
        }
        checkState(1, 0, 1);
        assert buffer != null;
        parser.parse("xxx1", null, buffer);
        checkState(0, 0, 0);

        // add one more line ==> scroll position hits top
        assert buffer != null;
        parser.parse("xxx2", null, buffer);
        checkState(0, 0, 0);
    }

    private void checkState(final int expectedTopIndex, final int expectedTopOffset, final int expectedScrollPos) {
        assert rs != null;
        final int topIndex = rs.getTopIndex();
        assert rs != null;
        final int topOffset = rs.getTopOffset();
        assert rs != null;
        final int scrollPos = rs.getScrollPos();
        assertEquals(formatState(expectedTopIndex, expectedTopOffset, expectedScrollPos), formatState(topIndex, topOffset, scrollPos));
    }

    @NotNull
    private String formatState(final int topIndex, final int topOffset, final int scrollPos) {
        assert buffer != null;
        return "top="+topIndex+"/"+topOffset+" pos="+scrollPos+"/"+buffer.getTotalHeight();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setUp() throws FontFormatException, IOException {
        final GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        final GraphicsDevice gd = ge.getDefaultScreenDevice();
        final GraphicsConfiguration gconf = gd.getDefaultConfiguration();
        final BufferedImage image = gconf.createCompatibleImage(1, 1, Transparency.TRANSLUCENT);
        final Graphics2D g = image.createGraphics();
        final Font font;
        final FileInputStream fis = new FileInputStream("skins/ragnorok/fonts/regular.ttf");
        try {
            font = Font.createFont(Font.TRUETYPE_FONT, fis);
        } finally {
            fis.close();
        }
        buffer = new Buffer(new Fonts(font, font, font, font), g.getFontRenderContext(), 100);
        g.dispose();
        rs = new RenderState();
        assert buffer != null;
        rs.setHeight(buffer, HEIGHT);

        final BufferListener bufferListener = new BufferListener() {
            /** {@inheritDoc} */
            @Override
            public void linesAdded(final int lines) {
                assert rs != null;
                assert buffer != null;
                rs.linesAdded(buffer);
            }

            /** {@inheritDoc} */
            @Override
            public void linesReplaced(final int lines) {
                assert rs != null;
                assert buffer != null;
                rs.linesReplaced(buffer);
            }

            /** {@inheritDoc} */
            @Override
            public void linesRemoved(@NotNull final List<Line> lines) {
                assert rs != null;
                assert buffer != null;
                rs.linesRemoved(buffer, lines);
            }
        };
        assert buffer != null;
        buffer.addBufferListener(bufferListener);
    }

}
