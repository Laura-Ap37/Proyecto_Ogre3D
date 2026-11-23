// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __SolarSystem__Model__h__
#define __SolarSystem__Model__h__

#include <memory>
#include <Ogre.h>
#include <OgreBullet.h>

/**
 */
class Model
{
public:
  using Self = Model;

public:
  Model( );
  virtual ~Model( );

  void toggle_simulation( );

  btDynamicsWorld* getBtWorld( ) const
    {
      return( this->m_DynWorld? this->m_DynWorld->getBtWorld( ): nullptr );
    }
  
  void build( Ogre::SceneNode* root );
  void update( double t );

  void shoot (Ogre::SceneManager* mgr, const Ogre::Vector3& pos, const Ogre::Vector3& dir);

protected:
  void _configure( Ogre::Entity* e, const Ogre::UserObjectBindings& b );

protected:
  std::unique_ptr< Ogre::Bullet::DynamicsWorld > m_DynWorld;
  // std::unique_ptr< Bullet::DebugDrawer > m_DbgDraw;

  bool m_Simulating { false };
};

#endif // __SolarSystem__Model__h__

// eof - Model.h
