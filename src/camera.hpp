#ifndef CAMERA_H
#define CAMERA_H

#include "gl.hpp"


void camera_update_matrix(State *state);
void camera_move_front_back(State *state, real32 sign);
void camera_move_left_right(State *state, real32 sign);
void camera_move_up_down(State *state, real32 sign);
void camera_update_mouse(State *state, real64 x, real64 y);

#endif
