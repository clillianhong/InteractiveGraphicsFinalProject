#include "Importer.hpp"
#include <assimp/postprocess.h> // Post processing flags
#include <stdio.h>
#include <iostream>

// // Reference http://sir-kimmi.de/assimp/lib_html/usage.html#access_cpp

const aiScene *Importer::importFromFile(const std::string &pFile)
{

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    // importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
    const aiScene *scene = importer.ReadFile(pFile,
                                            //  aiProcess_RemoveComponent | 
                                             aiProcess_CalcTangentSpace |
                                                 aiProcess_Triangulate |
                                                 aiProcess_JoinIdenticalVertices |
                                                 aiProcess_SortByPType |
                                                 aiProcess_GenSmoothNormals);

    // If the import failed, report it
    if (!scene)
    {
        std::cout << importer.GetErrorString();
    }

    return scene;
}
