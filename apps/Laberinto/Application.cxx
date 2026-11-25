// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include "Application.h"

#include <filesystem>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>

#include <Ogre.h>
#include <OgreOverlaySystem.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayElement.h>

// -------------------------------------------------------------------------
namespace fs = std::filesystem;

// -------------------------------------------------------------------------
static Ogre::Real readSceneReal(
  const Ogre::UserObjectBindings& bindings, const Ogre::String& name,
  Ogre::Real fallback
  )
{
  const Ogre::Any& value = bindings.getUserAny( name );
  if( const Ogre::Real* real = Ogre::any_cast< Ogre::Real >( &value ) )
    return( *real );
  return( fallback );
}

// -------------------------------------------------------------------------
static Ogre::Vector3 readSceneVector(
  const Ogre::UserObjectBindings& bindings, const Ogre::String& name,
  const Ogre::Vector3& fallback
  )
{
  const Ogre::Any& value = bindings.getUserAny( name );
  const Ogre::String* text = Ogre::any_cast< Ogre::String >( &value );
  if( text == nullptr )
    return( fallback );

  Ogre::Vector3 result;
  std::istringstream input( *text );
  if( !( input >> result.x >> result.y >> result.z ) )
    throw std::runtime_error( "Invalid scene vector property: " + name );
  return( result );
}

// -------------------------------------------------------------------------
Application::
Application( int& ac, char** av )
  : TContext( fs::path( av[ 0 ] ).filename( ).stem( ).string( ) ),
    TListener( )
{
  this->m_ExecutionDir
    = fs::canonical( fs::path( av[ 0 ] ).parent_path( ) ).string( );
}

// -------------------------------------------------------------------------
Application::
~Application( )
{
  this->m_CooldownHud.destroy( );
}

// -------------------------------------------------------------------------
void Application::
loadResources( )
{
  this->enableShaderCache( );
  auto rgm = Ogre::ResourceGroupManager::getSingletonPtr( );
  try
  {
    rgm->initialiseAllResourceGroups( );
    rgm->loadResourceGroup( Ogre::RGN_DEFAULT );
  }
  catch( ... )
  {
    // Do nothing
  }
}

// -------------------------------------------------------------------------
void Application::
locateResources( )
{
  // Required by Ogre itself
  const Ogre::String ogreMedia = this->getDefaultMediaDir( );
  auto rgm = Ogre::ResourceGroupManager::getSingletonPtr( );
  rgm->addResourceLocation(
    ogreMedia + "/Main", "FileSystem", Ogre::RGN_INTERNAL
    );
  rgm->addResourceLocation(
    ogreMedia + "/RTShaderLib", "FileSystem", Ogre::RGN_INTERNAL
    );

  std::string media_dir
    = std::filesystem::path( this->m_ExecutionDir ) / "Laberinto_resources";
  rgm->addResourceLocation( media_dir, "FileSystem", Ogre::RGN_DEFAULT );
}

// -------------------------------------------------------------------------
void Application::
setup( )
{
  this->_loadExtraPlugins( );
  this->_initSceneManager( );
  this->_loadScene( );
  this->_configureCamera( );
}

// -------------------------------------------------------------------------
void Application::
go( )
{
  this->initApp( );
  this->getRoot( )->startRendering( );
  this->closeApp( );
}

// -------------------------------------------------------------------------
bool Application::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
  if( evt.keysym.sym == OgreBites::SDLK_ESCAPE )
  {
    this->getRoot( )->queueEndRendering( );
    return( true );
  }
  else
    return( false );
}

