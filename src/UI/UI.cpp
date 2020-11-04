#include <fstream>
#include <streambuf>
#include <string>
#include <iostream>

#include <UI/TextEditor.h>
#include <UI/UI.h>
#include <Platform/Win64Platform.h>

/**
 * @brief GLFW KEYS
 */
#define GLFW_KEY_S 83

static TextEditor editor;
static auto lang = TextEditor::LanguageDefinition::GLSL();
static std::string currentFile;

static int selectedGameObjectIndex = -1;

void EditorUI::SetupContext(GLFWwindow *window)
{
  // Setup ImGui context
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  ImGui::StyleColorsClassic();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  editor.SetLanguageDefinition(lang);
}

void EditorUI::NewFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void EditorUI::ShaderEditorOpenFile(const char *file)
{
  std::ifstream t(file);
  if (t.good())
  {
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    editor.SetText(str);
  }
  t.close();
  currentFile = std::string(file);
}

void EditorUI::ShaderEditorSaveFile(const char *file, std::string &textToSave)
{
  std::ofstream t(file);
  if (t.good())
  {
    t << textToSave;
  }
  t.close();
}

void EditorUI::Update(bool &uiWindow, bool &hasChanges, GameData &gameData)
{
  auto cpos = editor.GetCursorPosition();
  ImGuiIO &io = ImGui::GetIO();
  if (ImGui::BeginMainMenuBar())
  {

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);
    ImGui::EndMainMenuBar();
  }
  if (uiWindow)
  {
    ImGui::Begin("Shader Editor", nullptr,
                 ImGuiWindowFlags_HorizontalScrollbar |
                     ImGuiWindowFlags_MenuBar);
    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (io.KeyCtrl && io.KeysDown[GLFW_KEY_S])
    {
      std::string textToSave = editor.GetText();
      EditorUI::ShaderEditorSaveFile(currentFile.c_str(), textToSave);
      hasChanges = true;
    }

    if (ImGui::BeginMenuBar())
    {
      bool ro = editor.IsReadOnly();
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Save") || (io.KeyCtrl && io.KeysDown[GLFW_KEY_S]))
        {
          std::string textToSave = editor.GetText();
          EditorUI::ShaderEditorSaveFile(currentFile.c_str(), textToSave);
          hasChanges = true;
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit"))
      {

        if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
          editor.SetReadOnly(ro);
        ImGui::Separator();

        if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr,
                            !ro && editor.CanUndo()))
          editor.Undo();
        if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
          editor.Redo();

        ImGui::Separator();

        if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
          editor.Copy();
        if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr,
                            !ro && editor.HasSelection()))
          editor.Cut();
        if (ImGui::MenuItem("Delete", "Del", nullptr,
                            !ro && editor.HasSelection()))
          editor.Delete();
        if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                            !ro && ImGui::GetClipboardText() != nullptr))
          editor.Paste();

        ImGui::Separator();

        if (ImGui::MenuItem("Select all", nullptr, nullptr))
          editor.SetSelection(
              TextEditor::Coordinates(),
              TextEditor::Coordinates(editor.GetTotalLines(), 0));

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("View"))
      {
        if (ImGui::MenuItem("Dark palette"))
          editor.SetPalette(TextEditor::GetDarkPalette());
        if (ImGui::MenuItem("Light palette"))
          editor.SetPalette(TextEditor::GetLightPalette());
        if (ImGui::MenuItem("Retro blue palette"))
          editor.SetPalette(TextEditor::GetRetroBluePalette());
        ImGui::EndMenu();
      }

      if (ImGui::Button("Close"))
        uiWindow = false;

      ImGui::EndMenuBar();
      ImGui::Text(
          "%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1,
          cpos.mColumn + 1, editor.GetTotalLines(),
          editor.IsOverwrite() ? "Ovr" : "Ins", editor.CanUndo() ? "*" : " ",
          editor.GetLanguageDefinition().mName.c_str(), currentFile.c_str());
      editor.Render("ShaderEditor");
    }
    ImGui::End();

    if (ImGui::Begin("Debug counters", nullptr, 0))
    {
      ImGui::SetWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
      for (unsigned int i = 0; i < NumRegions; i++)
      {
        ImGui::LabelText(DebugRegionStrings[i], "%.3f ms",
                         TicksToMilliseconds(AvgCycles[i]));
      }
    }
    ImGui::End();

    if (ImGui::Begin("Entity Browser", nullptr, 0))
    {
      // Use ImGui::Selectable to create a list of selectables and update the
      // FLAG_SELECTED of gameObjects in gameData struct using
      // EntityManager::SetFlags()
      const int previousSelection = selectedGameObjectIndex;
      for (int i = 0; i < gameData.gameObjects->size(); i++)
      {
        string name = string("(" + to_string(gameData.gameObjects->at(i).id) +
                             ") " + gameData.gameObjects->at(i).name);

        if (ImGui::Selectable(name.c_str(), selectedGameObjectIndex == i))
        {
          if (previousSelection != i)
          {
            selectedGameObjectIndex = i;
            if (previousSelection >= 0)
              EntityManager::UnsetFlags(
                  gameData.gameObjects->at(previousSelection), FLAG_SELECTED);
            EntityManager::SetFlags(gameData.gameObjects->at(i), FLAG_SELECTED);
          }
          else if (previousSelection == i && previousSelection >= 0)
          {
            selectedGameObjectIndex = -1;
            EntityManager::UnsetFlags(
                gameData.gameObjects->at(previousSelection), FLAG_SELECTED);
          }
        }
      }
    }
    ImGui::End();
  }
  ImGui::Render();
}

void EditorUI::Render()
{
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorUI::Shutdown()
{
  editor.Delete();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}