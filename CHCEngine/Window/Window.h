#pragma once
#include <chrono>
#include <deque>
#include <functional>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <atomic>
#include "InputTable.h"
struct GLFWwindow;
namespace CHCEngine {
namespace Window {
enum class InputCallbackType {
  INPUT_CALLBACK_TYPE_KEY,
  INPUT_CALLBACK_TYPE_MOUSE_MOVE,
  INPUT_CALLBACK_TYPE_MOUSE_BUTTON,
  INPUT_CALLBACK_TYPE_WINDOW_SIZE,
  INPUT_CALLBACK_TYPE_FRAMEBUFFER_SIZE,
  INPUT_CALLBACK_TYPE_COUNT
};
// probally going to have a hide for multi-thread wait 
enum class LoopCallbackType {
  LOOP_CALLBACK_TYPE_START,
  LOOP_CALLBACK_TYPE_MAIN,
  LOOP_CALLBACK_TYPE_END,
  LOOP_CALLBACK_TYPE_COUNT
};
struct Condition {
  std::unordered_map<Key, Action> key;
  std::unordered_map<MouseButton, Action> mouse;
};
struct Vector {
  int X;
  int Y;
};
using KeyEvent = std::pair<Key, Action>;
using MouseButtonEvent = std::pair<MouseButton, Action>;
using KeyCallBack = std::function<void(Key key, Action button)>;
using MouseMoveCallback =
    std::function<void(const Vector& position, const Vector& offset)>;
using MouseButtonCallback = std::function<void(
    MouseButton mouse_button, Action action, const Vector& position)>;
using WindowSizeCallback = std::function<void(int width, int height)>;
using FrameBufferSizeCallback = std::function<void(int width, int height)>;
using LoopCallback =
    std::function<void(std::chrono::duration<long long, std::nano> const& delta,
                       uint64_t const& frame)>;

   using dur = std::chrono::duration<long long, std::nano>;
class Window {
 private:
  friend class Renderer;
  std::ostream* output_stream_;
  std::atomic<GLFWwindow*> glfw_window_ = nullptr;
  std::thread loop_thread_;
  void loop(std::string name, unsigned int width, unsigned int height,
            bool full_screen);
  std::atomic<unsigned int> width_;
  std::atomic <unsigned int> height_;
  std::string name_;
  bool full_screen_;
  Vector cursor_position_;

  // all internal used Callbacks
  void resetInputState();
  void keyStateUpdate(Key key, Action action);
  void mouseStateUpdate(MouseButton mouse_button, Action action);
  void mouseMove(Vector position);
  void sizeChange(int width, int height);
  void framebufferChange(int width, int height);
  void loopCalls();

  // for custom Callbacks
  bool conditionCheck(Condition const& condition);
  std::unordered_map<InputCallbackType, std::deque<std::string>>
      remove_callback_queue_;
  bool checkInputCallbackAndAdd(std::string name,
                                InputCallbackType input_callback_type);
  bool checkLoopCallbackAndAdd(std::string name,
                                LoopCallbackType loop_callback_type);

  void removeInputCallbackFromNames();
  void addInputCallbackFromQueue();

  void addKeyCallBackFromQueue();
  void removeKeyCallBack(std::string& name);
  void addMouseMoveCallbackFromQueue();
  void removeMouseMoveCallback(std::string& name);
  void addMouseButtonCallbackFromQueue();
  void removeMouseButtonCallback(std::string& name);
  void addWindowSizeCallbackFromQueue();
  void removeWindowSizeCallback(std::string& name);
  void addFrameBufferSizeCallbackFromQueue();
  void removeFrameBufferSizeCallback(std::string& name);
  void addLoopCallbackFromQueue();
  void removeLoopCallbackFromNames();
  std::unordered_map<InputCallbackType, std::function<void(std::string& name)>>
      remove_methods_;
  std::unordered_map<InputCallbackType, std::function<void()>> adding_methods_;
  // key Callbacks
  std::unordered_map<
      Key,
      std::unordered_map<Action, std::unordered_map<std::string, Condition>>>
      key_callback_names_;
  std::unordered_map<std::string, std::pair<KeyCallBack, KeyEvent>>
      key_callbacks_;
  std::deque<std::tuple<std::string, KeyCallBack, Key, Action, Condition>>
      key_adding_queue_;

