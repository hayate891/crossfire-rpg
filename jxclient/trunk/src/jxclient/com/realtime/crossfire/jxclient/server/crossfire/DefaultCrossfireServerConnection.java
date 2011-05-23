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

package com.realtime.crossfire.jxclient.server.crossfire;

import com.realtime.crossfire.jxclient.server.crossfire.messages.Map2;
import com.realtime.crossfire.jxclient.server.crossfire.messages.UpdItem;
import com.realtime.crossfire.jxclient.server.server.DefaultServerConnection;
import com.realtime.crossfire.jxclient.server.server.ReceivedPacketListener;
import com.realtime.crossfire.jxclient.server.socket.ClientSocketListener;
import com.realtime.crossfire.jxclient.server.socket.ClientSocketState;
import com.realtime.crossfire.jxclient.server.socket.UnknownCommandException;
import com.realtime.crossfire.jxclient.util.DebugWriter;
import com.realtime.crossfire.jxclient.util.HexCodec;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.regex.Pattern;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Default implementation of {@link CrossfireServerConnection}.
 * @author Andreas Kirschbaum
 */
public class DefaultCrossfireServerConnection extends DefaultServerConnection implements CrossfireServerConnection {

    /**
     * The default map width when no "setup mapsize" command has been sent.
     */
    private static final int DEFAULT_MAP_WIDTH = 11;

    /**
     * The default map height when no "setup mapsize" command has been sent.
     */
    private static final int DEFAULT_MAP_HEIGHT = 11;

    /**
     * The default number of ground objects when no "setup num_look_objects"
     * command has been sent.
     */
    private static final int DEFAULT_NUM_LOOK_OBJECTS = 50;

    /**
     * Pattern to split a string by ":".
     */
    @NotNull
    private static final Pattern PATTERN_DOT = Pattern.compile(":");

    /**
     * The {@link Charset} used for parsing or encoding strings received from or
     * sent to the Crossfire server.
     */
    @NotNull
    private static final Charset UTF8 = Charset.forName("UTF-8");

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_NAME = 1;

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_CLASS = 2;

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_RACE = 3;

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_LEVEL = 4;

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_FACE = 5;

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_PARTY = 6;

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_MAP = 7;

    /**
     * Parameter type in the "accountplayers" command.
     */
    private static final int ACL_FACE_NUM = 8;

    /**
     * Bitmask in magic map information to extract the color information of a
     * tile.
     */
    private static final int FACE_COLOR_MASK = 0x0F;

    /**
     * Bitmask in magic map information to denote a floor tile.
     */
    private static final int FACE_FLOOR = 0x80;

    /**
     * Bitmask in magic map information to denote a wall tile.
     */
    private static final int FACE_WALL = 0x40;

    /**
     * The map width in tiles that is negotiated with the server.
     */
    private int preferredMapWidth = 17;

    /**
     * The map height in tiles that is negotiated with the server.
     */
    private int preferredMapHeight = 13;

    /**
     * The map width that is being negotiated with the server. Set to
     * <code>0</code> when not negotiating.
     */
    private int pendingMapWidth = 0;

    /**
     * The map height that is being negotiated with the server. Set to
     * <code>0</code> when not negotiating.
     */
    private int pendingMapHeight = 0;

    /**
     * The currently active map width.
     */
    private int currentMapWidth = DEFAULT_MAP_WIDTH;

    /**
     * The currently active map height.
     */
    private int currentMapHeight = DEFAULT_MAP_HEIGHT;

    /**
     * The number of ground view objects to be negotiated with the server.
     */
    private int preferredNumLookObjects = DEFAULT_NUM_LOOK_OBJECTS;

    /**
     * The number of ground view objects being negotiated with the server. Set
     * to <code>0</code> when not negotiating.
     */
    private int pendingNumLookObjects = 0;

    /**
     * The currently active number of ground view objects.
     */
    private int currentNumLookObjects = 0;

    /**
     * The {@link CrossfireServerConnectionListener}s to notify.
     */
    @NotNull
    private final Collection<CrossfireServerConnectionListener> crossfireServerConnectionListeners = new CopyOnWriteArrayList<CrossfireServerConnectionListener>();

    /**
     * The {@link CrossfireDrawinfoListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireDrawinfoListener> drawinfoListeners = new CopyOnWriteArrayList<CrossfireDrawinfoListener>();

    /**
     * The {@link CrossfireDrawextinfoListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireDrawextinfoListener> drawextinfoListeners = new CopyOnWriteArrayList<CrossfireDrawextinfoListener>();

    /**
     * The {@link CrossfireQueryListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireQueryListener> queryListeners = new CopyOnWriteArrayList<CrossfireQueryListener>();

    /**
     * The {@link CrossfireMagicmapListener}s to be notified of received
     * magicmap commands.
     */
    @NotNull
    private final Collection<CrossfireMagicmapListener> magicmapListeners = new CopyOnWriteArrayList<CrossfireMagicmapListener>();

    /**
     * The {@link CrossfireUpdateFaceListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireUpdateFaceListener> crossfireUpdateFaceListeners = new CopyOnWriteArrayList<CrossfireUpdateFaceListener>();

    /**
     * The {@link CrossfireStatsListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireStatsListener> crossfireStatsListeners = new CopyOnWriteArrayList<CrossfireStatsListener>();

    /**
     * The {@link CrossfireUpdateItemListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireUpdateItemListener> crossfireUpdateItemListeners = new CopyOnWriteArrayList<CrossfireUpdateItemListener>();

    /**
     * The {@link CrossfireUpdateMapListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireUpdateMapListener> crossfireUpdateMapListeners = new CopyOnWriteArrayList<CrossfireUpdateMapListener>();

    /**
     * The {@link CrossfireTickListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireTickListener> crossfireTickListeners = new CopyOnWriteArrayList<CrossfireTickListener>();

    /**
     * The {@link CrossfireSoundListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireSoundListener> crossfireSoundListeners = new CopyOnWriteArrayList<CrossfireSoundListener>();

    /**
     * The {@link CrossfireMusicListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireMusicListener> crossfireMusicListeners = new CopyOnWriteArrayList<CrossfireMusicListener>();

    /**
     * The {@link CrossfireComcListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireComcListener> crossfireComcListeners = new CopyOnWriteArrayList<CrossfireComcListener>();

    /**
     * The {@link CrossfireFaceListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireFaceListener> crossfireFaceListeners = new CopyOnWriteArrayList<CrossfireFaceListener>();

    /**
     * The {@link CrossfireSpellListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireSpellListener> crossfireSpellListeners = new CopyOnWriteArrayList<CrossfireSpellListener>();

    /**
     * The {@link ReceivedPacketListener}s to be notified.
     */
    @NotNull
    private final Collection<ReceivedPacketListener> receivedPacketListeners = new CopyOnWriteArrayList<ReceivedPacketListener>();

    /**
     * The {@link SentReplyListener}s to be notified.
     */
    @NotNull
    private final Collection<SentReplyListener> sentReplyListeners = new CopyOnWriteArrayList<SentReplyListener>();

    /**
     * The {@link CrossfireExpTableListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireExpTableListener> crossfireExpTableListeners = new CopyOnWriteArrayList<CrossfireExpTableListener>();

    /**
     * The {@link CrossfireSkillInfoListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireSkillInfoListener> crossfireSkillInfoListeners = new CopyOnWriteArrayList<CrossfireSkillInfoListener>();

    /**
     * The {@link CrossfirePickupListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfirePickupListener> crossfirePickupListeners = new CopyOnWriteArrayList<CrossfirePickupListener>();

    /**
     * The {@link CrossfireAccountListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireAccountListener> crossfireAccountListeners = new CopyOnWriteArrayList<CrossfireAccountListener>();

    /**
     * The {@link CrossfireFailureListener}s to be notified.
     */
    @NotNull
    private final Collection<CrossfireFailureListener> crossfireFailureListeners = new CopyOnWriteArrayList<CrossfireFailureListener>();

    /**
     * Buffer to build commands to send. It is shared between all sendXxx()
     * functions. It is used to synchronize these functions.
     */
    @NotNull
    private final byte[] writeBuffer = new byte[65536];

    /**
     * A byte buffer using {@link #writeBuffer} to store the data.
     */
    @NotNull
    private final ByteBuffer byteBuffer = ByteBuffer.wrap(writeBuffer);

    /**
     * The packet id for the next "ncom" command to send.
     */
    private int packet = 1;

    /**
     * The command prefix for the "accountlogin" command.
     */
    @NotNull
    private static final byte[] ACCOUNT_LOGIN_PREFIX = {
        'a',
        'c',
        'c',
        'o',
        'u',
        'n',
        't',
        'l',
        'o',
        'g',
        'i',
        'n',
        ' ',
    };

    /**
     * The command prefix for the "accountplay" command.
     */
    @NotNull
    private static final byte[] ACCOUNT_PLAY_PREFIX = {
        'a',
        'c',
        'c',
        'o',
        'u',
        'n',
        't',
        'p',
        'l',
        'a',
        'y',
        ' ',
    };

    /**
     * The command prefix for the "accountaddplayer" command.
     */
    @NotNull
    private static final byte[] ACCOUNT_ADD_PLAYER_PREFIX = {
        'a',
        'c',
        'c',
        'o',
        'u',
        'n',
        't',
        'a',
        'd',
        'd',
        'p',
        'l',
        'a',
        'y',
        'e',
        'r',
        ' ',
    };

    /**
     * The command prefix for the "accountnew" command.
     */
    @NotNull
    private static final byte[] ACCOUNT_NEW_PREFIX = {
        'a',
        'c',
        'c',
        'o',
        'u',
        'n',
        't',
        'n',
        'e',
        'w',
        ' ',
    };

    /**
     * The command prefix for the "accountpw" command.
     */
    @NotNull
    private static final byte[] ACCOUNT_PASSWORD_PREFIX = {
        'a',
        'c',
        'c',
        'o',
        'u',
        'n',
        't',
        'p',
        'w',
        ' ',
    };

    /**
     * The command prefix for the "createplayer" command.
     */
    @NotNull
    private static final byte[] CREATE_PLAYER_PREFIX = {
        'c',
        'r',
        'e',
        'a',
        't',
        'e',
        'p',
        'l',
        'a',
        'y',
        'e',
        'r',
        ' ',
    };

    /**
     * The command prefix for the "addme" command.
     */
    @NotNull
    private static final byte[] ADDME_PREFIX = {
        'a',
        'd',
        'd',
        'm',
        'e',
        ' ',
    };

    /**
     * The command prefix for the "apply" command.
     */
    @NotNull
    private static final byte[] APPLY_PREFIX = {
        'a',
        'p',
        'p',
        'l',
        'y',
        ' ',
    };

    /**
     * The command prefix for the "askface" command.
     */
    @NotNull
    private static final byte[] ASKFACE_PREFIX = {
        'a',
        's',
        'k',
        'f',
        'a',
        'c',
        'e',
        ' ',
    };

    /**
     * The command prefix for the "examine" command.
     */
    @NotNull
    private static final byte[] EXAMINE_PREFIX = {
        'e',
        'x',
        'a',
        'm',
        'i',
        'n',
        'e',
        ' ',
    };

    /**
     * The command prefix for the "lock" command.
     */
    @NotNull
    private static final byte[] LOCK_PREFIX = {
        'l',
        'o',
        'c',
        'k',
        ' ',
    };

    /**
     * The command prefix for the "lookat" command.
     */
    @NotNull
    private static final byte[] LOOKAT_PREFIX = {
        'l',
        'o',
        'o',
        'k',
        'a',
        't',
        ' ',
    };

    /**
     * The command prefix for the "mark" command.
     */
    @NotNull
    private static final byte[] MARK_PREFIX = {
        'm',
        'a',
        'r',
        'k',
        ' ',
    };

    /**
     * The command prefix for the "move" command.
     */
    @NotNull
    private static final byte[] MOVE_PREFIX = {
        'm',
        'o',
        'v',
        'e',
        ' ',
    };

    /**
     * The command prefix for the "ncom" command.
     */
    @NotNull
    private static final byte[] NCOM_PREFIX = {
        'n',
        'c',
        'o',
        'm',
        ' ',
    };

    /**
     * The command prefix for the "reply" command.
     */
    @NotNull
    private static final byte[] REPLY_PREFIX = {
        'r',
        'e',
        'p',
        'l',
        'y',
        ' ',
    };

    /**
     * The command prefix for the "requestinfo" command.
     */
    @NotNull
    private static final byte[] REQUESTINFO_PREFIX = {
        'r',
        'e',
        'q',
        'u',
        'e',
        's',
        't',
        'i',
        'n',
        'f',
        'o',
        ' ',
    };

