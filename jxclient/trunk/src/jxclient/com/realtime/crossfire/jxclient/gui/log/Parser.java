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

import java.awt.Color;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Parser for parsing drawextinfo messages received from a Crossfire server to
 * update a {@link Buffer} instance.
 * @author Andreas Kirschbaum
 */
public class Parser {

    /**
     * Maps font tag name to font instance.
     */
    @NotNull
    private static final Map<String, FontID> fonts = new HashMap<String, FontID>();

    static {
        fonts.put("print", FontID.PRINT);
        fonts.put("fixed", FontID.FIXED);
        fonts.put("arcane", FontID.ARCANE);
        fonts.put("hand", FontID.HAND);
        fonts.put("strange", FontID.STRANGE);
    }

    /**
     * Maps color tag name to color instance. The keys must be lower case.
     */
    @NotNull
    private static final Map<String, Color> colors = new HashMap<String, Color>();

    static {
        colors.put("black", Color.BLACK);
        colors.put("blue", Color.BLUE);
        colors.put("green", Color.GREEN);
        colors.put("red", Color.RED);
        colors.put("white", Color.WHITE);
    }

    /**
     * The pattern to split a string into words.
     */
    @NotNull
    private static final Pattern WORD_SEPARATOR_PATTERN = Pattern.compile(" ");

    /**
     * Pattern to match line breaks.
     */
    @NotNull
    private static final Pattern END_OF_LINE_PATTERN = Pattern.compile(" *\n");

    /**
     * Whether bold face is enabled.
     */
    private boolean bold = false;

    /**
     * Whether italic face is enabled.
     */
    private boolean italic = false;

    /**
     * Whether underlining is enabled.
     */
    private boolean underline = false;

    /**
     * The font to use.
     */
    private FontID font = FontID.PRINT;

    /**
     * The color to use. <code>null</code> means default color.
     */
    @Nullable
    private Color color = null;

    /**
     * The number of repetitions of the previously added line of text.
     */
    private int lastCount = 0;

    /**
     * The color of the previously added line of text.
     */
    @Nullable
    private Color lastColor = null;

    /**
     * The contents of the previously added line of text.
     */
    @NotNull
    private String lastText = "";

    /**
     * Parses a text message.
     * @param text the text message to parse
     * @param defaultColor the default color to use
     * @param buffer the buffer to update
     */
    public void parse(@NotNull final CharSequence text, @Nullable final Color defaultColor, @NotNull final Buffer buffer) {
        if (text.length() == 0) {
            return;
        }

        resetAttributes(defaultColor);
        for (final String line : END_OF_LINE_PATTERN.split(text, -1)) {
            parseLine(line, defaultColor, buffer);
        }
        buffer.prune();
    }

    /**
     * Parses a plain text message without media tags.
     * @param text the text message to parse
     * @param color the color to use
     * @param buffer the buffer to update
     */
    public void parseWithoutMediaTags(@NotNull final CharSequence text, @NotNull final Color color, @NotNull final Buffer buffer) {
        if (text.length() == 0) {
            return;
        }

        resetAttributes(color);
        for (final String line : END_OF_LINE_PATTERN.split(text, -1)) {
            parseLineWithoutMediaTags(line, buffer);
        }
        buffer.prune();
    }

    /**
     * Parses one text line.
     * @param text the text to process
     * @param defaultColor the default color to use
     * @param buffer the buffer instance to add to
     */
    private void parseLine(@NotNull final String text, @Nullable final Color defaultColor, @NotNull final Buffer buffer) {
        if (lastCount > 0 && text.equals(lastText) && lastColor != null && lastColor.equals(defaultColor)) {
            lastCount++;
            buffer.replaceLine(parseLine(text+" [["+lastCount+" times]", defaultColor));
        } else {
            lastCount = 1;
            lastText = text;
            lastColor = defaultColor;
            buffer.addLine(parseLine(text, defaultColor));
        }
    }