  // mouse move Callbacks
  std::unordered_map<std::string, std::pair<MouseMoveCallback, Condition>>
      mouse_move_callbacks_;
  std::deque<std::tuple<std::string, MouseMoveCallback, Condition>>
      mouse_move_adding_queue_;

  // mouse button Callbacks

  std::unordered_map<
      MouseButton,
      std::unordered_map<Action, std::unordered_map<std::string, Condition>>>
      mouse_button_callback_names_;
  std::unordered_map<std::string,
                     std::pair<MouseButtonCallback, MouseButtonEvent>>
      mouse_button_callbacks_;
  std::deque<std::tuple<std::string, MouseButtonCallback, MouseButton, Action,
                        Condition>>
      mouse_button_adding_queue_;
  // std::vector<std::string> mouse_button_input_remove_names_;

  // window size Callbacks
  std::unordered_map<std::string, WindowSizeCallback> window_size_callbacks_;
  std::deque<std::tuple<std::string, WindowSizeCallback>>
      window_size_adding_queue_;

  // framebuffer size Callbacks
  std::unordered_map<std::string, FrameBufferSizeCallback>
      framebuffer_size_callbacks_;
  std::deque<std::tuple<std::string, FrameBufferSizeCallback>>
      framebuffer_size_adding_queue_;
  // std::vector<std::string> window_size_input_remove_names_;

  // use for conditional input event like when press A+K+L then move mouse
  Condition input_state_;
  std::unordered_map<std::string, InputCallbackType> input_callback_names_;

  std::mutex input_callback_names_mutex_;

  std::unordered_map<InputCallbackType, std::vector<std::string>> input_remove_names_;
  std::unordered_map<InputCallbackType, std::mutex> input_queue_mutex_;
  std::unordered_map<InputCallbackType, std::mutex> input_remove_mutex_;

  void calculateDelta();

  std::chrono::duration<long long, std::nano> delta_;
  std::atomic<std::chrono::duration<long long, std::nano>> delta_upper_bound_;
  std::atomic <std::chrono::duration<long long, std::nano>> delta_lower_bound_;
  std::chrono::time_point<std::chrono::high_resolution_clock> previous_time_;
  uint64_t frame_;

  std::unordered_map<LoopCallbackType,
                     std::unordered_map<std::string, LoopCallback>>
      loop_callbacks;
  std::unordered_map<std::string, LoopCallbackType> loop_callback_names_;
  std::mutex loop_callback_names_mutex_;
  std::unordered_map<LoopCallbackType,
                     std::deque<std::tuple<std::string, LoopCallback>>>
      loop_callback_adding_queue_;
  std::unordered_map<LoopCallbackType, std::vector<std::string>> loop_remove_names_;
  std::unordered_map<LoopCallbackType, std::mutex> loop_queue_mutex_;
  std::unordered_map<LoopCallbackType, std::mutex> loop_remove_mutex_;
  std::atomic<bool> should_closed_;
 public:
  Window();
  ~Window();
  void* getWindow();
  void openWindow(std::string name, unsigned int width = 1920,
                  unsigned int height = 1080, bool full_screen = false);
  bool addKeyCallBack(std::string name, KeyCallBack callback, Key key,
                      Action action, Condition condition = Condition());

  bool addMouseButtonCallback(std::string name, MouseButtonCallback callback,
                              MouseButton mouse_button, Action action,
                              Condition condition = Condition());

  bool addMouseMoveCallback(std::string name, MouseMoveCallback callback,
                            Condition condition = Condition());
  bool addWindowSizeCallback(std::string name, WindowSizeCallback callback);
  bool addFramebufferSizeCallback(std::string name,
                                  FrameBufferSizeCallback callback);
  bool addLoopCallback(
      std::string name, LoopCallback callback,
      LoopCallbackType type = LoopCallbackType::LOOP_CALLBACK_TYPE_MAIN);
  bool checkInputCallbackNameExist(std::string name);
  bool checkLoopCallbackNameExist(std::string name);
  void waitUntilClose();
  void removeInputCallback(std::string name);
  void removeLoopCallback(std::string name);
  void setMessageOutput(std::ostream& output);
  void setFrameTimeUpperBound(long long nano_second);
  void setFrameTimeLowerBound(long long nano_second);
  Vector getFrameSize();
  bool shouldWindowClose();
};
}  // namespace Window
}  // namespace CHCEngine