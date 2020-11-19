InputManager::InputManager(
  GLFWwindow *window
) :
  window(window)
{
  this->mouse_pos = glm::vec2(0.0f, 0.0f);
  this->mouse_3d_sensitivity = 0.1f;
  this->current_cursor = nullptr;
  this->arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  this->ibeam_cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
  this->crosshair_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
  this->hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
  this->hresize_cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
  this->vresize_cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
  this->have_ever_gotten_mouse_pos = false;

  memset(this->mouse_button_states, 0, sizeof(this->mouse_button_states));
  memset(
    this->n_mouse_button_state_changes_this_frame,
    0,
    sizeof(this->n_mouse_button_state_changes_this_frame)
  );
  memset(this->key_states, 0, sizeof(this->key_states));
  memset(
    this->n_key_state_changes_this_frame,
    0,
    sizeof(this->n_key_state_changes_this_frame)
  );
}


void InputManager::update_mouse_button(int button, int action, int mods) {
  bool32 new_state = (action == GLFW_PRESS);
  if (new_state != this->mouse_button_states[button]) {
    this->mouse_button_states[button] = new_state;
    this->n_mouse_button_state_changes_this_frame[button]++;
  }
}


bool32 InputManager::is_mouse_button_down(int button) {
  return this->mouse_button_states[button];
}


bool32 InputManager::is_mouse_button_up(int button) {
  return !is_mouse_button_down(button);
}


bool32 InputManager::is_mouse_button_now_down(int button) {
  return is_mouse_button_down(button) &&
    this->n_mouse_button_state_changes_this_frame[button] > 0;
}


bool32 InputManager::is_mouse_button_now_up(int button) {
  return is_mouse_button_up(button) &&
    this->n_mouse_button_state_changes_this_frame[button] > 0;
}


void InputManager::update_mouse(glm::vec2 new_mouse_pos) {
  if (!this->have_ever_gotten_mouse_pos) {
    this->mouse_pos = new_mouse_pos;
    this->have_ever_gotten_mouse_pos = true;
  }

  this->mouse_offset = new_mouse_pos - this->mouse_pos;
  this->mouse_3d_offset = this->mouse_offset * (real32)this->mouse_3d_sensitivity;
  this->mouse_pos = new_mouse_pos;
}


void InputManager::update_keys(int key, int scancode, int action, int mods) {
  bool32 new_state = (action == GLFW_PRESS || action == GLFW_REPEAT);
  if (new_state != this->key_states[key]) {
    this->key_states[key] = new_state;
    this->n_key_state_changes_this_frame[key]++;
  }
}


bool32 InputManager::is_key_down(int key) {
  return this->key_states[key];
}


bool32 InputManager::is_key_up(int key) {
  return !is_key_down(key);
}


bool32 InputManager::is_key_now_down(int key) {
  return is_key_down(key) &&
    this->n_key_state_changes_this_frame[key] > 0;
}


bool32 InputManager::is_key_now_up(int key) {
  return is_key_up(key) &&
    this->n_key_state_changes_this_frame[key] > 0;
}


bool32 InputManager::is_mouse_in_bb(
  glm::vec2 topleft, glm::vec2 bottomright
) {
  return this->mouse_pos.x > topleft.x &&
    this->mouse_pos.x < bottomright.x &&
    this->mouse_pos.y > topleft.y &&
    this->mouse_pos.y < bottomright.y;
}


void InputManager::set_cursor(GLFWcursor *new_cursor) {
  if (this->current_cursor == new_cursor) {
    return;
  }
  this->current_cursor = new_cursor;
  glfwSetCursor(this->window, new_cursor);
}


void InputManager::reset_n_mouse_button_state_changes_this_frame() {
  memset(
    this->n_mouse_button_state_changes_this_frame,
    0,
    sizeof(this->n_mouse_button_state_changes_this_frame)
  );
}


void InputManager::reset_n_key_state_changes_this_frame() {
  memset(
    this->n_key_state_changes_this_frame,
    0,
    sizeof(this->n_key_state_changes_this_frame)
  );
}
