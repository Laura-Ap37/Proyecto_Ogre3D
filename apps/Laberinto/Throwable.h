#ifndef __Laberinto__Throwable__h__
#define __Laberinto__Throwable__h__

#include <memory>
#include <Ogre.h>
#include <OgreBullet.h>

class Throwable
{
public:
  enum class State { IDLE_HELD, IN_FLIGHT, INACTIVE };

  ~Throwable( );

  void build( btDynamicsWorld* world );
  void configure(
    Ogre::Real speed, Ogre::Real lifetime, Ogre::Real radius,
    Ogre::Real cooldown, const Ogre::Vector3& gravity
    );
  void setIdlePosition( const Ogre::Vector3& position );
  bool throwFrom( const Ogre::Vector3& origin, const Ogre::Vector3& direction );
  void update( Ogre::Real dt );
  void deactivate( );
  void reset( );

  State getState( ) const { return( this->m_State ); }
  const Ogre::Vector3& getPosition( ) const { return( this->m_Position ); }
  const Ogre::Vector3& getPreviousPosition( ) const
    { return( this->m_PreviousPosition ); }
  Ogre::Real getRadius( ) const { return( this->m_Radius ); }
  Ogre::Real getCooldownRemaining( ) const
    { return( this->m_CooldownRemaining ); }
  Ogre::Real getCooldownDuration( ) const { return( this->m_Cooldown ); }
  Ogre::Vector3 closestPointTo( const Ogre::Vector3& point ) const;
  bool intersects( const Ogre::Vector3& point, Ogre::Real radius ) const;

protected:
  void _removeBody( );

protected:
  State m_State { State::IDLE_HELD };
  Ogre::Vector3 m_Position { Ogre::Vector3::ZERO };
  Ogre::Vector3 m_PreviousPosition { Ogre::Vector3::ZERO };
  Ogre::Vector3 m_Gravity { 0, -9.81, 0 };
  btDynamicsWorld* m_World { nullptr };
  std::unique_ptr< btSphereShape > m_Shape;
  std::unique_ptr< btDefaultMotionState > m_MotionState;
  std::unique_ptr< btRigidBody > m_Body;
  Ogre::Real m_Speed { 12 };
  Ogre::Real m_Lifetime { 5 };
  Ogre::Real m_Cooldown { 60 };
  Ogre::Real m_CooldownRemaining { 0 };
  Ogre::Real m_Elapsed { 0 };
  Ogre::Real m_Radius { 0.15 };
};

#endif // __Laberinto__Throwable__h__