    /**
     * The command prefix for the "setup" command.
     */
    @NotNull
    private static final byte[] SETUP_PREFIX = {
        's',
        'e',
        't',
        'u',
        'p',
    }; // note that this command does not have a trailing space

    /**
     * The command prefix for the "toggleextendedtext" command.
     */
    @NotNull
    private static final byte[] TOGGLEEXTENDEDTEXT_PREFIX = {
        't',
        'o',
        'g',
        'g',
        'l',
        'e',
        'e',
        'x',
        't',
        'e',
        'n',
        'd',
        'e',
        'd',
        't',
        'e',
        'x',
        't',
    }; // note that this command does not have a trailing space

    /**
     * The command prefix for the "version" command.
     */
    @NotNull
    private static final byte[] VERSION_PREFIX = {
        'v',
        'e',
        'r',
        's',
        'i',
        'o',
        'n',
        ' ',
    };

    /**
     * The semaphore used to synchronized map model updates and map view
     * redraws.
     */
    @NotNull
    private final Object redrawSemaphore;

    /**
     * The version information to send when connecting to the server.
     */
    @NotNull
    private final String version;

    /**
     * The appender to write protocol commands to. May be <code>null</code> to
     * not write anything.
     */
    @Nullable
    private final DebugWriter debugProtocol;

    /**
     * The current connection state.
     */
    @NotNull
    private ClientSocketState clientSocketState = ClientSocketState.CONNECTING;

    /**
     * The account name. Set to <code>null</code> if no account name is known.
     */
    @Nullable
    private String accountName = null;

