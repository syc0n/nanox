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

#ifndef _NANOS_SMP_PROCESSOR
#define _NANOS_SMP_PROCESSOR

#include "config.hpp"
#include "smpthread.hpp"
#include "cache.hpp"
#include "smpmemory.hpp"
#ifdef SMP_NUMA
#include "accelerator.hpp"
#else
#include "processingelement.hpp"
#endif

//TODO: Make smp independent from pthreads? move it to OS?

namespace nanos {
namespace ext
{

#ifdef SMP_NUMA
   class SMPProcessor : public Accelerator
#else
   class SMPProcessor : public PE
#endif
   {


      private:
         // config variables
         static bool _useUserThreads;
         static size_t _threadsStackSize;

         // disable copy constructor and assignment operator
         SMPProcessor( const SMPProcessor &pe );
         const SMPProcessor & operator= ( const SMPProcessor &pe );

#ifdef SMP_NUMA
         Cache<SMPMemory> _cache;
#endif

      public:
         // constructors
#ifdef SMP_NUMA
         SMPProcessor( int id ) : Accelerator( id,&SMP ), _cache() {}
#else
         SMPProcessor( int id ) : PE( id,&SMP ) {}
#endif

         virtual ~SMPProcessor() {}

         virtual WD & getWorkerWD () const;
         virtual WD & getMasterWD () const;
         virtual BaseThread & createThread ( WorkDescriptor &wd );

         static void prepareConfig ( Config &config );

         // capability query functinos
         virtual bool supportsUserLevelThreads () const { return _useUserThreads; }
#ifdef SMP_NUMA
         virtual bool hasSeparatedMemorySpace() const { return true; }
         /* Memory space suport */
         virtual void registerDataAccessDependent( void *tag, size_t size );
         virtual void copyDataDependent( void *tag, size_t size );
         virtual void unregisterDataAccessDependent( void *tag );
         virtual void copyBackDependent( void *tag, size_t size );
         virtual void* getAddressDependent( void* tag );
         virtual void copyToDependent( void* dst, void *tag, size_t size );
#else
         virtual bool hasSeparatedMemorySpace() const { return false; }
#endif
   };

}
}

#endif
