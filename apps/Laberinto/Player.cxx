#include "Player.h"
#include <algorithm>

// -------------------------------------------------------------------------
// Lee un numero del userData del nodo; si no esta, usa el valor por defecto
static Ogre::Real readReal(
  const Ogre::UserObjectBindings& b, const std::string& key, Ogre::Real def
  )
{
  const Ogre::Any& a = b.getUserAny( key );
  if( a.has_value( ) )
    if( const Ogre::Real* p = Ogre::any_cast< Ogre::Real >( &a ) )
      return( *p );
  return( def );
}

// -------------------------------------------------------------------------
Player::
~Player( )
{
  if( this->m_World != nullptr && this->m_Body )
    this->m_World->removeRigidBody( this->m_Body.get( ) );
}

// -------------------------------------------------------------------------
void Player::
build( btDynamicsWorld* world, Ogre::SceneNode* node, const Ogre::Vector3& feet )
{
  this->m_World = world;
  this->m_Node = node;

  // 1. Parametros desde el XML
  const Ogre::UserObjectBindings& ud = node->getUserObjectBindings( );
  const Ogre::Real mass   = readReal( ud, "mass", 70 );
  const Ogre::Real radius = readReal( ud, "radius", 0.3 );
  this->m_Radius = radius;
  this->m_Height    = readReal( ud, "height", 1.7 );
  this->m_EyeHeight = readReal( ud, "eye_height", 1.6 );
  this->m_Speed     = readReal( ud, "walk_speed", 3 );

  // 2. Forma: capsula. En Bullet el segundo argumento es solo el tramo
  //    cilindrico, sin las dos semiesferas de los extremos.
  this->m_Shape = std::make_unique< btCapsuleShape >(
    radius, this->m_Height - 2 * radius
    );
  btVector3 inertia( 0, 0, 0 );
  this->m_Shape->calculateLocalInertia( mass, inertia );

  // 3. Posicion inicial: el origen de la capsula es su CENTRO, no los pies
  btTransform t;
  t.setIdentity( );
  t.setOrigin( btVector3(
    feet.x, feet.y + this->m_Height / 2 + 0.02, feet.z
    ) );
  this->m_State = std::make_unique< btDefaultMotionState >( t );

  // 4. Cuerpo rigido
  btRigidBody::btRigidBodyConstructionInfo info(
    mass, this->m_State.get( ), this->m_Shape.get( ), inertia
    );
  this->m_Body = std::make_unique< btRigidBody >( info );
  this->m_Body->setAngularFactor( btVector3( 0, 0, 0 ) ); // no se voltea
  this->m_Body->setFriction( 0 );                         // desliza en paredes
  this->m_Body->setDamping( 0, 0 );
  this->m_Body->setActivationState( DISABLE_DEACTIVATION ); // nunca se duerme
    
  this->m_Body->setUserPointer( &this->m_Tag );
  
  this->m_World->addRigidBody( this->m_Body.get( ) );
}

// -------------------------------------------------------------------------
void Player::
update( )
{
  if( !this->m_Body )
    return;

  // 1. Cuerpo -> camara: el nodo va a la altura de los ojos
  btVector3 p = this->m_Body->getWorldTransform( ).getOrigin( );
  this->m_Node->setPosition(
    p.x( ), p.y( ) - this->m_Height / 2 + this->m_EyeHeight, p.z( )
    );
  this->m_Node->setOrientation(
    Ogre::Quaternion( this->m_Yaw, Ogre::Vector3::UNIT_Y )
    *
    Ogre::Quaternion( this->m_Pitch, Ogre::Vector3::UNIT_X )
    );

  // 2. Teclas -> velocidad horizontal. La vertical se deja a la gravedad.
  Ogre::Vector3 local(
    Ogre::Real( this->m_Right - this->m_Left ), 0,
    Ogre::Real( this->m_Back - this->m_Forward )
    );
  Ogre::Vector3 v = Ogre::Vector3::ZERO;
  if( local.squaredLength( ) > 0 )
  {
    local.normalise( );
    v = ( Ogre::Quaternion( this->m_Yaw, Ogre::Vector3::UNIT_Y ) * local )
        * this->m_Speed;
  } // end if
  btVector3 cur = this->m_Body->getLinearVelocity( );
  this->m_Body->setLinearVelocity( btVector3( v.x, cur.y( ), v.z ) );
}

// -------------------------------------------------------------------------
Ogre::Vector3 Player::
getFeetPosition( ) const
{
  btVector3 p = this->m_Body->getWorldTransform( ).getOrigin( );
  return( Ogre::Vector3( p.x( ), p.y( ) - this->m_Height / 2, p.z( ) ) );
}

