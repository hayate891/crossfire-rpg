/*
 * static char *rcsid_door_c =
 *   "$Id$";
 */

/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2002 Mark Wedel & Crossfire Development Team
    Copyright (C) 1992 Frank Tore Johansen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The authors can be reached via e-mail at crossfire-devel@real-time.com
*/

/**
 * @file
 * door-related functions.
 */

#include <global.h>
#include <random_map.h>
#include <rproto.h>

/**
 * Serch for doors or walls around a spot.
 * @param layout
 * maze.
 * @param i
 * @param j
 * coordinates to check.
 * @param Xsize
 * @param Ysize
 * maze size.
 * @return
 * Combination of flags:
 * - 1 = door or wall to left.
 * - 2 = door or wall to right.
 * - 4 = door or wall above.
 * - 8 = door or wall below.
 */
int surround_check2(char **layout, int i, int j, int Xsize, int Ysize) {
    int surround_index = 0;

    if ((i > 0) && (layout[i-1][j] == 'D' || layout[i-1][j] == '#'))
        surround_index += 1;
    if ((i < Xsize-1) && (layout[i+1][j] == 'D' || layout[i+1][j] == '#'))
        surround_index += 2;
    if ((j > 0) && (layout[i][j-1] == 'D' || layout[i][j-1] == '#'))
        surround_index += 4;
    if ((j < Ysize-1) && (layout[i][j+1] == 'D' || layout[i][j+1] == '#'))
        surround_index += 8;
    return surround_index;
}

/**
 * Add doors to a map.
 * @param the_map
 * map we're adding doors to.
 * @param maze
 * maze layout.
 * @param doorstyle
 * door style to be. If "none", won't do anything. If NULL, will choose one randomly.
 * @param RP
 * random map parameters.
 */
void put_doors(mapstruct *the_map, char **maze, const char *doorstyle, RMParms *RP) {
    int i, j;
    mapstruct *vdoors;
    mapstruct *hdoors;
    char doorpath[128];

    if (!strcmp(doorstyle, "none"))
        return;
    vdoors = find_style("/styles/doorstyles", doorstyle, -1);
    if (vdoors)
        hdoors = vdoors;
    else {
        vdoors = find_style("/styles/doorstyles/vdoors", doorstyle, -1);
        if (!vdoors)
            return;
        snprintf(doorpath, sizeof(doorpath), "/styles/doorstyles/hdoors%s", strrchr(vdoors->path, '/'));
        hdoors = find_style(doorpath, NULL, -1);
    }

    for (i = 0; i < RP->Xsize; i++)
        for (j = 0; j < RP->Ysize; j++) {
            if (maze[i][j] == 'D') {
                int sindex;
                object *this_door, *new_door;

                sindex = surround_check2(maze, i, j, RP->Xsize, RP->Ysize);
                if (sindex == 3)
                    this_door = pick_random_object(hdoors);
                else
                    this_door = pick_random_object(vdoors);
                new_door = arch_to_object(this_door->arch);
                object_copy(this_door, new_door);
                object_insert_in_map_at(new_door, the_map, NULL, 0, i, j);
            }
        }
}
