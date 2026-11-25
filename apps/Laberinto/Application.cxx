// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include "Application.h"

#include <filesystem>
#include <queue>
#include <sstream>
#include <stdexcept>

#include <Ogre.h>
#include <OgreOverlaySystem.h>

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
/*bool Application::
mousePressed( const OgreBites::MouseButtonEvent& evt )
{
  if( evt.button == OgreBites::BUTTON_LEFT )
  {
    Ogre::Vector3 pos
      =
      this->m_CameraMan->getBodyNode( )->_getDerivedPosition( );
    Ogre::Vector3 dir
      =
      this->m_CameraMan->getBodyNode( )->_getDerivedOrientation( )
      *
      Ogre::Vector3::NEGATIVE_UNIT_Z;

    // TODO: shoot!
    Ogre::SceneNode* head = this->m_CameraMan->getHeadNode( );
    Ogre::Quaternion q = head->_getDerivedOrientation( );
    dir = q * Ogre::Vector3::NEGATIVE_UNIT_Z;
    pos = head->_getDerivedPosition( ) + q * Ogre::Vector3( 0, -0.05, -0.45 );
    this->m_Model.shoot( this->getRoot( )->getSceneManager( "scnMgr" ), pos, dir);
    
    return( true );
  }
  else
    return( false );
}
*/
// -------------------------------------------------------------------------
void Application::
frameRendered( const Ogre::FrameEvent& evt )
{
  this->m_Model.update( evt.timeSinceLastFrame );
  this->m_Player.update( );
  const Maze::Cell nextCell = this->m_Enemy.update(
    evt.timeSinceLastFrame,
    this->m_Player.getFeetPosition( ),
    this->m_EnemyPresentation.getCurrentCell( )
    );
  this->m_EnemyPresentation.update(
    nextCell, evt.timeSinceLastFrame,
    this->m_Enemy.getState( ) == Enemy::State::STUNNED
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
      this->m_Enemy.stun( );
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
  mgr->setAmbientLight( Ogre::ColourValue( 0.9, 0.9, 0.9 ) );

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
  Maze::Cell enemySpawn;
  if( !this->m_Maze.worldToCell( enemyNode->getPosition( ), enemySpawn ) ||
      !this->m_Maze.isWalkable( enemySpawn ) )
    throw std::runtime_error( "MonsterPlaceholder must start on a walkable maze cell" );
  this->m_EnemyPresentation.build( enemyNode, this->m_Maze, enemySpawn, 1.8 );

  Ogre::SceneNode* throwableNode = mgr->getSceneNode( "ThrowablePlaceholder" );
  const Ogre::UserObjectBindings& enemyData = enemyNode->getUserObjectBindings( );
  this->m_Enemy.build(
    this->m_Maze, enemySpawn,
    readSceneReal( enemyData, "stun_duration", 30 ),
    readSceneReal( enemyData, "stun_radius", 1.5 )
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

// eof - Application.cxx