    /**
     * Parses one text line.
     * @param text the text to process
     * @param defaultColor the default color to use
     * @return the <code>Line</code> instance
     */
    @NotNull
    private Line parseLine(@NotNull final String text, @Nullable final Color defaultColor) {
        final Line line = new Line();

        int begin = 0;
        boolean active = false;
        final int imax = text.length();
        for (int i = 0; i < imax; i++) {
            final char ch = text.charAt(i);
            if (active) {
                if (ch == ']') {
                    processTag(text.substring(begin, i), defaultColor);
                    begin = i+1;
                    active = false;
                } else if (ch == '[' && i == begin) {
                    processText("[", line);
                    begin = i+1;
                    active = false;
                }
            } else {
                if (ch == '[') {
                    processText(text.substring(begin, i), line);
                    begin = i+1;
                    active = true;
                }
            }
        }
        if (!active) {
            processText(text.substring(begin, imax), line);
        }

        return line;
    }

    /**
     * Parses one text line of a plain text message without media tags.
     * @param text the text to process
     * @param buffer the buffer instance to add to
     */
    private void parseLineWithoutMediaTags(@NotNull final String text, @NotNull final Buffer buffer) {
        final Line line = new Line();
        if (lastCount > 0 && text.equals(lastText) && lastColor == null) {
            lastCount++;
            processText(text+" ["+lastCount+" times]", line);
            buffer.replaceLine(line);
        } else {
            lastCount = 1;
            lastText = text;
            lastColor = null;
            processText(text, line);
            buffer.addLine(line);
        }
    }

    /**
     * Resets all attributes to default values.
     * @param defaultColor the default color to use
     */
    private void resetAttributes(@Nullable final Color defaultColor) {
        bold = false;
        italic = false;
        underline = false;
        font = FontID.PRINT;
        color = defaultColor;
    }

    /**
     * Processes a tag.
     * @param tag the tag name to process. Leading and trailing brackets have
     * been removed
     * @param defaultColor the default color to use
     */
    private void processTag(@NotNull final String tag, @Nullable final Color defaultColor) {
        if (tag.length() == 0) {
            return;
        }

        if (tag.equals("b")) {
            bold = true;
        } else if (tag.equals("/b")) {
            bold = false;
        } else if (tag.equals("i")) {
            italic = true;
        } else if (tag.equals("/i")) {
            italic = false;
        } else if (tag.equals("ul")) {
            underline = true;
        } else if (tag.equals("/ul")) {
            underline = false;
        } else if (fonts.containsKey(tag)) {
            font = fonts.get(tag);
            assert font != null;
        } else if (tag.startsWith("color=")) {
            final String colorName = tag.substring(6).toLowerCase();
            if (colors.containsKey(colorName)) {
                color = colors.get(colorName);
                assert color != null;
            } else {
                // ignore unknown color
            }
        } else if (tag.equals("/color")) {
            color = defaultColor;
        } else {
            // ignore unknown tag
        }
    }

    /**
     * Processes one text segment.
     * @param text the text segment to process
     * @param line the line to add to
     */
    private void processText(@NotNull final String text, @NotNull final Line line) {
        if (text.length() == 0) {
            return;
        }

        final CharSequence newText;
        final Segment prevSegment = line.getLastSegment();
        if (prevSegment == null || !(prevSegment instanceof TextSegment)) {
            newText = text;
        } else {
            final TextSegment prevTextSegment = (TextSegment)prevSegment;
            if (prevTextSegment.matches(bold, italic, underline, font, color)) {
                newText = prevTextSegment.getText()+text;
                line.removeLastSegment();
            } else {
                newText = text;
            }
        }

        final String[] words = WORD_SEPARATOR_PATTERN.split(newText, -1);
        for (int i = 0; i < words.length-1; i++) {
            line.addSegment(words[i]+" ", bold, italic, underline, font, color);
        }
        if (words[words.length-1].length() > 0) {
            line.addSegment(words[words.length-1], bold, italic, underline, font, color);
        }
    }

    /**
     * Returns the string representation for a color.
     * @param color the color to convert
     * @return the string representation
     */
    @NotNull
    public static String toString(@NotNull final Color color) {
        // function need not be efficient since it is used for regression tests
        // only
        for (final Map.Entry<String, Color> e : colors.entrySet()) {
            if (e.getValue() == color) {
                return e.getKey();
            }
        }

        return "unknown";
    }

}
