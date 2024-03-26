#include "stdafx.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "loadscreen_cfg.h"

void parse_cfg(uint8_t* file_buff, cfg_var* vals, size_t size);
void parse_key_val(char* key, char* val, cfg_var* vals, int flt_pos);

bool cfg_read(cfg_var* vals)
{
    FILE* cfg_file = fopen("../gamedata/configs/ui/loadscreen.cfg", "rb");
    if (!cfg_file) {
        return false;
    }
    //printf("read test\n");
    fseek(cfg_file, 0, SEEK_END);
    size_t file_size = ftell(cfg_file);
    fseek(cfg_file, 0, SEEK_SET);
    uint8_t* buffer = (uint8_t*)malloc(file_size);
    fread(buffer, file_size, 1, cfg_file);
    fclose(cfg_file);

    parse_cfg(buffer, vals, file_size);
    return true;
}

bool cfg_write(cfg_var* vals)
{
    FILE* cfg_file = fopen("../gamedata/configs/ui/loadscreen.cfg", "wb");
    //TODO: add folder creation if /configs/ui/ doesn't exist


    if (!cfg_file) {
        //I don't know how local error reporting works
        //so I'm just "printf"ing it just to have an error message
        printf("Error: Can't create loadscreen.cfg");
        return false;
    }

    char buffer[1536];
    snprintf(buffer, sizeof(buffer),
    "#no clue what this is\n"
        "bw=%f\n"
        "bh=%f\n"
    "#or this\n"
        "back_tex_size_x=%f\n"
        "back_tex_size_y=%f\n"
    "#loading bar bottom right corner (width/height)\n"
        "back_size_x=%f\n"
        "back_size_y=%f\n"
    "#what is this offset for?\n"
        "offs=%f\n"
    "#load bar position?\n"
        "back_tex_coords_lt_x=%f\n"
        "back_tex_coords_lt_y=%f\n"
    "#also load bar position?\n"
        "back_coords_lt_x=%f\n"
        "back_coords_lt_y=%f\n"
    "#loading bar crossfade width (inverted?)\n"
        "v_cnt=%d\n"
    "#loading bar color\n"
    "#note: the alpha channel is modified internally\n"
        "r=%f\n"
        "g=%f\n"
        "b=%f\n"
    "#menu background texture size\n"
    "#making this larger shrinks the texture\n"
        "back_tex_size_x2=%f\n"
        "back_tex_size_y2=%f\n"
    "#also affects background texture size?\n"
    "#making this larger stretches the texture?\n"
        "back_size_x2=%f\n"
        "back_size_y2=%f\n"
    "#left panel?\n"
        "back_size_y3=%f\n"
        "back_tex_coords_rb_x=%f\n"
        "back_tex_coords_rb_y=%f\n"
    "#right panel?\n"
        "back_tex_coords_lt_x2=%f\n"
        "back_tex_coords_lt_y2=%f\n"
        "back_tex_coords_rb_x2=%f\n"
        "back_tex_coords_rb_y2=%f\n"
    "#level-specific screenshot position\n"
        "r_lt_x=%f\n"
        "r_lt_y=%f\n"
    "#level-specific screenshot size\n"
        "back_size_x4=%f\n"
        "back_size_y4=%f\n"
    "#logo text coords?\n"
        "logo_tex_coords_lt_x=%f\n"
        "logo_tex_coords_lt_y=%f\n"
        "logo_tex_coords_rb_x=%f\n"
        "logo_tex_coords_rb_y=%f\n"

        "\0",
        vals->bw,                   vals->bh,
        vals->back_tex_size_x,      vals->back_tex_size_y,
        vals->back_size_x,          vals->back_size_y,
        vals->offs,
        vals->back_tex_coords_lt_x, vals->back_tex_coords_lt_y,
        vals->back_coords_lt_x,     vals->back_coords_lt_y,
        vals->v_cnt,
        vals->r,
        vals->g,
        vals->b,
        vals->back_tex_size_x2,     vals->back_tex_size_y2,
        vals->back_size_x2,         vals->back_size_y2,

        vals->back_size_y3,
        vals->back_tex_coords_rb_x, vals->back_tex_coords_rb_y,

        vals->back_tex_coords_lt_x2,vals->back_tex_coords_lt_y2,
        vals->back_tex_coords_rb_x2,vals->back_tex_coords_rb_y2,

        vals->r_lt_x,               vals->r_lt_y,
        vals->back_size_x4,         vals->back_size_y4,

        vals->logo_tex_coords_lt_x, vals->logo_tex_coords_lt_y,
        vals->logo_tex_coords_rb_x, vals->logo_tex_coords_rb_y
    );

    fwrite(buffer, strlen(buffer), 1, cfg_file);
    fclose(cfg_file);
    return true;
}

