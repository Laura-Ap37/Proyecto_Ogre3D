#ifndef __Laberinto__ThrowablePresentation__h__
#define __Laberinto__ThrowablePresentation__h__

#include "Throwable.h"

class ThrowablePresentation
{
public:
  void build( Ogre::SceneNode* node, Throwable& throwable );
  void update(
    const Throwable& throwable, const Ogre::Vector3& playerPosition,
    const Ogre::Vector3& aimDirection
    );

protected:
  Ogre::SceneNode* m_Node { nullptr };
};

#endif // __Laberinto__ThrowablePresentation__h__
