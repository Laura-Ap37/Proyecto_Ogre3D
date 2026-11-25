#ifndef __Laberinto__OverlayMaterial__h__
#define __Laberinto__OverlayMaterial__h__

#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgrePixelFormat.h>
#include <OgreResourceGroupManager.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreTechnique.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace Laberinto
{
  inline Ogre::String overlayMaterialName( const Ogre::ColourValue& colour )
  {
    const auto channel = []( Ogre::Real value )
    {
      return( int( std::lround(
        std::clamp( value, Ogre::Real( 0 ), Ogre::Real( 1 ) ) * 255
        ) ) );
    };
    const int red = channel( colour.r );
    const int green = channel( colour.g );
    const int blue = channel( colour.b );
    const int alpha = channel( colour.a );
    const Ogre::String suffix
      = std::to_string( red ) + "_" + std::to_string( green ) + "_"
      + std::to_string( blue ) + "_" + std::to_string( alpha );
    const Ogre::String name = "Laberinto/OverlayMaterial_" + suffix;
    const Ogre::String textureName = "Laberinto/OverlayTexture_" + suffix;
    const Ogre::String group
      = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
    auto& materials = Ogre::MaterialManager::getSingleton( );
    Ogre::MaterialPtr material = materials.getByName( name );
    if( !material )
    {
      Ogre::TexturePtr texture
        = Ogre::TextureManager::getSingleton( ).getByName( textureName );
      if( !texture )
      {
        texture = Ogre::TextureManager::getSingleton( ).createManual(
          textureName, group, Ogre::TEX_TYPE_2D, 1, 1, 0,
          Ogre::PF_A8R8G8B8
          );
        Ogre::uint32 pixel = 0;
        Ogre::PixelUtil::packColour(
          Ogre::ColourValue(
            Ogre::Real( red ) / 255, Ogre::Real( green ) / 255,
            Ogre::Real( blue ) / 255, Ogre::Real( alpha ) / 255
            ),
          Ogre::PF_A8R8G8B8, &pixel
          );
        texture->getBuffer( )->blitFromMemory(
          Ogre::PixelBox( 1, 1, 1, Ogre::PF_A8R8G8B8, &pixel )
          );
        texture->load( );
      }

      material = materials.create( name, group );
      Ogre::Pass* pass = material->getTechnique( 0 )->getPass( 0 );
      pass->setLightingEnabled( false );
      pass->setDiffuse( Ogre::ColourValue::White );
      pass->setSceneBlending( Ogre::SBT_TRANSPARENT_ALPHA );
      pass->setDepthCheckEnabled( false );
      pass->setDepthWriteEnabled( false );
      pass->setCullingMode( Ogre::CULL_NONE );
      pass->createTextureUnitState( textureName );
      material->load( );
    }

    return( name );
  }
}

#endif // __Laberinto__OverlayMaterial__h__
