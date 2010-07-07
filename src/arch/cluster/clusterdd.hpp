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

#ifndef _NANOS_CLUSTER_WD
#define _NANOS_CLUSTER_WD

#include "config.hpp"
#include "clusterdevice.hpp"
#include "workdescriptor.hpp"

namespace nanos {
namespace ext
{

   extern ClusterDevice Cluster;

   class ClusterPlugin;
   class ClusterDD : public DD
   {
      friend class ClusterPlugin;
      public:
         typedef void ( *work_fct ) ( void *self );

      private:
         work_fct       _work;

      public:
         // constructors
         ClusterDD( work_fct w ) : DD( &Cluster ), _work( w ) {}

         ClusterDD() : DD( &Cluster ), _work( 0 ) {}

         // copy constructors
         ClusterDD( const ClusterDD &dd ) : DD( dd ), _work( dd._work ) {}

         // assignment operator
         const ClusterDD & operator= ( const ClusterDD &wd );

         // destructor
         virtual ~ClusterDD() { }

         work_fct getWorkFct() const { return _work; }

         virtual void lazyInit (WD &wd, bool isUserLevelThread, WD *previous) { }
         virtual size_t size ( void ) { return sizeof(ClusterDD); }
         virtual ClusterDD *copyTo ( void *toAddr );
   };

   inline const ClusterDD & ClusterDD::operator= ( const ClusterDD &dd )
   {
      // self-assignment: ok
      if ( &dd == this ) return *this;

      DD::operator= ( dd );
      _work = dd._work;

      return *this;
   }
}
}

#endif
