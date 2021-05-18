#pragma once

#include <vector>

#include <assimp/scene.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <GLWrap/Mesh.hpp>

struct Node
{
    // Node(std::vector<GLWrap::Mesh, std::allocator<GLWrap::Mesh>> meshes, Eigen::Affine3f transformation, Node *parent)
    Node(std::vector<std::shared_ptr<GLWrap::Mesh>> meshes, Eigen::Affine3f transformation, std::shared_ptr<Node> parent)
    {
        for (std::shared_ptr<GLWrap::Mesh> mesh : meshes)
        {
            this->meshes.push_back(mesh);
        }

        // std::cout << "BEFORE" << std::endl;

        this->parent = parent;
        this->transformation = transformation;
    }

    std::vector<std::shared_ptr<GLWrap::Mesh>> meshes;
    std::vector<std::shared_ptr<Node>> children;
    Eigen::Affine3f transformation;
    std::shared_ptr<Node> parent;
};
