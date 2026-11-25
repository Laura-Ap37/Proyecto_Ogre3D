#include "Maze.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <queue>

// -------------------------------------------------------------------------
bool Maze::
load( const std::string& file )
{
  std::ifstream in( file );
  if( !in )
    return( false );

  this->m_Grid.clear( );
  std::string line;
  size_t width = 0;
  while( std::getline( in, line ) )
  {
    if( !line.empty( ) && line.back( ) == '\r' )
      line.pop_back( );
    if( line.empty( ) )
      continue;
    this->m_Grid.push_back( line );
    width = std::max( width, line.size( ) );
  }
  if( this->m_Grid.empty( ) )
    return( false );

  for( int r = 0; r < this->rows( ); ++r )
  {
    this->m_Grid[ r ].resize( width, '#' );
    for( int c = 0; c < int( width ); ++c )
    {
      if( this->m_Grid[ r ][ c ] == 'S' ) { m_StartRow = r; m_StartCol = c; }
      if( this->m_Grid[ r ][ c ] == 'E' ) { m_ExitRow  = r; m_ExitCol  = c; }
    }
  }
  return( true );
}

// -------------------------------------------------------------------------
bool Maze::
isWall( int row, int col ) const
{
  if( row < 0 || row >= this->rows( ) || col < 0 || col >= this->cols( ) )
    return( true );
  return( this->m_Grid[ row ][ col ] == '#' );
}

// -------------------------------------------------------------------------
// Laberinto centrado en el origen. Columna -> eje X, fila -> eje Z.
Ogre::Vector3 Maze::
cellToWorld( int row, int col ) const
{
  Ogre::Real x0 = -( this->cols( ) * this->m_CellSize ) / 2;
  Ogre::Real z0 = -( this->rows( ) * this->m_CellSize ) / 2;
  return( Ogre::Vector3(
    x0 + ( col + 0.5 ) * this->m_CellSize, 0,
    z0 + ( row + 0.5 ) * this->m_CellSize
    ) );
}

// -------------------------------------------------------------------------
bool Maze::
worldToCell( const Ogre::Vector3& position, Cell& cell ) const
{
    if( this->rows( ) == 0 || this->cols( ) == 0 )
      return( false );

    const Ogre::Real x0 = -( this->cols( ) * this->m_CellSize ) / 2;
    const Ogre::Real z0 = -( this->rows( ) * this->m_CellSize ) / 2;
    cell.col = int( std::floor( ( position.x - x0 ) / this->m_CellSize ) );
    cell.row = int( std::floor( ( position.z - z0 ) / this->m_CellSize ) );
    return( cell.row >= 0 && cell.row < this->rows( ) &&
            cell.col >= 0 && cell.col < this->cols( ) );
}

// -------------------------------------------------------------------------
bool Maze::
isWalkable( Cell cell ) const
{
    return( !this->isWall( cell.row, cell.col ) );
}

// -------------------------------------------------------------------------
std::vector< Maze::Cell > Maze::
getWalkableNeighbors( Cell cell ) const
{
    std::vector< Cell > neighbors;
    const Cell candidates[] =
    {
      { cell.row - 1, cell.col }, { cell.row, cell.col - 1 },
      { cell.row, cell.col + 1 }, { cell.row + 1, cell.col }
    };
    for( const Cell& candidate: candidates )
      if( this->isWalkable( candidate ) )
        neighbors.push_back( candidate );
    return( neighbors );
}

// -------------------------------------------------------------------------
Maze::Cell Maze::
getWalkableNeighborToward(
    Cell cell, const Ogre::Vector3& direction
    ) const
{
    Ogre::Vector3 horizontalDirection( direction.x, 0, direction.z );
    if( horizontalDirection.squaredLength( ) <= Ogre::Real( 1e-6 ) )
      return( Cell( ) );
    horizontalDirection.normalise( );

    const Ogre::Vector3 origin = this->cellToWorld( cell );
    Cell best;
    Ogre::Real bestAlignment = -Ogre::Real( 2 );
    for( const Cell& neighbor: this->getWalkableNeighbors( cell ) )
    {
      Ogre::Vector3 offset = this->cellToWorld( neighbor ) - origin;
      offset.normalise( );
      const Ogre::Real alignment = horizontalDirection.dotProduct( offset );
      if( alignment > bestAlignment )
      {
        bestAlignment = alignment;
        best = neighbor;
      }
    }
    return( best );
}

// -------------------------------------------------------------------------
std::vector< Maze::Cell > Maze::
getPatrolWaypoints( ) const
{
    std::vector< Cell > waypoints;
    for( int row = 0; row < this->rows( ); ++row )
      for( int col = 0; col < this->cols( ); ++col )
      {
        const Cell cell { row, col };
        if( this->isWalkable( cell ) &&
            this->getWalkableNeighbors( cell ).size( ) != 2 )
          waypoints.push_back( cell );
      }

    if( waypoints.empty( ) )
      for( int row = 0; row < this->rows( ); ++row )
        for( int col = 0; col < this->cols( ); ++col )
          if( this->isWalkable( { row, col } ) )
            waypoints.push_back( { row, col } );
    return( waypoints );
}

