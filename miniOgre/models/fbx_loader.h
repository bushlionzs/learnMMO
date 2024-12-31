#pragma once
#include "model_loader.h"

class  FbxLoader : public ModelLoader
{
public:
    FbxLoader();
    ~FbxLoader();

    std::shared_ptr<Ogre::Mesh> loadMeshFromFile(std::shared_ptr<Ogre::DataStream>& stream);
};