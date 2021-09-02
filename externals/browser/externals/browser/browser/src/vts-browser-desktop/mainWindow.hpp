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

#ifndef MAINWINDOW_H_wuiegfzbn
#define MAINWINDOW_H_wuiegfzbn

#include "smoothVariable.hpp"

#include <vts-browser/log.hpp>
#include <vts-browser/math.hpp>
#include <vts-browser/map.hpp>
#include <vts-browser/camera.hpp>
#include <vts-browser/navigation.hpp>
#include <vts-browser/mapOptions.hpp>
#include <vts-browser/cameraOptions.hpp>
#include <vts-browser/navigationOptions.hpp>
#include <vts-browser/resources.hpp>
#include <vts-renderer/renderer.hpp>
#include <vts-renderer/classes.hpp>

namespace vts
{

class Map;

} // namespace vts

struct Mark
{
    vts::vec3 coord;
    vts::vec3f color;
    int open;

    Mark();
};

struct MapPaths
{
    std::string mapConfig;
    std::string auth;
};

struct AppOptions
{
    std::vector<MapPaths> paths;
    std::string initialPosition;
    uint32 oversampleRender;
    int renderCompas;
    int simulatedFpsSlowdown;
    bool screenshotOnFullRender;
    bool closeOnFullRender;
    bool purgeDiskCache;

    AppOptions();
};

class MainWindow
{
public:
    MainWindow(struct SDL_Window *window, void *renderContext,
        vts::Map *map, vts::Camera *camera, vts::Navigation *navigation,
        const AppOptions &appOptions,
        const vts::renderer::RenderOptions &renderOptions);
    ~MainWindow();

    class Gui
    {
    public:
        void initialize(MainWindow *window);
        void finalize();
        void render(int width, int height);
        void inputBegin();
        bool input(union SDL_Event &event);
        void inputEnd();
        void visible(bool visible);
    private:
        std::shared_ptr<class GuiImpl> impl;
    } gui;

    void colorizeMarks();
    vts::vec3 getWorldPositionFromCursor();

    void run();
    void prepareMarks();
    void renderFrame();
    bool processEvents();
    void updateWindowSize();
    void makeScreenshot();

    void setMapConfigPath(const MapPaths &paths);

    AppOptions appOptions;
    vts::renderer::RenderContext context;
    std::shared_ptr<vts::renderer::RenderView> view;
    std::shared_ptr<vts::renderer::Mesh> meshSphere;
    std::shared_ptr<vts::renderer::Mesh> meshLine;
    std::vector<Mark> marks;
    smoothVariable<double, 60> timingMapSmooth;
    smoothVariable<double, 60> timingFrameSmooth;
    double timingMapProcess;
    double timingAppProcess;
    double timingTotalFrame;
    vts::Map *const map;
    vts::Camera *const camera;
    vts::Navigation *const navigation;
    struct SDL_Window *window;
    void *renderContext;
};

#endif
