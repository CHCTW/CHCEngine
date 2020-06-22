#include <chrono>
#include <iostream>
#include <sstream>

#include "CHCEngine.h"
#include "Renderer/Pipeline/Shader.h"

#include <functional>

using CHCEngine::Renderer::BindType;
using CHCEngine::Renderer::DataFormat;
using CHCEngine::Renderer::Renderer;
using CHCEngine::Renderer::ResourceState;
using CHCEngine::Renderer::ResourceTransitionFlag;
using CHCEngine::Renderer::ShaderType;
using CHCEngine::Renderer::Pipeline::Shader;
using CHCEngine::Renderer::Pipeline::ShaderSet;
using CHCEngine::Window::Action;
using CHCEngine::Window::Condition;
using CHCEngine::Window::Key;
using CHCEngine::Window::MouseButton;
using CHCEngine::Window::Vector;
using CHCEngine::Window::Window;
using namespace CHCEngine::Renderer;

int t = 0;
Renderer renderer;
int main() {
  Window window;
  window.openWindow("Window", 800, 600);
  window.setFrameTimeLowerBound(30000000);
  std::cout << "simple window test" << std::endl;
  window.addKeyCallBack(
      "test call back",
      [&](Key key, Action action) {
        std::cout << t++ << std::endl;
        window.removeInputCallback("test call back");
      },
      Key::KEY_W, Action::ACTION_PRESS,
      {{{Key::KEY_S, Action::ACTION_REPEAT}},
       {{MouseButton::MOUSE_BUTTON_LEFT, Action::ACTION_REPEAT}}});
  window.addKeyCallBack(
      "test call back2",
      [&](Key key, Action action) { std::cout << t-- << std::endl; },
      Key::KEY_W, Action::ACTION_PRESS,
      {{{Key::KEY_S, Action::ACTION_REPEAT}},
       {{MouseButton::MOUSE_BUTTON_LEFT, Action::ACTION_REPEAT}}});
  Condition c = {{{Key::KEY_LEFT_SHIFT, Action::ACTION_REPEAT}},
                 {{MouseButton::MOUSE_BUTTON_LEFT, Action::ACTION_REPEAT}}};
  window.addMouseMoveCallback(
      "mouse_move",
      [](const Vector &position, const Vector &offset) {
        std::cout << "Position : " << position.X << "   " << position.Y
                  << std::endl;
      },
      c);
  window.addMouseButtonCallback(
      "mouse_button",
      [](MouseButton mouse_button, Action action, const Vector &position) {
        std::cout << "Mouse right click" << std::endl;
      },
      MouseButton::MOUSE_BUTTON_RIGHT, Action::ACTION_PRESS, c);
  window.addMouseButtonCallback(
      "mouse_button",
      [](MouseButton mouse_button, Action action, const Vector &position) {
        std::cout << "Mouse right click  2" << std::endl;
      },
      MouseButton::MOUSE_BUTTON_RIGHT, Action::ACTION_PRESS, c);
  window.addFramebufferSizeCallback("window_sizse", [](int width, int height) {
    std::cout << " new size : " << width << "  " << height << std::endl;
  });

  /*window.addLoopCallback("loop_start", [&](auto duration, uint64_t frame_) {
    std::cout << " start "<< std::endl;
    if (frame_ > 30000) window.removeLoopCallback("loop_start");
  },CHCEngine::Window::LoopCallbackType::LOOP_CALLBACK_TYPE_START);*/
  /*window.addKeyCallBack("test call back2", [&](Key key, Action action) {
          std::cout << t-- << std::endl;
          }, Key::KEY_W, Action::ACTION_PRESS);*/
  renderer.initializeDevice();
  // std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  renderer.setSwapChain(window);
  renderer.waitUntilWindowClose();
  window.waitUntilClose();
  return 0;
}