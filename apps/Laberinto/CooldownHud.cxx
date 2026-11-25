#include "CooldownHud.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
  const Ogre::Real kLeft = Ogre::Real( 0.916 );
  const Ogre::Real kTop = Ogre::Real( 0.91 );
  const Ogre::Real kDigitWidth = Ogre::Real( 0.027 );
  const Ogre::Real kDigitHeight = Ogre::Real( 0.048 );
  const Ogre::Real kThickness = Ogre::Real( 0.0045 );
  const Ogre::Real kGap = Ogre::Real( 0.007 );
  const Ogre::Real kBarLeft = Ogre::Real( 0.86 );
  const Ogre::Real kBarTop = Ogre::Real( 0.968 );
  const Ogre::Real kBarWidth = Ogre::Real( 0.12 );
  const Ogre::Real kBarHeight = Ogre::Real( 0.007 );

  const bool kDigitSegments[ 10 ][ 7 ] =
  {
    { true, true, true, true, true, true, false },
    { false, true, true, false, false, false, false },
    { true, true, false, true, true, false, true },
    { true, true, true, true, false, false, true },
    { false, true, true, false, false, true, true },
    { true, false, true, true, false, true, true },
    { true, false, true, true, true, true, true },
    { true, true, true, false, false, false, false },
    { true, true, true, true, true, true, true },
    { true, true, true, true, false, true, true }
  };
}

// -------------------------------------------------------------------------
void CooldownHud::
build( )
{
  if( this->m_Overlay != nullptr )
    return;

  this->m_Manager = Ogre::OverlayManager::getSingletonPtr( );
  if( this->m_Manager == nullptr )
    OGRE_EXCEPT(
      Ogre::Exception::ERR_INVALID_STATE,
      "Ogre overlay manager is not initialized",
      "CooldownHud::build"
      );

  this->m_Overlay = this->m_Manager->create( "Laberinto/CooldownHud" );
  this->m_Overlay->setZOrder( 500 );

  const Ogre::Real digitXs[ 2 ] =
  {
    kLeft, kLeft + kDigitWidth + kGap
  };
  for( size_t digit = 0; digit < 2; ++digit )
  {
    const Ogre::Real x = digitXs[ digit ];
    const Ogre::Real y = kTop;
    const Ogre::Real h = kDigitHeight;
    const Ogre::Real t = kThickness;
    const Ogre::Real middleY = y + h / 2 - t / 2;
    this->m_Digits[ digit ][ 0 ] = this->_createPanel(
      "CooldownDigit" + Ogre::StringConverter::toString( digit ) + "Top",
      x + t, y, kDigitWidth - 2 * t, t
      );
    this->m_Digits[ digit ][ 1 ] = this->_createPanel(
      "CooldownDigit" + Ogre::StringConverter::toString( digit ) + "UpperRight",
      x + kDigitWidth - t, y + t, t, h / 2 - t
      );
    this->m_Digits[ digit ][ 2 ] = this->_createPanel(
      "CooldownDigit" + Ogre::StringConverter::toString( digit ) + "LowerRight",
      x + kDigitWidth - t, middleY + t, t, h / 2 - t
      );
    this->m_Digits[ digit ][ 3 ] = this->_createPanel(
      "CooldownDigit" + Ogre::StringConverter::toString( digit ) + "Bottom",
      x + t, y + h - t, kDigitWidth - 2 * t, t
      );
    this->m_Digits[ digit ][ 4 ] = this->_createPanel(
      "CooldownDigit" + Ogre::StringConverter::toString( digit ) + "LowerLeft",
      x, middleY + t, t, h / 2 - t
      );
    this->m_Digits[ digit ][ 5 ] = this->_createPanel(
      "CooldownDigit" + Ogre::StringConverter::toString( digit ) + "UpperLeft",
      x, y + t, t, h / 2 - t
      );
    this->m_Digits[ digit ][ 6 ] = this->_createPanel(
      "CooldownDigit" + Ogre::StringConverter::toString( digit ) + "Middle",
      x + t, middleY, kDigitWidth - 2 * t, t
      );
  }

  this->m_BarBackground = this->_createPanel(
    "CooldownBarBackground", kBarLeft, kBarTop, kBarWidth, kBarHeight
    );
  this->m_BarBackground->setColour( Ogre::ColourValue( 0.16, 0.16, 0.18, 0.85 ) );
  this->m_BarFill = this->_createPanel(
    "CooldownBarFill", kBarLeft, kBarTop, kBarWidth, kBarHeight
    );
  this->m_Overlay->show( );
}

