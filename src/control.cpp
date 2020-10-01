Control::Control() {
  this->mouse_has_moved = false;
  this->mouse_last_x = 0.0f;
  this->mouse_last_y = 0.0f;
  this->mouse_sensitivity = 0.1f;

  for (uint32 idx = 0; idx < LEN(this->key_states); idx++) {
    this->key_states[idx] = false;
    this->prev_key_states[idx] = true;
  }
}

glm::vec2 Control::update_mouse(real64 mouse_x, real64 mouse_y) {
  if (!this->mouse_has_moved) {
    this->mouse_last_x = mouse_x;
    this->mouse_last_y = mouse_y;
    this->mouse_has_moved = true;
  }

  glm::vec2 mouse_offset;
  mouse_offset.x = (real32)(mouse_x - this->mouse_last_x);
  mouse_offset.y = (real32)(mouse_y - this->mouse_last_y);
  this->mouse_last_x = mouse_x;
  this->mouse_last_y = mouse_y;

  mouse_offset.x *= (real32)this->mouse_sensitivity;
  mouse_offset.y *= (real32)this->mouse_sensitivity;

  return mouse_offset;
}

void Control::update_keys(int key, int scancode, int action, int mods) {
  this->prev_key_states[key] = this->key_states[key];
  if (action == GLFW_PRESS) {
    this->key_states[key] = true;
  } else if (action == GLFW_RELEASE) {
    this->key_states[key] = false;
  }
}

bool32 Control::is_key_down(int key) {
  return this->key_states[key];
}

bool32 Control::is_key_up(int key) {
  return !this->key_states[key];
}

bool32 Control::is_key_now_down(int key) {
  return this->key_states[key] && !this->prev_key_states[key];
}

bool32 Control::is_key_now_up(int key) {
  return !this->key_states[key] && this->prev_key_states[key];
}
