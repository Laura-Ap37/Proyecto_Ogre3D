#ifndef __Laberinto__LoseScreen__h__
#define __Laberinto__LoseScreen__h__

#include <OgreOverlay.h>
#include <OgreOverlayManager.h>
#include <OgrePanelOverlayElement.h>
#include <OgreInput.h>
#include <vector>

class LoseScreen
{
public:
  enum class Action { NONE, REPLAY, EXIT };

  void build( );
  void show( );
  void hide( );
  Action mousePressed(
    const OgreBites::MouseButtonEvent& evt,
    Ogre::Real windowWidth, Ogre::Real windowHeight
    ) const;
  void destroy( );

protected:
  Ogre::PanelOverlayElement* _panel(
    const Ogre::String& name, Ogre::Real x, Ogre::Real y,
    Ogre::Real width, Ogre::Real height, const Ogre::ColourValue& colour
    );
  void _drawText(
    const Ogre::String& text, Ogre::Real x, Ogre::Real y,
    Ogre::Real pixelWidth, Ogre::Real pixelHeight,
    const Ogre::ColourValue& colour
    );
  void _drawTextCentered(
    const Ogre::String& text, Ogre::Real centerX, Ogre::Real y,
    Ogre::Real pixelWidth, Ogre::Real pixelHeight,
    const Ogre::ColourValue& colour
    );
  void _button(
    const Ogre::String& name, Ogre::Real y, const Ogre::String& label,
    const Ogre::ColourValue& frame, const Ogre::ColourValue& fill,
    const Ogre::ColourValue& text
    );

protected:
  Ogre::OverlayManager* m_Manager { nullptr };
  Ogre::Overlay* m_Overlay { nullptr };
  std::vector< Ogre::PanelOverlayElement* > m_Elements;
  bool m_Visible { false };
};

#endif // __Laberinto__LoseScreen__h__
