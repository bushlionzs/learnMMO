#include "OgreHeader.h"
#include <cmath>
#include "game_camera.h"
#include "OgreCamera.h"
#include "OgreSceneManager.h"
#include "OgreSceneNOde.h"

GameCamera::GameCamera(Camera* camera, SceneManager* sceneMgr)
{
    mCamera = camera;
    mSceneMgr = sceneMgr;

    mCameraType = CameraMoveType_FirstPerson;
}

const Ogre::Vector3& GameCamera::getPosition() const
{
    return position;
}

void GameCamera::setDistance(float distance)
{
    mDistance = distance;
    
}

void GameCamera::setHeight(float height)
{
}

void GameCamera::setMoveSpeed(Real speed)
{
    mMoveSpeed = speed;
}

void GameCamera::setRotateSpeed(Real speed)
{
    mRotateSpeed = speed;
}

void GameCamera::updatePosition(const Ogre::Vector3& camPosition)
{
    position = camPosition;
}

void GameCamera::updateRotate(const Ogre::Vector3& camRotate)
{
    rotation = camRotate;
}

void GameCamera::lookAt(
    const Ogre::Vector3& camPosition,
    const Ogre::Vector3& targetPosition,
    const Ogre::Vector3& up)
{
    position = camPosition;
    Ogre::Vector3 lookDir = targetPosition - camPosition;
    lookDir.normalise();
    float y = lookDir.y;
    rotation.x = -asinf(y) ;

    float x = lookDir.x;
    float z = lookDir.z;
    float n = sqrtf((x * x) + (z * z));
    if (n > 0.01f)
    {
        // don't change the Y rotation if we're too close to vertical
        x /= n;
        z /= n;
        rotation.y = atan2f(x, z) ;
    }

    rotation.y *=  180.0f / Ogre::Math::PI;
    rotation.x *= 180.0f / Ogre::Math::PI;
}

void GameCamera::injectMouseWheel(int _absz)
{
    mDistance += _absz * 0.2f;

    if (mDistance <= 50.0f)
    {
        mDistance = 50.0f;
    }

    if (mDistance >= 1200.0f)
    {
        mDistance = 1200.0f;
    }
}
void GameCamera::injectMouseMove(int _absx, int _absy, int _absz)
{
    if (!mPickup)
        return;
    mChanged = true;
    mMouseDeltaX = _absx - mMousePickX;
    mMouseDeltaY = _absy - mMousePickY;

    float pitch = 0.0f;

    bool need = false;

    float yaw = 0.0f;
    if (mMouseDeltaX != 0)
    {
        mYaw -= mMouseDeltaX * 0.15f;
        need = true;

        yaw = mMouseDeltaX * 0.15f;
    }


    if (mMouseDeltaY != 0)
    {
        mPitch -= mMouseDeltaY * 0.15f;
        pitch = mMouseDeltaY * 0.15f;
        need = true;
    }
    rotation += Ogre::Vector3(pitch, -yaw, 0.0f);


    mMousePickX = _absx;
    mMousePickY = _absy;
}

void GameCamera::injectMousePress(int _absx, int _absy, OIS::MouseButtonID _id)
{
    if (_id == OIS::MouseButtonID::MB_Right)
    {
        mMousePickX = _absx;
        mMousePickY = _absy;
        mPickup = true;
    }
}

void GameCamera::injectMouseRelease(int _absx, int _absy, OIS::MouseButtonID _id)
{
    if (_id == OIS::MouseButtonID::MB_Right)
    {
        mPickup = false;
    }
}

void GameCamera::injectKeyPress(KeyCode _key, uint32_t _text)
{
    if (_key == KeyCode::W || _key == KeyCode::ArrowUp)
    {
        mGoingForward = true;
    }
    else if (_key == KeyCode::S || _key == KeyCode::ArrowDown)
    {
        mGoingBack = true;
    }
    else if (_key == KeyCode::A || _key == KeyCode::ArrowLeft)
    {
        mGoingLeft = true;
    }
    else if (_key == KeyCode::D || _key == KeyCode::ArrowRight)
    {
        mGoingRight = true;
    }
    else if (_key == KeyCode::LeftShift)
    {
        mFastMove = true;
    }
    else if (_key == KeyCode::Q)
    {
        mGoingUp = true;
    }
    else if (_key == KeyCode::E)
    {
        mGoingDown = true;
    }
}

void GameCamera::injectKeyRelease(KeyCode _key)
{
    if (_key == KeyCode::W || _key == KeyCode::ArrowUp)
    {
        mGoingForward = false;
    }
    else if (_key == KeyCode::S || _key == KeyCode::ArrowDown)
    {
        mGoingBack = false;
    }
    else if (_key == KeyCode::A || _key == KeyCode::ArrowLeft)
    {
        mGoingLeft = false;
    }
    else if (_key == KeyCode::D || _key == KeyCode::ArrowRight)
    {
        mGoingRight = false;
    }
    else if (_key == KeyCode::LeftShift)
    {
        mFastMove = false;
    }
    else if (_key == KeyCode::Q)
    {
        mGoingUp = false;
    }
    else if (_key == KeyCode::E)
    {
        mGoingDown = false;
    }
}

bool GameCamera::update(float delta)
{
    float x = rotation.x / 180.f * Ogre::Math::PI;
    float y = rotation.y / 180.f * Ogre::Math::PI;

    
    if (mGoingForward || mGoingBack || mGoingLeft || mGoingRight || mGoingUp || mGoingDown)
    {
        mChanged = true;
        auto rot = Ogre::Math::makeRotateMatrixYX(x, y);

        auto right = rot.getRight();
        auto up = rot.getUp();
        auto forward = right.crossProduct(up);

        float moveSpeed = delta * mMoveSpeed;

        Ogre::Vector3 move = Ogre::Vector3::ZERO;

        if (mGoingForward)
        {
            move += forward * moveSpeed;
        }

        if (mGoingBack)
        {
            move -= forward * moveSpeed;
        }

        if (mGoingLeft)
        {
            move -= right * moveSpeed;
        }


        if (mGoingRight)
        {
            move += right * moveSpeed;
        }

        if (mGoingUp)
        {
            move += Ogre::Vector3(0.0f, 1.0f, 0.0f) * moveSpeed;
        }

        if (mGoingDown)
        {
            move -= Ogre::Vector3(0.0f, 1.0f, 0.0f) * moveSpeed;
        }

        position += move;
    }
  
    auto rotM = Ogre::Math::makeRotateMatrixXY(-x, -y);
    
    //Ogre::Vector4 t = rotM * Ogre::Vector4(-position);
    //rotM.setTrans(t.xyz());

    Ogre::Matrix4 m;
    if (mCameraType == CameraMoveType_FirstPerson)
    {
        auto transM = Ogre::Math::makeTranslateMatrix(-position);
        m = rotM * transM;
    }
    else
    {
        auto transM = Ogre::Math::makeTranslateMatrix(position);
        m = transM * rotM;
    }
    
    mCamera->updateViewMatrix(m);
    mCamera->updatePosition(position);
    return true;
}

bool GameCamera::changed()
{
    return mChanged;
}
void GameCamera::updateChanged(bool change)
{
    mChanged = change;
}

Ogre::String GameCamera::getCameraString()
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "x:%d,y:%d,yaw:%f,pitch:%f", 
        mMousePickX, mMousePickY, mYaw, mPitch);
    return Ogre::String(buffer);
}
