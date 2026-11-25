#include "ThrowablePresentation.h"

// -------------------------------------------------------------------------
void ThrowablePresentation::
build( Ogre::SceneNode* node, Throwable& throwable )
{
  this->m_Node = node;
  if( this->m_Node == nullptr )
    return;

  throwable.setIdlePosition( this->m_Node->getPosition( ) );
  this->m_Node->setVisible(
    throwable.getState( ) != Throwable::State::INACTIVE, true
    );
}

// -------------------------------------------------------------------------
void ThrowablePresentation::
update(
  const Throwable& throwable, const Ogre::Vector3& playerPosition,
  const Ogre::Vector3& aimDirection
  )
{
  if( this->m_Node == nullptr )
    return;

  const Throwable::State state = throwable.getState( );
  this->m_Node->setVisible( state != Throwable::State::INACTIVE, true );
  if( state == Throwable::State::INACTIVE )
    return;

  if( state == Throwable::State::IDLE_HELD )
  {
    Ogre::Vector3 direction = aimDirection;
    if( direction.squaredLength( ) > Ogre::Real( 1e-8 ) )
      direction.normalise( );
    this->m_Node->setPosition(
      playerPosition + direction * Ogre::Real( 0.42 )
      + Ogre::Vector3( 0, Ogre::Real( -0.22 ), 0 )
      );
    this->m_Node->setOrientation( Ogre::Vector3::NEGATIVE_UNIT_Z.getRotationTo( direction ) );
  }
  else
    this->m_Node->setPosition( throwable.getPosition( ) );
}
