/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <thread>
#include <chrono>
#include <limits>
#include <cmath>
#include <cstdio>

#include <vts-browser/buffer.hpp>
#include <vts-browser/exceptions.hpp>
#include <vts-browser/celestial.hpp>
#include <vts-browser/mapCallbacks.hpp>
#include <vts-browser/mapStatistics.hpp>
#include <vts-browser/cameraDraws.hpp>
#include <vts-browser/cameraCredits.hpp>
#include <vts-browser/position.hpp>

#include <SDL2/SDL.h>
#include <optick.h>

#include "mainWindow.hpp"

void initializeDesktopData();

namespace
{

struct Initializer
{
    Initializer()
    {
        initializeDesktopData();
    }
} initializer;

void windowSwap(SDL_Window *window)
{
    OPTICK_EVENT();
    SDL_GL_SwapWindow(window);
}

} // namespace

AppOptions::AppOptions() :
    oversampleRender(1),
    renderCompas(0),
    simulatedFpsSlowdown(0),
    screenshotOnFullRender(false),
    closeOnFullRender(false),
    purgeDiskCache(false)
{}

MainWindow::MainWindow(struct SDL_Window *window, void *renderContext,
    vts::Map *map, vts::Camera *camera, vts::Navigation *navigation,
    const AppOptions &appOptions,
    const vts::renderer::RenderOptions &renderOptions) :
    appOptions(appOptions),
    map(map), camera(camera), navigation(navigation),
    window(window), renderContext(renderContext)
{
    {
        int major = 0, minor = 0;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
        std::stringstream s;
        s << "OpenGL version: " << major << "." << minor;
        vts::log(vts::LogLevel::info2, s.str());
    }

    context.bindLoadFunctions(map);
    view = context.createView(camera);
    view->options() = renderOptions;

    // load mesh sphere
    {
        meshSphere = std::make_shared<vts::renderer::Mesh>();
        vts::GpuMeshSpec spec(vts::readInternalMemoryBuffer(
                                  "data/meshes/sphere.obj"));
        assert(spec.faceMode == vts::GpuMeshSpec::FaceMode::Triangles);
        spec.attributes[0].enable = true;
        spec.attributes[0].stride = sizeof(vts::vec3f) + sizeof(vts::vec2f);
        spec.attributes[0].components = 3;
        spec.attributes[1].enable = true;
        spec.attributes[1].stride = sizeof(vts::vec3f) + sizeof(vts::vec2f);
        spec.attributes[1].components = 2;
        spec.attributes[1].offset = sizeof(vts::vec3f);
        vts::ResourceInfo info;
        meshSphere->load(info, spec, "data/meshes/sphere.obj");
    }

    // load mesh line
    {
        meshLine = std::make_shared<vts::renderer::Mesh>();
        vts::GpuMeshSpec spec(vts::readInternalMemoryBuffer(
                                  "data/meshes/line.obj"));
        assert(spec.faceMode == vts::GpuMeshSpec::FaceMode::Lines);
        spec.attributes[0].enable = true;
        spec.attributes[0].stride = sizeof(vts::vec3f) + sizeof(vts::vec2f);
        spec.attributes[0].components = 3;
        spec.attributes[1].enable = true;
        spec.attributes[1].stride = sizeof(vts::vec3f) + sizeof(vts::vec2f);
        spec.attributes[1].components = 2;
        spec.attributes[1].offset = sizeof(vts::vec3f);
        vts::ResourceInfo info;
        meshLine->load(info, spec, "data/meshes/line.obj");
    }
}

MainWindow::~MainWindow()
{}

void MainWindow::renderFrame()
{
    OPTICK_EVENT();

    vts::renderer::RenderOptions &ro = view->options();
    view->render();

    // compas
    if (appOptions.renderCompas)
    {
        OPTICK_EVENT("compas");
        double size = std::min(ro.width, ro.height) * 0.09;
        double offset = size * (0.5 + 0.2);
        double posSize[3] = { offset, offset, size };
        double rot[3];
        navigation->getRotation(rot);
        view->renderCompass(posSize, rot);
    }

    // gui
    {
        OPTICK_EVENT("gui");
        gui.render(ro.targetViewportW, ro.targetViewportH);
    }
}

