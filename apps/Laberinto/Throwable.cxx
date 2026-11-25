#include "Throwable.h"
#include <algorithm>

// -------------------------------------------------------------------------
Throwable::
~Throwable( )
{
  this->_removeBody( );
}

// -------------------------------------------------------------------------
void Throwable::
build( btDynamicsWorld* world )
{
  this->_removeBody( );
  this->m_World = world;
}

// -------------------------------------------------------------------------
void Throwable::
configure(
  Ogre::Real speed, Ogre::Real lifetime, Ogre::Real radius,
  Ogre::Real cooldown, const Ogre::Vector3& gravity
  )
{
  this->m_Speed = std::max( Ogre::Real( 0 ), speed );
  this->m_Lifetime = std::max( Ogre::Real( 0 ), lifetime );
  this->m_Radius = std::max( Ogre::Real( 0 ), radius );
  this->m_Cooldown = std::max( Ogre::Real( 0 ), cooldown );
  this->m_Gravity = gravity;
  this->m_Shape = std::make_unique< btSphereShape >( this->m_Radius );
}

// -------------------------------------------------------------------------
void Throwable::
setIdlePosition( const Ogre::Vector3& position )
{
  if( this->m_State == State::IDLE_HELD )
  {
    this->m_Position = position;
    this->m_PreviousPosition = position;
  }
}

// -------------------------------------------------------------------------
bool Throwable::
throwFrom( const Ogre::Vector3& origin, const Ogre::Vector3& direction )
{
  if( this->m_State != State::IDLE_HELD ||
      this->m_World == nullptr || this->m_Shape == nullptr ||
      direction.squaredLength( ) <= Ogre::Real( 1e-8 ) )
    return( false );

  btTransform transform;
  transform.setIdentity( );
  transform.setOrigin( btVector3( origin.x, origin.y, origin.z ) );
  this->m_MotionState
    = std::make_unique< btDefaultMotionState >( transform );
  const btScalar mass = btScalar( 0.25 );
  btVector3 inertia( 0, 0, 0 );
  this->m_Shape->calculateLocalInertia( mass, inertia );
  btRigidBody::btRigidBodyConstructionInfo info(
    mass, this->m_MotionState.get( ), this->m_Shape.get( ), inertia
    );
  this->m_Body = std::make_unique< btRigidBody >( info );
  this->m_Body->setRestitution( btScalar( 0.35 ) );
  this->m_Body->setFriction( btScalar( 0.45 ) );
  this->m_Body->setDamping( btScalar( 0.02 ), btScalar( 0.02 ) );
  this->m_Body->setGravity(
    btVector3( this->m_Gravity.x, this->m_Gravity.y, this->m_Gravity.z )
    );
  this->m_Body->setActivationState( DISABLE_DEACTIVATION );
  const Ogre::Vector3 velocity = direction.normalisedCopy( ) * this->m_Speed;
  this->m_Body->setLinearVelocity(
    btVector3(
      velocity.x, velocity.y, velocity.z
      )
    );
  this->m_World->addRigidBody( this->m_Body.get( ) );

  this->m_State = State::IN_FLIGHT;
  this->m_Position = origin;
  this->m_PreviousPosition = origin;
  this->m_Elapsed = 0;
  this->m_CooldownRemaining = this->m_Cooldown;
  return( true );
}

// -------------------------------------------------------------------------
void Throwable::
update( Ogre::Real dt )
{
  const Ogre::Real step = std::max( Ogre::Real( 0 ), dt );
  if( this->m_State == State::INACTIVE )
  {
    this->m_CooldownRemaining
      = std::max( Ogre::Real( 0 ), this->m_CooldownRemaining - step );
    if( this->m_CooldownRemaining <= 0 )
      this->m_State = State::IDLE_HELD;
    return;
  }
  if( this->m_State != State::IN_FLIGHT )
    return;

  this->m_CooldownRemaining
    = std::max( Ogre::Real( 0 ), this->m_CooldownRemaining - step );
  this->m_Elapsed += step;
  this->m_PreviousPosition = this->m_Position;
  if( this->m_Body != nullptr )
  {
    const btVector3 position
      = this->m_Body->getWorldTransform( ).getOrigin( );
    this->m_Position = Ogre::Vector3(
      Ogre::Real( position.x( ) ), Ogre::Real( position.y( ) ),
      Ogre::Real( position.z( ) )
      );
  }
  if( this->m_Elapsed >= this->m_Lifetime )
    this->deactivate( );
}

// -------------------------------------------------------------------------
void Throwable::
deactivate( )
{
  if( this->m_State != State::IN_FLIGHT )
    return;
  this->_removeBody( );
  this->m_State = State::INACTIVE;
}

// -------------------------------------------------------------------------
void Throwable::
_removeBody( )
{
  if( this->m_World != nullptr && this->m_Body != nullptr )
    this->m_World->removeRigidBody( this->m_Body.get( ) );
  this->m_Body.reset( );
  this->m_MotionState.reset( );
}

// -------------------------------------------------------------------------
Ogre::Vector3 Throwable::
closestPointTo( const Ogre::Vector3& point ) const
{
  const Ogre::Vector3 segment
    = this->m_Position - this->m_PreviousPosition;
  const Ogre::Real lengthSquared = segment.squaredLength( );
  if( lengthSquared <= Ogre::Real( 1e-8 ) )
    return( this->m_Position );

  const Ogre::Real fraction = std::clamp(
    ( point - this->m_PreviousPosition ).dotProduct( segment ) / lengthSquared,
    Ogre::Real( 0 ), Ogre::Real( 1 )
    );
  return( this->m_PreviousPosition + segment * fraction );
}

// -------------------------------------------------------------------------
bool Throwable::
intersects( const Ogre::Vector3& point, Ogre::Real radius ) const
{
  const Ogre::Real combinedRadius = this->m_Radius + std::max( Ogre::Real( 0 ), radius );
  return(
    this->closestPointTo( point ).squaredDistance( point )
    <= combinedRadius * combinedRadius
    );
}
