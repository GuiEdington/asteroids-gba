
// physics.c
#include "physics.h"

int check_circle_collision(int x1, int y1, int r1, int x2, int y2, int r2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int distance_squared = (dx * dx) + (dy * dy);
    
    int radii_sum = r1 + r2;
    int radii_squared = radii_sum * radii_sum;
    
    return (distance_squared <= radii_squared);
}