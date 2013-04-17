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

#ifndef _NANOS_COPY_DESCRIPTOR_DECL
#define _NANOS_COPY_DESCRIPTOR_DECL

#include "compatibility.hpp"

namespace nanos {

   /*! \breif Class representing a copy used to synchronize the device with the cache */
   class CopyDescriptor : public nanos_copy_descriptor_internal_t
   {
      public:
        /*! \brief Default constructor
         */
         CopyDescriptor( uint64_t t, unsigned int dv = 0, bool copy = false, bool flush = false )
         {
            tag = t;
            dirVersion = dv;
            copying = copy;
            flushing = flush;
         }

        /*! \brief Copy constructor
         *  \param Another CopyDescriptor
         */
         CopyDescriptor( const CopyDescriptor &cd )
         {
            tag = cd.tag;
            dirVersion = cd.dirVersion;
            copying = cd.copying;
            flushing = cd.flushing;
         }

        /* \brief Destructor
         */
         ~CopyDescriptor() {}

        /* \brief Assign operator
         */
         CopyDescriptor& operator=( const CopyDescriptor &cd )
         {
            if ( this == &cd ) return *this;
            this->tag = cd.tag;
            this->dirVersion = cd.dirVersion;
            this->copying = cd.copying;
            this->flushing = cd.flushing;
            return *this;
         }

         uint64_t getTag() const;
         unsigned int getDirectoryVersion() const;
         bool isCopying () const;
         bool isFlushing () const;
   };
}

#endif

