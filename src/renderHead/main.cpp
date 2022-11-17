#include <vsg/all.h>

#ifdef vsgXchange_FOUND
#include <vsgXchange/all.h>
#endif

#include <iostream>


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
        /*
         vsg::ComputeBounds computeBounds;
    scene->accept(computeBounds);
    vsg::dvec3 centre       = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    double     radius       = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
    double     nearFarRatio = 0.0001;
        */
        auto  centre = vsg::dvec3();
        float radius = 10.0;
        auto  lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, -radius * 3.5, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));

        vsg::ref_ptr<vsg::ProjectionMatrix> perspective;
        perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), /*nearfar ratio*/0.00001 * radius, radius * 4.5);

        camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));
        // add close handler to respond to pressing the window close window button and pressing escape
        viewer->addEventHandler(vsg::CloseHandler::create(viewer));
        // add a trackball event handler to control the camera view use the mouse
        viewer->addEventHandler(vsg::Trackball::create(camera));
        
    }

    void createDebugScene()
    {
        auto builder     = vsg::Builder::create();
        builder->options = options;
        sceneRoot->addChild(builder->createCone());
    }

    void firstFrame() {
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