#include "Enemy.h"
#include <algorithm>
#include <cstdlib>
#include <limits>

// -------------------------------------------------------------------------
void Enemy::
build( const Maze& maze, Maze::Cell spawn )
{
  this->m_Maze = &maze;
  this->m_Waypoints = maze.getPatrolWaypoints( );
  this->m_State = State::PATROL;
  this->m_TargetCell = spawn;
  this->m_LastKnownPlayerCell = spawn;
  this->m_PreviousCell = spawn;
  this->m_LastCell = spawn;
  this->m_PatrolIndex = 0;
  this->m_SearchTimer = 0;
  this->m_HasCurrentCell = true;
  this->m_Investigating = false;

  Ogre::Real nearestDistance = std::numeric_limits< Ogre::Real >::max( );
  for( size_t i = 0; i < this->m_Waypoints.size( ); ++i )
  {
    const Ogre::Real dr = Ogre::Real( this->m_Waypoints[ i ].row - spawn.row );
    const Ogre::Real dc = Ogre::Real( this->m_Waypoints[ i ].col - spawn.col );
    const Ogre::Real distance = dr * dr + dc * dc;
    if( distance < nearestDistance )
    {
      nearestDistance = distance;
      this->m_PatrolIndex = i;
    }
  }
}

// -------------------------------------------------------------------------
Maze::Cell Enemy::
update(
  Ogre::Real dt, const Ogre::Vector3& playerPosition, Maze::Cell currentCell
  )
{
  if( this->m_Maze == nullptr || !this->m_Maze->isWalkable( currentCell ) )
    return( currentCell );

  if( this->m_HasCurrentCell && currentCell != this->m_LastCell )
  {
    this->m_PreviousCell = this->m_LastCell;
    this->m_LastCell = currentCell;
  }
  else if( !this->m_HasCurrentCell )
  {
    this->m_LastCell = currentCell;
    this->m_PreviousCell = currentCell;
    this->m_HasCurrentCell = true;
  }

  Maze::Cell playerCell;
  const bool playerOnGrid
    = this->m_Maze->worldToCell( playerPosition, playerCell ) &&
      this->m_Maze->isWalkable( playerCell );
  bool playerVisible = false;
  if( playerOnGrid )
  {
    const Ogre::Real dr = Ogre::Real( currentCell.row - playerCell.row );
    const Ogre::Real dc = Ogre::Real( currentCell.col - playerCell.col );
    const Ogre::Real distanceSquared
      = ( dr * dr + dc * dc ) * this->m_Maze->getCellSize( )
        * this->m_Maze->getCellSize( );
    playerVisible
      = distanceSquared <= this->m_DetectionRange * this->m_DetectionRange &&
        this->m_Maze->hasLineOfSight( currentCell, playerCell );
  }

  if( playerVisible )
  {
    this->m_LastKnownPlayerCell = playerCell;
    if( this->m_State != State::CHASE )
    {
      this->m_State = State::CHASE;
      this->m_Investigating = false;
    }
  }
  else if( this->m_State == State::CHASE )
  {
    this->m_State = State::SEARCH;
    this->m_TargetCell = this->m_LastKnownPlayerCell;
    this->m_SearchTimer = this->m_SearchDuration;
    this->m_Investigating = false;
  }

  if( this->m_State == State::PATROL )
  {
    if( currentCell == this->m_TargetCell )
      this->m_TargetCell = this->_nextPatrolWaypoint( currentCell );
  }
  else if( this->m_State == State::CHASE )
    this->m_TargetCell = this->m_LastKnownPlayerCell;
  else
  {
    this->m_SearchTimer -= std::max( Ogre::Real( 0 ), dt );
    if( this->m_SearchTimer <= 0 )
    {
      this->m_State = State::PATROL;
      this->m_TargetCell = currentCell;
      this->m_Investigating = false;
    }
    else if( currentCell == this->m_TargetCell && !this->m_Investigating )
    {
      this->m_TargetCell
        = this->_investigationWaypoint( this->m_LastKnownPlayerCell );
      this->m_Investigating = true;
    }
  }

  if( this->m_State == State::PATROL && currentCell == this->m_TargetCell )
    this->m_TargetCell = this->_nextPatrolWaypoint( currentCell );

  return( this->m_Maze->getNextCell(
    currentCell, this->m_TargetCell, this->m_PreviousCell
    ) );
}

// -------------------------------------------------------------------------
Maze::Cell Enemy::
_nextPatrolWaypoint( Maze::Cell current )
{
  if( this->m_Waypoints.empty( ) )
    return( current );

  for( size_t attempts = 0; attempts < this->m_Waypoints.size( ); ++attempts )
  {
    this->m_PatrolIndex = ( this->m_PatrolIndex + 1 ) % this->m_Waypoints.size( );
    if( this->m_Waypoints[ this->m_PatrolIndex ] != current )
      return( this->m_Waypoints[ this->m_PatrolIndex ] );
  }
  return( current );
}

// -------------------------------------------------------------------------
Maze::Cell Enemy::
_investigationWaypoint( Maze::Cell origin ) const
{
  Maze::Cell nearest = origin;
  Ogre::Real nearestDistance = std::numeric_limits< Ogre::Real >::max( );
  for( const Maze::Cell& waypoint: this->m_Waypoints )
  {
    const int dr = waypoint.row - origin.row;
    const int dc = waypoint.col - origin.col;
    const int manhattanDistance = std::abs( dr ) + std::abs( dc );
    if( manhattanDistance == 0 || manhattanDistance > 3 )
      continue;

    const Maze::Cell firstStep
      = this->m_Maze->getNextCell( origin, waypoint, origin );
    if( firstStep == origin )
      continue;
    const Ogre::Real distance = Ogre::Real( dr * dr + dc * dc );
    if( distance < nearestDistance )
    {
      nearest = waypoint;
      nearestDistance = distance;
    }
  }
  return( nearest );
}
