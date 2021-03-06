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
 * i18n support definitions.
 * This one-liner will check to be sure the defines are lined up properly
 * with the values they represent:<br>
 * awk -v FS="[ _]" '/I18N_MSG/ { if ($5 != $6) print $5, $6 }' languages.h
 */
#ifndef LANGUAGES_H
#define LANGUAGES_H

#define NUM_LANGUAGES 5
#define NUM_I18N_STRINGS   218
extern const char *language_codes[]; /**< Language codes ("en", "fr", etc.) table */
extern const char *language_names[]; /**< Language names ("English", "French", etc.) table */
extern const char *i18n_strings[NUM_LANGUAGES][NUM_I18N_STRINGS]; /**< Translation table */

#define I18N_MSG_CMISC_000 0
#define I18N_MSG_CMISC_001 1
#define I18N_MSG_CMISC_002 2
#define I18N_MSG_CMISC_003 3
#define I18N_MSG_CMISC_004 4
#define I18N_MSG_CMISC_005 5
#define I18N_MSG_CMISC_006 6
#define I18N_MSG_CMISC_007 7
#define I18N_MSG_CMISC_008 8
#define I18N_MSG_CMISC_009 9
#define I18N_MSG_CMISC_010 10
#define I18N_MSG_CMISC_011 11
#define I18N_MSG_CMISC_012 12
#define I18N_MSG_CMISC_013 13
#define I18N_MSG_CMISC_014 14
#define I18N_MSG_CMISC_015 15
#define I18N_MSG_CMISC_016 16
#define I18N_MSG_CMISC_017 17
#define I18N_MSG_CMISC_018 18
#define I18N_MSG_CMISC_019 19
#define I18N_MSG_CMISC_020 20
#define I18N_MSG_CMISC_021 21
#define I18N_MSG_CMISC_022 22
#define I18N_MSG_CMISC_023 23
#define I18N_MSG_CMISC_024 24
#define I18N_MSG_CMISC_025 25
#define I18N_MSG_CMISC_026 26
#define I18N_MSG_CMISC_027 27
#define I18N_MSG_CMISC_028 28
#define I18N_MSG_CMISC_029 29
#define I18N_MSG_CMISC_030 30
#define I18N_MSG_CMISC_031 31
#define I18N_MSG_CMISC_032 32
#define I18N_MSG_CMISC_033 33
#define I18N_MSG_CMISC_034 34
#define I18N_MSG_CMISC_035 35
#define I18N_MSG_CMISC_036 36
#define I18N_MSG_CMISC_037 37
#define I18N_MSG_CMISC_038 38
#define I18N_MSG_CMISC_039 39
#define I18N_MSG_CMISC_040 40
#define I18N_MSG_CMISC_041 41
#define I18N_MSG_CMISC_042 42
#define I18N_MSG_CMISC_043 43
#define I18N_MSG_CMISC_044 44
#define I18N_MSG_CMISC_045 45
#define I18N_MSG_CMISC_046 46
#define I18N_MSG_CMISC_047 47
#define I18N_MSG_CMISC_048 48
#define I18N_MSG_CMISC_049 49
#define I18N_MSG_CMISC_050 50
#define I18N_MSG_CMISC_051 51
#define I18N_MSG_CMISC_052 52
#define I18N_MSG_CMISC_053 53
#define I18N_MSG_CMISC_054 54
#define I18N_MSG_CMISC_055 55
#define I18N_MSG_CMISC_056 56
#define I18N_MSG_CMISC_057 57
#define I18N_MSG_CMISC_058 58
#define I18N_MSG_CMISC_059 59
#define I18N_MSG_CMISC_060 60
#define I18N_MSG_CMISC_061 61
#define I18N_MSG_CMISC_062 62
#define I18N_MSG_CMISC_063 63
#define I18N_MSG_CMISC_064 64
#define I18N_MSG_CMISC_065 65
#define I18N_MSG_CMISC_066 66
#define I18N_MSG_CMISC_067 67
#define I18N_MSG_CMISC_068 68
#define I18N_MSG_CMISC_069 69
#define I18N_MSG_CMISC_070 70
#define I18N_MSG_CMISC_071 71
#define I18N_MSG_CMISC_072 72
#define I18N_MSG_CMISC_073 73
#define I18N_MSG_CMISC_074 74
#define I18N_MSG_CMISC_075 75
#define I18N_MSG_CMISC_076 76
#define I18N_MSG_CMISC_077 77
#define I18N_MSG_CMISC_078 78
#define I18N_MSG_CMISC_079 79
#define I18N_MSG_CMISC_080 80
#define I18N_MSG_CMISC_081 81
#define I18N_MSG_CMISC_082 82
#define I18N_MSG_CMISC_083 83
#define I18N_MSG_CMISC_084 84
#define I18N_MSG_CMISC_085 85
#define I18N_MSG_CMISC_086 86
#define I18N_MSG_CMISC_087 87
#define I18N_MSG_CMISC_088 88
#define I18N_MSG_CMISC_089 89
#define I18N_MSG_CMISC_090 90
#define I18N_MSG_CMISC_091 91
#define I18N_MSG_CMISC_092 92
#define I18N_MSG_CMISC_093 93
#define I18N_MSG_CMISC_094 94
#define I18N_MSG_CMISC_095 95
#define I18N_MSG_CMISC_096 96
#define I18N_MSG_CMISC_097 97
#define I18N_MSG_CMISC_098 98
#define I18N_MSG_CMISC_099 99
#define I18N_MSG_CMISC_100 100
#define I18N_MSG_CMISC_101 101
#define I18N_MSG_CMISC_102 102
#define I18N_MSG_CMISC_103 103
#define I18N_MSG_CMISC_104 104
#define I18N_MSG_CMISC_105 105
#define I18N_MSG_CMISC_106 106
#define I18N_MSG_CMISC_107 107
#define I18N_MSG_CMISC_108 108
#define I18N_MSG_CMISC_109 109
#define I18N_MSG_CMISC_110 110
#define I18N_MSG_CMISC_111 111
#define I18N_MSG_CMISC_112 112
#define I18N_MSG_CMISC_113 113
#define I18N_MSG_CMISC_114 114
#define I18N_MSG_CMISC_115 115
#define I18N_MSG_CMISC_116 116
#define I18N_MSG_CMISC_117 117
#define I18N_MSG_CMISC_118 118
#define I18N_MSG_CMISC_119 119
#define I18N_MSG_CMISC_120 120
#define I18N_MSG_CMISC_121 121
#define I18N_MSG_CMISC_122 122
#define I18N_MSG_CMISC_123 123
#define I18N_MSG_CMISC_124 124
#define I18N_MSG_CMISC_125 125
#define I18N_MSG_CMISC_126 126
#define I18N_MSG_CMISC_127 127
#define I18N_MSG_CMISC_128 128
#define I18N_MSG_CMISC_129 129
#define I18N_MSG_CMISC_130 130
#define I18N_MSG_CMISC_131 131
#define I18N_MSG_CMISC_132 132
#define I18N_MSG_CMISC_133 133
#define I18N_MSG_CMISC_134 134
#define I18N_MSG_CMISC_135 135
#define I18N_MSG_CMISC_136 136
#define I18N_MSG_CMISC_137 137
#define I18N_MSG_CMISC_138 138
#define I18N_MSG_CMISC_139 139
#define I18N_MSG_CMISC_140 140
#define I18N_MSG_CMISC_141 141
#define I18N_MSG_CMISC_142 142
#define I18N_MSG_CMISC_143 143
#define I18N_MSG_CMISC_144 144
#define I18N_MSG_CMISC_145 145
#define I18N_MSG_CMISC_146 146
#define I18N_MSG_CMISC_147 147
#define I18N_MSG_CMISC_148 148
#define I18N_MSG_CMISC_149 149
#define I18N_MSG_CMISC_150 150
#define I18N_MSG_CMISC_151 151
#define I18N_MSG_CMISC_152 152
#define I18N_MSG_CMISC_153 153
#define I18N_MSG_CMISC_154 154
#define I18N_MSG_CMISC_155 155
#define I18N_MSG_CMISC_156 156
#define I18N_MSG_CMISC_157 157
#define I18N_MSG_CMISC_158 158
#define I18N_MSG_CMISC_159 159
#define I18N_MSG_CMISC_160 160
#define I18N_MSG_CMISC_161 161
#define I18N_MSG_CMISC_162 162
#define I18N_MSG_CMISC_163 163
#define I18N_MSG_CMISC_164 164
#define I18N_MSG_CMISC_165 165
#define I18N_MSG_CMISC_166 166
#define I18N_MSG_CMISC_167 167
#define I18N_MSG_CMISC_168 168
#define I18N_MSG_CMISC_169 169
#define I18N_MSG_CMISC_170 170
#define I18N_MSG_CMISC_171 171
#define I18N_MSG_CMISC_172 172
#define I18N_MSG_CMISC_173 173
#define I18N_MSG_CMISC_174 174
#define I18N_MSG_CMISC_175 175
#define I18N_MSG_CMISC_176 176
#define I18N_MSG_CMISC_177 177
#define I18N_MSG_CMISC_178 178
#define I18N_MSG_CMISC_179 179
#define I18N_MSG_CMISC_180 180
#define I18N_MSG_CMISC_181 181
#define I18N_MSG_CMISC_182 182
#define I18N_MSG_CMISC_183 183
#define I18N_MSG_CMISC_184 184
#define I18N_MSG_CMISC_185 185
#define I18N_MSG_CMISC_186 186
#define I18N_MSG_CMISC_187 187
#define I18N_MSG_CMISC_188 188
#define I18N_MSG_CMISC_189 189
#define I18N_MSG_CMISC_190 190
#define I18N_MSG_CMISC_191 191
#define I18N_MSG_CMISC_192 192
#define I18N_MSG_CMISC_193 193
#define I18N_MSG_CMISC_194 194
#define I18N_MSG_CMISC_195 195
#define I18N_MSG_CMISC_196 196
#define I18N_MSG_CMISC_197 197
#define I18N_MSG_CMISC_198 198
#define I18N_MSG_CMISC_199 199
#define I18N_MSG_CMISC_200 200
#define I18N_MSG_CMISC_201 201
#define I18N_MSG_CMISC_202 202
#define I18N_MSG_CMISC_203 203
#define I18N_MSG_CMISC_204 204
#define I18N_MSG_CMISC_205 205
#define I18N_MSG_CMISC_206 206
#define I18N_MSG_CMISC_207 207
#define I18N_MSG_CMISC_208 208
#define I18N_MSG_CMISC_209 209
#define I18N_MSG_CMISC_210 210
#define I18N_MSG_CMISC_211 211
#define I18N_MSG_CMISC_212 212
#define I18N_MSG_CMISC_213 213
#define I18N_MSG_CMISC_214 214
#define I18N_MSG_CMISC_215 215
#define I18N_MSG_CMISC_216 216
#define I18N_MSG_CMISC_217 217

#endif /* LANGUAGES_H */
