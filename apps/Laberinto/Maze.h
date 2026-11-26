#ifndef __Laberinto__Maze__h__
#define __Laberinto__Maze__h__

#include <string>
#include <vector>
#include <Ogre.h>

class Maze
{
public:
  struct Cell
  {
    int row { -1 };
    int col { -1 };

    bool operator==( const Cell& other ) const
      { return( this->row == other.row && this->col == other.col ); }
    bool operator!=( const Cell& other ) const
      { return( !( *this == other ) ); }
  };

public:
  Maze( ) = default;

  // Lee la grilla desde un archivo de texto. false si falla.
  bool load( const std::string& file );

  // Crea piso y paredes como hijos de 'parent'
  void build( Ogre::SceneManager* mgr, Ogre::SceneNode* parent );

  // Celda (fila, columna) -> centro de esa celda en el mundo
  Ogre::Vector3 cellToWorld( int row, int col ) const;
  Ogre::Vector3 cellToWorld( Cell cell ) const
    { return( this->cellToWorld( cell.row, cell.col ) ); }
  bool worldToCell( const Ogre::Vector3& position, Cell& cell ) const;

  Ogre::Vector3 getStart( ) const { return( this->cellToWorld( m_StartRow, m_StartCol ) ); }
  Ogre::Vector3 getExit( ) const  { return( this->cellToWorld( m_ExitRow, m_ExitCol ) ); }
  Ogre::Real getCellSize( ) const { return( m_CellSize ); }
  bool isWall( int row, int col ) const;
  bool isWalkable( Cell cell ) const;
  std::vector< Cell > getWalkableNeighbors( Cell cell ) const;
  Cell getWalkableNeighborToward( Cell cell, const Ogre::Vector3& direction ) const;
  std::vector< Cell > getPatrolWaypoints( ) const;
  Cell getNextCell( Cell current, Cell target, Cell previous ) const;
  bool hasLineOfSight( Cell from, Cell to ) const;
  bool blocksProjectile(
    const Ogre::Vector3& from, const Ogre::Vector3& to, Ogre::Real radius
    ) const;
  int rows( ) const { return( int( m_Grid.size( ) ) ); }
  int cols( ) const { return( m_Grid.empty( )? 0: int( m_Grid[ 0 ].size( ) ) ); }

protected:
  void _addWall( Ogre::SceneManager* mgr, Ogre::SceneNode* parent,
                 int row, int c0, int c1 );

protected:
  std::vector< std::string > m_Grid;
  Ogre::Real m_CellSize   { 2.0 };   // metros por celda
  Ogre::Real m_WallHeight { 3.0 };   // metros
  int m_StartRow { 1 }, m_StartCol { 1 };
  int m_ExitRow  { 1 }, m_ExitCol  { 1 };
};

#endif // __Laberinto__Maze__h__