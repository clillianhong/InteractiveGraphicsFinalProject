#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <string>
#include <iostream>
#include <assimp/scene.h>
#include <RTUtil/conversions.hpp>

class Utils
{
public:
    static void print(const Eigen::Vector3f vec, std::string name)
    {
        std::cout << name << ": (" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")\n";
    }

    static void print(const Eigen::Affine3f mat, std::string name)
    {
        std::cout << name << "\n";
        std::cout << "\t[ " << mat(0, 0) << ", " << mat(0, 1) << ", " << mat(0, 2) << ", " << mat(0, 3) << std::endl;
        std::cout << "\t" << mat(1, 0) << ", " << mat(1, 1) << ", " << mat(1, 2) << ", " << mat(1, 3) << std::endl;
        std::cout << "\t" << mat(2, 0) << ", " << mat(2, 1) << ", " << mat(2, 2) << ", " << mat(2, 3) << std::endl;
        std::cout << "\t" << mat(3, 0) << ", " << mat(3, 1) << ", " << mat(3, 2) << ", " << mat(3, 3) << " ]" << std::endl;
    }

    static Eigen::Affine3f collectNodeToWorldTransform(const aiScene *scene, const aiString &nodeName)
    {
        aiNode *node = scene->mRootNode->FindNode(nodeName);

        Eigen::Affine3f accumeTransform = Eigen::Affine3f::Identity();

        for (auto currNode = node; currNode != nullptr; currNode = currNode->mParent)
        {
            accumeTransform = RTUtil::a2e(currNode->mTransformation) * accumeTransform; //TODO multiplication between 4x4 and affine3f could yield bad result
        }

        return accumeTransform;
    }

    static Eigen::Vector3f calculateClosestOnPointRay(Eigen::Vector3f dir, Eigen::Vector3f rayPoint, Eigen::Vector3f point)
    {
        Eigen::Vector3f pointRay = point - rayPoint;
        auto t = pointRay.dot(dir) / (dir.dot(dir));

        return rayPoint + t * dir;
    }
};
