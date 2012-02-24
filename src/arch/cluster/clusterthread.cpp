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

#include <iostream>
#include "instrumentation.hpp"
#include "clusterthread_decl.hpp"
#include "clusternode_decl.hpp"
#include "system_decl.hpp"
#include "workdescriptor_decl.hpp"

using namespace nanos;
using namespace ext;


void ClusterThread::runDependent ()
{
   WD &work = getThreadWD();
   setCurrentWD( work );

   SMPDD &dd = ( SMPDD & ) work.activateDevice( SMP );

   dd.getWorkFct()( work.getData() );
}

void ClusterThread::outlineWorkDependent ( WD &wd )
{
   unsigned int i;
   SMPDD &dd = ( SMPDD & )wd.getActiveDevice();
   ProcessingElement *pe = myThread->runningOn();
   if (dd.getWorkFct() == NULL ) return;

   wd.start(WorkDescriptor::IsNotAUserLevelThread);
   wd.getGE()->setNode( ( ( ClusterNode * ) pe )->getClusterNodeNum() );

   //std::cerr << "run remote task, target pe: " << pe << " node num " << (unsigned int) ((ClusterNode *) pe)->getClusterNodeNum() << " " << (void *) &wd << ":" << (unsigned int) wd.getId() << " data size is " << wd.getDataSize() << std::endl;

   CopyData newCopies[ wd.getNumCopies() ]; 

   for (i = 0; i < wd.getNumCopies(); i += 1) {
      new ( &newCopies[i] ) CopyData( wd.getCopies()[i] );
   }

   //NANOS_INSTRUMENT ( static nanos_event_key_t key = sys.getInstrumentation()->getInstrumentationDictionary()->getEventKey("user-code") );
   //NANOS_INSTRUMENT ( nanos_event_value_t val = wd.getId() );
   //NANOS_INSTRUMENT ( sys.getInstrumentation()->raiseOpenStateAndBurst ( NANOS_RUNNING, key, val ) );

   // merge uintptr_t offset = (uintptr_t) &wd.getDimensions(); 

   for (i = 0; i < wd.getNumCopies(); i += 1) {
      newCopies[i].setAddress( ( uint64_t ) pe->getAddress( wd, newCopies[i].getAddress(), newCopies[i].getSharing() ) );

      // merhe uintptr_t value = (((uintptr_t ) &newCopies[i].getDataAccess()->getDimensions() ) - offset );
      // merhe newCopies[i].getDataAccess()->setDimensions( (nanos_region_dimension_internal_t const *) value );
   }

   size_t totalBufferSize = wd.getDataSize() + 
      sizeof(int) + wd.getNumCopies() * sizeof( CopyData ) + 
      // merge sizeof(int) + wd.getNumDimensions() * sizeof( nanos_region_dimension_t ) + 
      sizeof(int) + wd.getNumCopies() * sizeof( uint64_t );

   char *buff = new char[ totalBufferSize ];

   if ( wd.getDataSize() > 0 )
   {
      memcpy( &buff[ 0 ], wd.getData(), wd.getDataSize() );
   }

   *((int *) &buff[ wd.getDataSize() ] ) = wd.getNumCopies();
   for (i = 0; i < wd.getNumCopies(); i += 1) {
      memcpy( &buff[ wd.getDataSize() + sizeof(int) + sizeof( CopyData ) * i ], &newCopies[i], sizeof( CopyData ) );
   }

   // merge *((int *) &buff[ wd.getDataSize() + sizeof(int) + sizeof( CopyData ) * wd.getNumCopies() ] ) = (int) wd.getNumDimensions();
   //for (i = 0; i < wd.getNumDimensions(); i += 1) {
   // merge    memcpy( &buff[ wd.getDataSize() + sizeof(int) + sizeof( CopyData ) * wd.getNumCopies() + sizeof( int ) ], &wd.getDimensions(), sizeof( nanos_region_dimension_t ) * wd.getNumDimensions() );
   //}
   

#ifdef GPU_DEV
   int arch = -1;
   if (wd.canRunIn( GPU) )
   {
      arch = 1;
   }
   else if (wd.canRunIn( SMP ) )
   {
      arch = 0;
   }
#else
   int arch = 0;
#endif

   ( ( ClusterNode * ) pe )->incExecutedWDs();
   sys.getNetwork()->sendWorkMsg( ( ( ClusterNode * ) pe )->getClusterNodeNum(), dd.getWorkFct(), wd.getDataSize(), wd.getId(), /* this should be the PE id */ arch, totalBufferSize, buff, wd.getTranslateArgs(), arch, (void *) &wd );

}