// -------------------------------------------------------------------------
void CooldownHud::
update( Ogre::Real remaining, Ogre::Real duration )
{
  if( this->m_Overlay == nullptr )
    return;

  const Ogre::Real safeDuration = std::max( Ogre::Real( 0 ), duration );
  const Ogre::Real safeRemaining = std::clamp(
    remaining, Ogre::Real( 0 ), safeDuration
    );
  const bool ready = safeRemaining <= 0;
  const Ogre::ColourValue colour = ready
    ? Ogre::ColourValue( 0.15, 0.95, 0.55, 1 )
    : Ogre::ColourValue( 1, 0.62, 0.16, 1 );
  const int seconds = int( std::ceil( safeRemaining ) );
  if( seconds != this->m_LastSeconds )
  {
    this->_setDigit( 0, seconds / 10, colour );
    this->_setDigit( 1, seconds % 10, colour );
    this->m_LastSeconds = seconds;
  }
  else
  {
    for( auto& digit: this->m_Digits )
      for( auto* segment: digit )
        segment->setColour( colour );
  }

  const Ogre::Real fraction
    = ( safeDuration > 0 )? safeRemaining / safeDuration: 0;
  this->m_BarFill->setWidth( kBarWidth * fraction );
  this->m_BarFill->setColour( colour );
}

// -------------------------------------------------------------------------
void CooldownHud::
destroy( )
{
  if( this->m_Manager == nullptr || this->m_Overlay == nullptr )
    return;
  if( Ogre::OverlayManager::getSingletonPtr( ) != this->m_Manager )
  {
    this->m_Manager = nullptr;
    this->m_Overlay = nullptr;
    return;
  }

  this->m_Overlay->clear( );
  for( auto& digit: this->m_Digits )
    for( auto*& segment: digit )
    {
      if( segment != nullptr )
        this->m_Manager->destroyOverlayElement( segment );
      segment = nullptr;
    }
  if( this->m_BarBackground != nullptr )
    this->m_Manager->destroyOverlayElement( this->m_BarBackground );
  if( this->m_BarFill != nullptr )
    this->m_Manager->destroyOverlayElement( this->m_BarFill );
  this->m_BarBackground = nullptr;
  this->m_BarFill = nullptr;
  this->m_Manager->destroy( this->m_Overlay );
  this->m_Overlay = nullptr;
  this->m_Manager = nullptr;
}

// -------------------------------------------------------------------------
Ogre::PanelOverlayElement* CooldownHud::
_createPanel(
  const Ogre::String& name, Ogre::Real left, Ogre::Real top,
  Ogre::Real width, Ogre::Real height
  )
{
  auto* panel = static_cast< Ogre::PanelOverlayElement* >(
    this->m_Manager->createOverlayElement( "Panel", name )
    );
  panel->setMaterialName( "BaseWhite" );
  panel->setMetricsMode( Ogre::GMM_RELATIVE );
  panel->setPosition( left, top );
  panel->setDimensions( width, height );
  panel->setColour( Ogre::ColourValue::White );
  this->m_Overlay->add2D( panel );
  return( panel );
}

// -------------------------------------------------------------------------
void CooldownHud::
_setDigit( size_t index, int digit, const Ogre::ColourValue& colour )
{
  digit = std::clamp( digit, 0, 9 );
  for( size_t segment = 0; segment < 7; ++segment )
  {
    this->m_Digits[ index ][ segment ]->setVisible(
      kDigitSegments[ digit ][ segment ]
      );
    this->m_Digits[ index ][ segment ]->setColour( colour );
  }
}
