namespace input {
  // -----------------------------------------------------------
  // Constants
  // -----------------------------------------------------------
  constexpr uint32 MAX_TEXT_INPUT_LENGTH = 512;
  constexpr uint32 MAX_TEXT_INPUT_COMMAND_LENGTH = 50;
  constexpr uint32 MAX_TEXT_INPUT_ARGUMENTS_LENGTH =
    MAX_TEXT_INPUT_LENGTH - MAX_TEXT_INPUT_COMMAND_LENGTH;


  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
  struct InputState {
    GLFWwindow *window;
    v2 mouse_pos;
    v2 mouse_offset;
    v2 mouse_3d_offset;
    real64 mouse_3d_sensitivity;
    bool32 mouse_button_states[GLFW_MOUSE_BUTTON_LAST];
    uint32 n_mouse_button_state_changes_this_frame[GLFW_MOUSE_BUTTON_LAST];
    bool32 key_states[GLFW_KEY_LAST];
    uint32 n_key_state_changes_this_frame[GLFW_KEY_LAST];
    GLFWcursor *current_cursor;
    GLFWcursor *arrow_cursor;
    GLFWcursor *ibeam_cursor;
    GLFWcursor *crosshair_cursor;
    GLFWcursor *hand_cursor;
    GLFWcursor *hresize_cursor;
    GLFWcursor *vresize_cursor;
    bool32 have_ever_gotten_mouse_pos;
    bool32 is_text_input_enabled;
    char text_input[MAX_TEXT_INPUT_LENGTH];
  };


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
  void update_mouse_button(
    InputState *input_state, int button, int action, int mods
  ) {
    bool32 new_state = (action == GLFW_PRESS);
    if (new_state != input_state->mouse_button_states[button]) {
      input_state->mouse_button_states[button] = new_state;
      input_state->n_mouse_button_state_changes_this_frame[button]++;
    }
  }


  bool32 is_mouse_button_down(
    InputState *input_state, int button
  ) {
    return input_state->mouse_button_states[button];
  }


  bool32 is_mouse_button_up(
    InputState *input_state, int button
  ) {
    return !is_mouse_button_down(input_state, button);
  }


  bool32 is_mouse_button_now_down(
    InputState *input_state, int button
  ) {
    return is_mouse_button_down(input_state, button) &&
      input_state->n_mouse_button_state_changes_this_frame[button] > 0;
  }


  bool32 is_mouse_button_now_up(
    InputState *input_state, int button
  ) {
    return is_mouse_button_up(input_state, button) &&
      input_state->n_mouse_button_state_changes_this_frame[button] > 0;
  }


  void update_mouse(
    InputState *input_state, v2 new_mouse_pos
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


  void clear_text_input(InputState *input_state) {
    str::clear(input_state->text_input);
  }


  void enable_text_input(InputState *input_state) {
    input_state->is_text_input_enabled = true;
    clear_text_input(input_state);
  }


  void disable_text_input(InputState *input_state) {
    input_state->is_text_input_enabled = false;
    clear_text_input(input_state);
  }


  void do_text_input_backspace(InputState *input_state) {
    size_t text_input_length = strlen(input_state->text_input);
    if (text_input_length == 0) {
      return;
    }
    input_state->text_input[text_input_length - 1] = '\0';
  }


  void update_text_input(
    InputState *input_state, uint32 codepoint
  ) {
    if (!input_state->is_text_input_enabled) {
      return;
    }
    char poor_smashed_ascii_character = (char)codepoint;
    size_t text_input_length = strlen(input_state->text_input);

    if (text_input_length + 1 >= MAX_TEXT_INPUT_LENGTH - 1) {
      logs::error("Can't add another character to text_input, it's full.");
      return;
    }

    input_state->text_input[text_input_length] = poor_smashed_ascii_character;
    input_state->text_input[text_input_length + 1] = '\0';
  }


  void update_keys(
    InputState *input_state, int key, int scancode, int action, int mods
  ) {
    bool32 new_state = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (new_state != input_state->key_states[key]) {
      input_state->key_states[key] = new_state;
      input_state->n_key_state_changes_this_frame[key]++;
    }
  }


  bool32 is_key_down(
    InputState *input_state, int key
  ) {
    return input_state->key_states[key];
  }


  bool32 is_key_up(
    InputState *input_state, int key
  ) {
    return !is_key_down(input_state, key);
  }


  bool32 is_key_now_down(
    InputState *input_state, int key
  ) {
    return is_key_down(input_state, key) &&
      input_state->n_key_state_changes_this_frame[key] > 0;
  }


  bool32 is_key_now_up(
    InputState *input_state, int key
  ) {
    return is_key_up(input_state, key) &&
      input_state->n_key_state_changes_this_frame[key] > 0;
  }


  bool32 is_mouse_in_bb(
    InputState *input_state, v2 topleft, v2 bottomright
  ) {
    return input_state->mouse_pos.x > topleft.x &&
      input_state->mouse_pos.x < bottomright.x &&
      input_state->mouse_pos.y > topleft.y &&
      input_state->mouse_pos.y < bottomright.y;
  }


  void set_cursor(
    InputState *input_state, GLFWcursor *new_cursor
  ) {
    if (input_state->current_cursor == new_cursor) {
      return;
    }
    input_state->current_cursor = new_cursor;
    glfwSetCursor(input_state->window, new_cursor);
  }


  void reset_n_mouse_button_state_changes_this_frame(
    InputState *input_state
  ) {
    memset(
      input_state->n_mouse_button_state_changes_this_frame,
      0,
      sizeof(input_state->n_mouse_button_state_changes_this_frame)
    );
  }


  void reset_n_key_state_changes_this_frame(
    InputState *input_state
  ) {
    memset(
      input_state->n_key_state_changes_this_frame,
      0,
      sizeof(input_state->n_key_state_changes_this_frame)
    );
  }


  InputState* init_input_state(
    InputState *input_state,
    GLFWwindow *window
  ) {
    input_state->window = window;
    input_state->mouse_pos = v2(0.0f, 0.0f);
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
}

using input::InputState;
