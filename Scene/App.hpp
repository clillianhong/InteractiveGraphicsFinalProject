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
    std::unique_ptr<GLWrap::Framebuffer> ambientFBO;
    std::unique_ptr<GLWrap::Program> ambientProg;

    // std::unique_ptr<GLWrap::Program> edgeProg;
    std::unique_ptr<GLWrap::Program> filterProg;

    std::shared_ptr<GLWrap::Framebuffer> outputFBO;
    std::unique_ptr<GLWrap::Program> lightProg;
    std::shared_ptr<GLWrap::Framebuffer> lightingFBO;

    std::shared_ptr<GLWrap::Framebuffer> sobelTempFBO;
    std::shared_ptr<GLWrap::Framebuffer> sobelOutXFBO;
    std::shared_ptr<GLWrap::Framebuffer> sobelOutYFBO;
    std::unique_ptr<GLWrap::Framebuffer> edgesFBO;
    std::shared_ptr<GLWrap::Framebuffer> outlineFBO;

    std::unique_ptr<GLWrap::Program> shadowProg;
    std::unique_ptr<GLWrap::Framebuffer> shadowFBO;

    std::unique_ptr<GLWrap::Program> mergeEdgesProg;

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

    // Draw functions
    void drawAmbientAndSky();
    void drawEdges();
    void drawLight(PositionalLight light);
    void blurPass(float stdev, float weight, int k, std::shared_ptr<GLWrap::Framebuffer> inFBO);
    void drawBloom(std::shared_ptr<GLWrap::Framebuffer> inFBO);

    void drawSobel();
    void sobelPass(bool horizontal, const GLWrap::Texture2D* depthMap, std::shared_ptr<GLWrap::Framebuffer> outFBO);

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
