// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __SolarSystem__Application__h__
#define __SolarSystem__Application__h__

#include <string>
#include <Ogre.h>
#include <OgreApplicationContext.h>
#include "Model.h"
#include "Maze.h"
#include "Player.h"
#include "Enemy.h"
#include "EnemyPresentation.h"
#include "Throwable.h"
#include "ThrowablePresentation.h"
#include "CooldownHud.h"

/**
 */
class Application
  : public OgreBites::ApplicationContext,
    public OgreBites::InputListener
{
public:
  using Self      = Application;
  using TContext  = OgreBites::ApplicationContext;
  using TListener = OgreBites::InputListener;

  enum class GameState { PLAYING, WON, LOST };

public:
  Application( int& ac, char** av );
  virtual ~Application( ) override;

  // Configuration
  virtual void loadResources( ) override;
  virtual void locateResources( ) override;
  virtual void setup( ) override;

  // Interactions
  virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;
  //virtual bool mousePressed( const OgreBites::MouseButtonEvent& evt ) override;

  // Animation
  virtual void frameRendered( const Ogre::FrameEvent& evt ) override;

  // Main method
  void go( );

protected:
  void _loadExtraPlugins( );
  void _initSceneManager( );
  void _configureCamera( );
  void _loadScene( );
  void _setEndMessage( const Ogre::String& text, const Ogre::ColourValue& colour );
  static void _bounds( Ogre::SceneNode* node, Ogre::AxisAlignedBox* box );

protected:
  std::string m_ExecutionDir { "" };
  GameState m_GameState { GameState::PLAYING };
  Model m_Model;
  Maze m_Maze;
  Player m_Player;
  Enemy m_Enemy;
  EnemyPresentation m_EnemyPresentation;
  Throwable m_Throwable;
  ThrowablePresentation m_ThrowablePresentation;
  CooldownHud m_CooldownHud;
};

#endif // __SolarSystem__Application__h__

// eof - Application.h