// -------------------------------------------------------------------------
void Player::
reset( const Ogre::Vector3& feet )
{
  if( this->m_Body == nullptr )
    return;

  btTransform transform;
  transform.setIdentity( );
  transform.setOrigin( btVector3(
    feet.x, feet.y + this->m_Height / 2 + Ogre::Real( 0.02 ), feet.z
    ) );
  this->m_Body->setWorldTransform( transform );
  this->m_Body->setInterpolationWorldTransform( transform );
  this->m_Body->setLinearVelocity( btVector3( 0, 0, 0 ) );
  this->m_Body->setAngularVelocity( btVector3( 0, 0, 0 ) );
  this->m_Body->activate( true );
  this->m_State->setWorldTransform( transform );
  this->m_Yaw = Ogre::Radian( 0 );
  this->m_Pitch = Ogre::Radian( 0 );
  this->m_Forward = this->m_Back = this->m_Left = this->m_Right = false;
  this->m_ThrowableRequested = false;
  this->m_InputEnabled = false;
  this->m_World->updateSingleAabb( this->m_Body.get( ) );
  this->update( );
}

// -------------------------------------------------------------------------
void Player::
setInputEnabled( bool enabled )
{
  this->m_InputEnabled = enabled;
  this->m_Forward = this->m_Back = this->m_Left = this->m_Right = false;
  this->m_ThrowableRequested = false;
  if( !enabled && this->m_Body != nullptr )
  {
    const btVector3 velocity = this->m_Body->getLinearVelocity( );
    this->m_Body->setLinearVelocity( btVector3( 0, velocity.y( ), 0 ) );
  }
}

// -------------------------------------------------------------------------
Ogre::Vector3 Player::
getEyePosition( ) const
{
  return( this->m_Node != nullptr? this->m_Node->_getDerivedPosition( ): Ogre::Vector3::ZERO );
}

// -------------------------------------------------------------------------
Ogre::Vector3 Player::
getLookDirection( ) const
{
  return( this->m_Node->getOrientation( ) * Ogre::Vector3::NEGATIVE_UNIT_Z );
}

// -------------------------------------------------------------------------
bool Player::
consumeThrowableRequest( )
{
  const bool requested = this->m_ThrowableRequested;
  this->m_ThrowableRequested = false;
  return( requested );
}

// -------------------------------------------------------------------------
bool Player::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
  if( !this->m_InputEnabled )
    return( true );
  if( evt.keysym.sym == 'q' )
  {
    this->m_ThrowableRequested = true;
    return( true );
  }
  return( this->_setKey( evt.keysym.sym, true ) );
}

// -------------------------------------------------------------------------
bool Player::
keyReleased( const OgreBites::KeyboardEvent& evt )
{
  if( !this->m_InputEnabled )
    return( true );
  return( this->_setKey( evt.keysym.sym, false ) );
}

// -------------------------------------------------------------------------
bool Player::
mousePressed( const OgreBites::MouseButtonEvent& evt )
{
  if( !this->m_InputEnabled )
    return( true );
  if( evt.button == OgreBites::BUTTON_LEFT )
  {
    this->m_ThrowableRequested = true;
    return( true );
  }
  return( false );
}

// -------------------------------------------------------------------------
bool Player::
mouseMoved( const OgreBites::MouseMotionEvent& evt )
{
  if( !this->m_InputEnabled )
    return( true );
  this->m_Yaw   -= Ogre::Degree( evt.xrel * this->m_MouseSensitivity );
  this->m_Pitch -= Ogre::Degree( evt.yrel * this->m_MouseSensitivity );
  this->m_Pitch = std::clamp(
    this->m_Pitch,
    Ogre::Radian( Ogre::Degree( -89 ) ), Ogre::Radian( Ogre::Degree( 89 ) )
    );
  return( true );
}

// -------------------------------------------------------------------------
bool Player::
_setKey( OgreBites::Keycode key, bool down )
{
  switch( key )
  {
  case 'w': case OgreBites::SDLK_UP:    this->m_Forward = down; return( true );
  case 's': case OgreBites::SDLK_DOWN:  this->m_Back    = down; return( true );
  case 'a': case OgreBites::SDLK_LEFT:  this->m_Left    = down; return( true );
  case 'd': case OgreBites::SDLK_RIGHT: this->m_Right   = down; return( true );
  default: return( false );
  } // end switch
}