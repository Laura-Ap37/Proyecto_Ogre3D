#include "LoseScreen.h"
#include "OverlayMaterial.h"
#include <array>
#include <cctype>

namespace
{
  struct Glyph
  {
    char character;
    std::array< const char*, 7 > rows;
  };

  const Glyph kGlyphs[] =
  {
    { 'A', { "01110", "10001", "10001", "11111", "10001", "10001", "10001" } },
    { 'E', { "11111", "10000", "10000", "11110", "10000", "10000", "11111" } },
    { 'I', { "11111", "00100", "00100", "00100", "00100", "00100", "11111" } },
    { 'L', { "10000", "10000", "10000", "10000", "10000", "10000", "11111" } },
    { 'O', { "01110", "10001", "10001", "10001", "10001", "10001", "01110" } },
    { 'P', { "11110", "10001", "10001", "11110", "10000", "10000", "10000" } },
    { 'R', { "11110", "10001", "10001", "11110", "10100", "10010", "10001" } },
    { 'S', { "01111", "10000", "10000", "01110", "00001", "00001", "11110" } },
    { 'T', { "11111", "00100", "00100", "00100", "00100", "00100", "00100" } },
    { 'U', { "10001", "10001", "10001", "10001", "10001", "10001", "01110" } },
    { 'Y', { "10001", "10001", "01010", "00100", "00100", "00100", "00100" } }
  };
}

// -------------------------------------------------------------------------
void LoseScreen::
build( )
{
  if( this->m_Overlay != nullptr )
    return;

  this->m_Manager = Ogre::OverlayManager::getSingletonPtr( );
  if( this->m_Manager == nullptr )
    OGRE_EXCEPT(
      Ogre::Exception::ERR_INVALID_STATE,
      "Ogre overlay manager is not initialized",
      "LoseScreen::build"
      );

  this->m_Overlay = this->m_Manager->create( "Laberinto/LoseScreen" );
  this->m_Overlay->setZOrder( 600 );
  this->_panel(
    "LoseScreenBackdrop", 0, 0, 1, 1,
    Ogre::ColourValue( 0, 0, 0, 0.72 )
    );
  this->_panel(
    "LoseScreenPanel", Ogre::Real( 0.22 ), Ogre::Real( 0.2 ),
    Ogre::Real( 0.56 ), Ogre::Real( 0.6 ),
    Ogre::ColourValue( 0.025, 0.018, 0.03, 0.97 )
    );
  this->_drawText(
    "YOU LOSE", Ogre::Real( 0.355 ), Ogre::Real( 0.32 ),
    Ogre::Real( 0.009 ), Ogre::Real( 0.018 ),
    Ogre::ColourValue( 0.92, 0.12, 0.16, 1 )
    );
  this->_panel(
    "ReplayButton", Ogre::Real( 0.34 ), Ogre::Real( 0.4 ),
    Ogre::Real( 0.32 ), Ogre::Real( 0.09 ),
    Ogre::ColourValue( 0.25, 0.07, 0.08, 1 )
    );
  this->_drawText(
    "REPLAY", Ogre::Real( 0.405 ), Ogre::Real( 0.433 ),
    Ogre::Real( 0.007 ), Ogre::Real( 0.012 ),
    Ogre::ColourValue( 1, 0.88, 0.82, 1 )
    );
  this->_panel(
    "ExitButton", Ogre::Real( 0.34 ), Ogre::Real( 0.52 ),
    Ogre::Real( 0.32 ), Ogre::Real( 0.09 ),
    Ogre::ColourValue( 0.12, 0.12, 0.15, 1 )
    );
  this->_drawText(
    "EXIT", Ogre::Real( 0.426 ), Ogre::Real( 0.553 ),
    Ogre::Real( 0.007 ), Ogre::Real( 0.012 ),
    Ogre::ColourValue( 0.86, 0.87, 0.92, 1 )
    );
  this->m_Overlay->hide( );
}

// -------------------------------------------------------------------------
void LoseScreen::
show( )
{
  if( this->m_Overlay != nullptr )
  {
    this->m_Visible = true;
    this->m_Overlay->show( );
  }
}

