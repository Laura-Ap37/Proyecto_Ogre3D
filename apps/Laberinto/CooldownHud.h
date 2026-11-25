#ifndef __Laberinto__CooldownHud__h__
#define __Laberinto__CooldownHud__h__

#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgrePanelOverlayElement.h>
#include <array>

class CooldownHud
{
public:
  void build( );
  void update( Ogre::Real remaining, Ogre::Real duration );
  void destroy( );

protected:
  Ogre::PanelOverlayElement* _createPanel(
    const Ogre::String& name, Ogre::Real left, Ogre::Real top,
    Ogre::Real width, Ogre::Real height
    );
  void _setDigit( size_t index, int digit, const Ogre::ColourValue& colour );

protected:
  Ogre::Overlay* m_Overlay { nullptr };
  Ogre::OverlayManager* m_Manager { nullptr };
  std::array< Ogre::PanelOverlayElement*, 7 > m_Digits[ 2 ] {};
  Ogre::PanelOverlayElement* m_BarBackground { nullptr };
  Ogre::PanelOverlayElement* m_BarFill { nullptr };
  int m_LastSeconds { -1 };
};

#endif // __Laberinto__CooldownHud__h__
