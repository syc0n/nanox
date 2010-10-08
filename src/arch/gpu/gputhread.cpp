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

#include "gputhread.hpp"
#include "gpuprocessor.hpp"
#include "instrumentationmodule_decl.hpp"
#include "schedule.hpp"
#include "system.hpp"

#include <cuda_runtime.h>

using namespace nanos;
using namespace nanos::ext;


void GPUThread::runDependent ()
{
   WD &work = getThreadWD();
   setCurrentWD( work );
   setNextWD( (WD *) 0 );

   cudaError_t err = cudaSetDevice( _gpuDevice );
   if ( err != cudaSuccess )
      warning( "Couldn't set the GPU device for the thread: " << cudaGetErrorString( err ) );

   if ( GPUDevice::getTransferMode() == nanos::PINNED_CUDA || GPUDevice::getTransferMode() == nanos::WC ) {
      err = cudaSetDeviceFlags( cudaDeviceMapHost | cudaDeviceBlockingSync );
      if ( err != cudaSuccess )
         warning( "Couldn't set the GPU device flags: " << cudaGetErrorString( err ) );
   }
   else {
      err = cudaSetDeviceFlags( cudaDeviceBlockingSync );
      if ( err != cudaSuccess )
         warning( "Couldn't set the GPU device flags:" << cudaGetErrorString( err ) );
   }

   if ( GPUDevice::getTransferMode() != nanos::NORMAL ) {
      ((GPUProcessor *) myThread->runningOn())->getGPUProcessorInfo()->init();
   }

   // Avoid the so slow first data allocation and transfer to device
   //bool b = true;
   //bool * b_d = ( bool * ) GPUDevice::allocate( sizeof( b ) );
   //GPUDevice::copyIn( ( void * ) b_d, ( uint64_t ) &b, sizeof( b ) );
   //GPUDevice::free( b_d );

   SMPDD &dd = ( SMPDD & ) work.activateDevice( SMP );

   dd.getWorkFct()( work.getData() );

   //( ( GPUProcessor * ) myThread->runningOn() )->freeWholeMemory();

}

void GPUThread::inlineWorkDependent ( WD &wd )
{
   GPUDD &dd = ( GPUDD & )wd.getActiveDevice();

   if ( GPUDevice::getTransferMode() != nanos::NORMAL ) {
      // Wait for the input transfer stream to finish
      cudaStreamSynchronize( ( (GPUProcessor *) myThread->runningOn() )->getGPUProcessorInfo()->getInTransferStream() );
      // Erase the wait input list and synchronize it with cache
      ( (GPUProcessor *) myThread->runningOn() )->getInTransferList()->clearMemoryTransfers();
   }

   // We wait for wd inputs, but as we have just waited for them, we could skip this step
   wd.start();

   NANOS_INSTRUMENT ( InstrumentStateAndBurst inst1( "user-code", wd.getId(), NANOS_RUNNING ) );
   ( dd.getWorkFct() )( wd.getData() );

   if ( GPUDevice::getTransferMode() != nanos::NORMAL ) {
      NANOS_INSTRUMENT ( InstrumentSubState inst2( NANOS_RUNTIME ) );
      // Get next task in order to prefetch data to device memory
      WD *next = Scheduler::prefetch( ( nanos::BaseThread * ) this, wd );

      setNextWD( next );
      if ( next != 0 ) {
         next->init(false);
      }

      // Copy out results from tasks executed previously
      ( (GPUProcessor *) myThread->runningOn() )->getOutTransferList()->executeMemoryTransfers();
   }

   // Wait for the GPU kernel to finish
   cudaThreadSynchronize();
}

void GPUThread::yield()
{
   ( ( GPUProcessor * ) runningOn() )->getOutTransferList()->executeMemoryTransfers();
}

void GPUThread::idle()
{
   ( ( GPUProcessor * ) runningOn() )->getOutTransferList()->removeMemoryTransfer();
}

