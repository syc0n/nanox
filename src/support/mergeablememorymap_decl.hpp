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
#ifndef _NANOS_MERGEABLE_MEMORYMAP_DECL_H
#define _NANOS_MERGEABLE_MEMORYMAP_DECL_H

#include "memorymap_decl.hpp"

namespace nanos {
   template <typename _Type>
      class MergeableMemoryMap : public MemoryMap< _Type > { 
         public:
            typedef typename MemoryMap< _Type >::const_iterator const_iterator;
            typedef typename MemoryMap< _Type >::iterator iterator;
            typedef typename MemoryMap< _Type>::MemChunkPair MemChunkPair;
            typedef typename MemoryMap< _Type>::MemChunkList MemChunkList;
            typedef typename MemoryMap< _Type>::ConstMemChunkPair ConstMemChunkPair;
            typedef typename MemoryMap< _Type>::ConstMemChunkList ConstMemChunkList;
            typedef typename MemoryMap< _Type>::BaseMap BaseMap;
            void merge( const MemoryMap< _Type > &mm );
            void merge2( const MemoryMap< _Type > &mm );
      };
};

#endif /* _NANOS_MERGEABLE_MEMORYMAP_DECL_H */
