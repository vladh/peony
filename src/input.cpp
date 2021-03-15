void Input::update_mouse_button(
  InputState *input_state, int button, int action, int mods
) {
  bool32 new_state = (action == GLFW_PRESS);
  if (new_state != input_state->mouse_button_states[button]) {
    input_state->mouse_button_states[button] = new_state;
    input_state->n_mouse_button_state_changes_this_frame[button]++;
  }
}


bool32 Input::is_mouse_button_down(
  InputState *input_state, int button
) {
  return input_state->mouse_button_states[button];
}


bool32 Input::is_mouse_button_up(
  InputState *input_state, int button
) {
  return !is_mouse_button_down(input_state, button);
}


bool32 Input::is_mouse_button_now_down(
  InputState *input_state, int button
) {
  return is_mouse_button_down(input_state, button) &&
    input_state->n_mouse_button_state_changes_this_frame[button] > 0;
}


bool32 Input::is_mouse_button_now_up(
  InputState *input_state, int button
) {
  return is_mouse_button_up(input_state, button) &&
    input_state->n_mouse_button_state_changes_this_frame[button] > 0;
}


void Input::update_mouse(
  InputState *input_state, glm::vec2 new_mouse_pos
) {
  if (!input_state->have_ever_gotten_mouse_pos) {
    input_state->mouse_pos = new_mouse_pos;
    input_state->have_ever_gotten_mouse_pos = true;
  }

  input_state->mouse_offset = new_mouse_pos - input_state->mouse_pos;
  input_state->mouse_3d_offset =
    input_state->mouse_offset * (real32)input_state->mouse_3d_sensitivity;
  input_state->mouse_pos = new_mouse_pos;
}


void Input::update_keys(
  InputState *input_state, int key, int scancode, int action, int mods
) {
  bool32 new_state = (action == GLFW_PRESS || action == GLFW_REPEAT);
  if (new_state != input_state->key_states[key]) {
    input_state->key_states[key] = new_state;
    input_state->n_key_state_changes_this_frame[key]++;
  }
}


bool32 Input::is_key_down(
  InputState *input_state, int key
) {
  return input_state->key_states[key];
}


bool32 Input::is_key_up(
  InputState *input_state, int key
) {
  return !is_key_down(input_state, key);
}


bool32 Input::is_key_now_down(
  InputState *input_state, int key
) {
  return is_key_down(input_state, key) &&
    input_state->n_key_state_changes_this_frame[key] > 0;
}


bool32 Input::is_key_now_up(
  InputState *input_state, int key
) {
  return is_key_up(input_state, key) &&
    input_state->n_key_state_changes_this_frame[key] > 0;
}


bool32 Input::is_mouse_in_bb(
  InputState *input_state, glm::vec2 topleft, glm::vec2 bottomright
) {
  return input_state->mouse_pos.x > topleft.x &&
    input_state->mouse_pos.x < bottomright.x &&
    input_state->mouse_pos.y > topleft.y &&
    input_state->mouse_pos.y < bottomright.y;
}


void Input::set_cursor(
  InputState *input_state, GLFWcursor *new_cursor
) {
  if (input_state->current_cursor == new_cursor) {
    return;
  }
  input_state->current_cursor = new_cursor;
  glfwSetCursor(input_state->window, new_cursor);
}


void Input::reset_n_mouse_button_state_changes_this_frame(
  InputState *input_state
) {
  memset(
    input_state->n_mouse_button_state_changes_this_frame,
    0,
    sizeof(input_state->n_mouse_button_state_changes_this_frame)
  );
}


void Input::reset_n_key_state_changes_this_frame(
  InputState *input_state
) {
  memset(
    input_state->n_key_state_changes_this_frame,
    0,
    sizeof(input_state->n_key_state_changes_this_frame)
  );
}


InputState* Input::init_input_state(
  InputState *input_state,
  GLFWwindow *window
) {
  input_state->window = window;
  input_state->mouse_pos = glm::vec2(0.0f, 0.0f);
  input_state->mouse_3d_sensitivity = 0.1f;
  input_state->current_cursor = nullptr;
  input_state->arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  input_state->ibeam_cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
  input_state->crosshair_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
  input_state->hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
  input_state->hresize_cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
  input_state->vresize_cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
  input_state->have_ever_gotten_mouse_pos = false;

  memset(
    input_state->mouse_button_states, 0, sizeof(input_state->mouse_button_states)
  );
  memset(
    input_state->n_mouse_button_state_changes_this_frame,
    0,
    sizeof(input_state->n_mouse_button_state_changes_this_frame)
  );
  memset(
    input_state->key_states, 0, sizeof(input_state->key_states)
  );
  memset(
    input_state->n_key_state_changes_this_frame,
    0,
    sizeof(input_state->n_key_state_changes_this_frame)
  );

  return input_state;
}