void ClusterThread::join()
{
   unsigned int i;
   message( "Node " << ( ( ClusterNode * ) this->runningOn() )->getClusterNodeNum() << " executed " <<( ( ClusterNode * ) this->runningOn() )->getExecutedWDs() << " WDs" );
   for ( i = 1; i < sys.getNetwork()->getNumNodes(); i++ )
      sys.getNetwork()->sendExitMsg( i );
}

BaseThread * ClusterThread::getNextThread ()
{
   BaseThread *next;
   if ( getParent() != NULL )
   {
      next = getParent()->getNextThread();
   }
   else
   {
      next = this;
   }
   return next;
}

void ClusterThread::notifyOutlinedCompletionDependent( WD &completedWD ) {
#ifdef GPU_DEV
   int arch = -1;
   if ( completedWD.canRunIn( GPU ) )
   {
      arch = 1;
   }
   else if ( completedWD.canRunIn( SMP ) )
   {
      arch = 0;
   }
#else
   int arch = 0;
#endif
   if ( arch == 0) 
      completeWDSMP_2( &completedWD );
   else if ( arch == 1)
      completeWDGPU_2( &completedWD );
   else
      std::cerr << "unhandled arch" << std::endl;
}

void ClusterThread::addRunningWDSMP( WorkDescriptor *wd ) { 
   _numRunningSMP++;
}
unsigned int ClusterThread::numRunningWDsSMP() {
   return _numRunningSMP.value();
}
void ClusterThread::clearCompletedWDsSMP2( ) {
   unsigned int lowval = _completedSMPTail % MAX_PRESEND;
   unsigned int highval = ( _completedSMPHead2.value() ) % MAX_PRESEND;
   unsigned int pos = lowval;
   if ( lowval > highval ) highval +=MAX_PRESEND;
   while ( lowval < highval )
   {
      WD *completedWD = _completedWDsSMP[pos];
      Scheduler::postOutlineWork( completedWD, false, this );
      delete[] (char *) completedWD;
      _completedWDsSMP[pos] =(WD *) 0xdeadbeef;
      pos = (pos+1) % MAX_PRESEND;
      lowval += 1;
      _completedSMPTail += 1;
   }
}
void ClusterThread::completeWDSMP_2( void *remoteWdAddr ) {
   unsigned int realpos = _completedSMPHead++;
   _numRunningSMP--;
   unsigned int pos = realpos %MAX_PRESEND;
   _completedWDsSMP[pos] = (WD *) remoteWdAddr;
   while( !_completedSMPHead2.cswap( realpos, realpos+1) ) {}
}

void ClusterThread::addRunningWDGPU( WorkDescriptor *wd ) { 
   _numRunningGPU++;
}

unsigned int ClusterThread::numRunningWDsGPU() {
   return _numRunningGPU.value();
}

void ClusterThread::clearCompletedWDsGPU2( ) {
   unsigned int lowval = _completedGPUTail % MAX_PRESEND;
   unsigned int highval = ( _completedGPUHead2.value() ) % MAX_PRESEND;
   unsigned int pos = lowval;
   if ( lowval > highval ) highval +=MAX_PRESEND;
   while ( lowval < highval )
   {
      WD *completedWD = _completedWDsGPU[pos];
      Scheduler::postOutlineWork( completedWD, false, this );
      delete[] (char *) completedWD;
      _completedWDsGPU[pos] =(WD *) 0xdeadbeef;
      pos = (pos+1) % MAX_PRESEND;
      lowval += 1;
      _completedGPUTail += 1;
   }
}

void ClusterThread::completeWDGPU_2( void *remoteWdAddr ) {
   unsigned int realpos = _completedGPUHead++;
   _numRunningGPU--;
   unsigned int pos = realpos %MAX_PRESEND;
   _completedWDsGPU[pos] = (WD *) remoteWdAddr;
   while( !_completedGPUHead2.cswap( realpos, realpos+1) ) {}
}

void ClusterThread::addBlockingWDSMP( WD * wd ) {
   _blockedWDsSMP.push(wd);
}

WD *ClusterThread::fetchBlockingWDSMP() {
   WD *wd = NULL;
   if ( !_blockedWDsSMP.empty() ) {
      wd = _blockedWDsSMP.front();
      _blockedWDsSMP.pop();
   }
   return wd;
}

void ClusterThread::addBlockingWDGPU( WD * wd ) {
   _blockedWDsGPU.push(wd);
}

WD *ClusterThread::fetchBlockingWDGPU() {
   WD *wd = NULL;
   if ( !_blockedWDsGPU.empty() ) {
      wd = _blockedWDsGPU.front();
      _blockedWDsGPU.pop();
   }
   return wd;
}
