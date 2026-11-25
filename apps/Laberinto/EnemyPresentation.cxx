#include "EnemyPresentation.h"
#include <algorithm>

// -------------------------------------------------------------------------
void EnemyPresentation::
build(
  Ogre::SceneNode* node, const Maze& maze, Maze::Cell spawn, Ogre::Real speed
  )
{
  this->m_Node = node;
  this->m_SceneMgr = ( node != nullptr ) ? node->getCreator( ) : nullptr;
  this->m_Maze = &maze;
  this->m_CurrentCell = spawn;
  this->m_Destination = spawn;
  this->m_Speed = speed;
  this->m_Height = ( node != nullptr ) ? node->getPosition( ).y : Ogre::Real( 0.8 );

  this->_buildVisuals( );
}

// -------------------------------------------------------------------------
void EnemyPresentation::
_buildVisuals( )
{
  if( this->m_Node == nullptr || this->m_SceneMgr == nullptr )
    return;

  if( this->m_Node->numChildren( ) > 0 )
    return;

  const Ogre::Real s = ( this->m_Maze != nullptr )
    ? ( this->m_Maze->getCellSize( ) * Ogre::Real( 0.005 ) )
    : Ogre::Real( 0.01 );

  auto addPart = [ this ](
    const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::String& mat
    )
  {
    Ogre::SceneNode* partNode = this->m_Node->createChildSceneNode( );
    partNode->setPosition( pos );
    partNode->setScale( scale );

    Ogre::Entity* ent = this->m_SceneMgr->createEntity( Ogre::SceneManager::PT_CUBE );
    ent->setMaterialName( mat );
    partNode->attachObject( ent );
  };

  // Cuerpo alargado / torso oscuro
  addPart(
    Ogre::Vector3( 0.0f, 0.15f, 0.0f ),
    Ogre::Vector3( 0.55f * s, 1.35f * s, 0.45f * s ),
    "Wall"
    );

  // Cabeza ligeramente adelantada hacia -Z
  addPart(
    Ogre::Vector3( 0.0f, 0.95f, -0.10f ),
    Ogre::Vector3( 0.42f * s, 0.42f * s, 0.42f * s ),
    "Wall"
    );

  // Ojos rojos brillantes en el frente (-Z)
  addPart(
    Ogre::Vector3( -0.11f, 1.00f, -0.32f ),
    Ogre::Vector3( 0.10f * s, 0.08f * s, 0.06f * s ),
    "MonsterPlaceholder"
    );
  addPart(
    Ogre::Vector3( 0.11f, 1.00f, -0.32f ),
    Ogre::Vector3( 0.10f * s, 0.08f * s, 0.06f * s ),
    "MonsterPlaceholder"
    );

  // Brazos largos a los lados
  addPart(
    Ogre::Vector3( 0.42f, 0.25f, -0.12f ),
    Ogre::Vector3( 0.16f * s, 0.95f * s, 0.22f * s ),
    "MonsterPlaceholder"
    );
  addPart(
    Ogre::Vector3( -0.42f, 0.25f, -0.12f ),
    Ogre::Vector3( 0.16f * s, 0.95f * s, 0.22f * s ),
    "MonsterPlaceholder"
    );
}

// -------------------------------------------------------------------------
void EnemyPresentation::
update( Maze::Cell destination, Ogre::Real dt, bool paused )
{
  if( this->m_Node == nullptr || this->m_Maze == nullptr )
    return;

  if( paused )
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

// -------------------------------------------------------------------------
Ogre::Vector3 EnemyPresentation::
getWorldPosition( ) const
{
  return(
    this->m_Node != nullptr
    ? this->m_Node->_getDerivedPosition( )
    : Ogre::Vector3::ZERO
    );
}