// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

// Ogre plugin that registers VTK and Wavefront OBJ files as mesh codecs
// (Ogre 13+), so sceneMgr->createEntity("model.obj") just works.
//
// Load with plugins.cfg:   Plugin=Plugin_VtkCodec
// or in code:              root->loadPlugin("Plugin_VtkCodec");

#include <pujOgre/vtkPolyData2OgreMesh.h>
#include <plugins/VTKCodec/Export.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>

#include <OgrePlugin.h>
#include <OgreRoot.h>
#include <OgreCodec.h>
#include <OgreMesh.h>
#include <OgreDataStream.h>
#include <OgreException.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkOBJReader.h>
#include <vtkTextureMapToSphere.h>
#include <vtkVersionMacros.h>
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK( 9, 4, 0 )
#include <vtkMemoryResourceStream.h>
#endif

namespace
{
  /**
   */
  class Plugin_pujOgreVTKCodec_EXPORT pujOgreVTKCodec
    : public Ogre::Codec
  {
  public:
    explicit pujOgreVTKCodec( const Ogre::String& extension )
      : m_Type( extension )
      {
      }

    virtual Ogre::String getType( ) const override
      {
        return( this->m_Type );
      }

    virtual
    Ogre::String magicNumberToFileExt( const char*, std::size_t ) const
    override
      {
        return( "" );
      }

    virtual
    void decode(
      const Ogre::DataStreamPtr& input, const Ogre::Any& output
      ) const
    override
    {
      Ogre::Mesh* mesh = Ogre::any_cast< Ogre::Mesh* >( output );
      const std::string data = input->getAsString( );

      vtkSmartPointer< vtkPolyData > polyData;
      if( this->m_Type == "obj" )
      {
        auto reader = vtkSmartPointer< vtkOBJReader >::New( );
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK( 9, 4, 0 )
        auto stream = vtkSmartPointer< vtkMemoryResourceStream >::New( );
        stream->SetBuffer( data.data( ), data.size( ), true );
        reader->SetStream( stream );
#else
        // VTK 9.1 reads OBJ only from a filename.
        const auto tmp =
          std::filesystem::temp_directory_path( ) /
          (
            "pujogre-"
            +
            std::to_string(
              std::chrono::steady_clock::now( ).time_since_epoch( ).count( )
              )
            +
            ".obj"
            );
        {
          std::ofstream out(
            tmp, std::ios::binary | std::ios::out | std::ios::noreplace
            );
          out.write(
            data.data( ), static_cast< std::streamsize >( data.size( ) )
            );
          if( !out )
            OGRE_EXCEPT(
              Ogre::Exception::ERR_CANNOT_WRITE_TO_FILE,
              "Could not write a temporary OBJ for '" + input->getName( ) + "'",
              "pujOgreVTKCodec::decode"
              );
        }
        struct TmpFile
        {
          std::filesystem::path Path;
          ~TmpFile( )
            {
              std::error_code ec;
              std::filesystem::remove( this->Path, ec );
            }
        } tmpFile{ tmp };
        reader->SetFileName( tmp.string( ).c_str( ) );
#endif
        reader->Update( );

        auto textureMap = vtkSmartPointer< vtkTextureMapToSphere >::New( );
        textureMap->SetInputConnection( reader->GetOutputPort( ) );
        textureMap->PreventSeamOn( );
        textureMap->Update( );
        polyData = vtkPolyData::SafeDownCast( textureMap->GetOutput( ) );
      }
      else if( this->m_Type == "vtp" )
      {
        auto reader = vtkSmartPointer< vtkXMLPolyDataReader >::New( );
        reader->ReadFromInputStringOn( );
        reader->SetInputString( data );
        reader->Update( );
        polyData = reader->GetOutput( );
      }
      else
      {
        auto reader = vtkSmartPointer< vtkPolyDataReader >::New( );
        reader->ReadFromInputStringOn( );
        reader->SetBinaryInputString( data.data( ), int( data.size( ) ) );
        reader->Update( );
        polyData = reader->GetOutput( );
      } // end if

      if(
        polyData.GetPointer( ) == nullptr
        ||
        polyData->GetNumberOfPoints( ) == 0
        )
        OGRE_EXCEPT(
          Ogre::Exception::ERR_INVALIDPARAMS,
          "No polydata could be read from '" + input->getName( ) + "'",
          "pujOgreVTKCodec::decode"
          );

      // Material can be changed later with entity->setMaterialName(...)
      pujOgre::FillOgreMeshFromvtkPolyData(
        polyData, mesh, "BaseWhite", true, true
        );
    }

  private:
    Ogre::String m_Type;
  };

  /**
   */
  class pujOgreVTKPlugin
    : public Ogre::Plugin
  {
  public:
    virtual const Ogre::String& getName( ) const override
      {
        static const Ogre::String name = "pujOgre VTK PolyData Codec";
        return( name );
      }

    virtual void install( ) override
      {
        for( const char* ext: { "vtp", "vtk", "obj" } )
        {
          auto* codec = new pujOgreVTKCodec( ext );
          Ogre::Codec::registerCodec( codec );
          this->m_Codecs.push_back( codec );
        } // end for
      }

    virtual void initialise( ) override { }
    virtual void shutdown  ( ) override { }

    virtual void uninstall( ) override
    {
      for( auto* codec: this->m_Codecs )
      {
        Ogre::Codec::unregisterCodec( codec );
        delete codec;
      } // end for
      this->m_Codecs.clear( );
    }

  private:
    std::vector< Ogre::Codec* > m_Codecs;
  };

  pujOgreVTKPlugin* gPlugin = nullptr;

} // end namespace

// Entry points Ogre looks up when it loads the shared library.
extern "C" void Plugin_pujOgreVTKCodec_EXPORT dllStartPlugin( )
{
  gPlugin = new pujOgreVTKPlugin( );
  Ogre::Root::getSingleton( ).installPlugin( gPlugin );
}

extern "C" void Plugin_pujOgreVTKCodec_EXPORT dllStopPlugin( )
{
  Ogre::Root::getSingleton( ).uninstallPlugin( gPlugin );
  delete gPlugin;
  gPlugin = nullptr;
}

// eof - VTKCodec.cxx
