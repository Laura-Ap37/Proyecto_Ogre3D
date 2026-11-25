#ifndef __Laberinto__Enemy__h__
#define __Laberinto__Enemy__h__

#include "Maze.h"
#include <cstddef>
#include <vector>

class Enemy
{
public:
  enum class State { PATROL, CHASE, SEARCH, STUNNED };

  void build(
    const Maze& maze, Maze::Cell spawn, Ogre::Real stunDuration = 15.0,
    Ogre::Real stunRadius = 1.5, Ogre::Real startDelay = 30.0
    );
  Maze::Cell update(
    Ogre::Real dt, const Ogre::Vector3& playerPosition, Maze::Cell currentCell
    );
  void stun( Ogre::Real duration = -1.0 );

  State getState( ) const { return( this->m_State ); }
  Ogre::Real getStunDuration( ) const { return( this->m_StunDuration ); }
  Ogre::Real getStunRadius( ) const { return( this->m_StunRadius ); }
  Ogre::Real getStartDelayRemaining( ) const { return( this->m_StartDelayTimer ); }
  bool isWaitingToStart( ) const { return( this->m_StartDelayTimer > 0 ); }

protected:
  Maze::Cell _nextPatrolWaypoint( Maze::Cell current );
  Maze::Cell _investigationWaypoint( Maze::Cell origin ) const;

protected:
  const Maze* m_Maze { nullptr };
  std::vector< Maze::Cell > m_Waypoints;
  State m_State { State::PATROL };
  Maze::Cell m_TargetCell;
  Maze::Cell m_LastKnownPlayerCell;
  Maze::Cell m_PreviousCell;
  Maze::Cell m_LastCell;
  size_t m_PatrolIndex { 0 };
  Ogre::Real m_DetectionRange { 12 };
  Ogre::Real m_SearchDuration { 5 };
  Ogre::Real m_SearchTimer { 0 };
  Ogre::Real m_StunDuration { 15 };
  Ogre::Real m_StunRadius { 1.5 };
  Ogre::Real m_StunTimer { 0 };
  Ogre::Real m_StartDelayTimer { 30 };
  State m_ResumeState { State::PATROL };
  bool m_HasCurrentCell { false };
  bool m_Investigating { false };
};

#endif // __Laberinto__Enemy__h__