void MainWindow::prepareMarks()
{
    vts::mat4 view = vts::rawToMat4(camera->draws().camera.view);

    const Mark *prev = nullptr;
    for (const Mark &m : marks)
    {
        vts::mat4 mv = view
                * vts::translationMatrix(m.coord)
                * vts::scaleMatrix(navigation->getViewExtent() * 0.005);
        vts::mat4f mvf = mv.cast<float>();
        vts::DrawInfographicsTask t;
        vts::vec4f c = vts::vec3to4(m.color, 1);
        for (int i = 0; i < 4; i++)
            t.color[i] = c(i);
        t.mesh = meshSphere;
        vts::matToRaw(mvf, t.mv);
        camera->draws().infographics.push_back(t);
        if (prev)
        {
            t.mesh = meshLine;
            mv = view * vts::lookAt(m.coord, prev->coord);
            mvf = mv.cast<float>();
            vts::matToRaw(mvf, t.mv);
            camera->draws().infographics.push_back(t);
        }
        prev = &m;
    }
}

bool MainWindow::processEvents()
{
    OPTICK_EVENT();

    SDL_Event event;
    gui.inputBegin();
    while (SDL_PollEvent(&event))
    {
        // window closing
        if (event.type == SDL_QUIT)
        {
            gui.inputEnd();
            return true;
        }

        // handle gui
        if (gui.input(event))
            continue;

        // fullscreen
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_F11)
        {
            bool c = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
            SDL_SetWindowFullscreen(window, c ? 0 : SDL_WINDOW_FULLSCREEN);
        }

        // screenshot
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_p)
        {
            makeScreenshot();
        }

        // add mark
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_m)
        {
            Mark mark;
            mark.coord = getWorldPositionFromCursor();
            if (mark.coord(0) == mark.coord(0))
            {
                marks.push_back(mark);
                colorizeMarks();
            }
        }

        // north-up button
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE)
        {
            if (map->getMapconfigAvailable())
            {
                navigation->setRotation({0,270,0});
                navigation->options().type
                        = vts::NavigationType::Quick;
                navigation->resetNavigationMode();
            }
        }

        // mouse wheel
        if (event.type == SDL_MOUSEWHEEL)
        {
            navigation->zoom(event.wheel.y);
            navigation->options().type = vts::NavigationType::Quick;
        }

        // camera jump to double click
        if (event.type == SDL_MOUSEBUTTONDOWN
                && event.button.clicks == 2
                && event.button.button == SDL_BUTTON_LEFT)
        {
            vts::vec3 posPhys = getWorldPositionFromCursor();
            if (!std::isnan(posPhys(0)))
            {
                double posNav[3];
                map->convert(posPhys.data(), posNav,
                             vts::Srs::Physical, vts::Srs::Navigation);
                navigation->setPoint(posNav);
                navigation->options().type
                                = vts::NavigationType::Quick;
            }
        }

        // camera panning or rotating
        if (event.type == SDL_MOUSEMOTION)
        {
            int mode = 0;
            if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                const Uint8 *keys = SDL_GetKeyboardState(nullptr);
                if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL]
                    || keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
                    mode = 2;
                else
                    mode = 1;
            }
            else if ((event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT))
                     || (event.motion.state & SDL_BUTTON(SDL_BUTTON_MIDDLE)))
                mode = 2;
            double p[3] = { (double)event.motion.xrel,
                            (double)event.motion.yrel, 0 };
            switch (mode)
            {
            case 1:
                navigation->pan(p);
                navigation->options().type
                    = vts::NavigationType::Quick;
                break;
            case 2:
                navigation->rotate(p);
                navigation->options().type
                    = vts::NavigationType::Quick;
                break;
            }
        }
    }
    gui.inputEnd();
    return false;
}

void MainWindow::updateWindowSize()
{
    vts::renderer::RenderOptions &ro = view->options();
    SDL_GL_GetDrawableSize(window, (int*)&ro.width, (int*)&ro.height);
    ro.targetViewportW = ro.width;
    ro.targetViewportH = ro.height;
    ro.width *= appOptions.oversampleRender;
    ro.height *= appOptions.oversampleRender;
    camera->setViewportSize(ro.width, ro.height);
}

