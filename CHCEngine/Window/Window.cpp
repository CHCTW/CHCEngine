#include "Window.h"

#include <iostream>

#include "GLFWInputConvert.h"
#include "glfw/glfw3.h"
namespace CHCEngine {
namespace Window {
static std::function<void(Key, Action)> key_callback_;
static std::function<void()> focus_callback_;
static std::function<void(Vector position)> mouse_move_callback_;
static std::function<void(MouseButton, Action)> mouse_button_callback_;
static std::function<void(int, int)> window_size_callback_;
static std::function<void(int, int)> framebuffer_size_callback_;

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  key_callback_(convertToKey(key), convertToAction(action));
}
void window_focus_callback(GLFWwindow *window, int focused) {
  focus_callback_();
}
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  mouse_button_callback_(convertToMouseButton(button), convertToAction(action));
}
void mouse_move_callback(GLFWwindow *window, double xpos, double ypos) {
  mouse_move_callback_({static_cast<int>(xpos), static_cast<int>(ypos)});
}
void window_size_callback(GLFWwindow *window, int width, int height) {
  window_size_callback_(width, height);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  framebuffer_size_callback_(width, height);
}
void Window::calculateDelta() {
  auto now = std::chrono::high_resolution_clock::now();
  delta_ = now - previous_time_;
  auto upper = delta_upper_bound_.load();
  auto lower = delta_lower_bound_.load();
  if (delta_ > upper) {
    delta_ = upper;
  }
  if (delta_ < lower) {
    std::this_thread::sleep_for(lower - delta_);
    delta_ = lower;
  }
  previous_time_ = std::chrono::high_resolution_clock::now();
}
Window::Window()
    : full_screen_(false), width_(0), height_(0), frame_(0),
      output_stream_(&std::cout), delta_upper_bound_(dur(60000000)),
      delta_lower_bound_(dur(0)), delta_(0), cursor_position_{0, 0} {

  adding_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_KEY] =
      std::bind(&Window::addKeyCallBackFromQueue, this);
  remove_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_KEY] =
      std::bind(&Window::removeKeyCallBack, this, std::placeholders::_1);
  adding_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_BUTTON] =
      std::bind(&Window::addMouseButtonCallbackFromQueue, this);
  remove_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_BUTTON] =
      std::bind(&Window::removeMouseButtonCallback, this,
                std::placeholders::_1);
  adding_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_MOVE] =
      std::bind(&Window::addMouseMoveCallbackFromQueue, this);
  remove_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_MOVE] =
      std::bind(&Window::removeMouseMoveCallback, this, std::placeholders::_1);
  adding_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_WINDOW_SIZE] =
      std::bind(&Window::addWindowSizeCallbackFromQueue, this);
  remove_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_WINDOW_SIZE] =
      std::bind(&Window::removeWindowSizeCallback, this, std::placeholders::_1);
  adding_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_FRAMEBUFFER_SIZE] =
      std::bind(&Window::addFrameBufferSizeCallbackFromQueue, this);
  remove_methods_[InputCallbackType::INPUT_CALLBACK_TYPE_FRAMEBUFFER_SIZE] =
      std::bind(&Window::removeFrameBufferSizeCallback, this,
                std::placeholders::_1);
}
Window::~Window() {
  glfwDestroyWindow(glfw_window_);
  glfwTerminate();
}
void *Window::getWindow() { return glfw_window_; }
void Window::resetInputState() {
  for (int key = Key::KEY_0; key != Key::KEY_TOTAL_COUNT; ++key) {
    input_state_.key[static_cast<Key>(key)] = Action::ACTION_RELEASE;
  }
  for (int button = MouseButton::MOUSE_BUTTON_0;
       button != MouseButton::MOUSE_BUTTON_LAST; ++button) {
    input_state_.mouse[static_cast<MouseButton>(button)] =
        Action::ACTION_RELEASE;
  }
}
void Window::openWindow(std::string name, unsigned int width,
                        unsigned int height, bool full_screen) {

  // an interesting test when move Window to global variable, the
  // static variable actually call after Window(), thus will bind a empty
  // function here, cause crash.
  key_callback_ = std::bind(&Window::keyStateUpdate, this,
                            std::placeholders::_1, std::placeholders::_2);
  focus_callback_ = std::bind(&Window::resetInputState, this);
  mouse_button_callback_ =
      std::bind(&Window::mouseStateUpdate, this, std::placeholders::_1,
                std::placeholders::_2);
  mouse_move_callback_ =
      std::bind(&Window::mouseMove, this, std::placeholders::_1);
  resetInputState();
  window_size_callback_ = std::bind(
      &Window::sizeChange, this, std::placeholders::_1, std::placeholders::_2);
  framebuffer_size_callback_ =
      std::bind(&Window::framebufferChange, this, std::placeholders::_1,
                std::placeholders::_2);
  loop_thread_ =
      std::thread(&Window::loop, this, name, width, height, full_screen);
  while (!glfw_window_ && loop_thread_.joinable()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
void Window::waitUntilClose() {
  if (loop_thread_.joinable())
    loop_thread_.join();
}
void Window::loop(std::string name, unsigned int width, unsigned int height,
                  bool full_screen) {
  auto currentTime = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::steady_clock::now() - currentTime;
  if (!glfwInit())
    return;
  glfw_window_ = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
  if (glfw_window_) {
    width_ = width;
    height_ = height;
    name_ = name;
    full_screen_ = full_screen;
    glfwSetKeyCallback(glfw_window_, key_callback);
    glfwSetWindowFocusCallback(glfw_window_, window_focus_callback);
    glfwSetCursorPosCallback(glfw_window_, mouse_move_callback);
    glfwSetMouseButtonCallback(glfw_window_, mouse_button_callback);
    glfwSetWindowSizeCallback(glfw_window_, window_size_callback);
    glfwSetFramebufferSizeCallback(glfw_window_, framebuffer_size_callback);
    previous_time_ = std::chrono::high_resolution_clock::now();
    should_closed_.store(glfwWindowShouldClose(glfw_window_));
    while (!should_closed_) {
      calculateDelta();
      addLoopCallbackFromQueue();
      removeLoopCallbackFromNames();
      addInputCallbackFromQueue();
      removeInputCallbackFromNames();
      loopCalls();
      glfwPollEvents();
      ++frame_;
      should_closed_.store(glfwWindowShouldClose(glfw_window_));
    }
  }
}
bool Window::conditionCheck(Condition const &condition) {
  for (auto &key_condition : condition.key) {
    if (input_state_.key[key_condition.first] != key_condition.second) {
      return false;
    }
  }
  for (auto &mouse_condition : condition.mouse) {
    if (input_state_.mouse[mouse_condition.first] != mouse_condition.second) {
      return false;
    }
  }
  return true;
}
void Window::keyStateUpdate(Key key, Action action) {
  if (key == Key::KEY_ESCAPE && action == ACTION_PRESS)
    glfwSetWindowShouldClose(glfw_window_, GLFW_TRUE);
  input_state_.key[key] = action;
  if (action == ACTION_PRESS)
    input_state_.key[key] = ACTION_REPEAT;
  // KeyEvent key_event = { key,action };
  if (key_callback_names_.find(key) != key_callback_names_.end()) {
    if (key_callback_names_[key].find(action) !=
        key_callback_names_[key].end()) {
      for (auto &callback : key_callback_names_[key][action]) {
        if (conditionCheck(callback.second)) {
          key_callbacks_[callback.first].first(key, action);
        }
      }
    }
  }
}
void Window::mouseStateUpdate(MouseButton mouse_button, Action action) {
  input_state_.mouse[mouse_button] = action;
  if (action == ACTION_PRESS) {
    input_state_.mouse[mouse_button] = ACTION_REPEAT;
  }
  if (mouse_button_callback_names_.find(mouse_button) !=
      mouse_button_callback_names_.end()) {
    if (mouse_button_callback_names_[mouse_button].find(action) !=
        mouse_button_callback_names_[mouse_button].end()) {
      for (auto &callback :
           mouse_button_callback_names_[mouse_button][action]) {
        if (conditionCheck(callback.second)) {
          mouse_button_callbacks_[callback.first].first(mouse_button, action,
                                                        cursor_position_);
        }
      }
    }
  }
}
void Window::mouseMove(Vector position) {
  Vector offset = {position.X - cursor_position_.X,
                   position.Y - cursor_position_.Y};
  cursor_position_ = position;
  for (auto &callback : mouse_move_callbacks_) {
    if (conditionCheck(callback.second.second)) {
      callback.second.first(cursor_position_, offset);
    }
  }
}
void Window::sizeChange(int width, int height) {
  for (auto &callback : window_size_callbacks_) {
    callback.second(width, height);
  }
}
void Window::framebufferChange(int width, int height) {
  width_ = width;
  height_ = height;
  for (auto &callback : framebuffer_size_callbacks_) {
    callback.second(width, height);
  }
}
void Window::loopCalls() {
  for (int i = 0;
       i < static_cast<int>(LoopCallbackType::LOOP_CALLBACK_TYPE_COUNT); ++i) {
    auto type = static_cast<LoopCallbackType>(i);
    for (auto &callback : loop_callbacks[type]) {
      callback.second(delta_, frame_);
    }
  }
}
bool Window::addKeyCallBack(std::string name, KeyCallBack callback, Key key,
                            Action action, Condition condition) {
  if (checkInputCallbackAndAdd(name,
                               InputCallbackType::INPUT_CALLBACK_TYPE_KEY)) {
    std::lock_guard<std::mutex> lock(
        input_queue_mutex_[InputCallbackType::INPUT_CALLBACK_TYPE_KEY]);
    key_adding_queue_.emplace_back(name, callback, key, action, condition);
    return true;
  }
  return false;
}
void Window::addKeyCallBackFromQueue() {
  std::lock_guard<std::mutex> lock(
      input_queue_mutex_[InputCallbackType::INPUT_CALLBACK_TYPE_KEY]);
  while (!key_adding_queue_.empty()) {
    auto &callback = key_adding_queue_.front();
    key_callbacks_[std::get<0>(callback)] =
        std::pair<std::function<void(Key, Action)>, KeyEvent>(
            std::move(std::get<1>(callback)),
            std::make_pair(std::get<2>(callback), std::get<3>(callback)));
    key_callback_names_[std::get<2>(callback)][std::get<3>(callback)].emplace(
        std::get<0>(callback), std::get<4>(callback));
    key_adding_queue_.pop_front();
  };
}

bool Window::checkInputCallbackAndAdd(std::string name,
                                      InputCallbackType input_callback_type) {
  std::lock_guard<std::mutex> lock(input_callback_names_mutex_);
  if (input_callback_names_.find(name) != input_callback_names_.end()) {
    (*output_stream_) << "Already have the same name function : " << name
                      << " already exsit, please change name" << std::endl;
    return false;
  }
  input_callback_names_.emplace(name, input_callback_type);
  return true;
}

bool Window::checkLoopCallbackAndAdd(std::string name,
                                     LoopCallbackType loop_callback_type) {
  std::lock_guard<std::mutex> lock(loop_callback_names_mutex_);
  if (loop_callback_names_.find(name) != loop_callback_names_.end()) {
    (*output_stream_) << "Already have the same name function : " << name
                      << " already exsit, please change name" << std::endl;
    return false;
  }
  loop_callback_names_.emplace(name, loop_callback_type);
  return true;
}

void Window::removeInputCallbackFromNames() {
  for (int i = 0;
       i < static_cast<int>(InputCallbackType::INPUT_CALLBACK_TYPE_COUNT);
       ++i) {
    InputCallbackType type = static_cast<InputCallbackType>(i);
    std::lock_guard<std::mutex> lock(input_remove_mutex_[type]);
    for (auto &name : input_remove_names_[type]) {
      remove_methods_[type](name);
    }
    input_remove_names_[type].clear();
  }
}

void Window::addInputCallbackFromQueue() {
  for (int i = 0;
       i < static_cast<int>(InputCallbackType::INPUT_CALLBACK_TYPE_COUNT);
       ++i) {
    adding_methods_[static_cast<InputCallbackType>(i)]();
  }
}

void Window::removeKeyCallBack(std::string &name) {
  if (key_callbacks_.find(name) != key_callbacks_.end()) {
    Key key = key_callbacks_[name].second.first;
    Action action = key_callbacks_[name].second.second;
    key_callbacks_.erase(name);
    if (key_callback_names_.find(key) != key_callback_names_.end()) {
      if (key_callback_names_[key].find(action) !=
          key_callback_names_[key].end()) {
        key_callback_names_[key][action].erase(name);
      }
    }
  }
}
void Window::addMouseMoveCallbackFromQueue() {
  std::lock_guard<std::mutex> lock(
      input_queue_mutex_[InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_MOVE]);
  while (!mouse_move_adding_queue_.empty()) {
    auto &callback = mouse_move_adding_queue_.front();
    mouse_move_callbacks_[std::get<0>(callback)] =
        std::make_pair(std::get<1>(callback), std::get<2>(callback));
    mouse_move_adding_queue_.pop_front();
  }
}
bool Window::addMouseButtonCallback(std::string name,
                                    MouseButtonCallback callback,
                                    MouseButton mouse_button, Action action,
                                    Condition condition) {
  if (checkInputCallbackAndAdd(
          name, InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_BUTTON)) {
    std::lock_guard<std::mutex> lock(
        input_queue_mutex_
            [InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_BUTTON]);
    mouse_button_adding_queue_.emplace_back(name, callback, mouse_button,
                                            action, condition);
  }
  return true;
}
void Window::removeMouseButtonCallback(std::string &name) {
  if (mouse_button_callbacks_.find(name) != mouse_button_callbacks_.end()) {
    MouseButton mouse_button = mouse_button_callbacks_[name].second.first;
    Action action = mouse_button_callbacks_[name].second.second;
    mouse_button_callbacks_.erase(name);
    if (mouse_button_callback_names_.find(mouse_button) !=
        mouse_button_callback_names_.end()) {
      if (mouse_button_callback_names_[mouse_button].find(action) !=
          mouse_button_callback_names_[mouse_button].end()) {
        mouse_button_callback_names_[mouse_button][action].erase(name);
      }
    }
  }
}
void Window::addWindowSizeCallbackFromQueue() {
  std::lock_guard<std::mutex> lock(
      input_queue_mutex_[InputCallbackType::INPUT_CALLBACK_TYPE_WINDOW_SIZE]);
  while (!window_size_adding_queue_.empty()) {
    auto &callback = window_size_adding_queue_.front();
    window_size_callbacks_.emplace(std::get<0>(callback),
                                   std::get<1>(callback));
    window_size_adding_queue_.pop_front();
  };
}
void Window::removeWindowSizeCallback(std::string &name) {
  window_size_callbacks_.erase(name);
}
void Window::addFrameBufferSizeCallbackFromQueue() {
  std::lock_guard<std::mutex> lock(
      input_queue_mutex_
          [InputCallbackType::INPUT_CALLBACK_TYPE_FRAMEBUFFER_SIZE]);
  while (!framebuffer_size_adding_queue_.empty()) {
    auto &callback = framebuffer_size_adding_queue_.front();
    framebuffer_size_callbacks_.emplace(std::get<0>(callback),
                                        std::get<1>(callback));
    framebuffer_size_adding_queue_.pop_front();
  };
}
void Window::removeFrameBufferSizeCallback(std::string &name) {
  framebuffer_size_callbacks_.erase(name);
}
void Window::addLoopCallbackFromQueue() {
  for (int i = 0;
       i < static_cast<int>(LoopCallbackType::LOOP_CALLBACK_TYPE_COUNT); ++i) {
    auto type = static_cast<LoopCallbackType>(i);
    std::lock_guard<std::mutex> lock(loop_queue_mutex_[type]);
    while (!loop_callback_adding_queue_[type].empty()) {
      auto &callback = loop_callback_adding_queue_[type].front();
      loop_callbacks[type].emplace(std::get<0>(callback),
                                   std::get<1>(callback));
      loop_callback_adding_queue_[type].pop_front();
    }
  }
}
void Window::removeLoopCallbackFromNames() {
  for (int i = 0;
       i < static_cast<int>(LoopCallbackType::LOOP_CALLBACK_TYPE_COUNT); ++i) {
    LoopCallbackType type = static_cast<LoopCallbackType>(i);
    std::lock_guard<std::mutex> lock(loop_remove_mutex_[type]);
    for (auto &name : loop_remove_names_[type]) {
      loop_callbacks[type].erase(name);
    }
    loop_remove_names_[type].clear();
  }
}
bool Window::addMouseMoveCallback(std::string name, MouseMoveCallback callback,
                                  Condition condition) {
  if (checkInputCallbackAndAdd(
          name, InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_MOVE)) {
    std::lock_guard<std::mutex> lock(
        input_queue_mutex_[InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_MOVE]);
    mouse_move_adding_queue_.emplace_back(name, callback, condition);
    return true;
  }
  return false;
}
bool Window::addWindowSizeCallback(std::string name,
                                   WindowSizeCallback callback) {
  if (checkInputCallbackAndAdd(
          name, InputCallbackType::INPUT_CALLBACK_TYPE_WINDOW_SIZE)) {
    std::lock_guard<std::mutex> lock(
        input_queue_mutex_[InputCallbackType::INPUT_CALLBACK_TYPE_WINDOW_SIZE]);
    window_size_adding_queue_.emplace_back(name, callback);
    return true;
  }
  return false;
}
bool Window::addFramebufferSizeCallback(std::string name,
                                        FrameBufferSizeCallback callback) {
  if (checkInputCallbackAndAdd(
          name, InputCallbackType::INPUT_CALLBACK_TYPE_FRAMEBUFFER_SIZE)) {
    std::lock_guard<std::mutex> lock(
        input_queue_mutex_
            [InputCallbackType::INPUT_CALLBACK_TYPE_FRAMEBUFFER_SIZE]);
    framebuffer_size_adding_queue_.emplace_back(name, callback);
    return true;
  }
  return false;
}
bool Window::addLoopCallback(std::string name, LoopCallback callback,
                             LoopCallbackType type) {
  if (checkLoopCallbackAndAdd(name, type)) {
    std::lock_guard<std::mutex> lock(loop_queue_mutex_[type]);
    loop_callback_adding_queue_[type].emplace_back(name, callback);
    return true;
  }
  return false;
}
void Window::removeMouseMoveCallback(std::string &name) {
  mouse_move_callbacks_.erase(name);
}
void Window::addMouseButtonCallbackFromQueue() {
  std::lock_guard<std::mutex> lock(
      input_queue_mutex_[InputCallbackType::INPUT_CALLBACK_TYPE_MOUSE_BUTTON]);
  while (!mouse_button_adding_queue_.empty()) {
    auto &callback = mouse_button_adding_queue_.front();
    mouse_button_callbacks_[std::get<0>(callback)] = std::make_pair(
        std::get<1>(callback),
        std::make_pair(std::get<2>(callback), std::get<3>(callback)));
    mouse_button_callback_names_[std::get<2>(callback)][std::get<3>(callback)]
        .emplace(std::get<0>(callback), std::get<4>(callback));
    mouse_button_adding_queue_.pop_front();
  }
}
void Window::removeInputCallback(std::string name) {
  bool find = false;
  InputCallbackType type = InputCallbackType::INPUT_CALLBACK_TYPE_COUNT;
  {
    std::lock_guard<std::mutex> lock(input_callback_names_mutex_);
    if (input_callback_names_.find(name) != input_callback_names_.end()) {
      find = true;
      type = input_callback_names_[name];
      input_callback_names_.erase(name);
    }
  }
  if (find) {
    if (type != InputCallbackType::INPUT_CALLBACK_TYPE_COUNT) {
      std::lock_guard<std::mutex> lock(input_remove_mutex_[type]);
      input_remove_names_[type].emplace_back(name);
    }
  }
}
void Window::removeLoopCallback(std::string name) {
  bool find = false;
  LoopCallbackType type = LoopCallbackType::LOOP_CALLBACK_TYPE_COUNT;
  {
    std::lock_guard<std::mutex> lock(loop_callback_names_mutex_);
    if (loop_callback_names_.find(name) != loop_callback_names_.end()) {
      find = true;
      type = loop_callback_names_[name];
      loop_callback_names_.erase(name);
    }
  }
  if (find) {
    if (type != LoopCallbackType::LOOP_CALLBACK_TYPE_COUNT) {
      std::lock_guard<std::mutex> lock(loop_remove_mutex_[type]);
      ;
      loop_remove_names_[type].emplace_back(name);
    }
  }
}
void Window::setMessageOutput(std::ostream &output_stream) {
  output_stream_ = &output_stream;
}
void Window::setFrameTimeUpperBound(long long nano_second) {
  delta_upper_bound_.store(dur(nano_second));
}
void Window::setFrameTimeLowerBound(long long nano_second) {
  delta_lower_bound_.store(dur(nano_second));
}
Vector Window::getFrameSize() {
  return Vector() = {static_cast<int>(width_.load()),
                     static_cast<int>(height_.load())};
}
bool Window::shouldWindowClose() { return should_closed_; }
bool Window::checkInputCallbackNameExist(std::string name) {
  std::lock_guard<std::mutex> lock(input_callback_names_mutex_);
  return input_callback_names_.find(name) != input_callback_names_.end();
}
bool Window::checkLoopCallbackNameExist(std::string name) {
  std::lock_guard<std::mutex> lock(loop_callback_names_mutex_);
  return loop_callback_names_.find(name) != loop_callback_names_.end();
}
} // namespace Window

} // namespace CHCEngine