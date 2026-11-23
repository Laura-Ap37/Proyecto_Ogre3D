#include "Maze.h"
#include <algorithm>
#include <fstream>

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