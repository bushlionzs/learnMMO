#include "OgreHeader.h"
#include "OgreVertexDeclaration.h"

VertexDeclaration::VertexDeclaration()
{

}

VertexDeclaration::~VertexDeclaration()
{

}

const VertexElement& VertexDeclaration::addElement(int16_t source, int16_t index, int32_t offset, VertexElementType theType,
    VertexElementSemantic semantic)
{
    mElementList.push_back(VertexElement(source, index, offset, theType, semantic));
    if (mElementList.size() > 10)
    {
        int kk = 0;
    }
    return mElementList.back();
}

const VertexDeclaration::VertexElementList& VertexDeclaration::getElementList()
{
    return mElementList;
}

int32_t VertexDeclaration::getVertexSize(int32_t source)
{
    int32_t size = 0;
    for (auto& v : mElementList)
    {
        if (v.getSource() == source)
        {
            size += v.getSize();
        }
    }
    return size;
}

void VertexDeclaration::removeAllElements()
{
    mElementList.clear();
}

const VertexElement* VertexDeclaration::findElementBySemantic(
    VertexElementSemantic sem, unsigned short index)
{
    VertexElementList::const_iterator ei, eiend;
    eiend = mElementList.end();
    for (ei = mElementList.begin(); ei != eiend; ++ei)
    {
        if (ei->getSemantic() == sem && ei->getIndex() == index)
        {
            return &(*ei);
        }
    }

    return nullptr;

}