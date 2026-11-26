#ifndef __Laberinto__EnemyPresentation__h__
#define __Laberinto__EnemyPresentation__h__

#include "Maze.h"

class EnemyPresentation
{
public:
  void build(
    Ogre::SceneNode* node, const Maze& maze, Maze::Cell spawn,
    Ogre::Real speed
    );
  void update( Maze::Cell destination, Ogre::Real dt, bool paused = false );
  void reset( Maze::Cell spawn );

  Maze::Cell getCurrentCell( ) const { return( this->m_CurrentCell ); }
  Ogre::Vector3 getWorldPosition( ) const;
  Ogre::AxisAlignedBox getWorldBounds( ) const;

protected:
  Ogre::SceneNode* m_Node { nullptr };
  const Maze* m_Maze { nullptr };
  Maze::Cell m_CurrentCell;
  Maze::Cell m_Destination;
  Ogre::Real m_Speed { 2.0 };
  Ogre::Real m_Height { 0.8 };
};

#endif // __Laberinto__EnemyPresentation__h__
