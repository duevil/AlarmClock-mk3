#ifndef MATRIX_FONT_H
#define MATRIX_FONT_H

#include <MD_MAX72xx.h>


/**
 * @brief The font used by the matrix display
 * A custom font containing only the following characters:\n
 * - Digits 0-9\n
 * - Letters A-Z and a-z\n
 * - Special characters: .: and space\n
 * - The degree sign (*), the bell character ($) and the bell outline (%)\n
 * - Subscript digits 0-9 (192-201)
 * @see <a href="https://pjrp.github.io/MDParolaFontEditor/">MDParolaFontEditor</a>
 */
constexpr MD_MAX72XX::fontType_t matrix_font[] = {
    0, // 0
    0, // 1
    0, // 2
    0, // 3
    0, // 4
    0, // 5
    0, // 6
    0, // 7
    0, // 8
    0, // 9
    0, // 10
    0, // 11
    0, // 12
    0, // 13
    0, // 14
    0, // 15
    0, // 16
    0, // 17
    0, // 18
    0, // 19
    0, // 20
    0, // 21
    0, // 22
    0, // 23
    0, // 24
    0, // 25
    0, // 26
    0, // 27
    0, // 28
    0, // 29
    0, // 30
    0, // 31
    1, 0, // 32
    0, // 33
    0, // 34
    5, 48, 44, 98, 44, 48, // 35
    5, 48, 60, 126, 60, 48, // 36
    7, 70, 41, 22, 8, 52, 74, 49, // 37
    0, // 38
    0, // 39
    0, // 40
    0, // 41
    0, // 42
    0, // 43
    0, // 44
    0, // 45
    2, 64, 0, // 46
    0, // 47
    5, 254, 129, 129, 127, 0, // 48
    5, 8, 4, 2, 255, 0, // 49
    5, 226, 145, 137, 135, 0, // 50
    5, 129, 137, 137, 119, 0, // 51
    5, 12, 10, 9, 255, 0, // 52
    5, 135, 137, 137, 113, 0, // 53
    5, 254, 137, 137, 121, 0, // 54
    5, 3, 1, 249, 7, 0, // 55
    5, 246, 137, 137, 119, 0, // 56
    5, 142, 137, 137, 127, 0, // 57
    2, 36, 0, // 58
    0, // 59
    0, // 60
    0, // 61
    0, // 62
    0, // 63
    0, // 64
    5, 126, 9, 9, 126, 0, // 65
    5, 127, 73, 73, 54, 0, // 66
    5, 62, 65, 65, 34, 0, // 67
    5, 127, 65, 65, 62, 0, // 68
    5, 127, 73, 73, 65, 0, // 69
    5, 127, 9, 9, 1, 0, // 70
    5, 62, 65, 73, 58, 0, // 71
    5, 127, 8, 8, 127, 0, // 72
    4, 65, 127, 65, 0, // 73
    4, 65, 65, 63, 0, // 74
    5, 127, 8, 20, 99, 0, // 75
    5, 127, 64, 64, 64, 0, // 76
    6, 127, 2, 12, 2, 127, 0, // 77
    5, 127, 12, 24, 127, 0, // 78
    5, 62, 65, 65, 62, 0, // 79
    5, 127, 9, 9, 6, 0, // 80
    5, 62, 65, 33, 94, 0, // 81
    5, 127, 9, 9, 118, 0, // 82
    5, 38, 73, 73, 50, 0, // 83
    4, 1, 127, 1, 0, // 84
    5, 127, 64, 64, 63, 0, // 85
    5, 63, 64, 64, 63, 0, // 86
    6, 63, 64, 60, 64, 63, 0, // 87
    5, 119, 8, 8, 119, 0, // 88
    5, 3, 4, 120, 7, 0, // 89
    5, 113, 73, 69, 67, 0, // 90
    0, // 91
    0, // 92
    0, // 93
    0, // 94
    0, // 95
    0, // 96
    5, 32, 84, 84, 120, 0, // 97
    5, 127, 68, 68, 56, 0, // 98
    5, 56, 68, 68, 68, 0, // 99
    5, 56, 68, 68, 127, 0, // 100
    5, 56, 84, 84, 8, 0, // 101
    5, 4, 126, 5, 5, 0, // 102
    5, 152, 164, 164, 124, 0, // 103
    5, 127, 4, 4, 120, 0, // 104
    3, 4, 125, 0, // 105
    3, 132, 125, 0, // 106
    5, 127, 16, 40, 68, 0, // 107
    4, 63, 64, 64, 0, // 108
    6, 124, 4, 120, 4, 120, 0, // 109
    5, 124, 4, 4, 120, 0, // 110
    5, 56, 68, 68, 56, 0, // 111
    5, 252, 36, 36, 24, 0, // 112
    5, 24, 36, 52, 248, 0, // 113
    5, 124, 8, 4, 4, 0, // 114
    5, 72, 84, 84, 36, 0, // 115
    5, 4, 63, 68, 68, 0, // 116
    5, 60, 64, 64, 124, 0, // 117
    5, 60, 64, 64, 60, 0, // 118
    6, 60, 64, 48, 64, 60, 0, // 119
    5, 108, 16, 16, 108, 0, // 120
    5, 156, 160, 96, 28, 0, // 121
    5, 100, 84, 84, 76, 0, // 122
    0, // 123
    0, // 124
    0, // 125
    0, // 126
    0, // 127
    0, // 128
    0, // 129
    0, // 130
    0, // 131
    0, // 132
    0, // 133
    0, // 134
    0, // 135
    0, // 136
    0, // 137
    0, // 138
    0, // 139
    0, // 140
    0, // 141
    0, // 142
    0, // 143
    0, // 144
    0, // 145
    0, // 146
    0, // 147
    0, // 148
    0, // 149
    0, // 150
    0, // 151
    0, // 152
    0, // 153
    0, // 154
    0, // 155
    0, // 156
    0, // 157
    0, // 158
    0, // 159
    0, // 160
    0, // 161
    0, // 162
    0, // 163
    0, // 164
    0, // 165
    0, // 166
    0, // 167
    0, // 168
    0, // 169
    0, // 170
    0, // 171
    0, // 172
    0, // 173
    0, // 174
    0, // 175
    3, 2, 5, 2, // 176
    0, // 177
    0, // 178
    0, // 179
    0, // 180
    0, // 181
    0, // 182
    0, // 183
    0, // 184
    0, // 185
    0, // 186
    0, // 187
    0, // 188
    0, // 189
    0, // 190
    0, // 191
    4, 248, 136, 120, 0, // 192
    4, 32, 16, 248, 0, // 193
    4, 200, 168, 184, 0, // 194
    4, 168, 168, 120, 0, // 195
    4, 48, 40, 248, 0, // 196
    4, 184, 168, 104, 0, // 197
    4, 240, 168, 232, 0, // 198
    4, 8, 232, 24, 0, // 199
    4, 248, 168, 120, 0, // 200
    4, 184, 168, 120, 0, // 201
    0, // 202
    0, // 203
    0, // 204
    0, // 205
    0, // 206
    0, // 207
    0, // 208
    0, // 209
    0, // 210
    0, // 211
    0, // 212
    0, // 213
    0, // 214
    0, // 215
    0, // 216
    0, // 217
    0, // 218
    0, // 219
    0, // 220
    0, // 221
    0, // 222
    0, // 223
    0, // 224
    0, // 225
    0, // 226
    0, // 227
    0, // 228
    0, // 229
    0, // 230
    0, // 231
    0, // 232
    0, // 233
    0, // 234
    0, // 235
    0, // 236
    0, // 237
    0, // 238
    0, // 239
    0, // 240
    0, // 241
    0, // 242
    0, // 243
    0, // 244
    0, // 245
    0, // 246
    0, // 247
    0, // 248
    0, // 249
    0, // 250
    0, // 251
    0, // 252
    0, // 253
    0, // 254
    0, // 255
};


inline void matrix_font_to_subscript(char& c)
{
    if (isDigit(c))
    {
        c = static_cast<char>(192 + c - '0');
    }
}


#endif //MATRIX_FONT_H
