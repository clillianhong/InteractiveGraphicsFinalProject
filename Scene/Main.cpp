#include <iostream>
// #include "DemoApp.hpp"
#include "TetraApp.hpp"
// #include "SkyApp.hpp"
#include "App.hpp"
#include "Importer.hpp"
#include <cpplocate/cpplocate.h>

int main(int argc, char const *argv[])
{
  std::string sceneName = argv[1];

  const std::string resourcePath =
      cpplocate::locatePath("resources/scenes", "", nullptr);

  Importer importer;
  const aiScene *importedScene = importer.importFromFile(resourcePath + "resources/scenes/" + sceneName + ".dae");

  RTUtil::SceneInfo sceneInfo;
  RTUtil::readSceneInfo(resourcePath + "resources/scenes/" + +"citystreet_single_info.json", sceneInfo);
  // RTUtil::readSceneInfo(resourcePath + "resources/scenes/" + sceneName + "_info.json", sceneInfo);

  nanogui::init();

  nanogui::ref<App> app = new App(importedScene, sceneInfo);
  nanogui::mainloop(16);

  // if (argc > 1 && std::string(argv[1]) == "Tetra") {
  //   nanogui::ref<TetraApp> app = new TetraApp();
  //   nanogui::mainloop(16);
  // } else if (argc > 1 && std::string(argv[1]) == "Sky") {
  //   nanogui::ref<SkyApp> app = new SkyApp();
  //   nanogui::mainloop(16);
  // } else {
  //   nanogui::ref<DemoApp> app = new DemoApp();
  //   nanogui::mainloop(16);
  // }

  nanogui::shutdown();
}
