const char *const def_keys[] = {
"#\n",
"# Default keys\n",
"#\n",
"# <keysym> <keynro> <flags> <string>\n",
"#\n",
"# <keynro> will typically be the keycode for that key.  While\n",
"# keysyms are defined to work across different servers, keycodes are\n",
"# server/machine specific. Keycodes are only needed because some keyboards\n",
"# (sun's type 5 for one) have keys with no corresponding keysym.\n",
"# When loading, all keysyms are converted to keycodes for matching.\n",
"#\n",
"# flags:\n",
"#   N - Normal mode\n",
"#   F - Fire mode\n",
"#   R - Run mode\n",
"#   A - All modes\n",
"#   E - Leave in line edit mode\n",
"#\n",
"#quotedbl 1 AE  \n",
"quotedbl 1 AE say \n",
"period 1 N stay fire\n",
"# nethack keys\n",
"h 1 A west\n",
"j 1 A south\n",
"k 1 A north\n",
"l 1 A east\n",
"y 1 A northwest\n",
"u 1 A northeast\n",
"b 1 A southwest\n",
"n 1 A southeast\n",
"H 1 A west\n",
"J 1 A south\n",
"K 1 A north\n",
"L 1 A east\n",
"Y 1 A northwest\n",
"U 1 A northeast\n",
"B 1 A southwest\n",
"N 1 A southeast\n",
"# default cursorkeys\n",
"Up 1 A north\n",
"Down 1 A south\n",
"Left 1 A west\n",
"Right 1 A east\n",
"# numpad\n",
"Up 76 A north\n",
"Down 120 A south\n",
"Left 98 A west\n",
"Right 100 A east\n",
"KP_8 1 A north\n",
"KP_2 1 A south\n",
"KP_4 1 A west\n",
"KP_6 1 A east\n",
"KP_7 1 A northwest\n",
"KP_9 1 A northeast\n",
"KP_5 1 A stay\n",
"KP_1 1 A southwest\n",
"KP_3 1 A southeast\n",
"# Sun type 4 keyboards will often have different key names.\n",
"KP_Up 1 A north\n",
"KP_Down 1 A south\n",
"KP_Right 1 A east\n",
"KP_Left 1 A west\n",
"KP_Home 1 A northwest\n",
"KP_Prior 1 A northeast\n",
"#Unfortunately, there is no name for the middle key.\n",
"#KP_5 1 A stay\n",
"KP_End 1 A southwest\n",
"KP_Next 1 A southeast\n",
"F28 1 A north\n",
"F34 1 A south\n",
"F30 1 A west\n",
"F32 1 A east\n",
"F27 1 A northwest\n",
"F29 1 A northeast\n",
"F31 1 A stay\n",
"F33 1 A southwest\n",
"F35 1 A southeast\n",
"#\n",
"a 1 N apply\n",
"A 1 N apply inventory\n",
"a 1 RF apply\n",
"A 1 RF apply\n",
"d 1 N disarm\n",
"e 1 N examine inventory\n",
"E 1 N examine inventory\n",
"e 1 RF examine\n",
"E 1 RF examine\n",
"s 1 A search\n",
"S 1 A brace\n",
"t 1 A ready_skill throw\n",
"x 1 NF show\n",
"comma 1 A take\n",
"Tab 1 N rotatespells 1\n",
"Tab 1 FR rotatespells -1\n",
"question 1 A help\n",
"KP_Add 1 A rotateshoottype\n",
"KP_Subtract 1 A rotateshoottype -\n",
"minus 1 N rotateshoottype -1\n",
"plus 1 NF rotateshoottype\n",
"at 1 A pickup\n",
};
