#ifndef TEXT_ENGINE_H
#define TEXT_ENGINE_H

void text_init(int init_press_start);
void text_draw(int grid_x, int grid_y, const unsigned char *text);
void text_draw_static(int grid_x, int grid_y, const unsigned char *text);

#endif