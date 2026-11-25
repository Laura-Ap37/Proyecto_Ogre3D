#ifndef __Laberinto__Game__h__
#define __Laberinto__Game__h__

#include <Ogre.h>

class Game
{
public:
  enum class State { PLAYING, LOST };

  State getState( ) const { return( this->m_State ); }
  bool updatePlayerContact(
    const Ogre::Vector3& playerFeet, Ogre::Real playerHeight,
    Ogre::Real playerRadius, const Ogre::AxisAlignedBox& enemyBounds,
    bool enemyStunned
    );
  void reset( ) { this->m_State = State::PLAYING; }

protected:
  State m_State { State::PLAYING };
};

#endif // __Laberinto__Game__h__
