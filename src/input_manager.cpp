InputManager::InputManager(
  GLFWwindow *window
) :
  window(window)
{
  this->mouse_pos = glm::vec2(0.0f, 0.0f);
  this->mouse_3d_sensitivity = 0.1f;
  this->current_cursor = nullptr;
  this->hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
  this->have_ever_gotten_mouse_pos = false;

  for (uint32 idx = 0; idx < LEN(this->key_states); idx++) {
    this->key_states[idx] = false;
    this->prev_key_states[idx] = true;
  }
}


void InputManager::update_mouse(glm::vec2 new_mouse_pos) {
  if (!this->have_ever_gotten_mouse_pos) {
    this->mouse_pos = new_mouse_pos;
    this->have_ever_gotten_mouse_pos = true;
  }

  this->mouse_3d_offset = (new_mouse_pos - this->mouse_pos) *
    (real32)this->mouse_3d_sensitivity;
  this->mouse_pos = new_mouse_pos;
}


void InputManager::update_keys(int key, int scancode, int action, int mods) {
  this->prev_key_states[key] = this->key_states[key];
  if (action == GLFW_PRESS) {
    this->key_states[key] = true;
  } else if (action == GLFW_RELEASE) {
    this->key_states[key] = false;
  }
}


bool32 InputManager::is_key_down(int key) {
  return this->key_states[key];
}


bool32 InputManager::is_key_up(int key) {
  return !this->key_states[key];
}


bool32 InputManager::is_key_now_down(int key) {
  return this->key_states[key] && !this->prev_key_states[key];
}


bool32 InputManager::is_key_now_up(int key) {
  return !this->key_states[key] && this->prev_key_states[key];
}


bool32 InputManager::is_mouse_in_bb(
  real32 x0, real32 y0, real32 x1, real32 y1
) {
  return this->mouse_pos.x > x0 && this->mouse_pos.x < x1 &&
    this->mouse_pos.y > y0 && this->mouse_pos.y < y1;
}


void InputManager::set_cursor(GLFWcursor *new_cursor) {
  if (this->current_cursor == new_cursor) {
    return;
  }
  this->current_cursor = new_cursor;
  glfwSetCursor(this->window, new_cursor);
}
