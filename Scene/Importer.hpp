#pragma once

#ifndef Importer_hpp
#define Importer_hpp
#include <string>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <RTUtil/geomtools.hpp>

/* [currently unused]

 */
struct Vertex
{
    float x, y, z, r;
};
struct Triangle
{
    int v0, v1, v2;
};
struct AppData
{
    // RTCScene g_scene;

    RTUtil::Vector3 *face_colors;
    RTUtil::Vector3 *vertex_colors;
};

class Importer
{
public:
    const aiScene *importFromFile(const std::string &pFile);

private:
    // An instance of the Importer class
    Assimp::Importer importer;
};

#endif
