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

#include <algorithm>
#include "accelerator_decl.hpp"
#include "debug.hpp"
#include "schedule.hpp"
#include "copydata.hpp"
#include "instrumentation.hpp"
#include "system.hpp"
#include "functors.hpp"

using namespace nanos;

#if LOCK_TRANSFER
Lock Accelerator::_transferLock;
#endif

Accelerator::Accelerator ( int newId, const Device *arch, const Device *subArch ) : ProcessingElement( newId, arch, subArch, sys.getMemorySpaceId() ) {}

#if 0
bool Accelerator::dataCanBlockUs( WorkDescriptor &work ) 
{
   bool result = false;
   CopyData *copies = work.getCopies();
   for ( unsigned int i = 0; i < work.getNumCopies() && !result; i++ ) {
      CopyData & cd = copies[i];
      if ( !cd.isPrivate() )
      {
	      uint64_t tag = cd.getAddress();
	      result = this->checkBlockingCacheAccessDependent( *(work.getParent()->getDirectory(true)), tag, cd.getSize(), cd.isInput(), cd.isOutput() ); 
      }
   }
   return result;
}
#endif

void Accelerator::copyDataIn( WorkDescriptor &work )
{
#if LOCK_TRANSFER
   _transferLock.acquire();
#endif
   //CopyData *copies = work.getCopies();

   this->copyDataInDependent( work );

   //for ( unsigned int i = 0; i < work.getNumCopies(); i++ ) {
   //   CopyData & cd = copies[i];
   //   uint64_t tag = (uint64_t) cd.isPrivate() ? ((uint64_t) work.getData() + (unsigned long)cd.getAddress()) : cd.getAddress();
   //   if ( cd.isInput() ) {
   //      NANOS_INSTRUMENT( static nanos_event_key_t key = sys.getInstrumentation()->getInstrumentationDictionary()->getEventKey("copy-in") );
   //      NANOS_INSTRUMENT( sys.getInstrumentation()->raisePointEvent( key, (nanos_event_value_t) cd.getSize() ) );
   //   }

   //   if ( cd.isPrivate() ) {
   //      this->registerPrivateAccessDependent( *(work.getParent()->getDirectory(true)), tag, cd.getSize(), cd.isInput(), cd.isOutput() );
   //   } else {
   //      this->registerCacheAccessDependent( *(work.getParent()->getDirectory(true)), tag, cd.getSize(), cd.isInput(), cd.isOutput() );
   //   }
   //}
#if LOCK_TRANSFER
   _transferLock.release();
#endif
}

void Accelerator::waitInputs( WorkDescriptor &work )
{
   //CopyData *copies = work.getCopies();
   //for ( unsigned int i = 0; i < work.getNumCopies(); i++ ) {
   //   CopyData & cd = copies[i];
   //   uint64_t tag = (uint64_t) cd.isPrivate() ? ((uint64_t) work.getData() + (unsigned long)cd.getAddress()) : cd.getAddress();
   //   if ( cd.isInput() ) {
   //        this->waitInputDependent( tag );
   //   }
   //}

   this->waitInputsDependent( work );
}

void Accelerator::copyDataOut( WorkDescriptor& work )
{
#if LOCK_TRANSFER
   _transferLock.acquire();
#endif
#if LOCK_TRANSFER
   _transferLock.release();
#endif
}

//void* Accelerator::getAddress( WorkDescriptor &wd, uint64_t tag, nanos_sharing_t sharing )
//{
//   uint64_t actualTag = (uint64_t) ( sharing == NANOS_PRIVATE ? (uint64_t) wd.getData() + (unsigned long) tag : tag );
//   return getAddressDependent( actualTag );
//}

#if 0
void* Accelerator::newGetAddress( CopyData const &cd )
{
   //uint64_t actualTag = (uint64_t) ( sharing == NANOS_PRIVATE ? (uint64_t) wd.getData() + (unsigned long) tag : tag );
   return newGetAddressDependent( cd );
}
#endif

#if 0
void Accelerator::copyTo( WorkDescriptor &wd, void *dst, uint64_t tag, nanos_sharing_t sharing, size_t size )
{
   uint64_t actualTag = (uint64_t) ( sharing == NANOS_PRIVATE ? (uint64_t) wd.getData() + (unsigned long) tag : tag );
   copyToDependent( dst, actualTag, size );
}
#endif
