#pragma once
#include <vsg/all.h>

/**
 * SceneObject represented as transform in scenegraph
 */
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

