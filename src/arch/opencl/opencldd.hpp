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

#ifndef _NANOS_OpenCL_WD
#define _NANOS_OpenCL_WD

#include "workdescriptor.hpp"
#include "debug.hpp"
#include "opencldevice_decl.hpp"

namespace nanos {
namespace ext {
    
    

extern OpenCLDevice OpenCLDev;

// OpenCL back-end Device Description.
//
// Since the OpenCL back-end is slightly different from a normal back-end, we
// have a set of specialized Device Description. This is the root of the class
// hierarchy.
//
// Every Device Description must have the following layout in memory:
//
// +------------------+
// | Custom section   |
// +------------------+
// | Number of events |
// +------------------+
// | Input event 1    |
// | ...              |
// | Input event n    |
// +------------------+
// | Output event     |
// +------------------+ -+-
// | Start tick       |  | Profiling section
// | End tick         |  | (optional)
// +------------------+ -+-
//
// The OLCDD class contains the EventIterator class that can be use to iterate
// over events given the custom section size. It is Device Description builder
// responsibility to push arguments in the right order!

class OpenCLDD : public DD
   {
    
      public:
         typedef void ( *work_fct ) ( void *self );

      private:
         work_fct       _work;

      public:
         // constructors
         OpenCLDD( work_fct w ) : DD( &OpenCLDev ), _work( w ) {}

         OpenCLDD() : DD( &OpenCLDev ), _work( 0 ) {}

         // copy constructors
         OpenCLDD( const OpenCLDD &dd ) : DD( dd ), _work( dd._work ) {}

         // assignment operator
         const OpenCLDD & operator= ( const OpenCLDD &wd );

         // destructor
         virtual ~OpenCLDD() { }

         work_fct getWorkFct() const { return _work; }

         virtual void lazyInit (WD &wd, bool isUserLevelThread, WD *previous) { }
         virtual size_t size ( void ) { return sizeof(OpenCLDD); }
         virtual OpenCLDD *copyTo ( void *toAddr );
         virtual OpenCLDD *clone () const { return NEW OpenCLDD ( *this); }
   };

   inline const OpenCLDD & OpenCLDD::operator= ( const OpenCLDD &dd )
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