// -------------------------------------------------------------------------
Maze::Cell Maze::
getNextCell( Cell current, Cell target, Cell previous ) const
{
    if( !this->isWalkable( current ) || !this->isWalkable( target ) ||
        current == target )
      return( current );

    std::vector< std::vector< int > > distances(
      this->rows( ), std::vector< int >( this->cols( ), -1 )
      );
    std::queue< Cell > pending;
    distances[ target.row ][ target.col ] = 0;
    pending.push( target );
    while( !pending.empty( ) )
    {
      const Cell cell = pending.front( );
      pending.pop( );
      for( const Cell& neighbor: this->getWalkableNeighbors( cell ) )
        if( distances[ neighbor.row ][ neighbor.col ] < 0 )
        {
          distances[ neighbor.row ][ neighbor.col ]
            = distances[ cell.row ][ cell.col ] + 1;
          pending.push( neighbor );
        }
    }

    const auto neighbors = this->getWalkableNeighbors( current );
    if( neighbors.empty( ) )
      return( current );

    std::vector< Cell > candidates = neighbors;
    if( neighbors.size( ) > 1 )
    {
      candidates.erase(
        std::remove( candidates.begin( ), candidates.end( ), previous ),
        candidates.end( )
        );
      if( candidates.empty( ) )
        candidates = neighbors;
    }

    int bestDistance = std::numeric_limits< int >::max( );
    int bestEuclidean = std::numeric_limits< int >::max( );
    std::vector< Cell > best;
    for( const Cell& neighbor: candidates )
    {
      const int routeDistance = distances[ neighbor.row ][ neighbor.col ];
      if( routeDistance < 0 )
        continue;
      const int rowDelta = neighbor.row - target.row;
      const int colDelta = neighbor.col - target.col;
      const int euclideanDistance = rowDelta * rowDelta + colDelta * colDelta;
      if( routeDistance < bestDistance ||
          ( routeDistance == bestDistance && euclideanDistance < bestEuclidean ) )
      {
        bestDistance = routeDistance;
        bestEuclidean = euclideanDistance;
        best.clear( );
        best.push_back( neighbor );
      }
      else if( routeDistance == bestDistance &&
               euclideanDistance == bestEuclidean )
        best.push_back( neighbor );
    }
    if( best.empty( ) )
      return( current );

    return( best.front( ) );
}

// -------------------------------------------------------------------------
bool Maze::
hasLineOfSight( Cell from, Cell to ) const
{
    if( !this->isWalkable( from ) || !this->isWalkable( to ) )
      return( false );

    int x = from.col, y = from.row;
    const int dx = std::abs( to.col - from.col );
    const int dy = std::abs( to.row - from.row );
    const int sx = ( from.col < to.col )? 1: -1;
    const int sy = ( from.row < to.row )? 1: -1;
    int error = dx - dy;

    while( x != to.col || y != to.row )
    {
      const int twiceError = 2 * error;
      if( twiceError > -dy ) { error -= dy; x += sx; }
      if( twiceError < dx )  { error += dx; y += sy; }
      if( ( x != to.col || y != to.row ) && this->isWall( y, x ) )
        return( false );
    }
    return( true );
}

// -------------------------------------------------------------------------
bool Maze::
blocksProjectile(
  const Ogre::Vector3& from, const Ogre::Vector3& to, Ogre::Real radius
  ) const
{
  const Ogre::Vector3 segment = to - from;
  const Ogre::Real length = segment.length( );
  const Ogre::Real spacing = std::max( Ogre::Real( 0.1 ), this->m_CellSize / 8 );
  const int samples = std::max( 1, int( std::ceil( length / spacing ) ) );
  const Ogre::Real safeRadius = std::max( Ogre::Real( 0 ), radius );

  for( int i = 1; i <= samples; ++i )
  {
    const Ogre::Vector3 point = from + segment * ( Ogre::Real( i ) / samples );
    if( point.y - safeRadius <= 0 ||
        point.y + safeRadius >= this->m_WallHeight )
      return( true );

    const Ogre::Real offsets[] = { -safeRadius, 0, safeRadius };
    for( Ogre::Real dx: offsets )
      for( Ogre::Real dz: offsets )
      {
        Cell cell;
        if( !this->worldToCell(
              point + Ogre::Vector3( dx, 0, dz ), cell
              ) || !this->isWalkable( cell ) )
          return( true );
      }
  }
  return( false );
}

// -------------------------------------------------------------------------
void Maze::
build( Ogre::SceneManager* mgr, Ogre::SceneNode* parent )
{
  // ---- Paredes: las '#' consecutivas de una fila forman un solo bloque
  for( int r = 0; r < this->rows( ); ++r )
  {
    int c = 0;
    while( c < this->cols( ) )
    {
      if( !this->isWall( r, c ) ) { ++c; continue; }
      int c0 = c;
      while( c < this->cols( ) && this->isWall( r, c ) ) ++c;
      this->_addWall( mgr, parent, r, c0, c - 1 );
    }
  }
}

// -------------------------------------------------------------------------
void Maze::
_addWall( Ogre::SceneManager* mgr, Ogre::SceneNode* parent,
          int row, int c0, int c1 )
{
  const Ogre::Real len = ( c1 - c0 + 1 ) * this->m_CellSize;
  Ogre::Vector3 a = this->cellToWorld( row, c0 );
  Ogre::Vector3 b = this->cellToWorld( row, c1 );

  Ogre::SceneNode* node = parent->createChildSceneNode( );
  node->setPosition( ( a + b ) / 2 + Ogre::Vector3( 0, this->m_WallHeight / 2, 0 ) );

  // PT_CUBE es un cubo de 100 unidades de lado: se escala al tamaño deseado
  Ogre::Entity* e = mgr->createEntity( Ogre::SceneManager::PT_CUBE );
  e->setMaterialName( "Wall" );
  node->setScale( len / 100, this->m_WallHeight / 100, this->m_CellSize / 100 );
  node->attachObject( e );

  // Model::build lee esto y crea el cuerpo estático de Bullet
  auto& ud = node->getUserObjectBindings( );
  ud.setUserAny( "mass", Ogre::Any( Ogre::Real( 0 ) ) );
  ud.setUserAny( "collider", Ogre::Any( Ogre::String( "box" ) ) );
}