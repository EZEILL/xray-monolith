#pragma once
#include <stdio.h>

struct cfg_var {
    bool has_cfg = false;
    bool read_once = true;
    bool write_once = true;
    //bool write_screenshot = true;

    float bw = 0.0f;
    float bh = 0.0f;
    float back_tex_size_x = 0.0f;
    float back_tex_size_y = 0.0f;
    float back_size_x = 0.0f;
    float back_size_y = 0.0f;
    float offs = 0.0f;
    float back_tex_coords_lt_x = 0.0f;
    float back_tex_coords_lt_y = 0.0f;
    float back_coords_lt_x = 0.0f;
    float back_coords_lt_y = 0.0f;

    unsigned int v_cnt = 0;

    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;

    float back_tex_size_x2 = 0.0f;
    float back_tex_size_y2 = 0.0f;
    float back_size_x2 = 0.0f;
    float back_size_y2 = 0.0f;

    float back_size_x3 = 0.0f;
    float back_size_y3 = 0.0f;
    float back_tex_coords_rb_x = 0.0f;
    float back_tex_coords_rb_y = 0.0f;

    float back_tex_coords_lt_x2 = 0.0f;
    float back_tex_coords_lt_y2 = 0.0f;
    float back_tex_coords_rb_x2 = 0.0f;
    float back_tex_coords_rb_y2 = 0.0f;

    //draw level-specific screenshot
    float r_lt_x = 0.0f;
    float r_lt_y = 0.0f;
    float back_size_x4 = 0.0f;
    float back_size_y4 = 0.0f;

    //draw logo?
    float logo_tex_coords_lt_x = 0.0f;
    float logo_tex_coords_lt_y = 0.0f;
    float logo_tex_coords_rb_x = 0.0f;
    float logo_tex_coords_rb_y = 0.0f;

};

//bool cfg_check(cfg_var* cfg_vars);
bool cfg_write(cfg_var* vals);
bool cfg_read(cfg_var* vals);
