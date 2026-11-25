#include "EnemyPresentation.h"
#include <algorithm>
#include <OgreEntity.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreResourceGroupManager.h>
#include <OgreTextureManager.h>
#include <OgreTextureUnitState.h>
#include <OgreTechnique.h>

namespace
{
  const Ogre::Quaternion kMeshOrientation(
    Ogre::Degree( 180 ), Ogre::Vector3::UNIT_Y
    );
}

// -------------------------------------------------------------------------
void EnemyPresentation::
build(
  Ogre::SceneNode* node, const Maze& maze, Maze::Cell spawn, Ogre::Real speed
  )
{
  if( node == nullptr )
    OGRE_EXCEPT(
      Ogre::Exception::ERR_INVALIDPARAMS,
      "Monster scene node is missing",
      "EnemyPresentation::build"
      );

  this->m_Node = node;
  this->m_Maze = &maze;
  this->m_CurrentCell = spawn;
  this->m_Destination = spawn;
  this->m_Speed = speed;
  this->m_Height = node->getPosition( ).y;
  node->setOrientation( kMeshOrientation );

  const Ogre::String group
    = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
  Ogre::TextureManager::getSingleton( ).load( "oxido.png", group );
  Ogre::MaterialPtr material
    = Ogre::MaterialManager::getSingleton( ).getByName(
      "MonsterPlaceholder", group
      );
  if( !material )
    OGRE_EXCEPT(
      Ogre::Exception::ERR_ITEM_NOT_FOUND,
      "MonsterPlaceholder material was not loaded",
      "EnemyPresentation::build"
      );

  Ogre::Pass* pass = material->getTechnique( 0 )->getPass( 0 );
  pass->setLightingEnabled( false );
  if( pass->getNumTextureUnitStates( ) == 0 )
    pass->createTextureUnitState( "oxido.png" );
  else
    pass->getTextureUnitState( 0 )->setTextureName( "oxido.png" );
  pass->getTextureUnitState( 0 )->setColourOperation( Ogre::LBO_REPLACE );
  material->load( );

  bool entityFound = false;
  for( Ogre::MovableObject* object: node->getAttachedObjects( ) )
    if( auto* entity = dynamic_cast< Ogre::Entity* >( object ) )
    {
      entity->setMaterial( material );
      entityFound = true;
    }

  if( !entityFound )
    OGRE_EXCEPT(
      Ogre::Exception::ERR_ITEM_NOT_FOUND,
      "Monster scene node has no mesh entity",
      "EnemyPresentation::build"
      );
}

// -------------------------------------------------------------------------
void EnemyPresentation::
reset( Maze::Cell spawn )
{
  this->m_CurrentCell = spawn;
  this->m_Destination = spawn;
  if( this->m_Node == nullptr || this->m_Maze == nullptr )
    return;

  Ogre::Vector3 position = this->m_Maze->cellToWorld( spawn );
  position.y = this->m_Height;
  this->m_Node->setPosition( position );
  this->m_Node->setOrientation( kMeshOrientation );
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
    * kMeshOrientation
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

// -------------------------------------------------------------------------
Ogre::AxisAlignedBox EnemyPresentation::
getWorldBounds( ) const
{
  Ogre::AxisAlignedBox bounds;
  if( this->m_Node == nullptr )
    return( bounds );

  for( Ogre::MovableObject* object: this->m_Node->getAttachedObjects( ) )
    bounds.merge( object->getWorldBoundingBox( true ) );
  return( bounds );
}
