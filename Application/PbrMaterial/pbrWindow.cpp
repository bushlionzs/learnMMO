#include "stdafx.h"
#include <pbrWindow.h>
#include "PbrMaterial.h"

PBRWindow::PBRWindow(PbrMaterial* self)
{
    mPbrMaterial = self;
    CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
    
    mRoot = (CEGUI::DefaultWindow*)winMgr.createWindow("DefaultWindow", "Root");
    auto* guiContext = CEGUIManager::getSingleton().getGUIContext();
    guiContext->setRootWindow(mRoot);

    auto* main = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("forge.xml");

    mRoot->addChild(main);

    CEGUI::Combobox* RenderMode = (CEGUI::Combobox*)main->getChildRecursive("RenderMode");

    CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem("Shaded", (CEGUI::uint)0);
    RenderMode->addItem(item);
    RenderMode->setItemSelectState(item, true);

    item = new CEGUI::ListboxTextItem("Albedo", (CEGUI::uint)1);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("Normals", (CEGUI::uint)2);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("Roughness", (CEGUI::uint)3);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("Metallic", (CEGUI::uint)4);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("AO", (CEGUI::uint)5);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("n", (CEGUI::uint)6);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("F(l,h)", (CEGUI::uint)7);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("G(l,v,h)", (CEGUI::uint)8);
    RenderMode->addItem(item);

    item = new CEGUI::ListboxTextItem("D(h)", (CEGUI::uint)9);
    RenderMode->addItem(item);

    

    RenderMode->subscribeEvent(
        CEGUI::Combobox::EventListSelectionAccepted,
        CEGUI::Event::Subscriber(&PBRWindow::handleItemSelected, this));

    CEGUI::Combobox* MaterialType = (CEGUI::Combobox*)main->getChildRecursive("MaterialType");

    item = new CEGUI::ListboxTextItem("Metal", (CEGUI::uint)0);
    MaterialType->addItem(item);
    MaterialType->setItemSelectState(item, true);

    item = new CEGUI::ListboxTextItem("Wood", (CEGUI::uint)1);
    MaterialType->addItem(item);
    

    MaterialType->subscribeEvent(
        CEGUI::Combobox::EventListSelectionAccepted,
        CEGUI::Event::Subscriber(&PBRWindow::handleMaterialTypeSelected, this));

    CEGUI::Scrollbar* x = (CEGUI::Scrollbar*)main->getChildRecursive("LightDirectionX");
    CEGUI::Scrollbar* y = (CEGUI::Scrollbar*)main->getChildRecursive("LightDirectionY");
    CEGUI::Scrollbar* z = (CEGUI::Scrollbar*)main->getChildRecursive("LightDirectionZ");


    x->setID(0);
    y->setID(1);
    z->setID(2);

    mLightDirection = Ogre::Vector3(0.74, 0.643, 0.198);

    mPbrMaterial->updateLightDirection(mLightDirection);
    x->setScrollPosition(mLightDirection.x);
    y->setScrollPosition(mLightDirection.y);
    z->setScrollPosition(mLightDirection.z);

    x->subscribeEvent(
        CEGUI::Scrollbar::EventScrollPositionChanged,
        CEGUI::Event::Subscriber(&PBRWindow::handleLightDirectionChanged, this));

    y->subscribeEvent(
        CEGUI::Scrollbar::EventScrollPositionChanged,
        CEGUI::Event::Subscriber(&PBRWindow::handleLightDirectionChanged, this));

    z->subscribeEvent(
        CEGUI::Scrollbar::EventScrollPositionChanged,
        CEGUI::Event::Subscriber(&PBRWindow::handleLightDirectionChanged, this));
}

PBRWindow::~PBRWindow()
{

}

bool PBRWindow::handleItemSelected(const CEGUI::EventArgs& args)
{
    CEGUI::Combobox* RenderMode = (CEGUI::Combobox*)(((CEGUI::WindowEventArgs&)args).window);
    CEGUI::ListboxItem* item = RenderMode->getSelectedItem();
    
    auto id = item->getID();

    mPbrMaterial->updateRenderMode(id);

    return true;
}

bool PBRWindow::handleMaterialTypeSelected(const CEGUI::EventArgs& args)
{
    CEGUI::Combobox* RenderMode = (CEGUI::Combobox*)(((CEGUI::WindowEventArgs&)args).window);
    CEGUI::ListboxItem* item = RenderMode->getSelectedItem();

    auto id = item->getID();

    mPbrMaterial->updateMaterialType(id);

    return true;
}

bool PBRWindow::handleLightDirectionChanged(const CEGUI::EventArgs& args)
{
    CEGUI::Scrollbar* scrollBar = (CEGUI::Scrollbar*)(((CEGUI::WindowEventArgs&)args).window);
    auto id = scrollBar->getID();

    float v = scrollBar->getScrollPosition();

    mLightDirection[id] = v;

    mPbrMaterial->updateLightDirection(mLightDirection);
    return true;
}