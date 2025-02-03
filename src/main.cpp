#include <filesystem>
#include <fstream>

#include "glad.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"

#include <GLFW/glfw3.h>
#include <thread>

#include "chip-8.hpp"

std::string read_file(const std::filesystem::path &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Failed to open file");
  size_t file_size = static_cast<size_t>(file.tellg());
  std::string buffer;
  buffer.reserve(file_size);
  file.seekg(0);
  size_t counter = 0;
  while (counter++ != file_size)
    buffer += file.get();
  file.close();
  return buffer;
}

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    throw std::runtime_error("chip8 rom");
  }

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    std::cerr << "GLFW failed!\n";
    return 1;
  }

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only

  GLFWwindow *window = glfwCreateWindow(
      1200, 800, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
  if (window == nullptr)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // Enable vsync

  if (!gladLoadGL()) {
    std::cerr << "OpenGL loader failed!\n";
    return 1;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;              // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                                      // Platform Windows
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  ImGui_ImplGlfw_InitForOpenGL(window, true);

  ImGui_ImplOpenGL3_Init(glsl_version);

  auto chip8 = chip8_t<64, 32>::create();
  chip8.upload_font(font, sizeof(font));

  auto rom = read_file(argv[1]);
  chip8.upload_rom((uint8_t *)rom.data(), rom.size());

  GLuint pbo;
  glGenBuffers(1, &pbo);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(chip8.fb), nullptr,
               GL_STREAM_DRAW);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);

  MemoryEditor memory_editor;
  memory_editor.Cols = 8;

  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  bool exit = false;

  auto tick = [&]() {
    float tick_target_fps = 60.f;
    auto tick_last_time = std::chrono::system_clock::now();

    while (!glfwWindowShouldClose(window)) {
      if (exit)
        break;
      auto tick_current_time = std::chrono::system_clock::now();
      auto tick_time_difference = tick_current_time - tick_last_time;
      if (tick_time_difference.count() / 1e6 < 1000.f / tick_target_fps) {
        continue;
      }
      tick_last_time = tick_current_time;
      chip8.tick();
    }
  };

  std::thread thread{tick};

  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      exit = true;
      break;
    }

    if (glfwGetKey(window, GLFW_KEY_1))
      chip8.press_key(1);
    else
      chip8.release_key(1);

    if (glfwGetKey(window, GLFW_KEY_2))
      chip8.press_key(2);
    else
      chip8.release_key(2);

    if (glfwGetKey(window, GLFW_KEY_3))
      chip8.press_key(3);
    else
      chip8.release_key(3);

    if (glfwGetKey(window, GLFW_KEY_4))
      chip8.press_key(C);
    else
      chip8.release_key(C);

    if (glfwGetKey(window, GLFW_KEY_Q))
      chip8.press_key(4);
    else
      chip8.release_key(4);

    if (glfwGetKey(window, GLFW_KEY_W))
      chip8.press_key(5);
    else
      chip8.release_key(5);

    if (glfwGetKey(window, GLFW_KEY_E))
      chip8.press_key(6);
    else
      chip8.release_key(6);

    if (glfwGetKey(window, GLFW_KEY_R))
      chip8.press_key(D);
    else
      chip8.release_key(D);

    if (glfwGetKey(window, GLFW_KEY_A))
      chip8.press_key(7);
    else
      chip8.release_key(7);

    if (glfwGetKey(window, GLFW_KEY_S))
      chip8.press_key(8);
    else
      chip8.release_key(8);

    if (glfwGetKey(window, GLFW_KEY_D))
      chip8.press_key(9);
    else
      chip8.release_key(9);

    if (glfwGetKey(window, GLFW_KEY_F))
      chip8.press_key(E);
    else
      chip8.release_key(E);

    if (glfwGetKey(window, GLFW_KEY_Z))
      chip8.press_key(A);
    else
      chip8.release_key(A);

    if (glfwGetKey(window, GLFW_KEY_X))
      chip8.press_key(0);
    else
      chip8.release_key(0);

    if (glfwGetKey(window, GLFW_KEY_C))
      chip8.press_key(B);
    else
      chip8.release_key(B);

    if (glfwGetKey(window, GLFW_KEY_V))
      chip8.press_key(F);
    else
      chip8.release_key(F);

    instruction_t inst = chip8.fetch();
    chip8.decode_and_execute();

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, sizeof(chip8.fb), &chip8.fb);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGBA, GL_UNSIGNED_BYTE,
                    nullptr);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiDockNodeFlags dockspaceFlags =
        ImGuiDockNodeFlags_None & ~ImGuiDockNodeFlags_PassthruCentralNode;
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoDecoration;

    bool dockSpace = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    auto mainViewPort = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(mainViewPort->WorkPos);
    ImGui::SetNextWindowSize(mainViewPort->WorkSize);
    ImGui::SetNextWindowViewport(mainViewPort->ID);

    ImGui::Begin("DockSpace", &dockSpace, windowFlags);
    ImGuiID dockspaceID = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspaceID, ImGui::GetContentRegionAvail(),
                     dockspaceFlags);
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGuiWindowFlags viewPortFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration;
    ImGui::Begin("viewport", nullptr, viewPortFlags);
    ImGui::Image(
        reinterpret_cast<ImTextureID>(reinterpret_cast<void *>(texture)),
        ImGui::GetContentRegionAvail(), ImVec2(0, -1), ImVec2(1, 0));
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::Begin("state");
    // stuff
    ImGui::Text("%f", ImGui::GetIO().Framerate);
    ImGui::Text("program counter: %04X", chip8.pc);
    ImGui::Text("index: %04X", chip8.I);
    ImGui::Text("V0: %02X", chip8.V0);
    ImGui::SameLine();
    ImGui::Text("V1: %02X", chip8.V1);
    ImGui::SameLine();
    ImGui::Text("V2: %02X", chip8.V2);
    ImGui::SameLine();
    ImGui::Text("V3: %02X", chip8.V3);
    ImGui::Text("V4: %02X", chip8.V4);
    ImGui::SameLine();
    ImGui::Text("V5: %02X", chip8.V5);
    ImGui::SameLine();
    ImGui::Text("V6: %02X", chip8.V6);
    ImGui::SameLine();
    ImGui::Text("V7: %02X", chip8.V7);
    ImGui::Text("V8: %02X", chip8.V8);
    ImGui::SameLine();
    ImGui::Text("V9: %02X", chip8.V9);
    ImGui::SameLine();
    ImGui::Text("VA: %02X", chip8.VA);
    ImGui::SameLine();
    ImGui::Text("VB: %02X", chip8.VB);
    ImGui::Text("VC: %02X", chip8.VC);
    ImGui::SameLine();
    ImGui::Text("VD: %02X", chip8.VD);
    ImGui::SameLine();
    ImGui::Text("VE: %02X", chip8.VE);
    ImGui::SameLine();
    ImGui::Text("VF: %02X", chip8.VF);
    ImGui::Text("stack: ");
    ImGui::SameLine();
    if (chip8.stack.size()) {
      ImGui::Text("%04X", chip8.stack.top());
    } else {
      ImGui::Text("null");
    }
    ImGui::Text("Delay: %02X", chip8.delay);
    ImGui::Text("Sound: %02X", chip8.sound);
    ImGui::Text("key[0]: %d", chip8.keys[0]);
    ImGui::SameLine();
    ImGui::Text("key[1]: %d", chip8.keys[1]);
    ImGui::SameLine();
    ImGui::Text("key[2]: %d", chip8.keys[2]);
    ImGui::SameLine();
    ImGui::Text("key[3]: %d", chip8.keys[3]);
    ImGui::Text("key[4]: %d", chip8.keys[4]);
    ImGui::SameLine();
    ImGui::Text("key[5]: %d", chip8.keys[5]);
    ImGui::SameLine();
    ImGui::Text("key[6]: %d", chip8.keys[6]);
    ImGui::SameLine();
    ImGui::Text("key[7]: %d", chip8.keys[7]);
    ImGui::Text("key[8]: %d", chip8.keys[8]);
    ImGui::SameLine();
    ImGui::Text("key[9]: %d", chip8.keys[9]);
    ImGui::SameLine();
    ImGui::Text("key[A]: %d", chip8.keys[A]);
    ImGui::SameLine();
    ImGui::Text("key[B]: %d", chip8.keys[B]);
    ImGui::Text("key[C]: %d", chip8.keys[C]);
    ImGui::SameLine();
    ImGui::Text("key[D]: %d", chip8.keys[D]);
    ImGui::SameLine();
    ImGui::Text("key[E]: %d", chip8.keys[E]);
    ImGui::SameLine();
    ImGui::Text("key[F]: %d", chip8.keys[F]);
    ImGui::End();
    memory_editor.DrawWindow("memory", chip8.memory, sizeof(chip8.memory));

    ImGui::Begin("disassembly");
    for (uint16_t i = chip8.pc - 18; i <= chip8.pc + 18; i += 2) {
      instruction_t inst;
      uint8_t hi = chip8.memory[i];
      uint8_t lo = chip8.memory[i + 1];
      inst._inst = (hi << 8) | lo;

      auto [dis, actual, comment] = chip8.disassembly(inst);
      if (i == chip8.pc) {
        ImGui::PushStyleColor(ImGuiCol_Text,
                              IM_COL32(255, 255, 0, 255)); // Yellow highlight
      }

      ImGui::Text("%04X %s %s   %s", i, dis.c_str(), actual.c_str(),
                  comment.c_str());

      if (i == chip8.pc) {
        ImGui::PopStyleColor();
      }
    }
    ImGui::End();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(window);
  }

  thread.join();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
