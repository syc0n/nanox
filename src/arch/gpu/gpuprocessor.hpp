/*************************************************************************************/
/*      Copyright 2009 Barcelona Supercomputing Center                               */
/*                                                                                   */
/*      This file is part of the NANOS++ library.                                    */
/*                                                                                   */
/*      NANOS++ is free software: you can redistribute it and/or modify              */
/*      it under the terms of the GNU Lesser General Public License as published by  */
/*      the Free Software Foundation, either version 3 of the License, or            */
/*      (at your option) any later version.                                          */
/*                                                                                   */
/*      NANOS++ is distributed in the hope that it will be useful,                   */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/*      GNU Lesser General Public License for more details.                          */
/*                                                                                   */
/*      You should have received a copy of the GNU Lesser General Public License     */
/*      along with NANOS++.  If not, see <http://www.gnu.org/licenses/>.             */
/*************************************************************************************/

#ifndef _NANOS_GPU_PROCESSOR
#define _NANOS_GPU_PROCESSOR

#include "gpuprocessor_decl.hpp"

#include <cuda_runtime.h>

#include <iostream>

#if __CUDA_API_VERSION < 3020
#define CUDANODEVERR cudaErrorDevicesUnavailable
#else
#define CUDANODEVERR cudaErrorNoDevice
#endif 

namespace nanos {
namespace ext
{

   class GPUProcessor::GPUProcessorInfo
   {
      private:
         // Device #
         unsigned int   _deviceId;

         // Memory
         size_t         _maxMemoryAvailable;

         // Transfers
         cudaStream_t   _inTransferStream;
         cudaStream_t   _outTransferStream;

      public:
         GPUProcessorInfo ( int device ) : _deviceId ( device ), _maxMemoryAvailable ( 0 ),
            _inTransferStream ( 0 ), _outTransferStream ( 0 )
         {}

         ~GPUProcessorInfo ()
         {
            if ( _inTransferStream ) {
               cudaError_t err = cudaStreamDestroy( _inTransferStream );
               if ( err != cudaSuccess ) {
                  warning( "Error while destroying the CUDA input transfer stream: " << cudaGetErrorString( err ) );
               }
            }

            if ( _outTransferStream ) {
               cudaError_t err = cudaStreamDestroy( _outTransferStream );
               if ( err != cudaSuccess ) {
                  warning( "Error while destroying the CUDA output transfer stream: " << cudaGetErrorString( err ) );
               }
            }
         }

         void init ()
         {
            // Each thread initializes its own GPUProcessorInfo so that initialization
            // can be done in parallel

            struct cudaDeviceProp gpuProperties;
            GPUConfig::getGPUsProperties( _deviceId, ( void * ) &gpuProperties );
            //cudaGetDeviceProperties( &gpuProperties, _deviceId );

            // Check if the user has set the amount of memory to use (and the value is valid)
            // Otherwise, use 95% of the total GPU global memory
            size_t userDefinedMem = GPUConfig::getGPUMaxMemory();
            _maxMemoryAvailable = ( size_t ) ( gpuProperties.totalGlobalMem * 0.95 );

            if ( userDefinedMem > 0 ) {
               if ( userDefinedMem > _maxMemoryAvailable ) {
                  warning( "Could not set memory size to " << userDefinedMem
                        << " for GPU #" << _deviceId
                        << " because maximum memory available is " << _maxMemoryAvailable
                        << " bytes. Using " << _maxMemoryAvailable << " bytes" );
               }
               else {
                  _maxMemoryAvailable = userDefinedMem;
               }
            }

            if ( !gpuProperties.deviceOverlap ) {
               // It does not support stream overlapping, disable this feature
               warning( "Device #" << _deviceId <<
                     " does not support computation and data transfer overlapping" );
            } else {
            if ( GPUDD::isOverlappingInputsDefined() ) {
               // Initialize the CUDA streams used for input data transfers
               cudaError_t err = cudaStreamCreate( &_inTransferStream );
               if ( err != cudaSuccess ) {
                  // If an error occurred, disable stream overlapping
                  _inTransferStream = 0;
                  if ( err == CUDANODEVERR ) {
                     fatal( "Error while creating the CUDA input transfer stream: all CUDA-capable devices are busy or unavailable" );
                  }
                  warning( "Error while creating the CUDA input transfer stream: " << cudaGetErrorString( err ) );
               }
            }
            if ( GPUDD::isOverlappingOutputsDefined() ) {
               // Initialize the CUDA streams used for output data transfers
               cudaError_t err = cudaStreamCreate( &_outTransferStream );
               if ( err != cudaSuccess ) {
                  // If an error occurred, disable stream overlapping
                  _outTransferStream = 0;
                  if ( err == CUDANODEVERR ) {
                     fatal( "Error while creating the CUDA output transfer stream: all CUDA-capable devices are busy or unavailable" );
                  }
                  warning( "Error while creating the CUDA output transfer stream: " << cudaGetErrorString( err ) );
               }
            }
            }

            // Initialize GPUProcessor --> we allocate the whole GPU memory
            // WARNING: GPUProcessor::init must be called after initializing CUDA Streams
            size_t allocatedMemory = _maxMemoryAvailable;
            ( ( GPUProcessor * ) myThread->runningOn() )->init( allocatedMemory );

            if ( allocatedMemory != _maxMemoryAvailable ) {
               _maxMemoryAvailable = allocatedMemory;
            }
         }

         size_t getMaxMemoryAvailable ()
         {
            return _maxMemoryAvailable;
         }

         cudaStream_t getInTransferStream ()
         {
            return _inTransferStream;
         }

         cudaStream_t getOutTransferStream ()
         {
            return _outTransferStream;
         }
   };
}
}

#endif
