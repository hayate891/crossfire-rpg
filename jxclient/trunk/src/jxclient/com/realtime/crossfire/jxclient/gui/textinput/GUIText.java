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
package com.realtime.crossfire.jxclient.gui.textinput;

import com.realtime.crossfire.jxclient.gui.ActivatableGUIElement;
import com.realtime.crossfire.jxclient.settings.CommandHistory;
import com.realtime.crossfire.jxclient.window.JXCWindow;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Toolkit;
import java.awt.Transparency;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.font.FontRenderContext;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.IOException;

/**
 * @author Lauwenmark
 * @author Andreas Kirschbaum
 */
public abstract class GUIText extends ActivatableGUIElement implements KeyListener
{
    /**
     * The serial version UID.
     */
    private static final long serialVersionUID = 1;

    /**
     * The number of characters to scroll left/right when the cursor would move
     * outside of the visible area.
     */
    private static final int SCROLL_CHARS = 8;

    /**
     * The associated {@link JXCWindow}.
     */
    private final JXCWindow window;

    /**
     * The command history for this text field.
     */
    private final CommandHistory commandHistory;

    private final BufferedImage activeImage;

    private final BufferedImage inactiveImage;

    /**
     * The clipboard for cut/copy/paste operations.
     */
    private final Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

    /**
     * The system selection for cut/copy/paste operations.
     */
    private final Clipboard selection = Toolkit.getDefaultToolkit().getSystemSelection();

    private final Font font;

    private final Color inactiveColor;

    private final Color activeColor;

    private final int margin;

    private final StringBuilder text;

    /**
     * The context used to determine character extents.
     */
    private final FontRenderContext fontRenderContext;

    /**
     * Whether UP and DOWN keys should be ignored. If unset, these keys cycle
     * through the history.
     */
    private final boolean ignoreUpDown;

    /**
     * If set, hide input; else show input.
     */
    private boolean hideInput = false;

    /**
     * The cursor location.
     */
    private int cursor;

    /**
     * The display offset: this many characters are hidden.
     */
    private int offset = 0;

    /**
     * Object used to synchronize on access to {@link #text}, {@link #cursor},
     * and {@link #offset}.
     */
    private final Object syncCursor = new Object();

    protected GUIText(final JXCWindow window, final String name, final int x, final int y, final int w, final int h, final BufferedImage activeImage, final BufferedImage inactiveImage, final Font font, final Color inactiveColor, final Color activeColor, final int margin, final String text, final boolean ignoreUpDown)
    {
        super(window, name, x, y, w, h, Transparency.TRANSLUCENT);
        if (2*margin >= w) throw new IllegalArgumentException("margin is too large");
        this.window = window;
        commandHistory = new CommandHistory(name);
        this.activeImage = activeImage;
        this.inactiveImage = inactiveImage;
        this.font = font;
        this.inactiveColor = inactiveColor;
        this.activeColor = activeColor;
        this.margin = margin;
        this.text = new StringBuilder(text);
        this.ignoreUpDown = ignoreUpDown;
        synchronized (bufferedImageSync)
        {
            final Graphics2D g = createBufferGraphics();
            try
            {
                fontRenderContext = g.getFontRenderContext();
            }
            finally
            {
                g.dispose();
            }
        }
        setCursor(this.text.length());
    }

    /** {@inheritDoc} */
    @Override
    public void dispose()
    {
        super.dispose();
    }

    public void setText(final String text)
    {
        this.text.setLength(0);
        this.text.append(text);
        setCursor(this.text.length());
    }

    public String getText()
    {
        return text.toString();
    }

