#pragma once
#include "LoadOperation.h"
#include "SceneObject.h"
#include "UpdateStream.h"

#include <nlohmann/json.hpp>
#include <thread>
#include <vsg/all.h>

class UpdateScene : public vsg::Inherit<vsg::Visitor, UpdateScene>
{
public:
    UpdateScene(vsg::ref_ptr<vsg::Group> root, vsg::ref_ptr<vsg::Viewer> viewer)
        : root(root)
        , viewer(viewer)
        , threads(vsg::OperationThreads::create(4, viewer->status))
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

    void apply(vsg::FrameEvent& frame) override
    {
        std::vector<ConsumerRecord> frame_records;

        us.swap(frame_records);
        //std::cout << frame_records.size() << std::endl;
        //
        for (const auto& record : frame_records)
        {
            std::string    object_key = (record.key().toString());
            nlohmann::json j          = nlohmann::json::parse(record.value().toString());

            // extract spatial
            auto      x    = j["spatial"]["pos"];
            auto      type = j["type"];
            vsg::vec3 v(x[0], x[1], x[2]);

            // check if we need to create it
            if (!objects.contains(object_key))
            {
                auto new_object = SceneObject::create();
                // gizmo
                auto builder     = vsg::Builder::create();
                builder->options = {};

                if (type == "cone")
                {
                    threads->add(CompileOperation::create(viewer, new_object, builder->createCone()));
                }
                else if (type == "cube")
                {
                    threads->add(CompileOperation::create(viewer, new_object, builder->createBox()));
                }
                else if (type == "sphere")
                {
                    threads->add(CompileOperation::create(viewer, new_object, builder->createSphere()));
                }
                else
                {
                    threads->add(CompileOperation::create(viewer, new_object, builder->createCylinder()));
                }

                auto                     parent = object_key.substr(0, object_key.find_last_of("."));
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
        }


        if (root)

            root->accept(*this);
    }

private:
    vsg::ref_ptr<vsg::Group>                         root;
    vsg::observer_ptr<vsg::Viewer>                   viewer;
    vsg::ref_ptr<vsg::OperationThreads>              threads;
    UpdateStream                                     us;
    std::jthread                                     updateThread = std::jthread{&UpdateStream::run, &us};
    std::map<std::string, vsg::ref_ptr<SceneObject>> objects;
};
