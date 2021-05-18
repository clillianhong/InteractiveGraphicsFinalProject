//
//  App.cpp
//
//  Based on TetraApp, which was created by srm, March 2020
//

#pragma once

#include <nanogui/screen.h>

#include <GLWrap/Program.hpp>
#include <GLWrap/Mesh.hpp>
#include <GLWrap/Framebuffer.hpp>
#include <RTUtil/Camera.hpp>
#include <chrono>
#include <RTUtil/CameraController.hpp>

#include "Scene.hpp"

class App : public nanogui::Screen
{
public:
    App();
    App(const aiScene *importedScene, RTUtil::SceneInfo sceneInfo);

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
    virtual bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers) override;
    virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel) override;

    virtual void drawContents() override;

private:
    static const int windowWidth;
    static const int windowHeight;

    aiCamera *firstCam;

    int screenWidth;
    int screenHeight;

    // General deferred shading
    std::unique_ptr<GLWrap::Mesh> fsqMesh;

    // Geometry pass
    std::unique_ptr<GLWrap::Program> geomProg;

    // Sky pass
    std::unique_ptr<GLWrap::Program> skyProg;

    // Lighting pass
    std::unique_ptr<GLWrap::Program> ambientProg;

    std::unique_ptr<GLWrap::Framebuffer> outputFBO;
    std::unique_ptr<GLWrap::Program> lightProg;
    std::unique_ptr<GLWrap::Framebuffer> lightingFBO;

    std::unique_ptr<GLWrap::Program> shadowProg;
    std::unique_ptr<GLWrap::Framebuffer> shadowFBO;

    // Bloom pass
    std::unique_ptr<GLWrap::Program> bloomProg;
    std::unique_ptr<GLWrap::Program> mergeProg;
    std::unique_ptr<GLWrap::Framebuffer> bloomTempFBO;
    std::unique_ptr<GLWrap::Framebuffer> bloomOutFBO;

    // Post processing pass
    std::unique_ptr<GLWrap::Program> srgbProg;
    std::unique_ptr<GLWrap::Framebuffer> gBuffer;

    bool deferredShading;

    // Forward rendering fields
    std::unique_ptr<GLWrap::Program> forwardProg;
    std::unique_ptr<GLWrap::Mesh> mesh;

    std::shared_ptr<RTUtil::PerspectiveCamera> cam;
    std::unique_ptr<RTUtil::DefaultCC> cc;

    nanogui::Color backgroundColor;

    std::unique_ptr<Scene> scene;

    RTUtil::PerspectiveCamera createLightCamera(PositionalLight light);

    //TANK CONTROL
    std::chrono::_V2::system_clock::time_point curTime = std::chrono::high_resolution_clock::now();
    const float CAMERA_VELOCITY = 0.001;
    const float CAMERA_ANGULAR_VELOCITY = 0.001;
    const int FORWARD_KEY = GLFW_KEY_W;
    const int BACKWARD_KEY = GLFW_KEY_S;
    const int LEFT_KEY = GLFW_KEY_A;
    const int RIGHT_KEY = GLFW_KEY_D;
    const int PIVOT_UP_KEY = GLFW_KEY_UP;
    const int PIVOT_DOWN_KEY = GLFW_KEY_DOWN;
    const int PIVOT_RIGHT_KEY = GLFW_KEY_RIGHT;
    const int PIVOT_LEFT_KEY = GLFW_KEY_LEFT;
    static constexpr int NUM_KEYS = 1024;
    std::array<bool, NUM_KEYS> keys;

    // Draw functions
    void drawAmbientAndSky();
    void drawLight(PositionalLight light);
    void blurPass(float stdev, float weight, int k);
    void drawBloom();
    void update(float deltaTime);
    void moveCameraFrontBack(float distance);
    void moveCameraLeftRight(float distance);
    void pivotCameraUpDown(float angle);
    void pivotCameraLeftRight(float angle);

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
