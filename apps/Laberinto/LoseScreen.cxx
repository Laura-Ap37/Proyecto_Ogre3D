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
    { 'G', { "01110", "10001", "10000", "10111", "10001", "10001", "01110" } },
    { 'M', { "10001", "11011", "10101", "10101", "10001", "10001", "10001" } },
    { 'V', { "10001", "10001", "10001", "10001", "10001", "01010", "00100" } },
    { 'L', { "10000", "10000", "10000", "10000", "10000", "10000", "11111" } },
    { 'O', { "01110", "10001", "10001", "10001", "10001", "10001", "01110" } },
    { 'P', { "11110", "10001", "10001", "11110", "10000", "10000", "10000" } },
    { 'R', { "11110", "10001", "10001", "11110", "10100", "10010", "10001" } },
    { 'S', { "01111", "10000", "10000", "01110", "00001", "00001", "11110" } },
    { 'T', { "11111", "00100", "00100", "00100", "00100", "00100", "00100" } },
    { 'U', { "10001", "10001", "10001", "10001", "10001", "10001", "01110" } },
    { 'Y', { "10001", "10001", "01010", "00100", "00100", "00100", "00100" } },
    { 'X', { "10001", "10001", "01010", "00100", "01010", "10001", "10001" } }
  };

  // Diseño compartido entre build() y mousePressed()
  // (coordenadas relativas a la ventana, de 0 a 1)
  const Ogre::Real kButtonX = 0.35;
  const Ogre::Real kButtonW = 0.30;
  const Ogre::Real kButtonH = 0.085;
  const Ogre::Real kReplayY = 0.45;
  const Ogre::Real kExitY   = 0.58;
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

  // Fondo con tinte rojizo
  this->_panel(
    "LoseScreenBackdrop", 0, 0, 1, 1,
    Ogre::ColourValue( 0.05, 0, 0, 0.8 )
    );

  // Marco rojo (panel un poco más grande detrás) y panel interior
  this->_panel(
    "LoseScreenFrame", 0.245, 0.17, 0.51, 0.66,
    Ogre::ColourValue( 0.55, 0.05, 0.07, 1 )
    );
  this->_panel(
    "LoseScreenPanel", 0.25, 0.18, 0.5, 0.64,
    Ogre::ColourValue( 0.03, 0.02, 0.035, 0.98 )
    );

  // Título: primero la sombra, luego el texto encima
  this->_drawTextCentered(
    "GAME OVER", 0.5015, 0.2627, 0.0075, 0.0135,
    Ogre::ColourValue( 0.25, 0, 0.02, 1 )
    );
  this->_drawTextCentered(
    "GAME OVER", 0.5, 0.26, 0.0075, 0.0135,
    Ogre::ColourValue( 0.92, 0.12, 0.16, 1 )
    );

  // Línea separadora
  this->_panel(
    "LoseScreenLine", 0.32, 0.385, 0.36, 0.004,
    Ogre::ColourValue( 0.55, 0.05, 0.07, 1 )
    );

  // Botones
  this->_button(
    "ReplayButton", kReplayY, "REPLAY",
    Ogre::ColourValue( 0.75, 0.12, 0.14, 1 ),
    Ogre::ColourValue( 0.25, 0.07, 0.08, 1 ),
    Ogre::ColourValue( 1, 0.88, 0.82, 1 )
    );
  this->_button(
    "ExitButton", kExitY, "EXIT",
    Ogre::ColourValue( 0.35, 0.35, 0.42, 1 ),
    Ogre::ColourValue( 0.12, 0.12, 0.15, 1 ),
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

  // Mismas constantes con las que se dibujan los botones
  const bool inX = x >= kButtonX && x <= kButtonX + kButtonW;
  if( inX && y >= kReplayY && y <= kReplayY + kButtonH )
    return( Action::REPLAY );
  if( inX && y >= kExitY && y <= kExitY + kButtonH )
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

// -------------------------------------------------------------------------
void LoseScreen::
_drawTextCentered(
  const Ogre::String& text, Ogre::Real centerX, Ogre::Real y,
  Ogre::Real pixelWidth, Ogre::Real pixelHeight,
  const Ogre::ColourValue& colour
  )
{
  if( text.empty( ) )
    return;
  const Ogre::Real width
    = Ogre::Real( text.size( ) - 1 ) * pixelWidth * 6 + pixelWidth * Ogre::Real( 4.82 );
  this->_drawText( text, centerX - width / 2, y, pixelWidth, pixelHeight, colour );
}

// -------------------------------------------------------------------------
void LoseScreen::
_button(
  const Ogre::String& name, Ogre::Real y, const Ogre::String& label,
  const Ogre::ColourValue& frame, const Ogre::ColourValue& fill,
  const Ogre::ColourValue& text
  )
{
  const Ogre::Real bx = 0.002, by = 0.0035; // grosor del borde
  this->_panel( name + "Frame", kButtonX - bx, y - by, kButtonW + 2 * bx, kButtonH + 2 * by, frame );
  this->_panel( name, kButtonX, y, kButtonW, kButtonH, fill );
  this->_drawTextCentered( label, 0.5, y + Ogre::Real( 0.0101 ), 0.0055, 0.0095, text );
}