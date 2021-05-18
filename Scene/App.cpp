//
//  App.cpp
//
//  Based on TetraApp, which was created by srm, March 2020
//  Also using some code copied from in class example
//  (https://github.coecis.cornell.edu/cs5625/Rasterization21/blob/inclass/tetra_fbo/Demo/TetraFBO.cpp)
//

#include "App.hpp"
#include <nanogui/window.h>
#include <nanogui/glcanvas.h>
#include <nanogui/layout.h>
#include <ctime>

#include <cpplocate/cpplocate.h>

// Fixed screen size is awfully convenient, but you can also
// call Screen::setSize to set the size after the Screen base
// class is constructed.
const int App::windowWidth = 800;
const int App::windowHeight = 600;
const int SCREEN_WIDTH_PIXELS = 800;

// Constructor runs after nanogui is initialized and the OpenGL context is current.
App::App(const aiScene *importedScene, RTUtil::SceneInfo sceneInfo)
    : nanogui::Screen(Eigen::Vector2i(windowWidth, windowHeight), "App", false),
      backgroundColor(0.3f, 0.4f, 0.7f, 1.0f)
{
    const std::string resourcePath =
        cpplocate::locatePath("resources/Common", "", nullptr) + "resources/";

    forwardProg.reset(new GLWrap::Program("forwardProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/smooth.vs"}}));

    geomProg.reset(new GLWrap::Program("geomProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/smooth.vs"},
                                                    {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/gbuff.frag"}}));

    skyProg.reset(new GLWrap::Program("skyProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/fsq.vert"},
                                                  {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/sky.frag"}}));

    shadowProg.reset(new GLWrap::Program("shadowProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/smooth.vs"},
                                                        {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/nothing.frag"}}));

    ambientProg.reset(new GLWrap::Program("ambientProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/fsq.vert"},
                                                          {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/ambient.frag"}}));

    lightProg.reset(new GLWrap::Program("lightProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/fsq.vert"},
                                                      {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/lightshader.frag"},
                                                      {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/microfacet.fs"}}));

    bloomProg.reset(new GLWrap::Program("bloomProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/fsq.vert"},
                                                      {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/blur.fs"}}));

    srgbProg.reset(new GLWrap::Program("srgbProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/fsq.vert"},
                                                    {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/srgb.frag"}}));

    mergeProg.reset(new GLWrap::Program("mergeProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/fsq.vert"},
                                                      {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/merge.frag"}}));

    // skyProg.reset(new GLWrap::Program("srgbProg", {{GL_VERTEX_SHADER, resourcePath + "Common/shaders/fsq.vert"},
    //                                                 {GL_FRAGMENT_SHADER, resourcePath + "Common/shaders/sunsky.fs"}}));

    keys[FORWARD_KEY] = false;
    keys[BACKWARD_KEY] = false;
    keys[PIVOT_UP_KEY] = false;
    keys[PIVOT_DOWN_KEY] = false;
    keys[PIVOT_LEFT_KEY] = false;
    keys[PIVOT_RIGHT_KEY] = false;
    keys[LEFT_KEY] = false;
    keys[RIGHT_KEY] = false;

    screenWidth = App::windowWidth;
    screenHeight = App::windowHeight;

    if (importedScene->mNumCameras == 0)
    {
        // Use the default camera
        cam = std::make_shared<RTUtil::PerspectiveCamera>(
            Eigen::Vector3f(6, 8, 10),         // eye
            Eigen::Vector3f(0, 0, 0),          // target
            Eigen::Vector3f(0, 1, 0),          // up
            screenWidth / (float)screenHeight, // aspect
            0.1, 50.0,                         // near, far
            15.0 * M_PI / 180                  // fov
        );
    }
    else
    {
        aiCamera *firstCam = importedScene->mCameras[0];

        // Find the point closest to the origin in the direction that the camera
        // looks

        auto camToWorld = Utils::collectNodeToWorldTransform(importedScene, firstCam->mName);

        Eigen::Vector3f worldCamPos = camToWorld * RTUtil::a2e(firstCam->mPosition);
        Eigen::Vector3f worldCamDir = camToWorld.linear() * RTUtil::a2e(firstCam->mLookAt);
        Eigen::Vector3f worldUpDir = camToWorld.linear() * RTUtil::a2e(firstCam->mUp);

        Eigen::Vector3f targetPoint = Utils::calculateClosestOnPointRay(
            worldCamDir, worldCamPos,
            Eigen::Vector3f(0, 0, 0));

        screenWidth = SCREEN_WIDTH_PIXELS;
        screenHeight = SCREEN_WIDTH_PIXELS / (float)firstCam->mAspect;

        cam = std::make_shared<RTUtil::PerspectiveCamera>(
            worldCamPos,               // eye
            targetPoint,               // target
            worldUpDir,                // up
            firstCam->mAspect,         // aspect
            firstCam->mClipPlaneNear,  // near
            firstCam->mClipPlaneFar,   // far
            firstCam->mHorizontalFOV); // fov

        this->firstCam = firstCam;

        Utils::print(RTUtil::a2e(firstCam->mPosition), "eye");
        Utils::print(targetPoint, "target");
        Utils::print(RTUtil::a2e(firstCam->mUp), "up");
        std::cout << "aspect: " << firstCam->mAspect << "\nnear: " << firstCam->mClipPlaneNear << "\nfar: " << firstCam->mClipPlaneFar << "\nfov: " << firstCam->mHorizontalFOV << std::endl;
    }

    Screen::setSize(nanogui::Vector2i(screenWidth, screenHeight));

    scene.reset(new Scene(screenWidth, screenHeight, importedScene, sceneInfo, cam));
    cc.reset(new RTUtil::DefaultCC(cam));
    mesh.reset(new GLWrap::Mesh());

    // Create a full screen quad with two triangles

    Eigen::MatrixXf vertices(5, 4);
    vertices.col(0) << -1.0f, -1.0f, 0.0f, 0.0f, 0.0f;
    vertices.col(1) << 1.0f, -1.0f, 0.0f, 1.0f, 0.0f;
    vertices.col(2) << 1.0f, 1.0f, 0.0f, 1.0f, 1.0f;
    vertices.col(3) << -1.0f, 1.0f, 0.0f, 0.0f, 1.0f;

    Eigen::Matrix<float, 3, Eigen::Dynamic> positions = vertices.topRows<3>();
    Eigen::Matrix<float, 2, Eigen::Dynamic> texCoords = vertices.bottomRows<2>();

    fsqMesh.reset(new GLWrap::Mesh());

    fsqMesh->setAttribute(0, positions);
    fsqMesh->setAttribute(1, texCoords);

    // Create the FBOs

    outputFBO.reset(new GLWrap::Framebuffer({mSize[0], mSize[1]}, {{GL_RGBA32F, GL_RGBA}}));

    lightingFBO.reset(new GLWrap::Framebuffer({mSize[0], mSize[1]}, {{GL_RGBA32F, GL_RGBA}}));
    bloomTempFBO.reset(new GLWrap::Framebuffer({mSize[0], mSize[1]}, {{GL_RGBA32F, GL_RGBA}}));
    bloomOutFBO.reset(new GLWrap::Framebuffer({mSize[0], mSize[1]}, {{GL_RGBA32F, GL_RGBA}}));

    shadowFBO.reset(new GLWrap::Framebuffer({mSize[0], mSize[0]}, {}, {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT}));
    gBuffer.reset(new GLWrap::Framebuffer({mSize[0], mSize[1]}, 3));

    deferredShading = true;

    // NanoGUI boilerplate
    performLayout();
    setVisible(true);
}

bool App::keyboardEvent(int key, int scancode, int action, int modifiers)
{

    if (Screen::keyboardEvent(key, scancode, action, modifiers))
        return true;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        setVisible(false);
        return true;
    }
    else if (key == GLFW_KEY_F && action == GLFW_RELEASE)
    {
        deferredShading = !deferredShading;
    }

    //tank controls set key values
    if (key >= 0 && key < NUM_KEYS)
    {
        if (!keys[key] && action == GLFW_PRESS)
        {
            keys[key] = true;
        }
        else if (keys[key] && action == GLFW_RELEASE)
        {
            keys[key] = false;
        }
    }

    return cc->keyboardEvent(key, scancode, action, modifiers);
}

bool App::mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers)
{
    return Screen::mouseButtonEvent(p, button, down, modifiers) ||
           cc->mouseButtonEvent(p, button, down, modifiers);
}

bool App::mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers)
{
    return Screen::mouseMotionEvent(p, rel, button, modifiers) ||
           cc->mouseMotionEvent(p, rel, button, modifiers);
}

bool App::scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel)
{
    return Screen::scrollEvent(p, rel) ||
           cc->scrollEvent(p, rel);
}

void App::update(float deltaTime)
{
    float distance = deltaTime * CAMERA_VELOCITY;
    float angle = deltaTime * CAMERA_ANGULAR_VELOCITY;
    if (keys[FORWARD_KEY])
    {
        std::cout << "Moving forward, dt = " << deltaTime << std::endl;
        // scene.moveForward(distance);
        moveCameraFrontBack(distance);
    }
    if (keys[BACKWARD_KEY])
    {
        std::cout << "Moving backward, dt = " << deltaTime << std::endl;
        moveCameraFrontBack(-distance);
        // scene.moveForward(-distance);
    }
    else if (keys[RIGHT_KEY])
    {
        std::cout << "rotate right, dt = " << deltaTime << std::endl;
        // scene.rotate(-angle, Eigen::Vector3f::UnitY());
        moveCameraLeftRight(distance);
    }
    if (keys[LEFT_KEY])
    {
        std::cout << "rotate left, dt = " << deltaTime << std::endl;
        // scene.rotate(angle, Eigen::Vector3f::UnitY());
        moveCameraLeftRight(-distance);
    }
    if (keys[PIVOT_DOWN_KEY])
    {
        std::cout << "rotate down, dt = " << deltaTime << std::endl;
        // scene.rotate(-angle, Eigen::Vector3f::UnitX());
        cam->setTarget(cam->getTarget() + -angle * cam->getUp().normalized());
    }
    if (keys[PIVOT_UP_KEY])
    {
        std::cout << "rotate up, dt = " << deltaTime << std::endl;
        // scene.rotate(angle, Eigen::Vector3f::UnitX());
        cam->setTarget(cam->getTarget() + angle * cam->getUp().normalized());
    }
    if (keys[PIVOT_RIGHT_KEY])
    {
        cam->setTarget(cam->getTarget() + angle * cam->getRight().normalized());
    }
    if (keys[PIVOT_LEFT_KEY])
    {
        cam->setTarget(cam->getTarget() + -angle * cam->getRight().normalized());
    }
}

void App::moveCameraFrontBack(float distance)
{
    Eigen::Vector3f camDir = cam->getTarget() - cam->getEye();
    camDir.normalize();
    Eigen::Vector3f newCamPos = cam->getEye() + distance * camDir;
    cam->setEye(newCamPos);
    cam->setTarget(cam->getTarget() + distance * camDir);
}

void App::moveCameraLeftRight(float distance)
{
    Eigen::Vector3f newCamPos = cam->getEye() + distance * cam->getRight().normalized();
    Eigen::Vector3f newTargetPos = cam->getTarget() + distance * cam->getRight().normalized();
    cam->setEye(newCamPos);
    cam->setTarget(newTargetPos);
}

void App::drawContents()
{
    GLWrap::checkGLError("drawContents start");
    glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());

    // tank control update
    auto now = std::chrono::high_resolution_clock::now();
    double deltaTime = std::chrono::duration<double, std::milli>(now - curTime).count();
    curTime = now;
    update(deltaTime);
    // First shading pass: geometry pass
    if (deferredShading)
    {
        // Do the geometry pass first

        gBuffer->bind();

        glViewport(0, 0, mSize[0], mSize[1]);

        // Note: looked at tutorial by Joey DeVries, "LearnOpenGL"
        // https://learnopengl.com/Advanced-Lighting/Deferred-Shading
        GLenum attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);

        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        scene->draw(geomProg, true);

        gBuffer->unbind();

        // Set up the lighting FBO

        lightingFBO->bind();

        glViewport(0, 0, mSize[0], mSize[1]);
        glClearColor(0.f, 0.f, 0.f, 0.f); // So that each quad we draw is on top of 0's
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST); // Otherwise, one quad won't write over the previous one

        // Simply add the light values
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        lightingFBO->unbind();

        // Do ambient shading pass

        drawAmbientAndSky();

        for (int i = 0; i < scene->thresholds.size(); i++)
        {
            auto uniName = "thresholds[" + std::to_string(i) + "]";
            lightProg->uniform(uniName, scene->thresholds[i]);
        }

        for (unsigned int lightIdx = 0; lightIdx < scene->numPointLights(); lightIdx++)
        {
            PositionalLight light = scene->getLight(lightIdx);
            drawLight(light);
        }

        drawBloom();

        glViewport(0, 0, mFBSize[0], mFBSize[1]);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // Last shading pass: post processing pass
        // lightingFBO->colorTexture().bindToTextureUnit(0);
        outputFBO->colorTexture().bindToTextureUnit(0);

        // bloomOutFBO->colorTexture().bindToTextureUnit(0);

        // gBuffer->colorTexture(0).bindToTextureUnit(0);
        // shadowFBO->depthTexture().bindToTextureUnit(0);
        // bloomOutFBO->colorTexture().bindToTextureUnit(0);

        srgbProg->use();
        srgbProg->uniform("exposure", 1.0f);
        srgbProg->uniform("backgroundColor", backgroundColor);

        // bloomTempFBO->colorTexture().bindToTextureUnit(0);
        srgbProg->uniform("image", 0);
        // Draw the full screen quad
        fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
        srgbProg->unuse();
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        // glViewport(0, 0, screenWidth, screenHeight);
        scene->draw(forwardProg, true);
    }
}

void App::drawAmbientAndSky()
{
    lightingFBO->bind();

    ambientProg->use();

    AmbientLight ambientLight = scene->getAmbientLight();

    RTUtil::Sky sky = scene->getSky();
    sky.setUniforms(*ambientProg);

    // std::cout << "range! " << ambientLight.range() << std::endl;
    ambientProg->uniform("ambientRadiance", ambientLight.radiance());
    ambientProg->uniform("mProjInv", cam->getProjectionMatrix().inverse().matrix());
    ambientProg->uniform("mProj", cam->getProjectionMatrix().matrix());
    ambientProg->uniform("occlusionRange", ambientLight.range());
    ambientProg->uniform("mViewInv", cam->getViewMatrix().inverse().matrix());

    gBuffer->colorTexture(0).bindToTextureUnit(0);
    ambientProg->uniform("diffuseReflectance", 0);
    gBuffer->colorTexture(1).bindToTextureUnit(1);
    ambientProg->uniform("shaderParamTexture", 1);
    gBuffer->colorTexture(2).bindToTextureUnit(2);
    ambientProg->uniform("normalTexture", 2);
    gBuffer->depthTexture().bindToTextureUnit(3);
    ambientProg->uniform("depthTexture", 3);

    // Draw the full screen quad
    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

    ambientProg->unuse();

    lightingFBO->unbind();
}

void App::drawLight(PositionalLight light)
{
    // Second shading pass: lighting pass
    shadowFBO->bind();

    glViewport(0, 0, mSize[0], mSize[0]);

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); // Otherwise, one quad won't write over the previous one

    RTUtil::PerspectiveCamera lightCamera = createLightCamera(light);
    scene->drawWithCamera(shadowProg, lightCamera, true);

    shadowFBO->unbind();

    lightingFBO->bind();

    glViewport(0, 0, mSize[0], mSize[1]);

    glDisable(GL_DEPTH_TEST);

    lightProg->use();

    gBuffer->colorTexture(0).bindToTextureUnit(0);
    lightProg->uniform("diffuseTexture", 0);
    gBuffer->colorTexture(1).bindToTextureUnit(1);
    lightProg->uniform("shaderParamTexture", 1);
    gBuffer->colorTexture(2).bindToTextureUnit(2);
    lightProg->uniform("normalTexture", 2);
    gBuffer->depthTexture().bindToTextureUnit(3);
    lightProg->uniform("depthTexture", 3);
    shadowFBO->depthTexture().bindToTextureUnit(4);
    lightProg->uniform("shadowMapTexture", 4);

    // Uniforms
    // lightProg->uniform("mProj", cam->getProjectionMatrix().matrix());
    lightProg->uniform("mProjInv", cam->getProjectionMatrix().inverse().matrix());
    lightProg->uniform("mV", cam->getViewMatrix().matrix());
    lightProg->uniform("mVInv", cam->getViewMatrix().inverse().matrix());
    lightProg->uniform("mL", light.getNodeToWorldTransform().matrix());
    //light camera uniforms
    lightProg->uniform("mLCProj", lightCamera.getProjectionMatrix().matrix());
    lightProg->uniform("mLCView", lightCamera.getViewMatrix().matrix());

    lightProg->uniform("lightPosition", light.getPos());
    lightProg->uniform("lightPower", light.getPower());

    // Draw the full screen quad
    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

    lightProg->unuse();

    lightingFBO->unbind();
}

void App::blurPass(float stdev, float weight, int level)
{
    float mipmapStdev = stdev / (std::pow(2, level));
    int radius = std::ceil(3.0 * mipmapStdev);

    // int radius = std::ceil(3.0 * stdev);

    // Horizontal blur

    bloomTempFBO->bind(level);
    glViewport(0, 0, mSize[0] / (std::pow(2, level)), mSize[1] / (std::pow(2, level)));

    // bloomTempFBO->bind();

    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    bloomProg->use();

    // Send the unblurred image to blur vertically
    lightingFBO->colorTexture().bindToTextureUnit(0);
    bloomProg->uniform("dir", Eigen::Vector2f(0.f, 1.f)); // Along y axis
    bloomProg->uniform("image", 0);
    bloomProg->uniform("stdev", mipmapStdev);
    bloomProg->uniform("radius", radius);
    // bloomProg->uniform("weight", weight);
    bloomProg->uniform("isActive", stdev > 0.0); // Active only if stdev is positive
    // // bloomProg->uniform("weight", 1.f);
    // bloomProg->uniform("active", true);
    bloomProg->uniform("level", level);

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

    bloomProg->unuse();

    bloomTempFBO->unbind();

    // Vertical blur

    bloomOutFBO->bind(level);
    glViewport(0, 0, mSize[0] / (std::pow(2, level)), mSize[1] / (std::pow(2, level)));
    // bloomOutFBO->bind();

    bloomProg->use();

    // Send the image that has been blurred vertically to blur horizontally
    bloomTempFBO->colorTexture().bindToTextureUnit(0);
    bloomProg->uniform("dir", Eigen::Vector2f(1.f, 0.f)); // Along x axis
    bloomProg->uniform("image", 0);
    bloomProg->uniform("stdev", mipmapStdev);
    bloomProg->uniform("radius", radius);
    // bloomProg->uniform("weight", weight);
    bloomProg->uniform("isActive", stdev > 0.0); // Active only if stdev is positive
    // bloomProg->uniform("weight", 1.f);
    // bloomProg->uniform("active", true);
    bloomProg->uniform("level", level);

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

    bloomProg->unuse();

    bloomOutFBO->unbind();
}

void App::drawBloom()
{
    // lightingFBO->colorTexture().generateMipmap();
    // lightingFBO->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    // bloomTempFBO->colorTexture().generateMipmap();
    // bloomTempFBO->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    // bloomOutFBO->colorTexture().generateMipmap();
    // bloomOutFBO->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    lightingFBO->colorTexture().generateMipmap();
    lightingFBO->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    bloomTempFBO->colorTexture().generateMipmap();
    bloomTempFBO->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    bloomOutFBO->colorTexture().generateMipmap();
    bloomOutFBO->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // bloomTempFBO is cleared at the start of every blur pass
    bloomOutFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT);
    bloomOutFBO->unbind();

    float weights[] = {0.8843, 0.1, 0.012, 0.0027, 0.001};
    float stdevs[] = {0.0, 6.2, 24.9, 81.0, 263.0};
    int levels[] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++)
    {
        blurPass(stdevs[i], weights[i], levels[i]);
    }

    outputFBO->bind();
    glViewport(0, 0, mSize[0], mSize[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    mergeProg->use();

    // // // for (int i = 0; i < 5; i++)
    // // // {
    // // //     std::stringstream weightStream;
    // // //     weightStream << "weights[" << i << "]";
    // // //     mergeProg->uniform(weightStream.str(), weights[i]);

    // // //     std::stringstream levelStream;
    // // //     levelStream << "levels[" << i << "]";
    // // //     mergeProg->uniform(levelStream.str(), levels[i]);
    // // // }
    // // // mergeProg->uniform()

    bloomOutFBO->colorTexture().bindToTextureUnit(0);

    mergeProg->uniform("blurredTextureMipMap", 0);

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

    mergeProg->unuse();
    outputFBO->unbind();
}

RTUtil::PerspectiveCamera App::createLightCamera(PositionalLight light)
{
    auto camera = RTUtil::PerspectiveCamera(
        light.getNodeToWorldTransform() * light.getPos(), // eye
        Eigen::Vector3f(0, 0, 0),                         // target
        Eigen::Vector3f(0, 1, 0),                         // up
        1,                                                // aspect
        1, 80,                                            // near, far
        1                                                 // fov
    );

    return camera;
}