    /** {@inheritDoc} */
    @Override
    protected void render(final Graphics g)
    {
        g.drawImage(isActive() ? activeImage : inactiveImage, 0, 0, null);
        g.setFont(font);
        final String tmp;
        final int y;
        synchronized (syncCursor)
        {
            tmp = getDisplayText();
            final Rectangle2D rect = font.getStringBounds(tmp, fontRenderContext);
            y = (int)Math.round((getHeight()-rect.getMaxY()-rect.getMinY()))/2;
            if (isActive())
            {
                final String tmpPrefix = tmp.substring(0, cursor-offset);
                final String tmpCursor = tmp.substring(0, cursor-offset+1);
                final Rectangle2D rectPrefix = font.getStringBounds(tmpPrefix, fontRenderContext);
                final Rectangle2D rectCursor = font.getStringBounds(tmpCursor, fontRenderContext);
                final int cursorX1 = (int)(rectPrefix.getWidth()+0.5);
                final int cursorX2 = (int)(rectCursor.getWidth()+0.5);
                g.setColor(inactiveColor);
                g.fillRect(margin+cursorX1, 0, cursorX2-cursorX1, getHeight());
            }
        }
        g.setColor(isActive() ? activeColor : inactiveColor);
        g.drawString(tmp, margin, y);
    }

    private String getDisplayText()
    {
        final String tmpText = text.substring(offset);
        if (!hideInput)
        {
            return tmpText+" ";
        }

        final String template = "********************";
        final String hiddenText = template.substring(0, Math.min(tmpText.length(), template.length()));
        return hiddenText+" ";
    }

    /** {@inheritDoc} */
    @Override
    public void mouseClicked(final MouseEvent e)
    {
        super.mouseClicked(e);
        final int b = e.getButton();
        switch (b)
        {
        case MouseEvent.BUTTON1:
            setActive(true);
            setChanged();
            break;

        case MouseEvent.BUTTON2:
            break;

        case MouseEvent.BUTTON3:
            break;
        }
    }

    /** {@inheritDoc} */
    @Override
    protected void activeChanged()
    {
        setChanged();
    }

    @Override
    public boolean keyPressed(final KeyEvent e)
    {
        switch (e.getKeyCode())
        {
        case KeyEvent.VK_BACK_SPACE:
            synchronized (syncCursor)
            {
                if (cursor > 0)
                {
                    text.delete(cursor-1, cursor);
                    setCursor(cursor-1);
                }
            }
            return true;

        case KeyEvent.VK_DELETE:
            synchronized (syncCursor)
            {
                if (cursor < text.length())
                {
                    text.delete(cursor, cursor+1);
                    setChanged();
                }
            }
            return true;

        case KeyEvent.VK_KP_LEFT:
        case KeyEvent.VK_LEFT:
            synchronized (syncCursor)
            {
                if (cursor > 0)
                {
                    setCursor(cursor-1);
                }
            }
            return true;

        case KeyEvent.VK_KP_RIGHT:
        case KeyEvent.VK_RIGHT:
            synchronized (syncCursor)
            {
                if (cursor < text.length())
                {
                    setCursor(cursor+1);
                }
            }
            return true;

        case KeyEvent.VK_KP_UP:
        case KeyEvent.VK_UP:
            if (!ignoreUpDown)
            {
                historyPrev();
                return true;
            }
            break;

        case KeyEvent.VK_KP_DOWN:
        case KeyEvent.VK_DOWN:
            if (!ignoreUpDown)
            {
                historyNext();
                return true;
            }
            break;

        case KeyEvent.VK_HOME:
            synchronized (syncCursor)
            {
                if (cursor > 0)
                {
                    setCursor(0);
                }
            }
            return true;

        case KeyEvent.VK_END:
            synchronized (syncCursor)
            {
                if (cursor < text.length())
                {
                    setCursor(text.length());
                }
            }
            return true;
        }

        return false;
    }

    /**
     * Activate the previous command from the command history.
     */
    private void historyPrev()
    {
        final String commandUp = commandHistory.up();
        if (commandUp != null)
        {
            setText(commandUp);
        }
    }

