#ifndef __Laberinto__Player__h__
#define __Laberinto__Player__h__

#include <memory>
#include <Ogre.h>
#include <OgreInput.h>
#include <OgreBullet.h>

/**
 * Jugador: cuerpo fisico (capsula de Bullet) + camara en primera persona.
 * Los parametros salen del userData del nodo "Player" en el .scenepp.
 */
class Player
  : public OgreBites::InputListener
{
public:
  Player( ) = default;
  virtual ~Player( ) override;

  // 'node' es el nodo "Player" del XML. 'feet' es donde quedan los pies.
  void build( btDynamicsWorld* world, Ogre::SceneNode* node,
              const Ogre::Vector3& feet );

  // Llamar una vez por frame, DESPUES de avanzar la simulacion
  void update( );

  // Para el enemigo (pasos siguientes)
  Ogre::Vector3 getFeetPosition( ) const;
  Ogre::Vector3 getLookDirection( ) const;

  // Interaccion
  virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;
  virtual bool keyReleased( const OgreBites::KeyboardEvent& evt ) override;
  virtual bool mouseMoved( const OgreBites::MouseMotionEvent& evt ) override;

protected:
  bool _setKey( OgreBites::Keycode key, bool down );

protected:
  btDynamicsWorld* m_World { nullptr };
  Ogre::SceneNode* m_Node  { nullptr };

  // El orden importa: se destruyen al reves (cuerpo primero)
  std::unique_ptr< btCapsuleShape > m_Shape;
  std::unique_ptr< btDefaultMotionState > m_State;
  std::unique_ptr< btRigidBody > m_Body;

  Ogre::Real m_Height    { 1.7 };
  Ogre::Real m_EyeHeight { 1.6 };
  Ogre::Real m_Speed     { 3.0 };
  Ogre::Real m_MouseSensitivity { 0.15 };

  Ogre::Radian m_Yaw { 0 }, m_Pitch { 0 };
  bool m_Forward { false }, m_Back { false }, m_Left { false }, m_Right { false };

  struct Tag { const void* entity; void* listener; };
  Tag m_Tag { nullptr, nullptr };
};

#endif // __Laberinto__Player__h__