// -------------------------------------------------------------------------
void Application::
frameRendered( const Ogre::FrameEvent& evt )
{
  if( this->m_GameState != GameState::PLAYING )
    return;

  this->m_Model.update( evt.timeSinceLastFrame );
  this->m_Player.update( );
  const Maze::Cell nextCell = this->m_Enemy.update(
    evt.timeSinceLastFrame,
    this->m_Player.getFeetPosition( ),
    this->m_EnemyPresentation.getCurrentCell( )
    );
  const bool enemyPaused =
    this->m_Enemy.isWaitingToStart( ) ||
    this->m_Enemy.getState( ) == Enemy::State::STUNNED;
  this->m_EnemyPresentation.update(
    nextCell, evt.timeSinceLastFrame, enemyPaused
    );

  if( this->m_Player.consumeThrowableRequest( ) )
  {
    const Ogre::Vector3 direction = this->m_Player.getLookDirection( );
    this->m_Throwable.throwFrom(
      this->m_Player.getEyePosition( ) + direction * Ogre::Real( 0.7 ),
      direction
      );
  }

  const bool wasInFlight
    = this->m_Throwable.getState( ) == Throwable::State::IN_FLIGHT;
  const Ogre::Vector3 previousPosition = this->m_Throwable.getPosition( );
  this->m_Throwable.update( evt.timeSinceLastFrame );

  if( wasInFlight )
  {
    const Ogre::Vector3 currentPosition = this->m_Throwable.getPosition( );
    const Ogre::Vector3 enemyPosition
      = this->m_EnemyPresentation.getWorldPosition( );
    const Ogre::Vector3 closestPoint = this->m_Throwable.closestPointTo( enemyPosition );
    Maze::Cell throwableCell, enemyCell;
    const bool clearPath
      = this->m_Maze.worldToCell( closestPoint, throwableCell ) &&
        this->m_Maze.worldToCell( enemyPosition, enemyCell ) &&
        this->m_Maze.hasLineOfSight( throwableCell, enemyCell );
    if( clearPath &&
        this->m_Throwable.intersects(
          enemyPosition, this->m_Enemy.getStunRadius( )
          ) )
    {
      this->m_Enemy.stun( Ogre::Real( 15.0 ) );
      this->m_Throwable.deactivate( );
    }
    else if( this->m_Maze.blocksProjectile(
               previousPosition, currentPosition, this->m_Throwable.getRadius( )
               ) )
      this->m_Throwable.deactivate( );
  }

  this->m_ThrowablePresentation.update(
    this->m_Throwable, this->m_Player.getEyePosition( ),
    this->m_Player.getLookDirection( )
    );
  this->m_CooldownHud.update(
    this->m_Throwable.getCooldownRemaining( ),
    this->m_Throwable.getCooldownDuration( )
    );

  // COMPROBACIÓN DE DERROTA Y VICTORIA
  const Ogre::Vector3 playerPos = this->m_Player.getFeetPosition( );
  const Ogre::Vector3 enemyPos  = this->m_EnemyPresentation.getWorldPosition( );
  const Ogre::Vector3 exitPos   = this->m_Maze.getExit( );

  // 1. Derrota: el monstruo te alcanza (en plano XZ)
  const Ogre::Real dxE = playerPos.x - enemyPos.x;
  const Ogre::Real dzE = playerPos.z - enemyPos.z;
  const Ogre::Real killRadius = 1.1f;

  if( this->m_Enemy.getState( ) != Enemy::State::STUNNED &&
      ( dxE * dxE + dzE * dzE ) <= ( killRadius * killRadius ) )
  {
    this->m_GameState = GameState::LOST;
    this->setWindowGrab( false );
    this->_setEndMessage( "PERDISTE", Ogre::ColourValue( 1.0f, 0.15f, 0.15f ) );
    return;
  }

  // 2. Victoria: llegas a la salida 'E' (en plano XZ)
  const Ogre::Real dxX = playerPos.x - exitPos.x;
  const Ogre::Real dzX = playerPos.z - exitPos.z;
  const Ogre::Real winRadius = this->m_Maze.getCellSize( ) * 0.55f;

  if( ( dxX * dxX + dzX * dzX ) <= ( winRadius * winRadius ) )
  {
    this->m_GameState = GameState::WON;
    this->setWindowGrab( false );
    this->_setEndMessage( "GANASTE", Ogre::ColourValue( 0.15f, 1.0f, 0.35f ) );
    return;
  }
}

// -------------------------------------------------------------------------
void Application::
_loadExtraPlugins( )
{
  auto* root = this->getRoot( );
  std::istringstream in( pujOgre_PLUGINS );
  std::string plugin;
  while( std::getline( in, plugin, '|' ) )
  {
    std::string plg_name
      = std::filesystem::path( this->m_ExecutionDir ) / plugin;
    plg_name += pujOgre_SHARED_EXT;
    root->loadPlugin( plg_name );
  } // end while
}

// -------------------------------------------------------------------------
void Application::
_initSceneManager( )
{
  this->TContext::setup( );
  this->addInputListener( this );

  auto* root = this->getRoot( );
  auto mgr = root->createSceneManager( "DefaultSceneManager", "scnMgr" );
  mgr->addRenderQueueListener( this->getOverlaySystem( ) );
  mgr->setAmbientLight( Ogre::ColourValue( 0.15f, 0.15f, 0.15f ) );

  // register our scene with the RTSS
  auto* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr( );
  shadergen->addSceneManager( mgr );
}

// -------------------------------------------------------------------------
void Application::
_configureCamera( )
{
  auto* scnMgr = this->getRoot( )->getSceneManager( "scnMgr" );
  Ogre::Camera* cam = scnMgr->getCameras( ).begin( )->second;
  cam->setAutoAspectRatio( true );

  // El jugador usa el nodo "Player" definido en el XML
  Ogre::SceneNode* node = scnMgr->getSceneNode( "Player" );
  this->m_Player.build(
    this->m_Model.getBtWorld( ), node, this->m_Maze.getStart( )
    );
  this->addInputListener( &this->m_Player );

  this->setWindowGrab( true );
  Ogre::Viewport* vp = this->getRenderWindow( )->addViewport( cam );
  vp->setBackgroundColour( Ogre::ColourValue( 0.5f, 0.5f, 0.52f ) );
}