void MainWindow::run()
{
    if (appOptions.purgeDiskCache)
        map->purgeDiskCache();

    updateWindowSize();

    setMapConfigPath(appOptions.paths[0]);
    gui.initialize(this);
    if (appOptions.screenshotOnFullRender)
        gui.visible(false);

    if (!appOptions.initialPosition.empty())
    {
        map->callbacks().mapconfigAvailable = [&](){
            vts::log(vts::LogLevel::info2,
                     "Setting initial position");
            try
            {
                navigation->setPosition(
                    vts::Position(appOptions.initialPosition));
                navigation->options().type
                        = vts::NavigationType::Instant;
            }
            catch (...)
            {
                vts::log(vts::LogLevel::warn3,
                         "Failed to set initial position");
            }
            map->callbacks().mapconfigAvailable = {};
        };
    }

    bool shouldClose = false;
    auto lastTime = std::chrono::high_resolution_clock::now();
    double accumulatedTime = 0;
    while (!shouldClose)
    {
        OPTICK_FRAME("frame");
        auto time1 = std::chrono::high_resolution_clock::now();
        try
        {
            updateWindowSize();
            map->renderUpdate(timingTotalFrame);
            camera->renderUpdate();
        }
        catch (const vts::MapconfigException &e)
        {
            std::stringstream s;
            s << "Exception <" << e.what() << ">";
            vts::log(vts::LogLevel::err4, s.str());
            if (appOptions.paths.size() > 1)
                setMapConfigPath(MapPaths());
            else
                throw;
        }

        auto time2 = std::chrono::high_resolution_clock::now();
        shouldClose = processEvents();
        prepareMarks();
        renderFrame();
        bool renderCompleted = map->getMapRenderComplete();
        if (appOptions.screenshotOnFullRender && renderCompleted)
        {
            appOptions.screenshotOnFullRender = false;
            makeScreenshot();
            gui.visible(true);
        }
        if (appOptions.closeOnFullRender && renderCompleted)
            shouldClose = true;
        if (map->statistics().renderTicks % 120 == 0)
        {
            std::string creditLine = std::string() + "vts-browser-desktop: "
                    + camera->credits().textFull();
            SDL_SetWindowTitle(window, creditLine.c_str());
        }

        auto time3 = std::chrono::high_resolution_clock::now();
        windowSwap(window);

        // simulated fps slowdown
        switch (appOptions.simulatedFpsSlowdown)
        {
        case 1:
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            break;
        case 2:
            std::this_thread::sleep_for(std::chrono::milliseconds(
                std::sin(accumulatedTime * 0.1) < 0 ? 50 : 0));
            break;
        }

        auto time4 = std::chrono::high_resolution_clock::now();
        timingMapProcess = std::chrono::duration<double>(
            time2 - time1).count();
        timingAppProcess = std::chrono::duration<double>(
            time3 - time2).count();
        timingTotalFrame = std::chrono::duration<double>(
            time4 - lastTime).count();
        lastTime = time4;
        accumulatedTime += timingTotalFrame;

        timingMapSmooth.add(timingMapProcess);
        timingFrameSmooth.add(timingTotalFrame);
    }

    // closing the whole app may take some time (waiting on pending downloads)
    //   therefore we hide the window here so that the user
    //   does not get disturbed by it
    SDL_HideWindow(window);

    gui.finalize();
    map->renderFinalize();
}

void MainWindow::colorizeMarks()
{
    if (marks.empty())
        return;
    float mul = 1.0f / marks.size();
    int index = 0;
    for (Mark &m : marks)
        m.color = vts::convertHsvToRgb(vts::vec3f(index++ * mul, 1, 1));
}

vts::vec3 MainWindow::getWorldPositionFromCursor()
{
    if (!map->getMapconfigAvailable())
        return vts::nan3();
    int xx, yy;
    SDL_GetMouseState(&xx, &yy);
    double screenPos[2] = { (double)xx, (double)yy };
    vts::vec3 result;
    view->getWorldPosition(screenPos, result.data());
    return result;
}

void MainWindow::setMapConfigPath(const MapPaths &paths)
{
    map->setMapconfigPath(paths.mapConfig, paths.auth);
}

