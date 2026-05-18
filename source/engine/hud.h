#ifndef HUD_H
#define HUD_H

void hud_init(int score, int lives);
void hud_update();
void update_score(int points);
void update_lives(int lives);
void hud_clean();

#endif