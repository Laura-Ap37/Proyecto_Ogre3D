#include "Game.h"
#include <algorithm>

// -------------------------------------------------------------------------
bool Game::
updatePlayerContact(
  const Ogre::Vector3& playerFeet, Ogre::Real playerHeight,
  Ogre::Real playerRadius, const Ogre::AxisAlignedBox& enemyBounds,
  bool enemyStunned
  )
{
  if( this->m_State != State::PLAYING || enemyStunned ||
      enemyBounds.isNull( ) || enemyBounds.isInfinite( ) )
    return( false );

  const Ogre::Vector3 minimum = enemyBounds.getMinimum( );
  const Ogre::Vector3 maximum = enemyBounds.getMaximum( );
  if( playerFeet.y > maximum.y ||
      playerFeet.y + playerHeight < minimum.y )
    return( false );

  const Ogre::Real closestX
    = std::clamp( playerFeet.x, minimum.x, maximum.x );
  const Ogre::Real closestZ
    = std::clamp( playerFeet.z, minimum.z, maximum.z );
  const Ogre::Real dx = playerFeet.x - closestX;
  const Ogre::Real dz = playerFeet.z - closestZ;
  const Ogre::Real safeRadius = std::max( Ogre::Real( 0 ), playerRadius );
  if( dx * dx + dz * dz > safeRadius * safeRadius )
    return( false );

  this->m_State = State::LOST;
  return( true );
}