    /**
     * Activate the next command from the command history.
     */
    private void historyNext()
    {
        final String commandDown = commandHistory.down();
        setText(commandDown != null ? commandDown : "");
    }

    /** {@inheritDoc} */
    @Override
    public boolean keyReleased(final KeyEvent e)
    {
        return false;
    }

    /** {@inheritDoc} */
    @Override
    public boolean keyTyped(final KeyEvent e)
    {
        final char ch = e.getKeyChar();
        switch (ch)
        {
        case '\r':
        case '\n':
            final String command = text.toString();
            window.updatePlayerName(command);
            execute((JXCWindow)e.getSource(), command);
            if (!hideInput)
            {
                commandHistory.addCommand(command);
            }
            setActive(false);
            return true;

        case 0x0e:              // CTRL-N
            historyNext();
            return true;

        case 0x10:              // CTRL-P
            historyPrev();
            return true;

        case 0x16:              // CTRL-V
            paste();
            return true;

        default:
            if (ch != KeyEvent.CHAR_UNDEFINED && ch != (char)127 && ch >= ' ')
            {
                insertChar(ch);
                return true;
            }
            break;
        }

        return false;
    }

    /**
     * Inserts a character at the cursort position.
     * @param ch the character
     */
    public void insertChar(final char ch)
    {
        synchronized (syncCursor)
        {
            text.insert(cursor, ch);
            setCursor(cursor+1);
        }
    }

    /**
     * Inserts a string at the cursort position.
     * @param str the string
     */
    public void insertString(final String str)
    {
        synchronized (syncCursor)
        {
            text.insert(cursor, str);
            setCursor(cursor+str.length());
        }
    }

    /**
     * Will be called to execute the entered command.
     *
     * @param window The JXCWindow instance.
     *
     * @param command The entered command.
     */
    protected abstract void execute(final JXCWindow window, final String command);

    /**
     * Enable or disable hidden text.
     *
     * @param hideInput If set, hide input; else show input.
     */
    public void setHideInput(final boolean hideInput)
    {
        if (this.hideInput != hideInput)
        {
            this.hideInput = hideInput;
            setChanged();
        }
    }

    /**
     * Set the cursor position. Make sure the cursor position is visible.
     *
     * @param cursor The new cursor position.
     */
    private void setCursor(final int cursor)
    {
        synchronized (syncCursor)
        {
            if (this.cursor < cursor)
            {
                // cursor moved right

                for (;;)
                {
                    final String tmp = getDisplayText();
                    final String tmpCursor = tmp.substring(0, cursor-offset+1);
                    final Rectangle2D rectCursor = font.getStringBounds(tmpCursor, fontRenderContext);
                    final int cursorX = (int)(rectCursor.getWidth()+0.5);
                    if (cursorX < getWidth())
                    {
                        break;
                    }

                    if (offset+SCROLL_CHARS <= cursor)
                    {
                        offset += SCROLL_CHARS;
                    }
                    else
                    {
                        offset = cursor;
                    }
                }
            }
            else if (this.cursor > cursor)
            {
                // cursor moved left

                while (cursor < offset)
                {
                    if (offset <= SCROLL_CHARS)
                    {
                        offset = 0;
                        break;
                    }

                    offset -= SCROLL_CHARS;
                }
            }

            this.cursor = cursor;
        }
        setChanged();
    }

    /**
     * Perform a "paste" operation from the system clipboard.
     */
    public void paste()
    {
        Transferable content = null;
        if (selection != null)
        {
            content = selection.getContents(this);
        }
        if (content == null)
        {
            content = clipboard.getContents(this);
        }
        if (content == null)
        {
            return;
        }

        final String str;
        try
        {
            str = (String)content.getTransferData(DataFlavor.stringFlavor);
        }
        catch (final UnsupportedFlavorException ex)
        {
            return;
        }
        catch (final IOException ex)
        {
            return;
        }
        insertString(str);
    }
}
