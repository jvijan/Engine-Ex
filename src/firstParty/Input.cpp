#include "Input.h"
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include "SDL_scancode.h"


void Input::Init() {
  // Start all input states as up
  for (int code = SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; code++) {
    keyboardStates[static_cast<SDL_Scancode>(code)] = INPUT_STATE_UP;
  }
}


void Input::ProcessEvent(const SDL_Event &e) {
  switch (e.type) {
    case SDL_KEYDOWN: {
      auto it = keyboardStates.find(e.key.keysym.scancode);
      if (it != keyboardStates.end() && it->second == INPUT_STATE_DOWN) {
        break;
      }
      keyboardStates[e.key.keysym.scancode] = INPUT_STATE_JUST_DOWN;
      justDownScancodes.push_back(e.key.keysym.scancode);
      break;
    }

    case SDL_KEYUP: {
      auto it = keyboardStates.find(e.key.keysym.scancode);
      if (it != keyboardStates.end() && it->second == INPUT_STATE_UP) {
        break;
      }
      keyboardStates[e.key.keysym.scancode] = INPUT_STATE_JUST_UP;
      justUpScancodes.push_back(e.key.keysym.scancode);
      break;
    }

    case SDL_MOUSEBUTTONDOWN: {
      auto it = mouseStates.find(e.button.button);
      if (it != mouseStates.end() && it->second == INPUT_STATE_DOWN) {
        break;
      }
      mouseStates[e.button.button] = INPUT_STATE_JUST_DOWN;
      justDownMouseButtons.push_back(e.button.button);
      break;
    }

    case SDL_MOUSEBUTTONUP: {
      auto it = mouseStates.find(e.button.button);
      if (it != mouseStates.end() && it->second == INPUT_STATE_UP) {
        break;
      }
      mouseStates[e.button.button] = INPUT_STATE_JUST_UP;
      justUpMouseButtons.push_back(e.button.button);
      break;
    }

    case SDL_MOUSEMOTION : {
      mousePos.x = e.motion.x;
      mousePos.y = e.motion.y;
      break;
    }

    case SDL_MOUSEWHEEL : {
      mouseScrollThisFrame = e.wheel.preciseY;
    }
  }
}

void Input::LateUpdate() {
  for (const SDL_Scancode & it : justDownScancodes) {
    keyboardStates[it] = INPUT_STATE_DOWN;
  }
  justDownScancodes.clear();
  for (const SDL_Scancode& it : justUpScancodes) {
    keyboardStates[it] = INPUT_STATE_UP;
  }
  justUpScancodes.clear();

  for (const int & it : justDownMouseButtons) {
    mouseStates[it] = INPUT_STATE_DOWN;
  }
  justDownMouseButtons.clear();
  for (const int & it : justUpMouseButtons) {
    mouseStates[it] = INPUT_STATE_UP;
  }
  justUpMouseButtons.clear();
  mouseScrollThisFrame = 0;
}

bool Input::GetKey(SDL_Scancode keycode) {
  auto it = keyboardStates.find(keycode);
  if (it == keyboardStates.end()) {
      return false;
  }
  return it->second == INPUT_STATE_DOWN || it->second == INPUT_STATE_JUST_DOWN;
}

bool Input::GetKeyLua(const std::string &keycode) {
  auto it = __keycode_to_scancode.find(keycode);
  if (it == __keycode_to_scancode.end()) {
    return false;
  }
  return GetKey(it->second);
}

bool Input::GetKeyDown(SDL_Scancode keycode) {
  auto it = keyboardStates.find(keycode);
  if (it == keyboardStates.end()) {
      return false;
  }
  return it->second == INPUT_STATE_JUST_DOWN;
}

bool Input::GetKeyDownLua(const std::string &keycode) {
  auto it = __keycode_to_scancode.find(keycode);
  if (it == __keycode_to_scancode.end()) {
    return false;
  }
  return GetKeyDown(it->second);
}

bool Input::GetKeyUp(SDL_Scancode keycode) {
  auto it = keyboardStates.find(keycode);
  if (it == keyboardStates.end()) {
      return false;
  }
  return it->second == INPUT_STATE_JUST_UP;
}

bool Input::GetKeyUpLua(const std::string &keycode) {
  auto it = __keycode_to_scancode.find(keycode);
  if (it == __keycode_to_scancode.end()) {
    return false;
  }
  return GetKeyUp(it->second);
}

bool Input::GetMouseButton(const int button) {
  auto it = mouseStates.find(button);
  if (it == mouseStates.end()) {
    return false;
  }
  return it->second == INPUT_STATE_JUST_DOWN || it->second == INPUT_STATE_DOWN;
}

bool Input::GetMouseButtonDown(const int button) {
  auto it = mouseStates.find(button);
  if (it == mouseStates.end()) {
    return false;
  }
  return it->second == INPUT_STATE_JUST_DOWN;
}

bool Input::GetMouseButtonUp(const int button) {
  auto it = mouseStates.find(button);
  if (it == mouseStates.end()) {
    return false;
  }
  return it->second == INPUT_STATE_JUST_UP;
}

glm::vec2 Input::GetMousePos() {
  return mousePos;
}

float Input::GetMouseScrollDelta() {
  return mouseScrollThisFrame;
}

void Input::ShowCursor() {
  SDL_ShowCursor(SDL_ENABLE);
}

void Input::HideCursor() {
  SDL_ShowCursor(SDL_DISABLE);
}