    /**
     * The {@link ClientSocketListener} attached to the server socket.
     */
    @NotNull
    private final ClientSocketListener clientSocketListener = new ClientSocketListener() {
        /** {@inheritDoc} */
        @Override
        public void connecting() {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void connected() {
            DefaultCrossfireServerConnection.this.connected();
        }

        /** {@inheritDoc} */
        @Override
        public void packetReceived(@NotNull final ByteBuffer packet) throws UnknownCommandException {
            processPacket(packet);
        }

        /** {@inheritDoc} */
        @Override
        public void packetSent(@NotNull final byte[] buf, final int len) {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void disconnecting(@NotNull final String reason, final boolean isError) {
            // ignore
        }

        /** {@inheritDoc} */
        @Override
        public void disconnected(@NotNull final String reason) {
            // ignore
        }
    };

    /**
     * The login method version supported by the server we're connected to.
     */
    private int loginMethod = 0;

    /**
     * Creates a new instance.
     * @param redrawSemaphore the semaphore used to synchronized map model
     * updates and map view redraws
     * @param debugProtocol tf non-<code>null</code>, write all protocol
     * commands to this writer
     * @param version the version information to send to the server when
     * connecting
     * @throws IOException if an internal error occurs
     */
    public DefaultCrossfireServerConnection(@NotNull final Object redrawSemaphore, @Nullable final DebugWriter debugProtocol, @NotNull final String version) throws IOException {
        super(debugProtocol);
        this.redrawSemaphore = redrawSemaphore;
        this.version = version;
        byteBuffer.order(ByteOrder.BIG_ENDIAN);
        this.debugProtocol = debugProtocol;
        addClientSocketListener(clientSocketListener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireServerConnectionListener(@NotNull final CrossfireServerConnectionListener listener) {
        crossfireServerConnectionListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireDrawinfoListener(@NotNull final CrossfireDrawinfoListener listener) {
        drawinfoListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireDrawinfoListener(@NotNull final CrossfireDrawinfoListener listener) {
        drawinfoListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireDrawextinfoListener(@NotNull final CrossfireDrawextinfoListener listener) {
        drawextinfoListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireDrawextinfoListener(@NotNull final CrossfireDrawextinfoListener listener) {
        drawextinfoListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireQueryListener(@NotNull final CrossfireQueryListener listener) {
        queryListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireQueryListener(@NotNull final CrossfireQueryListener listener) {
        queryListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireMagicmapListener(@NotNull final CrossfireMagicmapListener listener) {
        magicmapListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireMagicmapListener(@NotNull final CrossfireMagicmapListener listener) {
        magicmapListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireUpdateFaceListener(@NotNull final CrossfireUpdateFaceListener listener) {
        crossfireUpdateFaceListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireStatsListener(@NotNull final CrossfireStatsListener crossfireStatsListener) {
        crossfireStatsListeners.add(crossfireStatsListener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireUpdateItemListener(@NotNull final CrossfireUpdateItemListener crossfireUpdateItemListener) {
        crossfireUpdateItemListeners.add(crossfireUpdateItemListener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireUpdateItemListener(@NotNull final CrossfireUpdateItemListener crossfireUpdateItemListener) {
        crossfireUpdateItemListeners.remove(crossfireUpdateItemListener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireUpdateMapListener(@NotNull final CrossfireUpdateMapListener listener) {
        crossfireUpdateMapListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireTickListener(@NotNull final CrossfireTickListener listener) {
        crossfireTickListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireSoundListener(@NotNull final CrossfireSoundListener listener) {
        crossfireSoundListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireMusicListener(@NotNull final CrossfireMusicListener listener) {
        crossfireMusicListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireComcListener(@NotNull final CrossfireComcListener listener) {
        crossfireComcListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireFaceListener(@NotNull final CrossfireFaceListener listener) {
        crossfireFaceListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireSpellListener(@NotNull final CrossfireSpellListener listener) {
        crossfireSpellListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addPacketWatcherListener(@NotNull final ReceivedPacketListener listener) {
        receivedPacketListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireAccountListener(@NotNull final CrossfireAccountListener listener) {
        crossfireAccountListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireAccountListener(@NotNull final CrossfireAccountListener listener) {
        crossfireAccountListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removePacketWatcherListener(@NotNull final ReceivedPacketListener listener) {
        receivedPacketListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addSentReplyListener(@NotNull final SentReplyListener listener) {
        sentReplyListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeSentReplyListener(@NotNull final SentReplyListener listener) {
        sentReplyListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireExpTableListener(@NotNull final CrossfireExpTableListener crossfireExpTableListener) {
        crossfireExpTableListeners.add(crossfireExpTableListener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireSkillInfoListener(@NotNull final CrossfireSkillInfoListener listener) {
        crossfireSkillInfoListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireSkillInfoListener(@NotNull final CrossfireSkillInfoListener listener) {
        crossfireSkillInfoListeners.remove(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfirePickupListener(@NotNull final CrossfirePickupListener listener) {
        crossfirePickupListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfirePickupListener(@NotNull final CrossfirePickupListener listener) {
        crossfirePickupListeners.remove(listener);
    }

    /**
     * Called after the server connection has been established.
     */
    private void connected() {
        pendingMapWidth = 0;
        pendingMapHeight = 0;
        pendingNumLookObjects = 0;
        setCurrentMapSize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT);
        currentNumLookObjects = DEFAULT_NUM_LOOK_OBJECTS;

        fireNewMap();

        setClientSocketState(ClientSocketState.CONNECTING, ClientSocketState.VERSION);
        sendVersion(1023, 1027, version);
    }

    /**
     * Processes a received packet. This function does not avoid buffer
     * underflow exceptions when reading data from the packet. Instead, a
     * <code>try...catch</code> clause is used to detect invalid packets.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet cannot be parsed
     */
    private void processPacket(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        try {
            packet.mark();
            switch (packet.get()) {
            case 'a':
                switch (packet.get()) {
                case 'c':
                    //accountplayers
                    if (packet.get() != 'c') {
                        break;
                    }
                    if (packet.get() != 'o') {
                        break;
                    }
                    if (packet.get() != 'u') {
                        break;
                    }
                    if (packet.get() != 'n') {
                        break;
                    }
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != 'p') {
                        break;
                    }
                    if (packet.get() != 'l') {
                        break;
                    }
                    if (packet.get() != 'a') {
                        break;
                    }
                    if (packet.get() != 'y') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 'r') {
                        break;
                    }
                    if (packet.get() != 's') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers");
                    }
                    processAccountPlayers(packet);
                    return;

                case 'd':
                    if (packet.get() != 'd') {
                        break;
                    }
                    switch (packet.get()) {
                    case 'm':
                        if (packet.get() != 'e') {
                            break;
                        }
                        if (packet.get() != '_') {
                            break;
                        }
                        switch (packet.get()) {
                        case 'f':
                            if (packet.get() != 'a') {
                                break;
                            }
                            if (packet.get() != 'i') {
                                break;
                            }
                            if (packet.get() != 'l') {
                                break;
                            }
                            if (packet.get() != 'e') {
                                break;
                            }
                            if (packet.get() != 'd') {
                                break;
                            }
                            if (packet.hasRemaining()) {
                                break;
                            }
                            if (debugProtocol != null) {
                                debugProtocol.debugProtocolWrite("recv addme_failed");
                            }
                            processAddmeFailed(packet);
                            return;

                        case 's':
                            if (packet.get() != 'u') {
                                break;
                            }
                            if (packet.get() != 'c') {
                                break;
                            }
                            if (packet.get() != 'c') {
                                break;
                            }
                            if (packet.get() != 'e') {
                                break;
                            }
                            if (packet.get() != 's') {
                                break;
                            }
                            if (packet.get() != 's') {
                                break;
                            }
                            if (packet.hasRemaining()) {
                                break;
                            }
                            if (debugProtocol != null) {
                                debugProtocol.debugProtocolWrite("recv addme_success");
                            }
                            processAddmeSuccess(packet);
                            return;
                        }
                        break;

                    case 's':
                        if (packet.get() != 'p') {
                            break;
                        }
                        if (packet.get() != 'e') {
                            break;
                        }
                        if (packet.get() != 'l') {
                            break;
                        }
                        if (packet.get() != 'l') {
                            break;
                        }
                        if (packet.get() != ' ') {
                            break;
                        }
                        processAddSpell(packet);
                        return;
                    }
                    break;

                case 'n':
                    if (packet.get() != 'i') {
                        break;
                    }
                    if (packet.get() != 'm') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processAnim(packet);
                    return;
                }
                break;

            case 'c':
                if (packet.get() != 'o') {
                    break;
                }
                if (packet.get() != 'm') {
                    break;
                }
                if (packet.get() != 'c') {
                    break;
                }
                if (packet.get() != ' ') {
                    break;
                }
                processComc(packet);
                return;

            case 'd':
                switch (packet.get()) {
                case 'e':
                    if (packet.get() != 'l') {
                        break;
                    }
                    switch (packet.get()) {
                    case 'i':
                        switch (packet.get()) {
                        case 'n':
                            if (packet.get() != 'v') {
                                break;
                            }
                            if (packet.get() != ' ') {
                                break;
                            }
                            processDelInv(packet);
                            return;

                        case 't':
                            if (packet.get() != 'e') {
                                break;
                            }
                            if (packet.get() != 'm') {
                                break;
                            }
                            if (packet.get() != ' ') {
                                break;
                            }
                            processDelItem(packet);
                            return;
                        }
                        break;

                    case 's':
                        if (packet.get() != 'p') {
                            break;
                        }
                        if (packet.get() != 'e') {
                            break;
                        }
                        if (packet.get() != 'l') {
                            break;
                        }
                        if (packet.get() != 'l') {
                            break;
                        }
                        if (packet.get() != ' ') {
                            break;
                        }
                        processDelSpell(packet);
                        return;
                    }
                    break;

                case 'r':
                    if (packet.get() != 'a') {
                        break;
                    }
                    if (packet.get() != 'w') {
                        break;
                    }
                    switch (packet.get()) {
                    case 'e':
                        if (packet.get() != 'x') {
                            break;
                        }
                        if (packet.get() != 't') {
                            break;
                        }
                        if (packet.get() != 'i') {
                            break;
                        }
                        if (packet.get() != 'n') {
                            break;
                        }
                        if (packet.get() != 'f') {
                            break;
                        }
                        if (packet.get() != 'o') {
                            break;
                        }
                        if (packet.get() != ' ') {
                            break;
                        }
                        processDrawExtInfo(packet);
                        return;

                    case 'i':
                        if (packet.get() != 'n') {
                            break;
                        }
                        if (packet.get() != 'f') {
                            break;
                        }
                        if (packet.get() != 'o') {
                            break;
                        }
                        if (packet.get() != ' ') {
                            break;
                        }
                        processDrawInfo(packet);
                        return;
                    }
                    break;
                }
                break;

            case 'E':
                if (packet.get() != 'x') {
                    break;
                }
                if (packet.get() != 't') {
                    break;
                }
                if (packet.get() != 'e') {
                    break;
                }
                if (packet.get() != 'n') {
                    break;
                }
                if (packet.get() != 'd') {
                    break;
                }
                if (packet.get() != 'e') {
                    break;
                }
                if (packet.get() != 'd') {
                    break;
                }
                switch (packet.get()) {
                case 'I':
                    if (packet.get() != 'n') {
                        break;
                    }
                    if (packet.get() != 'f') {
                        break;
                    }
                    if (packet.get() != 'o') {
                        break;
                    }
                    if (packet.get() != 'S') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processExtendedInfoSet(packet);
                    return;

                case 'T':
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 'x') {
                        break;
                    }
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != 'S') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processExtendedTextSet(packet);
                    return;
                }
                break;

            case 'f':
                if (packet.get() != 'a') {
                    break;
                }
                switch (packet.get()) {
                case 'c':
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != '2') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processFace2(packet);
                    return;

                case 'i':
                    if (packet.get() != 'l') {
                        break;
                    }
                    if (packet.get() != 'u') {
                        break;
                    }
                    if (packet.get() != 'r') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processFailure(packet);
                    return;
                }

            case 'g':
                if (packet.get() != 'o') {
                    break;
                }
                if (packet.get() != 'o') {
                    break;
                }
                if (packet.get() != 'd') {
                    break;
                }
                if (packet.get() != 'b') {
                    break;
                }
                if (packet.get() != 'y') {
                    break;
                }
                if (packet.get() != 'e') {
                    break;
                }
                if (packet.get() != ' ') {
                    break;
                }
                processGoodbye(packet);
                return;

            case 'i':
                switch (packet.get()) {
                case 'm':
                    if (packet.get() != 'a') {
                        break;
                    }
                    if (packet.get() != 'g') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != '2') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processImage2(packet);
                    return;

                case 't':
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 'm') {
                        break;
                    }
                    if (packet.get() != '2') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processItem2(packet);
                    return;
                }
                break;

            case 'm':
                switch (packet.get()) {
                case 'a':
                    switch (packet.get()) {
                    case 'g':
                        if (packet.get() != 'i') {
                            break;
                        }
                        if (packet.get() != 'c') {
                            break;
                        }
                        if (packet.get() != 'm') {
                            break;
                        }
                        if (packet.get() != 'a') {
                            break;
                        }
                        if (packet.get() != 'p') {
                            break;
                        }
                        if (packet.get() != ' ') {
                            break;
                        }
                        processMagicMap(packet);
                        return;

                    case 'p':
                        switch (packet.get()) {
                        case '2':
                            if (packet.get() != ' ') {
                                break;
                            }
                            processMap2(packet);
                            return;

                        case 'e':
                            if (packet.get() != 'x') {
                                break;
                            }
                            if (packet.get() != 't') {
                                break;
                            }
                            if (packet.get() != 'e') {
                                break;
                            }
                            if (packet.get() != 'n') {
                                break;
                            }
                            if (packet.get() != 'd') {
                                break;
                            }
                            if (packet.get() != 'e') {
                                break;
                            }
                            if (packet.get() != 'd') {
                                break;
                            }
                            if (packet.get() != ' ') {
                                break;
                            }
                            processMapExtended(packet);
                            return;
                        }
                        break;
                    }
                    break;

                case 'u':
                    if (packet.get() != 's') {
                        break;
                    }
                    if (packet.get() != 'i') {
                        break;
                    }
                    if (packet.get() != 'c') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processMusic(packet);
                    return;
                }
                break;

            case 'n':
                if (packet.get() != 'e') {
                    break;
                }
                if (packet.get() != 'w') {
                    break;
                }
                if (packet.get() != 'm') {
                    break;
                }
                if (packet.get() != 'a') {
                    break;
                }
                if (packet.get() != 'p') {
                    break;
                }
                processNewMap(packet);
                return;

            case 'p':
                switch (packet.get()) {
                case 'i':
                    if (packet.get() != 'c') {
                        break;
                    }
                    if (packet.get() != 'k') {
                        break;
                    }
                    if (packet.get() != 'u') {
                        break;
                    }
                    if (packet.get() != 'p') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processPickup(packet);
                    return;

                case 'l':
                    if (packet.get() != 'a') {
                        break;
                    }
                    if (packet.get() != 'y') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 'r') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processPlayer(packet);
                    return;
                }
                break;

            case 'q':
                if (packet.get() != 'u') {
                    break;
                }
                if (packet.get() != 'e') {
                    break;
                }
                if (packet.get() != 'r') {
                    break;
                }
                if (packet.get() != 'y') {
                    break;
                }
                if (packet.get() != ' ') {
                    break;
                }
                processQuery(packet);
                return;

            case 'r':
                if (packet.get() != 'e') {
                    break;
                }
                if (packet.get() != 'p') {
                    break;
                }
                if (packet.get() != 'l') {
                    break;
                }
                if (packet.get() != 'y') {
                    break;
                }
                if (packet.get() != 'i') {
                    break;
                }
                if (packet.get() != 'n') {
                    break;
                }
                if (packet.get() != 'f') {
                    break;
                }
                if (packet.get() != 'o') {
                    break;
                }
                if (packet.get() != ' ') {
                    break;
                }
                processReplyInfo(packet);
                return;

            case 's':
                switch (packet.get()) {
                case 'e':
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != 'u') {
                        break;
                    }
                    if (packet.get() != 'p') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processSetup(packet);
                    return;

                case 'm':
                    if (packet.get() != 'o') {
                        break;
                    }
                    if (packet.get() != 'o') {
                        break;
                    }
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != 'h') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processSmooth(packet);
                    return;

                case 'o':
                    if (packet.get() != 'u') {
                        break;
                    }
                    if (packet.get() != 'n') {
                        break;
                    }
                    if (packet.get() != 'd') {
                        break;
                    }
                    switch (packet.get()) {
                    case ' ':
                        processSound(packet);
                        return;

                    case '2':
                        if (packet.get() != ' ') {
                            break;
                        }
                        processSound2(packet);
                        return;
                    }
                    break;

                case 't':
                    if (packet.get() != 'a') {
                        break;
                    }
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != 's') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processStats(packet);
                    return;
                }
                break;

            case 't':
                if (packet.get() != 'i') {
                    break;
                }
                if (packet.get() != 'c') {
                    break;
                }
                if (packet.get() != 'k') {
                    break;
                }
                if (packet.get() != ' ') {
                    break;
                }
                processTick(packet);
                return;

            case 'u':
                if (packet.get() != 'p') {
                    break;
                }
                if (packet.get() != 'd') {
                    break;
                }
                switch (packet.get()) {
                case 'i':
                    if (packet.get() != 't') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 'm') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processUpdItem(packet);
                    return;

                case 's':
                    if (packet.get() != 'p') {
                        break;
                    }
                    if (packet.get() != 'e') {
                        break;
                    }
                    if (packet.get() != 'l') {
                        break;
                    }
                    if (packet.get() != 'l') {
                        break;
                    }
                    if (packet.get() != ' ') {
                        break;
                    }
                    processUpdSpell(packet);
                    return;
                }
                break;

            case 'v':
                if (packet.get() != 'e') {
                    break;
                }
                if (packet.get() != 'r') {
                    break;
                }
                if (packet.get() != 's') {
                    break;
                }
                if (packet.get() != 'i') {
                    break;
                }
                if (packet.get() != 'o') {
                    break;
                }
                if (packet.get() != 'n') {
                    break;
                }
                if (packet.get() != ' ') {
                    break;
                }
                processVersion(packet);
                return;
            }
        } catch (final IllegalArgumentException ex) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("IllegalArgumentException while command parsing: "+ex+"\n"+hexDump(packet), ex);
            }
        } catch (final BufferUnderflowException ex) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("BufferUnderflowException while command parsing: "+ex+"\n"+hexDump(packet), ex);
            }
        } catch (final ArrayIndexOutOfBoundsException ex) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("ArrayIndexOutOfBoundsException while command parsing: "+ex+"\n"+hexDump(packet), ex);
            }
        } catch (final StringIndexOutOfBoundsException ex) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("StringIndexOutOfBoundsException while command parsing: "+ex+"\n"+hexDump(packet), ex);
            }
        } catch (final UnknownCommandException ex) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("UnknownCommandException while command parsing: "+ex+"\n"+hexDump(packet), ex);
            }
            throw ex;
        }

        packet.position(0);
        final String command = extractCommand(packet);
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv invalid command: "+command+"\n"+hexDump(packet));
        }
        throw new UnknownCommandException("Cannot parse command: "+command);
    }

    /**
     * Returns the command string for a received packet.
     * @param packet the packet contents
     * @return the command string
     */
    private static String extractCommand(@NotNull final ByteBuffer packet) {
        int cmdLen;
        for (cmdLen = 0; cmdLen < packet.limit(); cmdLen++) {
            final byte ch = packet.get(cmdLen);
            if ((ch&0xFF) <= 0x20 || (ch&0xFF) >= 0x80) {
                break;
            }
        }
        return newString(packet, 0, cmdLen);
    }

    /**
     * Processes the payload data for a map2 coordinate command.
     * @param packet the packet contents
     * @param x the x-coordinate of the currently processed square
     * @param y the y-coordinate of the currently processed square
     * @throws UnknownCommandException if the command cannot be parsed
     */
    private void cmdMap2Coordinate(@NotNull final ByteBuffer packet, final int x, final int y) throws UnknownCommandException {
        for (; ;) {
            final int lenType = getInt1(packet);
            if (lenType == 0xFF) {
                break;
            }

            final int len = (lenType>>5)&7;
            final int type = lenType&31;
            switch (type) {
            case Map2.COORD_CLEAR_SPACE:
                cmdMap2CoordinateClearSpace(x, y, len);
                break;

            case Map2.COORD_DARKNESS:
                cmdMap2CoordinateDarkness(packet, x, y, len);
                break;

            case Map2.COORD_LAYER0:
            case Map2.COORD_LAYER1:
            case Map2.COORD_LAYER2:
            case Map2.COORD_LAYER3:
            case Map2.COORD_LAYER4:
            case Map2.COORD_LAYER5:
            case Map2.COORD_LAYER6:
            case Map2.COORD_LAYER7:
            case Map2.COORD_LAYER8:
            case Map2.COORD_LAYER9:
                cmdMap2CoordinateLayer(packet, x, y, len, type-Map2.COORD_LAYER0);
                break;
            }
        }
    }

    /**
     * Processes the payload data for a map2 coordinate "clear_space"
     * sub-command.
     * @param x the x-coordinate of the currently processed square
     * @param y the y-coordinate of the currently processed square
     * @param len the payload length
     * @throws UnknownCommandException if the command cannot be parsed
     */
    private void cmdMap2CoordinateClearSpace(final int x, final int y, final int len) throws UnknownCommandException {
        if (len != 0) {
            throw new UnknownCommandException("map2 command contains clear command with length "+len);
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+" clear");
        }
        for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
            listener.mapClear(x, y);
        }
    }

    /**
     * Processes the payload data for a map2 coordinate "darkness" sub-command.
     * @param packet the packet contents
     * @param x the x-coordinate of the currently processed square
     * @param y the y-coordinate of the currently processed square
     * @param len the payload length
     * @throws UnknownCommandException if the command cannot be parsed
     */
    private void cmdMap2CoordinateDarkness(@NotNull final ByteBuffer packet, final int x, final int y, final int len) throws UnknownCommandException {
        if (len != 1) {
            throw new UnknownCommandException("map2 command contains darkness command with length "+len);
        }
        final int darkness = getInt1(packet);
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+" darkness="+darkness);
        }
        for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
            listener.mapDarkness(x, y, darkness);
        }
    }

    /**
     * Processes the payload data for a map2 coordinate "layer" sub-command.
     * @param packet the packet contents
     * @param x the x-coordinate of the currently processed square
     * @param y the y-coordinate of the currently processed square
     * @param len the payload length
     * @param layer the layer to update
     * @throws UnknownCommandException if the command cannot be parsed
     */
    private void cmdMap2CoordinateLayer(@NotNull final ByteBuffer packet, final int x, final int y, final int len, final int layer) throws UnknownCommandException {
        if (len < 2) {
            throw new UnknownCommandException("map2 command contains image command with length "+len);
        }
        final int face = getInt2(packet);
        if ((face&Map2.FACE_ANIMATION) == 0) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+"/"+layer+" face="+face);
            }
            for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
                listener.mapFace(x, y, layer, face);
            }
        } else {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+"/"+layer+" anim="+(face&Map2.ANIM_MASK)+" type="+((face>>Map2.ANIM_TYPE_SHIFT)&Map2.ANIM_TYPE_MASK));
            }
            for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
                listener.mapAnimation(x, y, layer, face&Map2.ANIM_MASK, (face>>Map2.ANIM_TYPE_SHIFT)&Map2.ANIM_TYPE_MASK);
            }
        }
        if (len == 3) {
            cmdMap2CoordinateLayer3(packet, x, y, layer, face);
        } else if (len == 4) {
            cmdMap2CoordinateLayer4(packet, x, y, layer, face);
        } else if (len != 2) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+"/"+layer+" <invalid>");
            }
            throw new UnknownCommandException("map2 command contains image command with length "+len);
        }
    }

    /**
     * Processes the additional payload data for a map2 coordinate "layer"
     * sub-command having 4 bytes payload.
     * @param packet the packet contents
     * @param face the face number
     * @param x the x-coordinate of the currently processed square
     * @param y the y-coordinate of the currently processed square
     * @param layer the layer to update
     * @throws UnknownCommandException if the command cannot be parsed
     */
    private void cmdMap2CoordinateLayer3(@NotNull final ByteBuffer packet, final int x, final int y, final int layer, final int face) throws UnknownCommandException {
        if (face == 0) {
            throw new UnknownCommandException("map2 command contains smoothing or animation information for empty face");
        }

        if ((face&Map2.FACE_ANIMATION) == 0) {
            final int smooth = getInt1(packet);
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+"/"+layer+" smooth="+smooth);
            }
            // XXX: update smoothing information
        } else {
            final int animSpeed = getInt1(packet);
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+"/"+layer+" anim_speed="+animSpeed);
            }
            for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
                listener.mapAnimationSpeed(x, y, layer, animSpeed);
            }
        }
    }

    /**
     * Processes the additional payload data for a map2 coordinate "layer"
     * sub-command having 4 bytes payload.
     * @param packet the packet contents
     * @param face the face number
     * @param x the x-coordinate of the currently processed square
     * @param y the y-coordinate of the currently processed square
     * @param layer the layer to update
     * @throws UnknownCommandException if the command cannot be parsed
     */
    private void cmdMap2CoordinateLayer4(@NotNull final ByteBuffer packet, final int x, final int y, final int layer, final int face) throws UnknownCommandException {
        if (face == 0) {
            throw new UnknownCommandException("map2 command contains smoothing or animation information for empty face");
        }

        final int animSpeed = getInt1(packet);
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+"/"+layer+" anim_speed="+animSpeed);
        }
        for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
            listener.mapAnimationSpeed(x, y, layer, animSpeed);
        }

        final int smooth = getInt1(packet);
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+"/"+layer+" smooth="+smooth);
        }
        // XXX: update smoothing information
    }

    /**
     * Requests a change of the map size from the server.
     * @param mapWidth the map width to request
     * @param mapHeight the map height to request
     */
    private void negotiateMapSize(final int mapWidth, final int mapHeight) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("negotiateMapSize: "+mapWidth+"x"+mapHeight);
        }

        if (clientSocketState == ClientSocketState.CONNECTING || clientSocketState == ClientSocketState.VERSION || clientSocketState == ClientSocketState.CONNECT_FAILED) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("negotiateMapSize: clientSocketState="+clientSocketState+", ignoring");
            }
            return;
        }
        if (pendingMapWidth != 0 || pendingMapHeight != 0) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("negotiateMapSize: already negotiating, ignoring");
            }
            return;
        }
        if (currentMapWidth == mapWidth && currentMapHeight == mapHeight) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("negotiateMapSize: same as current map size, ignoring");
            }
            return;
        }
        pendingMapWidth = mapWidth;
        pendingMapHeight = mapHeight;
        sendSetup("mapsize "+pendingMapWidth+"x"+pendingMapHeight);
    }

    /**
     * Requests a change of the number of ground objects from the server.
     * @param numLookObjects the number of ground objects to request
     */
    private void negotiateNumLookObjects(final int numLookObjects) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("negotiateNumLookObjects: "+numLookObjects);
        }

        if (clientSocketState == ClientSocketState.CONNECTING || clientSocketState == ClientSocketState.VERSION || clientSocketState == ClientSocketState.CONNECT_FAILED) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("negotiateNumLookObjects: clientSocketState="+clientSocketState+", ignoring");
            }
            return;
        }
        if (pendingNumLookObjects != 0) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("negotiateNumLookObjects: already negotiating, ignoring");
            }
            return;
        }
        if (currentNumLookObjects == numLookObjects) {
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("negotiateNumLookObjects: same as current num look objects, ignoring");
            }
            return;
        }
        pendingNumLookObjects = numLookObjects;
        sendSetup("num_look_objects "+pendingNumLookObjects);
    }

    /**
     * Handles the replyinfo server to client command.
     * @param infoType the info_type parameter
     * @param packet the packet payload data
     * @throws IOException if an I/O error occurs
     */
    private void cmdReplyinfo(@NotNull final String infoType, final ByteBuffer packet) throws IOException {
        if (infoType.equals("image_info")) {
            processImageInfoReplyinfo(packet);
        } else if (infoType.equals("skill_info")) {
            processSkillInfoReplyinfo(packet);
        } else if (infoType.equals("exp_table")) {
            processExpTableReplyinfo(packet);
        } else {
            System.err.println("Ignoring unexpected replyinfo type '"+infoType+"'.");
        }
    }

    /**
     * Processes a "replyinfo image_info" block.
     * @param packet the packet to process
     * @throws IOException if the packet cannot be parsed
     */
    private static void processImageInfoReplyinfo(@NotNull final ByteBuffer packet) throws IOException {
        final byte[] data = new byte[packet.remaining()];
        packet.get(data);
        final ByteArrayInputStream is = new ByteArrayInputStream(data);
        try {
            final InputStreamReader isr = new InputStreamReader(is);
            try {
                final BufferedReader d = new BufferedReader(isr);
                try {
                    final String info = d.readLine();
                    if (info == null) {
                        throw new IOException("Truncated parameter in image_info");
                    }
                    //noinspection UnusedAssignment
                    final int nrPics = Integer.parseInt(info);
                    // XXX: replyinfo image_info not implemented
                } finally {
                    d.close();
                }
            } finally {
                isr.close();
            }
        } finally {
            is.close();
        }
    }

    /**
     * Processes a "replyinfo skill_info" block.
     * @param packet the packet to process
     * @throws IOException if the packet cannot be parsed
     */
    private void processSkillInfoReplyinfo(@NotNull final ByteBuffer packet) throws IOException {
        for (final CrossfireSkillInfoListener crossfireSkillInfoListener : crossfireSkillInfoListeners) {
            crossfireSkillInfoListener.clearSkills();
        }
        final byte[] data = new byte[packet.remaining()];
        packet.get(data);
        final ByteArrayInputStream is = new ByteArrayInputStream(data);
        try {
            final InputStreamReader isr = new InputStreamReader(is);
            try {
                final BufferedReader d = new BufferedReader(isr);
                try {
                    for (; ;) {
                        final String r = d.readLine();
                        if (r == null) {
                            break;
                        }

                        final String[] sk = PATTERN_DOT.split(r, 2);
                        if (sk.length != 2) {
                            System.err.println("Ignoring skill definition for invalid skill: "+r+".");
                            continue;
                        }

                        final int skillId;
                        try {
                            skillId = Integer.parseInt(sk[0]);
                        } catch (final NumberFormatException ignored) {
                            System.err.println("Ignoring skill definition for invalid skill: "+r+".");
                            continue;
                        }

                        if (skillId < CrossfireStatsListener.CS_STAT_SKILLINFO || skillId >= CrossfireStatsListener.CS_STAT_SKILLINFO+CrossfireStatsListener.CS_NUM_SKILLS) {
                            System.err.println("Ignoring skill definition for invalid skill id "+skillId+": "+r+".");
                            continue;
                        }

                        for (final CrossfireSkillInfoListener crossfireSkillInfoListener : crossfireSkillInfoListeners) {
                            crossfireSkillInfoListener.addSkill(skillId, sk[1]);
                        }
                    }
                } finally {
                    d.close();
                }
            } finally {
                isr.close();
            }
        } finally {
            is.close();
        }
    }

    /**
     * Processes a "replyinfo exp_table" block.
     * @param packet the packet to process
     */
    private void processExpTableReplyinfo(@NotNull final ByteBuffer packet) {
        final int numLevels = getInt2(packet);
        final long[] expTable = new long[numLevels];
        for (int level = 1; level < numLevels; level++) {
            expTable[level] = getInt8(packet);
        }
        if (packet.hasRemaining()) {
            System.err.println("Ignoring excess data at end of exp_table");
        }

        for (final CrossfireExpTableListener crossfireExpTableListener : crossfireExpTableListeners) {
            crossfireExpTableListener.expTableReceived(expTable);
        }

        if (loginMethod == 0) {
            setClientSocketState(ClientSocketState.REQUESTINFO, ClientSocketState.ADDME);
            sendAddme();
        } else {
            setClientSocketState(ClientSocketState.REQUESTINFO, ClientSocketState.ACCOUNT_INFO);
            for (final CrossfireAccountListener crossfireAccountListener : crossfireAccountListeners) {
                crossfireAccountListener.manageAccount();
            }
        }
    }

    /**
     * Processes an 'accountplayers' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processAccountPlayers(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        for (final CrossfireAccountListener crossfireAccountListener : crossfireAccountListeners) {
            crossfireAccountListener.startAccountList(accountName);
        }

        // number of characters
        int count = getInt1(packet);
        while (count > 0) {
            String name = "";
            String cClass = "";
            String race = "";
            String face = "";
            String party = "";
            String map = "";
            int level = 0;
            int faceNumber = 0;

            while (packet.hasRemaining()) {
                final int len = getInt1(packet);

                if (len == 0) {
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers entry");
                    }
                    /* got all information on a character */
                    count--;

                    for (final CrossfireAccountListener crossfireAccountListener : crossfireAccountListeners) {
                        crossfireAccountListener.addAccount(name, cClass, race, face, party, map, level, faceNumber);
                    }
                    break;
                }

                final int type = getInt1(packet);

                switch (type) {
                case ACL_NAME:
                    name = getString(packet, len-1);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers name="+name);
                    }
                    break;

                case ACL_CLASS:
                    cClass = getString(packet, len-1);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers class="+cClass);
                    }
                    break;

                case ACL_RACE:
                    race = getString(packet, len-1);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers race="+race);
                    }
                    break;

                case ACL_LEVEL:
                    level = getInt2(packet);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers level="+level);
                    }
                    break;

                case ACL_FACE:
                    face = getString(packet, len-1);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers face="+face);
                    }
                    break;

                case ACL_PARTY:
                    party = getString(packet, len-1);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers party="+party);
                    }
                    break;

                case ACL_MAP:
                    map = getString(packet, len-1);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers map="+map);
                    }
                    break;

                case ACL_FACE_NUM:
                    faceNumber = getInt2(packet);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers face="+faceNumber);
                    }
                    break;

                default:
                    // ignore those values we don't understand
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv accountplayers unknown="+type);
                    }
                    packet.position(packet.position()+len-1);
                    break;
                }
            }
        }
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("invalid accountplayers reply, pos="+packet.position());
        }

        for (final CrossfireAccountListener crossfireAccountListener : crossfireAccountListeners) {
            crossfireAccountListener.endAccountList();
        }

        packet.reset();
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes an 'account_failed' server command.
     * @param packet the packet's payload
     */
    private void processAddmeFailed(@NotNull final ByteBuffer packet) {
        final int args = packet.position();
        // XXX: addme_failed command not implemented
        notifyPacketWatcherListenersNoData(packet, args);
    }

    /**
     * Processes an 'account_success' server command.
     * @param packet the packet's payload
     */
    private void processAddmeSuccess(@NotNull final ByteBuffer packet) {
        final int args = packet.position();

        if (clientSocketState != ClientSocketState.CONNECTED) {
            if (clientSocketState == ClientSocketState.ADDME) {
                // servers without account support
                setClientSocketState(ClientSocketState.ADDME, ClientSocketState.CONNECTED);
            } else if (clientSocketState == ClientSocketState.ACCOUNT_INFO) {
                for (final CrossfireAccountListener crossfireAccountListener : crossfireAccountListeners) {
                    crossfireAccountListener.startPlaying();
                }
                setClientSocketState(ClientSocketState.ACCOUNT_INFO, ClientSocketState.CONNECTED);
                negotiateNumLookObjects(preferredNumLookObjects);
            }
            negotiateMapSize(preferredMapWidth, preferredMapHeight);
        }

        notifyPacketWatcherListenersNoData(packet, args);
    }

    /**
     * Processes an 'addspell' server command.
     * @param packet the packet's payload
     */
    private void processAddSpell(@NotNull final ByteBuffer packet) {
        final int args = packet.position();
        while (packet.hasRemaining()) {
            final int tag = getInt4(packet);
            final int level = getInt2(packet);
            final int castingTime = getInt2(packet);
            final int mana = getInt2(packet);
            final int grace = getInt2(packet);
            final int damage = getInt2(packet);
            final int skill = getInt1(packet);
            final int path = getInt4(packet);
            final int face = getInt4(packet);
            final int nameLength = getInt1(packet);
            final String name = getString(packet, nameLength);
            final int messageLength = getInt2(packet);
            final String message = getString(packet, messageLength);
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv addspell tag="+tag+" lvl="+level+" time="+castingTime+" sp="+mana+" gr="+grace+" dam="+damage+" skill="+skill+" path="+path+" face="+face+" name="+name+" msg="+message);
            }
            for (final CrossfireSpellListener crossfireSpellListener : crossfireSpellListeners) {
                crossfireSpellListener.addSpell(tag, level, castingTime, mana, grace, damage, skill, path, face, name, message);
            }
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes an 'anim' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processAnim(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int num = getInt2(packet);
        final int flags = getInt2(packet);
        final int[] faces = new int[packet.remaining()/2];
        if (faces.length <= 0) {
            throw new UnknownCommandException("no faces in anim command");
        }
        for (int i = 0; i < faces.length; i++) {
            faces[i] = getInt2(packet);
        }
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of anim command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv anim num="+num+" flags="+flags+" faces="+Arrays.toString(faces));
        }
        if ((num&~0x1FFF) != 0) {
            throw new UnknownCommandException("invalid animation id "+num);
        }
        for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
            listener.addAnimation(num&0x1FFF, flags, faces);
        }
        notifyPacketWatcherListenersShortArray(packet, args);
    }

    /**
     * Processes a 'comc' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processComc(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int packetNo = getInt2(packet);
        final int time = getInt4(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of comc command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv comc no="+packetNo+" time="+time);
        }
        for (final CrossfireComcListener listener : crossfireComcListeners) {
            listener.commandComcReceived(packetNo, time);
        }
        notifyPacketWatcherListenersShortInt(packet, args);
    }

    /**
     * Processes a 'delinv' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processDelInv(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        int tag = 0;
        do {
            tag = tag*10+parseDigit(packet.get());
        } while (packet.hasRemaining());
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of delinv command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv delinv tag="+tag);
        }
        for (final CrossfireUpdateItemListener crossfireUpdateItemListener : crossfireUpdateItemListeners) {
            crossfireUpdateItemListener.delinvReceived(tag);
        }
        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * Processes a 'delitem' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processDelItem(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int[] tags = new int[packet.remaining()/4];
        for (int i = 0; i < tags.length; i++) {
            tags[i] = getInt4(packet);
        }
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of delitem command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv delitem tags="+Arrays.toString(tags));
        }
        for (final CrossfireUpdateItemListener crossfireUpdateItemListener : crossfireUpdateItemListeners) {
            crossfireUpdateItemListener.delitemReceived(tags);
        }
        notifyPacketWatcherListenersIntArray(packet, args);
    }

    /**
     * Processes a 'delspell' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processDelSpell(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int tag = getInt4(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of delspell command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv delspell tag="+tag);
        }
        for (final CrossfireSpellListener crossfireSpellListener : crossfireSpellListeners) {
            crossfireSpellListener.deleteSpell(tag);
        }
        notifyPacketWatcherListenersIntArray(packet, args);
    }

    /**
     * Processes a 'drawextinfo' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processDrawExtInfo(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        int color = 0;
        do {
            color = color*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();

        int type = 0;
        do {
            type = type*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();

        int subtype = 0;
        do {
            subtype = subtype*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();

        final String message = getString(packet, packet.remaining());

        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv drawextinfo color="+color+" type="+type+"/"+subtype+" msg="+message);
        }

        drawextinfo(color, type, subtype, message);
        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * Processes a 'drawinfo' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processDrawInfo(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        int color = 0;
        do {
            color = color*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();

        final String message = getString(packet, packet.remaining());

        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv drawinfo color="+color+" msg="+message);
        }

        drawInfo(message, color);
        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * Processes an 'ExtendedInfoSet' server command.
     * @param packet the packet's payload
     */
    private void processExtendedInfoSet(@NotNull final ByteBuffer packet) {
        final int args = packet.position();
        do {
            final int startPos = packet.position();
            while (packet.hasRemaining() && packet.get(packet.position()) != ' ') {
                packet.get();
            }
            final String string = newString(packet, startPos, packet.position()-startPos);
            packet.get();
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv ExtendedInfoSet "+string);
            }
            // XXX: ExtendedInfoSet command not implemented
        } while (packet.hasRemaining());
        notifyPacketWatcherListenersNoData(packet, args);
    }

    /**
     * Processes an 'ExtendedTextSet' server command.
     * @param packet the packet's payload
     */
    private void processExtendedTextSet(@NotNull final ByteBuffer packet) {
        final int args = packet.position();
        for (; ;) {
            final int startPos = packet.position();
            while (packet.hasRemaining() && packet.get(packet.position()) != ' ') {
                packet.get();
            }
            final String type = newString(packet, startPos, packet.position()-startPos);
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv ExtendedTextSet "+type);
            }
            // XXX: ExtendedTextSet command not implemented
            if (!packet.hasRemaining()) {
                break;
            }
            packet.get();
        }
        notifyPacketWatcherListenersNoData(packet, args);
    }

    /**
     * Processes a 'face2' server command.
     * @param packet the packet's payload
     */
    private void processFace2(@NotNull final ByteBuffer packet) {
        final int args = packet.position();
        final int faceNum = getInt2(packet);
        final int faceSetNum = getInt1(packet);
        final int faceChecksum = getInt4(packet);
        final String faceName = getString(packet, packet.remaining()).intern();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv face2 num="+faceNum+" set="+faceSetNum+" checksum="+faceChecksum+" name="+faceName);
        }
        for (final CrossfireFaceListener crossfireFaceListener : crossfireFaceListeners) {
            crossfireFaceListener.faceReceived(faceNum, faceSetNum, faceChecksum, faceName);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'failure' server command.
     * @param packet the packet's payload
     */
    private void processFailure(@NotNull final ByteBuffer packet) {
        final String full = getString(packet, packet.remaining());
        final String command;
        final String message;
        final int idx = full.indexOf(' ');
        if (idx == -1) {
            command = full;
            message = "";
        } else {
            command = full.substring(0, idx);
            message = full.substring(idx+1);
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv failure command="+command+" message="+message);
        }

        for (final CrossfireFailureListener crossfireFailureListener : crossfireFailureListeners) {
            crossfireFailureListener.failure(command, message);
        }
    }

    /**
     * Inform the various failure listeners that they can clean the last
     * displayed failure.
     */
    private void clearFailure() {
        for (final CrossfireFailureListener crossfireFailureListener : crossfireFailureListeners) {
            crossfireFailureListener.clearFailure();
        }
    }

    /**
     * Processes a 'goodbye' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processGoodbye(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of goodbye command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv goodbye");
        }
        // XXX: goodbye command not implemented
        notifyPacketWatcherListenersNoData(packet, args);
    }

    /**
     * Processes an 'image2' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processImage2(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int faceNum = getInt4(packet);
        final int faceSetNum = getInt1(packet);
        final int len = getInt4(packet);
        if (packet.remaining() != len) {
            throw new UnknownCommandException("excess data at end of image2 command");
        }
        final int faceDataPosition = packet.position();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv image2 face="+faceNum+" set="+faceSetNum+" len="+len);
        }
        for (final CrossfireUpdateFaceListener listener : crossfireUpdateFaceListeners) {
            packet.position(faceDataPosition);
            listener.updateFace(faceNum, faceSetNum, packet);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes an 'item2' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processItem2(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int location = getInt4(packet);
        while (packet.hasRemaining()) {
            final int tag = getInt4(packet);
            final int flags = getInt4(packet);
            final int weight = getInt4(packet);
            final int faceNum = getInt4(packet);
            final int nameLength = getInt1(packet);
            final String[] names = getString(packet, nameLength).split("\0", 2);
            final String name = names[0].intern();
            final String namePl = names.length < 2 ? name : names[1].intern();
            final int anim = getInt2(packet);
            final int animSpeed = getInt1(packet);
            final int nrof = getInt4(packet);
            final int type = getInt2(packet);
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv item2 location="+location+" tag="+tag+" flags="+flags+" weight="+weight+" face="+faceNum+" name="+name+" name_pl="+namePl+" anim="+anim+" anim_speed="+animSpeed+" nrof="+nrof+" type="+type);
            }
            for (final CrossfireUpdateItemListener crossfireUpdateItemListener : crossfireUpdateItemListeners) {
                crossfireUpdateItemListener.addItemReceived(location, tag, flags, weight, faceNum, name, namePl, anim, animSpeed, nrof, type);
            }
        }
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of item2 command");
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'magicmap' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processMagicMap(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();

        final boolean widthSign = packet.get(packet.position()) == '-';
        if (widthSign) {
            packet.get();
        }
        int width = 0;
        do {
            width = width*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();
        if (widthSign) {
            width = -width;
        }

        final boolean heightSign = packet.get(packet.position()) == '-';
        if (heightSign) {
            packet.get();
        }
        int height = 0;
        do {
            height = height*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();
        if (heightSign) {
            height = -height;
        }

        final boolean pxSign = packet.get(packet.position()) == '-';
        if (pxSign) {
            packet.get();
        }
        int px = 0;
        do {
            px = px*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();
        if (pxSign) {
            px = -px;
        }

        final boolean pySign = packet.get(packet.position()) == '-';
        if (pySign) {
            packet.get();
        }
        int py = 0;
        do {
            py = py*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();
        if (pySign) {
            py = -py;
        }

        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv magicmap size="+width+"x"+height+" player="+px+"/"+py+" len="+packet.remaining());
        }

        if (packet.remaining() != width*height) {
            throw new UnknownCommandException("invalid magicmap command");
        }

        for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
            listener.mapBegin();
        }
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                final byte ch = packet.get();
                for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
                    listener.mapMagicMap(x-px+(currentMapWidth-1)/2, y-py+(currentMapHeight-1)/2, ch&FACE_COLOR_MASK);
                }
            }
        }
        for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
            listener.mapEnd();
        }
        for (final CrossfireMagicmapListener listener : magicmapListeners) {
            listener.commandMagicmapReceived();
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'map2' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processMap2(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        synchronized (redrawSemaphore) {
            for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
                listener.mapBegin();
            }
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv map2 begin");
            }
            while (packet.hasRemaining()) {
                final int coord = getInt2(packet);
                final int x = ((coord>>10)&0x3F)-Map2.COORD_OFFSET;
                final int y = ((coord>>4)&0x3F)-Map2.COORD_OFFSET;
                final int coordType = coord&0xF;

                switch (coordType) {
                case Map2.TYPE_COORDINATE:
                    cmdMap2Coordinate(packet, x, y);
                    break;

                case Map2.TYPE_SCROLL:
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+" scroll");
                    }
                    for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
                        listener.scroll(x, y);
                    }
                    break;

                default:
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv map2 "+x+"/"+y+" <invalid>");
                    }
                    throw new UnknownCommandException("map2 command contains unexpected coordinate type "+coordType);
                }
            }
            if (debugProtocol != null) {
                debugProtocol.debugProtocolWrite("recv map2 end");
            }
            for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
                listener.mapEnd();
            }
        }
        notifyPacketWatcherListenersShortArray(packet, args);
    }

    /**
     * Processes a 'mapextended' server command.
     * @param packet the packet's payload
     */
    private void processMapExtended(@NotNull final ByteBuffer packet) {
        final int args = packet.position();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv mapextended");
        }

        // XXX: "MapExtended" command not yet implemented

        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'music' server command.
     * @param packet the packet's payload
     */
    private void processMusic(@NotNull final ByteBuffer packet) {
        final int args = packet.position();
        final String music = getString(packet, packet.remaining());
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv music "+music);
        }

        for (final CrossfireMusicListener listener : crossfireMusicListeners) {
            listener.commandMusicReceived(music);
        }
        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * Processes a 'newmap' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processNewMap(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of newmap command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv newmap");
        }
        fireNewMap();
        notifyPacketWatcherListenersNoData(packet, args);
    }

    /**
     * Processes a 'pickup' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processPickup(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int pickupOptions = getInt4(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of pickup command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv pickup options="+pickupOptions);
        }
        for (final CrossfirePickupListener crossfirePickupListener : crossfirePickupListeners) {
            crossfirePickupListener.pickupChanged(pickupOptions);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'pickup' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processPlayer(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int tag = getInt4(packet);
        final int weight = getInt4(packet);
        final int faceNum = getInt4(packet);
        final int nameLength = getInt1(packet);
        final String name = getString(packet, nameLength);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of player command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv player tag="+tag+" weight="+weight+" face="+faceNum+" name="+name);
        }
        for (final CrossfireUpdateItemListener crossfireUpdateItemListener : crossfireUpdateItemListeners) {
            crossfireUpdateItemListener.playerReceived(tag, weight, faceNum, name);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'pickup' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processQuery(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        int flags = 0;
        do {
            flags = flags*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();

        final String text = getString(packet, packet.remaining());

        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv query flags="+flags+" text="+text);
        }

        // XXX: hack to process "What is your name?" prompt even before addme_success is received
        if (clientSocketState != ClientSocketState.CONNECTED) {
            setClientSocketState(ClientSocketState.ADDME, ClientSocketState.CONNECTED);
            negotiateMapSize(preferredMapWidth, preferredMapHeight);
            negotiateNumLookObjects(preferredNumLookObjects);
        }
        for (final CrossfireQueryListener listener : queryListeners) {
            listener.commandQueryReceived(text, flags);
        }
        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * Processes a 'replyinfo' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processReplyInfo(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int startPos = packet.position();
        while (packet.hasRemaining() && packet.get(packet.position()) != '\n' && packet.get(packet.position()) != ' ') {
            packet.get();
        }
        final String infoType = newString(packet, startPos, packet.position()-startPos);
        if (packet.hasRemaining()) {
            packet.get();
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv replyinfo type="+infoType+" len="+packet.remaining());
        }
        try {
            cmdReplyinfo(infoType, packet);
        } catch (final IOException ex) {
            throw new UnknownCommandException("invalid replyinfo command: "+ex.getMessage());
        }
        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * Processes a 'replyinfo' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processSetup(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final List<String> options = new ArrayList<String>();
        while (packet.hasRemaining()) {
            while (packet.get(packet.position()) == ' ') {
                packet.get();
            }
            final int startPos = packet.position();
            while (packet.hasRemaining() && packet.get(packet.position()) != ' ') {
                packet.get();
            }
            options.add(newString(packet, startPos, packet.position()-startPos));
            if (packet.hasRemaining()) {
                packet.get();
            }
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv setup "+options);
        }
        if (options.size()%2 != 0) {
            throw new UnknownCommandException("odd number of arguments in setup command");
        }
        for (int i = 0; i+1 < options.size(); i += 2) {
            final String option = options.get(i);
            final String value = options.get(i+1);
            if (option.equals("spellmon")) {
                if (!value.equals("1")) {
                    throw new UnknownCommandException("Error: the server is too old for this client since it does not support the spellmon=1 setup option.");
                }
            } else if (option.equals("sound2")) {
                // ignore: if the server sends sound info it is processed
            } else if (option.equals("exp64")) {
                // Ignored since it only enables additional/improved stat
                // commands but the old version is also supported.
            } else if (option.equals("newmapcmd")) {
                if (!value.equals("1")) {
                    throw new UnknownCommandException("Error: the server is too old for this client since it does not support the newmapcmd=1 setup option.");
                }
            } else if (option.equals("facecache")) {
                if (!value.equals("1")) {
                    throw new UnknownCommandException("the server is too old for this client since it does not support the facecache=1 setup option.");
                }
            } else if (option.equals("extendedTextInfos")) {
                if (!value.equals("1")) {
                    throw new UnknownCommandException("the server is too old for this client since it does not support the extendedTextInfos=1 setup option.");
                }
            } else if (option.equals("itemcmd")) {
                if (!value.equals("2")) {
                    throw new UnknownCommandException("the server is too old for this client since it does not support the itemcmd=2 setup option.");
                }
            } else if (option.equals("mapsize")) {
                final String[] tmp = value.split("x", 2);
                if (tmp.length != 2) {
                    throw new UnknownCommandException("the server returned 'setup mapsize "+value+"'.");
                }
                final int thisMapWidth;
                final int thisMapHeight;
                try {
                    thisMapWidth = Integer.parseInt(tmp[0]);
                    thisMapHeight = Integer.parseInt(tmp[1]);
                } catch (final NumberFormatException ignored) {
                    throw new UnknownCommandException("the server returned 'setup mapsize "+value+"'.");
                }
                if (pendingMapWidth == 0 || pendingMapHeight == 0) {
                    System.err.println("the server sent an unexpected 'setup mapsize "+value+"'.");
                } else if (pendingMapWidth == thisMapWidth && pendingMapHeight == thisMapHeight) {
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    setCurrentMapSize(thisMapWidth, thisMapHeight);
                } else if (pendingMapWidth > thisMapWidth && pendingMapHeight > thisMapHeight) {
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    negotiateMapSize(thisMapWidth, thisMapHeight);
                } else if (pendingMapWidth > thisMapWidth) {
                    final int tmpMapHeight = pendingMapHeight;
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    negotiateMapSize(thisMapWidth, tmpMapHeight);
                } else if (pendingMapHeight > thisMapHeight) {
                    final int tmpMapWidth = pendingMapWidth;
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    negotiateMapSize(tmpMapWidth, thisMapHeight);
                } else if (pendingMapWidth == thisMapWidth) {
                    final int tmpMapHeight = pendingMapHeight+2;
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    negotiateMapSize(thisMapWidth, tmpMapHeight);
                } else if (pendingMapHeight == thisMapHeight) {
                    final int tmpMapWidth = pendingMapWidth+2;
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    negotiateMapSize(tmpMapWidth, thisMapHeight);
                } else if (pendingMapWidth <= pendingMapHeight) {
                    final int tmpMapWidth = pendingMapWidth+2;
                    final int tmpMapHeight = pendingMapHeight;
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    negotiateMapSize(tmpMapWidth, tmpMapHeight);
                } else {
                    final int tmpMapWidth = pendingMapWidth;
                    final int tmpMapHeight = pendingMapHeight+2;
                    pendingMapWidth = 0;
                    pendingMapHeight = 0;
                    negotiateMapSize(tmpMapWidth, tmpMapHeight);
                }
            } else if (option.equals("map2cmd")) {
                if (!value.equals("1")) {
                    throw new UnknownCommandException("the server is too old for this client since it does not support the map2cmd=1 setup option.");
                }
            } else if (option.equals("darkness")) {
                // do not care
            } else if (option.equals("tick")) {
                if (!value.equals("1")) {
                    throw new UnknownCommandException("the server is too old for this client since it does not support the tick=1 setup option.");
                }
            } else if (option.equals("num_look_objects")) {
                if (value.equals("FALSE")) {
                    System.err.println("Warning: the server is too old for this client since it does not support the num_look_objects setup option.");
                    System.err.println("Expect issues with the ground view display.");
                    pendingNumLookObjects = 0;
                } else {
                    final int thisNumLookObjects;
                    try {
                        thisNumLookObjects = Integer.parseInt(value);
                    } catch (final NumberFormatException ignored) {
                        throw new UnknownCommandException("the server returned 'setup num_look_objects "+value+"'.");
                    }
                    if (pendingNumLookObjects == 0) {
                        System.err.println("the server sent an unexpected 'setup num_look_objects "+value+"'.");
                    } else {
                        if (pendingNumLookObjects != thisNumLookObjects) {
                            System.err.println("Warning: the server didn't accept the num_look_objects setup option: requested "+pendingNumLookObjects+", returned "+thisNumLookObjects+".");
                            System.err.println("Expect issues with the ground view display.");
                        }
                        pendingNumLookObjects = 0;
                        currentNumLookObjects = thisNumLookObjects;
                    }
                }
            } else if (option.equals("faceset")) {
                // ignore: we do not care about the face set
            } else if (option.equals("want_pickup")) {
                // ignore: we do not care whether this option has been ignored
            } else if (option.equals("extended_stats")) {
                // ignore: we do not care whether this option has been ignored
            } else if (option.equals("loginmethod")) {
                if (value.equals("FALSE")) {
                    loginMethod = 0;
                    continue;
                }

                final int method;
                try {
                    method = Integer.parseInt(value);
                } catch (final NumberFormatException ignored) {
                    throw new UnknownCommandException("the server returned 'setup loginmethod "+value+"'.");
                }
                loginMethod = method;
            } else {
                System.err.println("Warning: ignoring unknown setup option from server: "+option+"="+value);
            }
        }

        if (options.size() != 2 || !options.get(0).equals("mapsize") && !options.get(0).equals("num_look_objects")) {
            setClientSocketState(ClientSocketState.SETUP, ClientSocketState.REQUESTINFO);
            sendRequestinfo("image_info");
            sendRequestinfo("skill_info");
            sendRequestinfo("exp_table");
            sendToggleextendedtext(MessageTypes.getAllTypes());
        }
        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * Processes a 'smooth' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processSmooth(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int faceNo = getInt2(packet);
        final int smoothPic = getInt2(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of smooth command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv smooth face="+faceNo+" smooth_pic="+smoothPic);
        }
        // XXX: smooth command not implemented
        notifyPacketWatcherListenersShortArray(packet, args);
    }

    /**
     * Processes a 'sound' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processSound(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int x = packet.get();
        final int y = packet.get();
        final int num = getInt2(packet);
        final int type = getInt1(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of sound command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv sound pos="+x+"/"+y+" num="+num+" type="+type);
        }

        for (final CrossfireSoundListener listener : crossfireSoundListeners) {
            listener.commandSoundReceived(x, y, num, type);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'sound2' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processSound2(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int x = packet.get();
        final int y = packet.get();
        final int dir = packet.get();
        final int volume = getInt1(packet);
        final int type = getInt1(packet);
        final int actionLength = getInt1(packet);
        final String action = getString(packet, actionLength);
        final int nameLength = getInt1(packet);
        final String name = getString(packet, nameLength);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of sound2 command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv sound2 pos="+x+"/"+y+" dir="+dir+" volume="+volume+" type="+type+" action="+action+" name="+name);
        }

        for (final CrossfireSoundListener listener : crossfireSoundListeners) {
            listener.commandSound2Received(x, y, dir, volume, type, action, name);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'stats' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processStats(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        while (packet.hasRemaining()) {
            final int stat = getInt1(packet);
            switch (stat) {
            case CrossfireStatsListener.CS_STAT_HP:
            case CrossfireStatsListener.CS_STAT_MAXHP:
            case CrossfireStatsListener.CS_STAT_SP:
            case CrossfireStatsListener.CS_STAT_MAXSP:
            case CrossfireStatsListener.CS_STAT_STR:
            case CrossfireStatsListener.CS_STAT_INT:
            case CrossfireStatsListener.CS_STAT_WIS:
            case CrossfireStatsListener.CS_STAT_DEX:
            case CrossfireStatsListener.CS_STAT_CON:
            case CrossfireStatsListener.CS_STAT_CHA:
            case CrossfireStatsListener.CS_STAT_LEVEL:
            case CrossfireStatsListener.CS_STAT_WC:
            case CrossfireStatsListener.CS_STAT_AC:
            case CrossfireStatsListener.CS_STAT_DAM:
            case CrossfireStatsListener.CS_STAT_ARMOUR:
            case CrossfireStatsListener.CS_STAT_FOOD:
            case CrossfireStatsListener.CS_STAT_POW:
            case CrossfireStatsListener.CS_STAT_GRACE:
            case CrossfireStatsListener.CS_STAT_MAXGRACE:
            case CrossfireStatsListener.CS_STAT_FLAGS:
            case CrossfireStatsListener.CS_STAT_RACE_STR:
            case CrossfireStatsListener.CS_STAT_RACE_INT:
            case CrossfireStatsListener.CS_STAT_RACE_WIS:
            case CrossfireStatsListener.CS_STAT_RACE_DEX:
            case CrossfireStatsListener.CS_STAT_RACE_CON:
            case CrossfireStatsListener.CS_STAT_RACE_CHA:
            case CrossfireStatsListener.CS_STAT_RACE_POW:
            case CrossfireStatsListener.CS_STAT_BASE_STR:
            case CrossfireStatsListener.CS_STAT_BASE_INT:
            case CrossfireStatsListener.CS_STAT_BASE_WIS:
            case CrossfireStatsListener.CS_STAT_BASE_DEX:
            case CrossfireStatsListener.CS_STAT_BASE_CON:
            case CrossfireStatsListener.CS_STAT_BASE_CHA:
            case CrossfireStatsListener.CS_STAT_BASE_POW:
            case CrossfireStatsListener.CS_STAT_APPLIED_STR:
            case CrossfireStatsListener.CS_STAT_APPLIED_INT:
            case CrossfireStatsListener.CS_STAT_APPLIED_WIS:
            case CrossfireStatsListener.CS_STAT_APPLIED_DEX:
            case CrossfireStatsListener.CS_STAT_APPLIED_CON:
            case CrossfireStatsListener.CS_STAT_APPLIED_CHA:
            case CrossfireStatsListener.CS_STAT_APPLIED_POW:
                final short int2Param = (short)getInt2(packet);
                if (debugProtocol != null) {
                    debugProtocol.debugProtocolWrite("recv stats stat="+stat+" int2="+int2Param+"="+(int2Param&0xFFFF));
                }
                for (final CrossfireStatsListener crossfireStatsListener : crossfireStatsListeners) {
                    crossfireStatsListener.statInt2Received(stat, int2Param);
                }
                notifyPacketWatcherListenersStats(stat, int2Param);
                break;

            case CrossfireStatsListener.CS_STAT_EXP:
            case CrossfireStatsListener.CS_STAT_SPEED:
            case CrossfireStatsListener.CS_STAT_WEAP_SP:
            case CrossfireStatsListener.CS_STAT_WEIGHT_LIM:
            case CrossfireStatsListener.CS_STAT_SPELL_ATTUNE:
            case CrossfireStatsListener.CS_STAT_SPELL_REPEL:
            case CrossfireStatsListener.CS_STAT_SPELL_DENY:
                final int int4Param = getInt4(packet);
                if (debugProtocol != null) {
                    debugProtocol.debugProtocolWrite("recv stats stat="+stat+" int4="+int4Param);
                }
                for (final CrossfireStatsListener crossfireStatsListener : crossfireStatsListeners) {
                    crossfireStatsListener.statInt4Received(stat, int4Param);
                }
                notifyPacketWatcherListenersStats(stat, int4Param);
                break;

            case CrossfireStatsListener.CS_STAT_EXP64:
                final long int8Param = getInt8(packet);
                if (debugProtocol != null) {
                    debugProtocol.debugProtocolWrite("recv stats stat="+stat+" int8="+int8Param);
                }
                for (final CrossfireStatsListener crossfireStatsListener : crossfireStatsListeners) {
                    crossfireStatsListener.statInt8Received(stat, int8Param);
                }
                notifyPacketWatcherListenersStats(stat, int8Param);
                break;

            case CrossfireStatsListener.CS_STAT_RANGE:
            case CrossfireStatsListener.CS_STAT_TITLE:
                final int length = getInt1(packet);
                final String strParam = getString(packet, length);
                if (debugProtocol != null) {
                    debugProtocol.debugProtocolWrite("recv stats stat="+stat+" str="+strParam);
                }
                for (final CrossfireStatsListener crossfireStatsListener : crossfireStatsListeners) {
                    crossfireStatsListener.statStringReceived(stat, strParam);
                }
                notifyPacketWatcherListenersStats(stat, strParam);
                break;

            default:
                if (CrossfireStatsListener.CS_STAT_RESIST_START <= stat && stat < CrossfireStatsListener.CS_STAT_RESIST_START+CrossfireStatsListener.RESIST_TYPES) {
                    final short int2Param2 = (short)getInt2(packet);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv stats stat="+stat+" int2="+int2Param2);
                    }
                    for (final CrossfireStatsListener crossfireStatsListener : crossfireStatsListeners) {
                        crossfireStatsListener.statInt2Received(stat, int2Param2);
                    }
                    notifyPacketWatcherListenersStats(stat, int2Param2);
                } else if (CrossfireStatsListener.CS_STAT_SKILLINFO <= stat && stat < CrossfireStatsListener.CS_STAT_SKILLINFO+CrossfireStatsListener.CS_NUM_SKILLS) {
                    final int level = getInt1(packet);
                    final long experience = getInt8(packet);
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv stats stat="+stat+" level="+level+" experience="+experience);
                    }
                    for (final CrossfireStatsListener crossfireStatsListener : crossfireStatsListeners) {
                        crossfireStatsListener.statSkillReceived(stat, level, experience);
                    }
                    notifyPacketWatcherListenersStats(stat, level, experience);
                } else {
                    if (debugProtocol != null) {
                        debugProtocol.debugProtocolWrite("recv stats stat="+stat+" <unknown parameter>");
                    }
                    throw new UnknownCommandException("unknown stat value: "+stat);
                }
                break;
            }
        }
    }

    /**
     * Processes a 'tick' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processTick(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int tickNo = getInt4(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of tick command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv tick "+tickNo);
        }
        for (final CrossfireTickListener listener : crossfireTickListeners) {
            listener.tick(tickNo);
        }
        notifyPacketWatcherListenersIntArray(packet, args);
    }

    /**
     * Processes an 'upditem' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processUpdItem(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int flags = getInt1(packet);
        final int tag = getInt4(packet);
        final int valLocation = (flags&UpdItem.UPD_LOCATION) == 0 ? 0 : getInt4(packet);
        final int valFlags = (flags&UpdItem.UPD_FLAGS) == 0 ? 0 : getInt4(packet);
        final int valWeight = (flags&UpdItem.UPD_WEIGHT) == 0 ? 0 : getInt4(packet);
        final int valFaceNum = (flags&UpdItem.UPD_FACE) == 0 ? 0 : getInt4(packet);
        final String valName;
        final String valNamePl;
        if ((flags&UpdItem.UPD_NAME) == 0) {
            valName = "";
            valNamePl = "";
        } else {
            final int nameLength = getInt1(packet);
            int namePlIndex = 0;
            while (namePlIndex < nameLength && packet.get(packet.position()+namePlIndex) != 0) {
                namePlIndex++;
            }
            valName = newString(packet, packet.position(), namePlIndex);
            valNamePl = namePlIndex+1 < nameLength ? newString(packet, packet.position()+namePlIndex+1, nameLength-(namePlIndex+1)) : valName;
            packet.position(packet.position()+nameLength);
        }
        final int valAnim = (flags&UpdItem.UPD_ANIM) == 0 ? 0 : getInt2(packet);
        final int valAnimSpeed = (flags&UpdItem.UPD_ANIMSPEED) == 0 ? 0 : getInt1(packet);
        final int valNrof = (flags&UpdItem.UPD_NROF) == 0 ? 0 : getInt4(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of upditem command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv upditem flags="+flags+" tag="+tag+" loc="+valLocation+" flags="+valFlags+" weight="+valWeight+" face="+valFaceNum+" name="+valName+" name_pl="+valNamePl+" anim="+valAnim+" anim_speed="+valAnimSpeed+" nrof="+valNrof);
        }
        for (final CrossfireUpdateItemListener crossfireUpdateItemListener : crossfireUpdateItemListeners) {
            crossfireUpdateItemListener.upditemReceived(flags, tag, valLocation, valFlags, valWeight, valFaceNum, valName, valNamePl, valAnim, valAnimSpeed, valNrof);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes an 'updspell' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processUpdSpell(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        final int flags = getInt1(packet);
        final int tag = getInt4(packet);
        final int mana = (flags&CrossfireSpellListener.UPD_SP_MANA) == 0 ? 0 : getInt2(packet);
        final int grace = (flags&CrossfireSpellListener.UPD_SP_GRACE) == 0 ? 0 : getInt2(packet);
        final int damage = (flags&CrossfireSpellListener.UPD_SP_DAMAGE) == 0 ? 0 : getInt2(packet);
        if (packet.hasRemaining()) {
            throw new UnknownCommandException("excess data at end of updspell command");
        }
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv updspell flags="+flags+" tag="+tag+" sp="+mana+" gr="+grace+" dam="+damage);
        }
        for (final CrossfireSpellListener crossfireSpellListener : crossfireSpellListeners) {
            crossfireSpellListener.updateSpell(flags, tag, mana, grace, damage);
        }
        notifyPacketWatcherListenersMixed(packet, args);
    }

    /**
     * Processes a 'version' server command.
     * @param packet the packet's payload
     * @throws UnknownCommandException if the packet is invalid.
     */
    private void processVersion(@NotNull final ByteBuffer packet) throws UnknownCommandException {
        final int args = packet.position();
        int csval = 0;
        do {
            csval = csval*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();

        int scval = 0;
        do {
            scval = scval*10+parseDigit(packet.get());
        } while (packet.get(packet.position()) != ' ');
        packet.get();

        final String vinfo = getString(packet, packet.remaining());

        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("recv version cs="+csval+" sc="+scval+" info="+vinfo);
        }

        setClientSocketState(ClientSocketState.VERSION, ClientSocketState.SETUP);
        sendSetup("want_pickup 1", "faceset 0", "sound2 3", "exp64 1", "map2cmd 1", "darkness 1", "newmapcmd 1", "facecache 1", "extendedTextInfos 1", "itemcmd 2", "spellmon 1", "tick 1", "extended_stats 1", "loginmethod 1");
        for (final CrossfireStatsListener crossfireStatsListener : crossfireStatsListeners) {
            crossfireStatsListener.setSimpleWeaponSpeed(scval >= 1029);
        }

        notifyPacketWatcherListenersAscii(packet, args);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void drawInfo(@NotNull final String message, final int color) {
        for (final CrossfireDrawinfoListener listener : drawinfoListeners) {
            listener.commandDrawinfoReceived(message, color);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void drawextinfo(final int color, final int type, final int subtype, final String message) {
        for (final CrossfireDrawextinfoListener listener : drawextinfoListeners) {
            listener.commandDrawextinfoReceived(color, type, subtype, message);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void drawInfoSetDebugMode(final boolean printMessageTypes) {
        for (final CrossfireDrawextinfoListener listener : drawextinfoListeners) {
            listener.setDebugMode(printMessageTypes);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendAccountLogin(@NotNull final String login, @NotNull final String password) {
        clearFailure();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send accountlogin "+login);
        }
        accountName = login;
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(ACCOUNT_LOGIN_PREFIX);
            byteBuffer.put((byte)login.length());
            byteBuffer.put(login.getBytes(UTF8));
            byteBuffer.put((byte)password.length());
            byteBuffer.put(password.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }

    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendAddme() {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send addme");
        }
        writePacket(ADDME_PREFIX, ADDME_PREFIX.length);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendApply(final int tag) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send apply tag="+tag);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(APPLY_PREFIX);
            putDecimal(tag);
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendAskface(final int num) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send askface face="+num);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(ASKFACE_PREFIX);
            putDecimal(num);
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendExamine(final int tag) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send examine tag="+tag);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(EXAMINE_PREFIX);
            putDecimal(tag);
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendLock(final boolean val, final int tag) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send lock tag="+tag+" val="+val);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(LOCK_PREFIX);
            byteBuffer.put((byte)(val ? 1 : 0));
            byteBuffer.putInt(tag);
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendLookat(final int dx, final int dy) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send lookat pos="+dx+"/"+dy);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(LOOKAT_PREFIX);
            putDecimal(dx);
            byteBuffer.put((byte)' ');
            putDecimal(dy);
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendMark(final int tag) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send mark tag="+tag);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(MARK_PREFIX);
            byteBuffer.putInt(tag);
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendMove(final int to, final int tag, final int nrof) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send move tag="+tag+" to="+to+" nrof="+nrof);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(MOVE_PREFIX);
            putDecimal(to);
            byteBuffer.put((byte)' ');
            putDecimal(tag);
            byteBuffer.put((byte)' ');
            putDecimal(nrof);
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int sendNcom(final int repeat, @NotNull final String command) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send ncom no="+packet+" repeat="+repeat+" cmd="+command);
        }
        final int thisPacket;
        synchronized (writeBuffer) {
            thisPacket = packet++&0x00FF;
            byteBuffer.clear();
            byteBuffer.put(NCOM_PREFIX);
            byteBuffer.putShort((short)thisPacket);
            byteBuffer.putInt(repeat);
            byteBuffer.put(command.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
        return thisPacket;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendReply(@NotNull final String text) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send reply text="+text);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(REPLY_PREFIX);
            byteBuffer.put(text.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
        for (final SentReplyListener sentReplyListener : sentReplyListeners) {
            sentReplyListener.replySent(text);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendRequestinfo(@NotNull final String infoType) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send requestinfo type="+infoType);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(REQUESTINFO_PREFIX);
            byteBuffer.put(infoType.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendSetup(@NotNull final String... options) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send setup options="+Arrays.toString(options));
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(SETUP_PREFIX);
            if (options.length <= 0) {
                byteBuffer.put((byte)' ');
            } else {
                for (final String option : options) {
                    byteBuffer.put((byte)' ');
                    byteBuffer.put(option.getBytes(UTF8));
                }
            }
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendToggleextendedtext(@NotNull final int... types) {
        if (types.length <= 0) {
            return;
        }

        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send toggleextendedtext types="+Arrays.toString(types));
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(TOGGLEEXTENDEDTEXT_PREFIX);
            for (final int type : types) {
                byteBuffer.put((byte)' ');
                putDecimal(type);
            }
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendVersion(final int csval, final int scval, @NotNull final String vinfo) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send version cs="+csval+" sc="+scval+" info="+vinfo);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(VERSION_PREFIX);
            putDecimal(csval);
            byteBuffer.put((byte)' ');
            putDecimal(scval);
            byteBuffer.put((byte)' ');
            byteBuffer.put(vinfo.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * Appends an integer in decimal ASCII representation to {@link
     * #byteBuffer}.
     * @param value the value to append
     */
    private void putDecimal(final int value) {
        if (value == 0) {
            byteBuffer.put((byte)'0');
        } else {
            final String str = Integer.toString(value);
            try {
                byteBuffer.put(str.getBytes("ISO-8859-1"));
            } catch (final UnsupportedEncodingException ex) {
                throw new AssertionError(ex); // every Java implementation must support UTF-8
            }
        }
    }

    /**
     * Parses a character into an integer.
     * @param ch the character to parse
     * @return the integer representing the character
     * @throws UnknownCommandException if <code>ch</code> is not a digit
     */
    private static int parseDigit(final byte ch) throws UnknownCommandException {
        final int digit = ch-'0';
        if (digit < 0 || digit > 9) {
            throw new UnknownCommandException("not a digit: "+ch);
        }
        return digit;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setPreferredMapSize(final int preferredMapWidth, final int preferredMapHeight) {
        final int preferredMapWidth2 = Math.max(1, preferredMapWidth|1);
        final int preferredMapHeight2 = Math.max(1, preferredMapHeight|1);
        if (this.preferredMapWidth == preferredMapWidth2 && this.preferredMapHeight == preferredMapHeight2) {
            return;
        }

        this.preferredMapWidth = preferredMapWidth2;
        this.preferredMapHeight = preferredMapHeight2;

        negotiateMapSize(this.preferredMapWidth, this.preferredMapHeight);
    }

    /**
     * Sets the current map size as negotiated with the server.
     * @param currentMapWidth the new map width
     * @param currentMapHeight the new map height
     */
    private void setCurrentMapSize(final int currentMapWidth, final int currentMapHeight) {
        if (this.currentMapWidth == currentMapWidth && this.currentMapHeight == currentMapHeight) {
            return;
        }

        this.currentMapWidth = currentMapWidth;
        this.currentMapHeight = currentMapHeight;
        fireNewMap();
    }

    /**
     * Sets the current ground view objects as negotiated with the server.
     * @param currentNumLookObjects the number of objects
     */
    private void setCurrentNumLookObjects(final int currentNumLookObjects) {
        this.currentNumLookObjects = currentNumLookObjects;
    }

    /**
     * Notifies all listeners that a "newmap" command has been received.
     */
    private void fireNewMap() {
        for (final CrossfireUpdateMapListener listener : crossfireUpdateMapListeners) {
            listener.newMap(currentMapWidth, currentMapHeight);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setPreferredNumLookObjects(final int preferredNumLookObjects) {
        final int preferredNumLookObjects2 = Math.max(3, preferredNumLookObjects);
        if (this.preferredNumLookObjects == preferredNumLookObjects2) {
            return;
        }

        this.preferredNumLookObjects = preferredNumLookObjects2;

        negotiateNumLookObjects(this.preferredNumLookObjects);
    }

    /**
     * {@inheritDoc}
     */
    @Nullable
    @Override
    public String getAccountName() {
        return accountName;
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about an empty packet.
     * @param command the command string
     */
    private void notifyPacketWatcherListenersEmpty(@NotNull final String command) {
        for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
            receivedPacketListener.processEmpty(command);
        }
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about a packet having ascii
     * parameters.
     * @param packet the packet contents
     * @param args the start index into <code>packet</code> of the packet's
     * arguments
     */
    private void notifyPacketWatcherListenersAscii(@NotNull final ByteBuffer packet, final int args) {
        if (!receivedPacketListeners.isEmpty()) {
            final String command = extractCommand(packet);
            if (packet.hasRemaining()) { // XXX: should check payload, not whole command?
                for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
                    packet.position(args);
                    receivedPacketListener.processAscii(command, packet);
                }
            } else {
                notifyPacketWatcherListenersEmpty(command);
            }
        }
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about a packet having an
     * array of short values as parameters.
     * @param packet the packet contents
     * @param args the start index into <code>packet</code> of the packet's
     * arguments
     */
    private void notifyPacketWatcherListenersShortArray(@NotNull final ByteBuffer packet, final int args) {
        if (!receivedPacketListeners.isEmpty()) {
            final String command = extractCommand(packet);
            if (packet.hasRemaining()) { // XXX: should check payload, not whole command?
                for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
                    packet.position(args);
                    receivedPacketListener.processShortArray(command, packet);
                }
            } else {
                notifyPacketWatcherListenersEmpty(command);
            }
        }
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about a packet having an
     * array of int values as parameters.
     * @param packet the packet contents
     * @param args the start index into <code>packet</code> of the packet's
     * arguments
     */
    private void notifyPacketWatcherListenersIntArray(@NotNull final ByteBuffer packet, final int args) {
        if (!receivedPacketListeners.isEmpty()) {
            final String command = extractCommand(packet);
            if (packet.hasRemaining()) { // XXX: should check payload, not whole command?
                for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
                    packet.position(args);
                    receivedPacketListener.processIntArray(command, packet);
                }
            } else {
                notifyPacketWatcherListenersEmpty(command);
            }
        }
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about a packet having a
     * short and an in value as parameters.
     * @param packet the packet contents
     * @param args the start index into <code>packet</code> of the packet's
     * arguments
     */
    private void notifyPacketWatcherListenersShortInt(@NotNull final ByteBuffer packet, final int args) {
        if (!receivedPacketListeners.isEmpty()) {
            final String command = extractCommand(packet);
            if (packet.hasRemaining()) { // XXX: should check payload, not whole command?
                for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
                    packet.position(args);
                    receivedPacketListener.processShortInt(command, packet);
                }
            } else {
                notifyPacketWatcherListenersEmpty(command);
            }
        }
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about a packet having mixed
     * parameters.
     * @param packet the packet contents
     * @param args the start index into <code>packet</code> of the packet's
     * arguments
     */
    private void notifyPacketWatcherListenersMixed(@NotNull final ByteBuffer packet, final int args) {
        if (!receivedPacketListeners.isEmpty()) {
            final String command = extractCommand(packet);
            if (packet.hasRemaining()) { // XXX: should check payload, not whole command?
                for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
                    packet.position(args);
                    receivedPacketListener.processMixed(command, packet);
                }
            } else {
                notifyPacketWatcherListenersEmpty(command);
            }
        }
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about a packet having stat
     * parameters.
     * @param stat the stat value
     * @param args the stat arguments depending on <code>type</code> and
     * <code>stat</code>
     */
    private void notifyPacketWatcherListenersStats(final int stat, @NotNull final Object... args) {
        if (!receivedPacketListeners.isEmpty()) {
            for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
                receivedPacketListener.processStats("stats", stat, args);
            }
        }
    }

    /**
     * Notifies all {@link ReceivedPacketListener}s about a packet having
     * unknown parameters.
     * @param packet the packet contents
     * @param args the start index into <code>packet</code> of the packet's
     * arguments
     */
    private void notifyPacketWatcherListenersNoData(@NotNull final ByteBuffer packet, final int args) {
        if (!receivedPacketListeners.isEmpty()) {
            final String command = extractCommand(packet);
            if (packet.hasRemaining()) { // XXX: should check payload, not whole command?
                for (final ReceivedPacketListener receivedPacketListener : receivedPacketListeners) {
                    packet.position(args);
                    receivedPacketListener.processNoData(command, packet);
                }
            } else {
                notifyPacketWatcherListenersEmpty(command);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void connect(@NotNull final String hostname, final int port) {
        accountName = null;
        clearFailure();
        clientSocketState = ClientSocketState.CONNECTING;
        setClientSocketState(ClientSocketState.CONNECTING, ClientSocketState.CONNECTING);
        super.connect(hostname, port);
    }

    /**
     * Updates the {@link #clientSocketState}.
     * @param prevState the expected current state
     * @param nextState the next state
     */
    private void setClientSocketState(@NotNull final ClientSocketState prevState, @NotNull final ClientSocketState nextState) {
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("connection state: "+nextState);
        }
        if (clientSocketState != prevState) {
            System.err.println("Warning: connection state is "+clientSocketState+" when switching to state "+nextState+", expecting state "+prevState);
        }
        clientSocketState = nextState;
        for (final CrossfireServerConnectionListener crossfireServerConnectionListener : crossfireServerConnectionListeners) {
            crossfireServerConnectionListener.clientSocketStateChanged(nextState);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendAccountPlay(@NotNull final String name) {
        clearFailure();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send accountplay "+name);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(ACCOUNT_PLAY_PREFIX);
            byteBuffer.put(name.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }

        final String tmpAccountName = accountName;
        if (tmpAccountName != null) {
            for (final CrossfireAccountListener crossfireAccountListener : crossfireAccountListeners) {
                crossfireAccountListener.selectCharacter(tmpAccountName, name);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendAccountLink(final int force, @NotNull final String login, @NotNull final String password) {
        clearFailure();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send accountaddplayer "+login);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(ACCOUNT_ADD_PLAYER_PREFIX);
            byteBuffer.put((byte)force);
            byteBuffer.put((byte)login.length());
            byteBuffer.put(login.getBytes(UTF8));
            byteBuffer.put((byte)password.length());
            byteBuffer.put(password.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendAccountCreate(@NotNull final String login, @NotNull final String password) {
        clearFailure();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send accountnew "+login);
        }
        accountName = login;
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(ACCOUNT_NEW_PREFIX);
            byteBuffer.put((byte)login.length());
            byteBuffer.put(login.getBytes(UTF8));
            byteBuffer.put((byte)password.length());
            byteBuffer.put(password.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void sendAccountCharacterCreate(@NotNull final String login, @NotNull final String password) {
        clearFailure();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send createplayer "+login);
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(CREATE_PLAYER_PREFIX);
            byteBuffer.put((byte)login.length());
            byteBuffer.put(login.getBytes(UTF8));
            byteBuffer.put((byte)password.length());
            byteBuffer.put(password.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    public void sendAccountPassword(String currentPassword, String newPassword) {
        clearFailure();
        if (debugProtocol != null) {
            debugProtocol.debugProtocolWrite("send accountpw");
        }
        synchronized (writeBuffer) {
            byteBuffer.clear();
            byteBuffer.put(ACCOUNT_PASSWORD_PREFIX);
            byteBuffer.put((byte)currentPassword.length());
            byteBuffer.put(currentPassword.getBytes(UTF8));
            byteBuffer.put((byte)newPassword.length());
            byteBuffer.put(newPassword.getBytes(UTF8));
            writePacket(writeBuffer, byteBuffer.position());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCrossfireFailureListener(@NotNull final CrossfireFailureListener listener) {
        crossfireFailureListeners.add(listener);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeCrossfireFailureListener(@NotNull final CrossfireFailureListener listener) {
        crossfireFailureListeners.remove(listener);
    }

    /**
     * Extracts a string from a {@link ByteBuffer} range.
     * @param byteBuffer the byte buffer
     * @param start the start index of the string
     * @param len the length of the string
     * @return the string
     */
    private static String newString(final ByteBuffer byteBuffer, final int start, final int len) {
        final byte[] tmp = new byte[len];
        for (int i = 0; i < len; i++) {
            tmp[i] = byteBuffer.get(start+i);
        }
        return new String(tmp, UTF8);
    }

    /**
     * Extracts and removes a 1 byte integer from a {@link ByteBuffer} at it's
     * current position.
     * @param byteBuffer the byte buffer
     * @return the integer
     */
    private static int getInt1(@NotNull final ByteBuffer byteBuffer) {
        return byteBuffer.get()&0xFF;
    }

    /**
     * Extracts and removes a 2 byte integer from a {@link ByteBuffer} at it's
     * current position.
     * @param byteBuffer the byte buffer
     * @return the integer
     */
    private static int getInt2(@NotNull final ByteBuffer byteBuffer) {
        return byteBuffer.getShort()&0xFFFF;
    }

    /**
     * Extracts and removes a 4 byte integer from a {@link ByteBuffer} at it's
     * current position.
     * @param byteBuffer the byte buffer
     * @return the integer
     */
    private static int getInt4(@NotNull final ByteBuffer byteBuffer) {
        return byteBuffer.getInt();
    }

    /**
     * Extracts and removes an 8 byte integer from a {@link ByteBuffer} at it's
     * current position.
     * @param byteBuffer the byte buffer
     * @return the integer
     */
    private static long getInt8(@NotNull final ByteBuffer byteBuffer) {
        return byteBuffer.getLong();
    }

    /**
     * Extracts and removes a string from a {@link ByteBuffer} at it's current
     * position.
     * @param byteBuffer the byte buffer
     * @param len the length of the string
     * @return the string
     */
    private static String getString(final ByteBuffer byteBuffer, final int len) {
        final byte[] tmp = new byte[len];
        byteBuffer.get(tmp);
        return new String(tmp, UTF8);
    }

    /**
     * Returns a hex-dump of a {@link ByteBuffer}.
     * @param byteBuffer the byte buffer
     * @return the hex-dump
     */
    @NotNull
    private static String hexDump(@NotNull final ByteBuffer byteBuffer) {
        final int len = byteBuffer.limit();
        final byte[] data = new byte[len];
        for (int i = 0; i < len; i++) {
            data[i] = byteBuffer.get(i);
        }
        return HexCodec.hexDump(data, 0, len);
    }

}
