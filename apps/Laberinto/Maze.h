#ifndef __Laberinto__Maze__h__
#define __Laberinto__Maze__h__

#include <string>
#include <vector>
#include <Ogre.h>

class Maze
{
public:
  Maze( ) = default;

  // Lee la grilla desde un archivo de texto. false si falla.
  bool load( const std::string& file );

  // Crea piso y paredes como hijos de 'parent'
  void build( Ogre::SceneManager* mgr, Ogre::SceneNode* parent );

  // Celda (fila, columna) -> centro de esa celda en el mundo
  Ogre::Vector3 cellToWorld( int row, int col ) const;

  Ogre::Vector3 getStart( ) const { return( this->cellToWorld( m_StartRow, m_StartCol ) ); }
  Ogre::Vector3 getExit( ) const  { return( this->cellToWorld( m_ExitRow, m_ExitCol ) ); }
  Ogre::Real getCellSize( ) const { return( m_CellSize ); }
  bool isWall( int row, int col ) const;
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