void parse_cfg(uint8_t* file_buff, cfg_var* vals, size_t size)
{ 
    int i = 0;
    int flt = -1;
    int strt = 0;
    char keybuff[32] = {0};
    char valbuff[32] = {0};
    char* ptr = keybuff;
    while (i < size) {
        switch (file_buff[i])
        {
        //case '<':
        //{
        //    while (file_buff[i] != '>')
        //    {
        //        i++;
        //    }
        //}
        case '\r': case '\n':
        {
            *ptr = '\0';
            if (keybuff[0] != '\0') {
                //printf("key: %s val: %s\n", keybuff, valbuff);
                parse_key_val(keybuff, valbuff, vals, flt);
            }

            ptr = keybuff;
            flt = -1;
            i++;
            break;
        }
        case '#':
        {
            while (file_buff[i] != '\n') {
                i++;
                //printf("file_buff[%d]: %c\n", i, file_buff[i]);
            }
            break;
        }
        case '=':
        {
            *ptr = '\0';
            ptr = valbuff;
            i++;
            strt = i;
            break;
        }
        case '.':
        {
            flt = i - strt;
            //fall through
        }

        default:
        {
            *ptr = file_buff[i];
            ptr++;
            i++;
            break;
        }
        }
    }
}

//convert val from ascii char to int
int parse_int(char* val)
{
    int val_int = 0;
    int arr_len = strlen(val);
    bool is_neg = false;
    int i = 0;
    if (val[0] == '-') {
        i++;
        is_neg = true;
    }
    for (; i < arr_len; i++)
    {
        val_int += (val[i] - '0') * pow(10, arr_len - i - 1);
    }
    if (is_neg) {
        val_int *= -1;
    }
    //printf("final val_int: %d\n", val_int);
    return val_int;
}

//convert val from ascii char to float
float parse_flt(char* val, int flt_pos)
{
    float val_flt = 0.0f;
    int arr_len = strlen(val);
    bool is_neg = false;
    int i = 0;
    if (val[0] == '-') {
        i++;
        is_neg = true;
    }
    while (i < arr_len - 1) {
        if (i < flt_pos) {
            //float value greater than 1
            val_flt += (val[i] - '0') * pow(10, flt_pos - i - 1);
            //printf("i: %d val_flt: %f flt_pos-i-1: %d\n", i, val_flt, flt_pos - i - 1);
            i++;
        }
        else {
            //float value fraction of 1
            i++;
            val_flt += (val[i] - '0') * pow(10, flt_pos - i);
            //printf("arr_len: %d flt_pos: %d i: %d\n", arr_len, flt_pos, i);
            //printf("i: %d val_flt: %f flt_pos-i-1: %d\n", i, val_flt, flt_pos - i);
        }
    } 
    if (is_neg) {
        val_flt *= -1;
    }

    //printf("final val_flt: %f\n", val_flt);
    return val_flt;
}

