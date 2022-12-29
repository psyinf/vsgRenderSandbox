#include <vsg/all.h>

#ifdef vsgXchange_FOUND
#include <vsgXchange/all.h>
#endif

#include <future>
#include <iostream>
#include <kafka/KafkaConsumer.h>
#include <nlohmann/json.hpp>
#include <random>
#include <ranges>
using namespace kafka;
using namespace kafka::clients;
using namespace kafka::clients::consumer;

/**
 * Class to aggregate the scene-root, camera and auxiliary items.
 * Long term idea is to have an interface to create, update and delete scene items via external drivers.
 */
const Properties props({
    {"bootstrap.servers", {"127.0.0.1:9092"}},
    {"enable.idempotence", {"true"}},
});
class UpdateStream
{
public:
    const std::string topic = "test";

    KafkaConsumer consumer = KafkaConsumer(props);
    bool          debug    = true;

    std::vector<ConsumerRecord> back_buffer;

    UpdateStream()
    {
        // Subscribe to topics
        consumer.subscribe({topic});
    }
    void swap(std::vector<ConsumerRecord>& target)
    {
        std::swap(target, back_buffer);
    }


    void run()
    {
        while (true)
        {
            auto records = consumer.poll(std::chrono::milliseconds(10));
            for (const auto& record : records)
            {
                // TODO: use dedicated message type here
                // In this example, quit on empty message
                if (record.value().size() == 0)
                {
                    std::cerr << "Quit due to empty message" << std::endl;
                    return;
                }


                if (!record.error())
                {
                    if (debug)
                    {
                        std::cout << "% Got a new message..." << std::endl;
                        std::cout << "    Topic    : " << record.topic() << std::endl;
                        std::cout << "    Partition: " << record.partition() << std::endl;
                        std::cout << "    Offset   : " << record.offset() << std::endl;
                        std::cout << "    Timestamp: " << record.timestamp().toString() << std::endl;
                        std::cout << "    Headers  : " << toString(record.headers()) << std::endl;
                        std::cout << "    Key   [" << record.key().toString() << "]" << std::endl;
                        std::cout << "    Value [" << record.value().toString() << "]" << std::endl;
                    }
                    back_buffer.emplace_back(record);
                }
                else if (record.error())
                {
                    std::cerr << record.toString() << std::endl;
                }
            }
        }
    }
};


class SceneObject : public vsg::Inherit<vsg::MatrixTransform, SceneObject>
{

public:
    SceneObject() = default;

    void update(vsg::vec3 pos)
    {
        position     = pos;
        this->matrix = vsg::translate(position);
    }

private:
    vsg::vec3 position;
};
class UpdateScene : public vsg::Inherit<vsg::Visitor, UpdateScene>
{
    



public:
    //TODO: compile dynamically
    vsg::ref_ptr<vsg::Node> cone;

    UpdateScene(vsg::ref_ptr<vsg::Group> root)
        : root(root)
    {
    
       
        
    }

    void apply(vsg::Object& object) override
    {
        object.traverse(*this);
    }

    void apply(vsg::MatrixTransform& mt) override
    {
        auto so = mt.cast<SceneObject>();
        if (so)
        {
            //    so->update();
        }
    }

    void apply(vsg::FrameEvent& frame)
    {

        std::vector<ConsumerRecord> frame_records;

        us.swap(frame_records);
        std::cout << frame_records.size() << std::endl;
        //
        for (const auto& record : frame_records)
        {
            std::string    object_key = (record.key().toString());
            nlohmann::json j          = nlohmann::json::parse(record.value().toString());

            // extract spatial
            auto      x = j["spatial"]["pos"];
            vsg::vec3 v(x[0], x[1], x[2]);

            // check if we need to create it
            if (!objects.contains(object_key))
            {
                auto new_object = SceneObject::create();
                // gizmo


                new_object->addChild(cone);
                // TODO: determine root
                auto parent = object_key.substr(0, object_key.find_last_of("."));
                vsg::ref_ptr<vsg::Group> attachment_point;
                if (parent == object_key)
                {
                    attachment_point = root;
                }
                else
                {
                    attachment_point = objects.at(parent);
                }
                attachment_point->addChild(new_object);

                objects.insert({object_key, new_object});
            }
            // update
            objects.at(object_key)->update(v);

            // TODO: remove the need to use toString
        }


        if (root)

            root->accept(*this);
    }

private:
    vsg::ref_ptr<vsg::Group>                         root;
    UpdateStream                                     us;
    std::jthread                                     updateThread = std::jthread{&UpdateStream::run, &us};
    std::map<std::string, vsg::ref_ptr<SceneObject>> objects;
};

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
        perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), /*nearfar ratio*/ 0.00001 * radius, radius * 4.5);

        camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));
        // add close handler to respond to pressing the window close window button and pressing escape
        viewer->addEventHandler(vsg::CloseHandler::create(viewer));
        // add a trackball event handler to control the camera view use the mouse
        viewer->addEventHandler(vsg::Trackball::create(camera));

        auto builder     = vsg::Builder::create();
        builder->options = options;
        cone             = builder->createCone();
    }

    void createDebugScene()
    {
        auto obj = SceneObject::create();
        obj->addChild(cone);
        sceneRoot->addChild(obj);
       
        updater->cone = cone;
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
    vsg::ref_ptr<UpdateScene>  updater   = UpdateScene::create(sceneRoot);
    vsg::ref_ptr<vsg::Node>  cone;
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
