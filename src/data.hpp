#define AXIS_SIZE 20.0f

#define AXES_VERTICES {\
  /* position                       normal             texture_coord */\
  0.0f,      0.0f,      0.0f,       1.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
  AXIS_SIZE, 0.0f,      0.0f,       1.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
  0.0f,      0.0f,      0.0f,       0.0f, 1.0f, 0.0f,  0.0f, 0.0f,\
  0.0f,      AXIS_SIZE, 0.0f,       0.0f, 1.0f, 0.0f,  0.0f, 0.0f,\
  0.0f,      0.0f,      0.0f,       0.0f, 0.0f, 1.0f,  0.0f, 0.0f,\
  0.0f,      0.0f,      AXIS_SIZE,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f\
}

#define ALPACA_VERTICES {\
  /* position           normal             texture_coords */\
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
   0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
   0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
   0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
  -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
                                                      \
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
   0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
   0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
   0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
                                                      \
  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
  -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
                                                      \
   0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
   0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
   0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
   0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
   0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
   0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
                                                      \
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
   0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
   0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
   0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
                                                      \
  -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
   0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,\
   0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
   0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
  -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f \
}

#define SCREENQUAD_VERTICES {\
  /* position           normal             texture_coords */\
  -1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
  -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f, 0.0f,\
   1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
                                                      \
  -1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  0.0f, 1.0f,\
   1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  1.0f, 0.0f,\
   1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 0.0f,  1.0f, 1.0f \
}