void parse_key_val(char* key, char* val, cfg_var* vals, int flt_pos)
{
    int val_int = 0;
    float val_flt = 0.0f;
    //if (val[1] == 'x' || val[1] == 'X') {
    //    val_int = (int)strtoul(val, NULL, 16);
    //    printf("clr hex val_int: 0x%08X\n", val_int);
    //}
    //else {
        if (flt_pos > -1) {
            val_flt = parse_flt(val, flt_pos);
        }
        else {
            val_int = parse_int(val);
        }
    //}

    char* match_key[40] = {
        "bw",
        "bh",
        "back_tex_size_x",
        "back_tex_size_y",
        "back_size_x",
        "back_size_y",
        "offs",
        "back_tex_coords_lt_x",
        "back_tex_coords_lt_y",
        "back_coords_lt_x",
        "back_coords_lt_y",
        "v_cnt",
        "r",
        "g",
        "b",
        "back_tex_size_x2",
        "back_tex_size_y2",
        "back_size_x2",
        "back_size_y2",
        "back_size_x3",
        "back_size_y3",
        "back_tex_coords_rb_x",
        "back_tex_coords_rb_y",
        "back_tex_coords_lt_x2",
        "back_tex_coords_lt_y2",
        "back_tex_coords_rb_x2",
        "back_tex_coords_rb_y2",
        "r_lt_x",
        "r_lt_y",
        "back_size_x4",
        "back_size_y4",
        "logo_tex_coords_lt_x",
        "logo_tex_coords_lt_y",
        "logo_tex_coords_rb_x",
        "logo_tex_coords_rb_y",
    };

    void* match_val[40] = {
        &vals->bw,
        &vals->bh,
        &vals->back_tex_size_x,
        &vals->back_tex_size_y,
        &vals->back_size_x,
        &vals->back_size_y,
        &vals->offs,
        &vals->back_tex_coords_lt_x,
        &vals->back_tex_coords_lt_y,
        &vals->back_coords_lt_x,
        &vals->back_coords_lt_y,
        &vals->v_cnt,
        &vals->r,
        &vals->g,
        &vals->b,
        &vals->back_tex_size_x2,
        &vals->back_tex_size_y2,
        &vals->back_size_x2,
        &vals->back_size_y2,
        &vals->back_size_x3,
        &vals->back_size_y3,
        &vals->back_tex_coords_rb_x,
        &vals->back_tex_coords_rb_y,
        &vals->back_tex_coords_lt_x2,
        &vals->back_tex_coords_lt_y2,
        &vals->back_tex_coords_rb_x2,
        &vals->back_tex_coords_rb_y2,
        &vals->r_lt_x,
        &vals->r_lt_y,
        &vals->back_size_x4,
        &vals->back_size_y4,
        &vals->logo_tex_coords_lt_x,
        &vals->logo_tex_coords_lt_y,
        &vals->logo_tex_coords_rb_x,
        &vals->logo_tex_coords_rb_y,
    };

    if (flt_pos > -1) {
        for (int i = 0; i < 40; i++)
        {
            //printf("%s : %s\n", key, match_key[i]);
            if (strcmp(key, match_key[i]) == 0) {
                //printf("%s : %s\n", key, match_key[i]);
                *(float*)(match_val[i]) = val_flt;
                //printf("match_key[i]: %s *(float*)(match_val[i]): %f\n", match_key[i], *(float*)(match_val[i]));
                break;
            }
        }
    }
    else {
        //*(unsigned int*)(match_val[11]) = val_int;
        //printf("match_key[11]: %s match_val[11]: %f *(unsigned int*)(match_val[11]): %f\n", match_key[11], match_val[11], *(unsigned int*)(match_val[11]));
        vals->v_cnt = val_int;
        //printf("key: %s vals->v_cnt: %d\n", key, vals->v_cnt);
    }


    ////check key and assign appropriately
    //if (strcmp(key, "bw") == 0) {
    //    vals->bw = val_flt;
    //}
    //if (strcmp(key, "bh") == 0) {
    //    vals->bh = val_flt;
    //}
    //if (strcmp(key, "back_tex_size_x") == 0) {
    //    vals->back_tex_size_x = val_flt;
    //}
    //if (strcmp(key, "back_tex_size_y") == 0) {
    //    vals->back_tex_size_y = val_flt;
    //}
    //if (strcmp(key, "back_size_x") == 0) {
    //    vals->back_size_x = val_flt;
    //}
    //if (strcmp(key, "back_size_y") == 0) {
    //    vals->back_size_y = val_flt;
    //}
    //if (strcmp(key, "offs") == 0) {
    //    vals->offs = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_lt_x") == 0) {
    //    vals->back_tex_coords_lt_x = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_lt_y") == 0) {
    //    vals->back_tex_coords_lt_y = val_flt;
    //}
    //if (strcmp(key, "back_coords_lt_x") == 0) {
    //    vals->back_coords_lt_x = val_flt;
    //}
    //if (strcmp(key, "back_coords_lt_y") == 0) {
    //    vals->back_coords_lt_y = val_flt;
    //}
    //if (strcmp(key, "v_cnt") == 0) {
    //    vals->v_cnt = val_int;
    //}
    //if (strcmp(key, "r") == 0) {
    //    vals->r = val_flt;
    //}
    //if (strcmp(key, "g") == 0) {
    //    vals->g = val_flt;
    //}
    //if (strcmp(key, "b") == 0) {
    //    vals->b = val_flt;
    //}
    //if (strcmp(key, "back_tex_size_x2") == 0) {
    //    vals->back_tex_size_x2 = val_flt;
    //}
    //if (strcmp(key, "back_tex_size_y2") == 0) {
    //    vals->back_tex_size_y2 = val_flt;
    //}
    //if (strcmp(key, "back_size_x2") == 0) {
    //    vals->back_size_x2 = val_flt;
    //}
    //if (strcmp(key, "back_size_y2") == 0) {
    //    vals->back_size_y2 = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_rb_x") == 0) {
    //    vals->back_tex_coords_rb_x = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_rb_y") == 0) {
    //    vals->back_tex_coords_rb_y = val_flt;
    //}
    //if (strcmp(key, "back_size_x3") == 0) {
    //    vals->back_size_x3 = val_flt;
    //}
    //if (strcmp(key, "back_size_y3") == 0) {
    //    vals->back_size_y3 = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_lt_x2") == 0) {
    //    vals->back_tex_coords_lt_x2 = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_lt_y2") == 0) {
    //    vals->back_tex_coords_lt_y2 = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_rb_x2") == 0) {
    //    vals->back_tex_coords_rb_x2 = val_flt;
    //}
    //if (strcmp(key, "back_tex_coords_rb_y2") == 0) {
    //    vals->back_tex_coords_rb_y2 = val_flt;
    //}
}