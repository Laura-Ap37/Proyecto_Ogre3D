#include "EnemyPresentation.h"
#include <algorithm>

// -------------------------------------------------------------------------
void EnemyPresentation::
build(
  Ogre::SceneNode* node, const Maze& maze, Maze::Cell spawn, Ogre::Real speed
  )
{
  this->m_Node = node;
  this->m_Maze = &maze;
  this->m_CurrentCell = spawn;
  this->m_Destination = spawn;
  this->m_Speed = speed;
  this->m_Height = node->getPosition( ).y;
}

// -------------------------------------------------------------------------
void EnemyPresentation::
update( Maze::Cell destination, Ogre::Real dt )
{
  if( this->m_Node == nullptr || this->m_Maze == nullptr )
    return;

  if( destination != this->m_CurrentCell )
    this->m_Destination = destination;
  else if( this->m_Destination == this->m_CurrentCell )
    return;

  Ogre::Vector3 target = this->m_Maze->cellToWorld( this->m_Destination );
  target.y = this->m_Height;
  Ogre::Vector3 position = this->m_Node->getPosition( );
  Ogre::Vector3 direction = target - position;
  direction.y = 0;
  const Ogre::Real distance = direction.length( );
  const Ogre::Real step = this->m_Speed * std::max( Ogre::Real( 0 ), dt );

  if( distance <= step || distance <= Ogre::Real( 1e-4 ) )
  {
    this->m_Node->setPosition( target );
    this->m_CurrentCell = this->m_Destination;
    this->m_Destination = this->m_CurrentCell;
    return;
  }

  direction /= distance;
  position += direction * step;
  position.y = this->m_Height;
  this->m_Node->setPosition( position );
  this->m_Node->setOrientation(
    Ogre::Vector3::NEGATIVE_UNIT_Z.getRotationTo( direction )
    );
}