// -------------------------------------------------------------------------
void LoseScreen::
hide( )
{
  if( this->m_Overlay != nullptr )
  {
    this->m_Visible = false;
    this->m_Overlay->hide( );
  }
}

// -------------------------------------------------------------------------
LoseScreen::Action LoseScreen::
mousePressed(
  const OgreBites::MouseButtonEvent& evt,
  Ogre::Real windowWidth, Ogre::Real windowHeight
  ) const
{
  if( !this->m_Visible || evt.button != OgreBites::BUTTON_LEFT ||
      windowWidth <= 0 || windowHeight <= 0 )
    return( Action::NONE );

  const Ogre::Real x = Ogre::Real( evt.x ) / windowWidth;
  const Ogre::Real y = Ogre::Real( evt.y ) / windowHeight;
  if( x >= Ogre::Real( 0.34 ) && x <= Ogre::Real( 0.66 ) &&
      y >= Ogre::Real( 0.4 ) && y <= Ogre::Real( 0.49 ) )
    return( Action::REPLAY );
  if( x >= Ogre::Real( 0.34 ) && x <= Ogre::Real( 0.66 ) &&
      y >= Ogre::Real( 0.52 ) && y <= Ogre::Real( 0.61 ) )
    return( Action::EXIT );
  return( Action::NONE );
}

// -------------------------------------------------------------------------
void LoseScreen::
destroy( )
{
  if( this->m_Manager == nullptr || this->m_Overlay == nullptr )
    return;
  if( Ogre::OverlayManager::getSingletonPtr( ) != this->m_Manager )
  {
    this->m_Manager = nullptr;
    this->m_Overlay = nullptr;
    this->m_Elements.clear( );
    return;
  }

  this->m_Overlay->clear( );
  for( Ogre::PanelOverlayElement* element: this->m_Elements )
    this->m_Manager->destroyOverlayElement( element );
  this->m_Elements.clear( );
  this->m_Manager->destroy( this->m_Overlay );
  this->m_Overlay = nullptr;
  this->m_Manager = nullptr;
}

// -------------------------------------------------------------------------
Ogre::PanelOverlayElement* LoseScreen::
_panel(
  const Ogre::String& name, Ogre::Real x, Ogre::Real y,
  Ogre::Real width, Ogre::Real height, const Ogre::ColourValue& colour
  )
{
  auto* panel = static_cast< Ogre::PanelOverlayElement* >(
    this->m_Manager->createOverlayElement( "Panel", name )
    );
  panel->setMaterialName( Laberinto::overlayMaterialName( colour ) );
  panel->setMetricsMode( Ogre::GMM_RELATIVE );
  panel->setPosition( x, y );
  panel->setDimensions( width, height );
  panel->setColour( Ogre::ColourValue::White );
  this->m_Overlay->add2D( panel );
  this->m_Elements.push_back( panel );
  return( panel );
}

// -------------------------------------------------------------------------
void LoseScreen::
_drawText(
  const Ogre::String& text, Ogre::Real x, Ogre::Real y,
  Ogre::Real pixelWidth, Ogre::Real pixelHeight,
  const Ogre::ColourValue& colour
  )
{
  Ogre::Real cursor = x;
  for( char character: text )
  {
    character = char( std::toupper( static_cast< unsigned char >( character ) ) );
    if( character == ' ' )
    {
      cursor += pixelWidth * Ogre::Real( 6 );
      continue;
    }

    for( const Glyph& glyph: kGlyphs )
      if( glyph.character == character )
      {
        for( size_t row = 0; row < glyph.rows.size( ); ++row )
          for( size_t column = 0; column < 5; ++column )
            if( glyph.rows[ row ][ column ] == '1' )
              this->_panel(
                "LoseScreenText" + Ogre::StringConverter::toString( this->m_Elements.size( ) ),
                cursor + pixelWidth * Ogre::Real( column ),
                y + pixelHeight * Ogre::Real( row ),
                pixelWidth * Ogre::Real( 0.82 ),
                pixelHeight * Ogre::Real( 0.82 ),
                colour
                );
        break;
      }
    cursor += pixelWidth * Ogre::Real( 6 );
  }
}