// -------------------------------------------------------------------------
void Application::
_loadScene( )
{
  auto* mgr = this->getRoot( )->getSceneManager( "scnMgr" );
  auto* root = mgr->getRootSceneNode( );
  root->loadChildren( "fps.scenepp" );

  // 1. Laberinto: grilla de texto -> piso + paredes
  const std::string maze_file
    = ( fs::path( this->m_ExecutionDir ) / "Laberinto_resources" / "maze.txt" ).string( );
  if( !this->m_Maze.load( maze_file ) )
    throw std::runtime_error( "No se pudo leer " + maze_file );
  this->m_Maze.build( mgr, root );

  // 2. Física: crea los cuerpos estáticos de todo lo que tenga userData
  this->m_Model.build( root );

  Ogre::SceneNode* enemyNode = mgr->getSceneNode( "MonsterPlaceholder" );
  const Ogre::Vector3 startPos = this->m_Maze.getStart( );
  Ogre::Vector3 behindPos = startPos + Ogre::Vector3( 0, 0, this->m_Maze.getCellSize( ) * 2.0 );

  Maze::Cell enemySpawn;
  if( !this->m_Maze.worldToCell( behindPos, enemySpawn ) ||
      !this->m_Maze.isWalkable( enemySpawn ) )
  {
    this->m_Maze.worldToCell( startPos, enemySpawn );
  }

  const Ogre::Vector3 spawnWorld = this->m_Maze.cellToWorld( enemySpawn );
  enemyNode->setPosition( spawnWorld.x, enemyNode->getPosition( ).y, spawnWorld.z );
  this->m_EnemyPresentation.build( enemyNode, this->m_Maze, enemySpawn, 1.8 );
  std::cout << "[Enemy] Spawn cell=(" << enemySpawn.row << "," << enemySpawn.col
            << ") pos=" << enemyNode->_getDerivedPosition( ) << std::endl;

  Ogre::SceneNode* throwableNode = mgr->getSceneNode( "ThrowablePlaceholder" );
  const Ogre::UserObjectBindings& enemyData = enemyNode->getUserObjectBindings( );
  this->m_Enemy.build(
    this->m_Maze, enemySpawn,
    Ogre::Real( 15.0 ),
    readSceneReal( enemyData, "stun_radius", 1.5 ),
    Ogre::Real( 30.0 )
    );
  const Ogre::UserObjectBindings& throwableData
    = throwableNode->getUserObjectBindings( );
  this->m_Throwable.build( this->m_Model.getBtWorld( ) );
  const Ogre::Vector3 gravity = readSceneVector(
    root->getUserObjectBindings( ), "gravity", Ogre::Vector3( 0, -9.81, 0 )
    );
  this->m_Throwable.configure(
    readSceneReal( throwableData, "launch_speed", 12 ),
    readSceneReal( throwableData, "lifetime", 5 ),
    readSceneReal( throwableData, "collision_radius", 0.15 ),
    readSceneReal( throwableData, "cooldown", 60 ),
    gravity
    );
  this->m_ThrowablePresentation.build( throwableNode, this->m_Throwable );
  this->m_CooldownHud.build( );
}

// -------------------------------------------------------------------------
void Application::
_bounds( Ogre::SceneNode* node, Ogre::AxisAlignedBox* box )
{
  for( Ogre::MovableObject* obj: node->getAttachedObjects( ) )
  {
    const Ogre::String& type = obj->getMovableType( );
    if( obj->isVisible( ) && type != "Camera" && type != "Light" )
      box->merge( obj->getWorldBoundingBox( true ) );
  } // end for

  for( Ogre::Node* child: node->getChildren( ) )
    Self::_bounds( static_cast< Ogre::SceneNode* >( child ), box );
}

// -------------------------------------------------------------------------
void Application::_setEndMessage( const Ogre::String& text, const Ogre::ColourValue& colour )
{
  auto* overlayMgr = Ogre::OverlayManager::getSingletonPtr( );
  if( !overlayMgr ) return;

  Ogre::Overlay* overlay = overlayMgr->getByName( "EndBannerOverlay" );
  if( !overlay )
  {
    overlay = overlayMgr->create( "EndBannerOverlay" );
    overlay->setZOrder( 500 );

    auto* panel = static_cast< Ogre::OverlayContainer* >(
      overlayMgr->createOverlayElement( "Panel", "EndBannerPanel" )
      );
    panel->setMetricsMode( Ogre::GMM_RELATIVE );
    panel->setPosition( 0.25f, 0.40f );
    panel->setDimensions( 0.50f, 0.20f );
    try
    {
      panel->setMaterialName( "BaseWhiteNoLighting" );
    }
    catch( ... )
    {
      // Ignorar si no existe el material en este entorno
    }

    auto* area = overlayMgr->createOverlayElement( "TextArea", "EndBannerText" );
    area->setMetricsMode( Ogre::GMM_RELATIVE );
    area->setPosition( 0.0f, 0.06f );
    area->setDimensions( 1.0f, 0.10f );
    area->setParameter( "char_height", "0.08" );
    area->setParameter( "alignment", "center" );
    area->setColour( colour );

    panel->addChild( area );
    overlay->add2D( panel );
  }

  if( auto* area = overlayMgr->getOverlayElement( "EndBannerText", false ) )
  {
    area->setCaption( text );
    area->setColour( colour );
  }
  overlay->show( );
}

// eof - Application.cxx