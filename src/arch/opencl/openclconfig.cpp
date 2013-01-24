
#include "openclconfig.hpp"
#include "system.hpp"

using namespace nanos;
using namespace nanos::ext;

bool OpenCLConfig::_disableOpenCL = false;
std::string OpenCLConfig::_devTy = "ALL";
int OpenCLConfig::_devCacheSize = 0;
unsigned int OpenCLConfig::_devNum = INT_MAX;
System::CachePolicyType OpenCLConfig::_cachePolicy = System::WRITE_BACK;

std::vector<cl_platform_id> OpenCLConfig::_plats;
std::vector<cl_device_id> OpenCLConfig::_devices;

Atomic<unsigned> OpenCLConfig::_freeDevice = 0;

cl_device_id OpenCLConfig::getFreeDevice() {
   if(_freeDevice == _devices.size())
      fatal( "No more free devices" );

   return _devices[_freeDevice++];
}

void OpenCLConfig::prepare( Config &cfg )
{
   cfg.setOptionsSection( "OpenCL Arch", "OpenCL specific options" );

   // Enable/disable OpenCL.
   cfg.registerConfigOption( "disable-opencl",
                             NEW Config::FlagOption( _disableOpenCL ),
                             "Enable or disable the use of "
                             "OpenCL back-end (enabled by default)" );
   cfg.registerEnvOption( "disable-opencl", "NX_DISABLE_OPENCL" );
   cfg.registerArgOption( "disable-opencl", "disable-opencl" );

   // Select the device to use.
   cfg.registerConfigOption( "opencl-device-type",
                             NEW Config::StringVar( _devTy ),
                             "Defines the OpenCL device type to use "
                             "(ALL, CPU, GPU, ACCELERATOR)" );
   cfg.registerEnvOption( "opencl-device-type", "NX_OPENCL_DEVICE_TYPE" );
   cfg.registerArgOption( "opencl-device-type", "opencl-device-type" );
   
   System::CachePolicyConfig *cachePolicyCfg = NEW System::CachePolicyConfig ( _cachePolicy );
   cachePolicyCfg->addOption("wt", System::WRITE_THROUGH );
   cachePolicyCfg->addOption("wb", System::WRITE_BACK );
   cachePolicyCfg->addOption( "nocache", System::NONE );
   // Set the cache policy for OpenCL devices
   cfg.registerConfigOption ( "opencl-cache-policy", cachePolicyCfg, "Defines the cache policy for OpenCL devices" );
   cfg.registerEnvOption ( "opencl-cache-policy", "NX_OPENCL_CACHE_POLICY" );
   cfg.registerArgOption( "opencl-cache-policy", "opencl-cache-policy" );

   // Select the size of the device cache.
   cfg.registerConfigOption( "opencl-cache",
                             NEW Config::IntegerVar( _devCacheSize ),
                             "Defines the amount of the cache "
                             "to be allocated on the device" );
   cfg.registerEnvOption( "opencl-cache", "NX_OPENCL_CACHE" );
   cfg.registerArgOption( "opencl-cache", "opencl-cache" );
   
    // Select the size of the device cache.
   cfg.registerConfigOption( "opencl-max-devices",
                             NEW Config::UintVar( _devNum ),
                             "Defines the total maximum number of devices "
                             "to be used by nanox" );
   cfg.registerEnvOption( "opencl-max-devices", "NX_OPENCL_MAX_DEVICES" );
   cfg.registerArgOption( "opencl-max-devices", "opencl-max-devices" );

}

void OpenCLConfig::apply()
{
   if( _disableOpenCL )
     return;

   cl_int errCode;

   // Get the number of available platforms.
   cl_uint numPlats;
   if( p_clGetPlatformIDs( 0, NULL, &numPlats ) != CL_SUCCESS )
      fatal0( "Cannot detect the number of available OpenCL platforms" );

   if ( numPlats == 0 )
      fatal0( "No OpenCL platform available" );

   // Read all platforms.
   cl_platform_id *plats = new cl_platform_id[numPlats];
   if( p_clGetPlatformIDs( numPlats, plats, NULL ) != CL_SUCCESS )
      fatal0( "Cannot load OpenCL platforms" );

   // Is platform available?
   if( !numPlats )
      fatal0( "No OpenCL platform available" );

   // Save platforms.
   _plats.assign(plats, plats + numPlats);
   delete [] plats;

   //TODO FIX THIS OR RETURN TO TRUNK MODE
   cl_device_type devTy;

   // Parse the requested device type.
   if( _devTy == "" || _devTy == "ALL" ){
      devTy = CL_DEVICE_TYPE_ALL;
   }  else if( _devTy == "CPU" )
      devTy = CL_DEVICE_TYPE_CPU;
   else if( _devTy == "GPU" )
      devTy = CL_DEVICE_TYPE_GPU;
   else if( _devTy == "ACCELERATOR" )
      devTy = CL_DEVICE_TYPE_ACCELERATOR;
   else
      fatal0( "Unable to parse device type" );

   // Read all devices.
   for( std::vector<cl_platform_id>::iterator i = _plats.begin(),
                                              e = _plats.end();
                                              i != e;
                                              ++i ) {
      // Get the number of available devices.
      cl_uint numDevices;
      errCode = p_clGetDeviceIDs( *i, devTy, 0, NULL, &numDevices );
      if (numDevices>_devNum){
          numDevices=_devNum;
      }
      if( errCode != CL_SUCCESS )
         continue;

      // Read all matching devices.
      cl_device_id *devs = new cl_device_id[numDevices];
      errCode = p_clGetDeviceIDs( *i, devTy, numDevices, devs, NULL );
      if( errCode != CL_SUCCESS )
         continue;

      // Put all available devices inside the vector.
      for( cl_device_id *j = devs, *f = devs + numDevices; j != f; ++j )
      {
         cl_bool available;

         errCode = p_clGetDeviceInfo( *j,
                                      CL_DEVICE_AVAILABLE,
                                      sizeof( cl_bool ),
                                      &available,
                                      NULL );
         if( errCode != CL_SUCCESS )
           continue;

         if( available )
           _devices.push_back( *j );
      }

      delete [] devs;
   }
}
