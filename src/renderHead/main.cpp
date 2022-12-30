#include <vsg/all.h>

#ifdef vsgXchange_FOUND
#include <vsgXchange/all.h>
#endif


#include "LoadOperation.h"
#include "SceneUpdater.h"

#include <iostream>
#include <ranges>

/**
 * Class to aggregate the scene-root, camera and auxiliary items.
 * Long term idea is to have an interface to create, update and delete scene items via external drivers.
 */


class ViewerCore
{


public:
    void setup()
    {
        options->paths         = {R"(e:\develop\install\vsgRenderSandbox\bin\data\)"};
        options->sharedObjects = vsg::SharedObjects::create();

        auto shaderSet              = vsg::createFlatShadedShaderSet(options);
        auto graphicsPipelineConfig = vsg::GraphicsPipelineConfigurator::create(shaderSet);
        graphicsPipelineConfig->init();

        vsg::ref_ptr<vsg::Window> window(
            vsg::Window::create(vsg::WindowTraits::create()));
        if (!window)
        {
            throw std::runtime_error("Failed to initialize the window");
        }

        viewer->addWindow(window);


        // set up the camera

        auto  centre = vsg::dvec3();
        float radius = 10.0;
        auto  lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, -radius * 3.5, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));

        vsg::ref_ptr<vsg::ProjectionMatrix> perspective;
        perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), /*nearfar ratio*/ 0.00001, 10000.0);

        camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));
        // add close handler to respond to pressing the window close window button and pressing escape
        viewer->addEventHandler(vsg::CloseHandler::create(viewer));
        // add a trackball event handler to control the camera view use the mouse
        viewer->addEventHandler(vsg::Trackball::create(camera));
    }

    void createDebugScene()
    {
        viewer->addEventHandler(updater);
    }

    void firstFrame()
    {
        // create a command graph to render the scene on specified window
        auto commandGraph = vsg::createCommandGraphForView(viewer->windows().front(), camera, sceneRoot);
        viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

        // compile all the the Vulkan objects and transfer data required to render the scene
        viewer->compile();
    }

    void frame()
    {
        // pass any events into EventHandlers assigned to the Viewer
        viewer->handleEvents();

        viewer->update();

        viewer->recordAndSubmit();

        viewer->present();
    }

    void run()
    {

        while (viewer->advanceToNextFrame())
        {
            frame();
        }
    }

private:
    vsg::ref_ptr<vsg::Viewer>  viewer    = vsg::Viewer::create();
    vsg::ref_ptr<vsg::Options> options   = vsg::Options::create();
    vsg::ref_ptr<vsg::Group>   sceneRoot = vsg::Group::create();
    vsg::ref_ptr<UpdateScene>  updater   = UpdateScene::create(sceneRoot, viewer);
    vsg::ref_ptr<vsg::Node>    cone;
    vsg::ref_ptr<vsg::Node>    cube;
    vsg::ref_ptr<vsg::Node>    sphere;
    vsg::ref_ptr<vsg::Camera>  camera;
};


int main(int argc, char** argv)
{
    ViewerCore core;
    core.setup();
    core.createDebugScene();
    core.firstFrame();
    core.run();
}
