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

#ifndef _NANOS_SLICER
#define _NANOS_SLICER

#include "workdescriptor.hpp"
#include "schedule.hpp"
#include "nanos-int.h"
#include "slicer_fwd.hpp"
#include <list>                                                                                                                                          

namespace nanos
{

   class Slicer
   {
      private:
         /*! \brief Slicer copy constructor (disabled)
          */
         Slicer ( const Slicer &s );
         /*! \brief Slicer copy assignment operator
          */
         Slicer & operator= ( const Slicer &s );
      public:
         /*! \brief Slicer default constructor
          */
         Slicer () { }
         /*! \brief Slicer destructor
          */
         virtual ~Slicer () { }
         /*! \brief Submit a WorkDescriptor (pure virtual)
          */
         virtual void submit ( SlicedWD &work ) = 0;
         /*! \brief Dequeue on a WorkDescriptor getting a slice (pure virtual)
          */
         virtual bool dequeue ( SlicedWD *wd, WorkDescriptor **slice ) = 0;
         /*! \brief Get Slicer specific data 
          */
         virtual void *getSpecificData ( ) const { return NULL; }
   };

   class SlicerData
   {
      private:
         /*! \brief SlicerData copy constructor (disabled)
          */
         SlicerData ( const SlicerData &sd );
         /*! \brief SlicerData copy assignment operator (disabled)
          */
         SlicerData & operator= ( const SlicerData &sd );
      public:
         /*! \brief SlicerData default constructor
          */
         SlicerData ( ) { }
         /*! \brief SlicerData destructor
          */
         ~SlicerData ( ) { }
   };

   class SlicedWD : public WD
   {
      private:
         Slicer      &_slicer;         /**< Related Slicer     */
         size_t       _slicerDataSize; /**< SlicerData size    */
         SlicerData  &_slicerData;     /**< Related SlicerData */
      private:
         /*! \brief SlicedWD default constructor (disabled)
          */
         SlicedWD ();
         /*! \brief SlicedWD copy constructor (disabled)
          */
         SlicedWD ( const SlicedWD &swd );
         /*! \brief SlicedWD copy assignment operator (disabled) 
          */
         SlicedWD & operator= ( const SlicedWD &swd );
      public:
         /*! \brief SlicedWD constructor - n devices
          */
          SlicedWD ( Slicer &slicer, size_t sdata_size, SlicerData &sdata, int ndevices, DeviceData **devs,
                     size_t data_size, void *wdata=0, size_t numCopies=0, CopyData *copies=NULL )
             : WorkDescriptor ( ndevices, devs, data_size, wdata, numCopies, copies ),
               _slicer(slicer), _slicerDataSize(sdata_size), _slicerData(sdata)  {}
         /*! \brief SlicedWD constructor - 1 device
          */
          SlicedWD ( Slicer &slicer, size_t sdata_size, SlicerData &sdata, DeviceData *device,
                     size_t data_size, void *wdata=0, size_t numCopies=0, CopyData* copies=NULL )
             : WorkDescriptor ( device, data_size, wdata, numCopies, copies ),
               _slicer(slicer), _slicerDataSize(sdata_size), _slicerData(sdata)  {}
         /*! \brief SlicedWD constructor - from wd
          */
          SlicedWD ( Slicer &slicer, size_t sdata_size, SlicerData &sdata, WD &wd,
                      DeviceData **device, CopyData *copies, void *wdata=0 )
             : WorkDescriptor ( wd, device, copies, wdata),
               _slicer(slicer), _slicerDataSize(sdata_size), _slicerData(sdata)  {}
         /*! \brief SlicedWD destructor
          */
         ~SlicedWD  ( ) { }
         /*! \brief Get related slicer
          */
         Slicer * getSlicer ( void ) const { return &_slicer; }
         /*! \brief Get SlicerData size, for duplicating purposes
          */
         size_t getSlicerDataSize ( void ) const { return _slicerDataSize; }
         /*! \brief Get SlicerData
          */
         SlicerData * getSlicerData ( void ) const { return &_slicerData; }
         /*! \brief WD submission
          *
          *  This function calls the specific code for WD submission which is
          *  implemented in the related slicer.
          */ 
         void submit () { _slicer.submit(*this); }
         /*! \brief WD dequeue
          *
          *  This function calls the specific code for WD dequeue which is
          *  implemented in the related slicer.
          *
          *  \param[in,out] slice : Resulting slice.
          *  \return  true if the resulting slice is the final slice and false otherwise.
          */ 
         bool dequeue ( WorkDescriptor **slice ) { return _slicer.dequeue( this, slice ); }
   };

   class SlicerDataRepeatN : public SlicerData
   {
      private:
         int _n; /**< Number of Repetitions */
      private:
         /*! \brief SlicerDataRepeatN default construcotr (disableD)
          */
         SlicerDataRepeatN ();
         /*! \brief SlicerDataRepeatN copy constructor (disableD)
          */
         SlicerDataRepeatN ( const SlicerDataRepeatN &sdrp );
         /*! \brief SlicerDataRepeatN copy assignment operator (disableD)
          */
         SlicerDataRepeatN & operator= ( const SlicerDataRepeatN &sdrp );
      public:
         /*! \brief SlicerDataRepeatN constructor
          */
         SlicerDataRepeatN ( int n) : _n (n) { }
         /*! \brief SlicerDataRepeatN destructor
          */
         ~SlicerDataRepeatN ( ) { }
         /*! \brief Set N
          */
         void setN ( int n ) { _n = n; }
         /*! \brief Get N
          */
         int getN ( void ) const { return _n; }
         /*! \brief Decrement internal counter by one
          *
          *  This function decrements the internal variable counter by one
          *
          *  \return Internal counter after decrementing its value
          */ 
         int decN () { return --_n; }
   };

   class SlicerDataFor : public nanos_slicer_data_for_internal_t, public SlicerData
   {
         /* int _lower: Loop lower bound */
         /* int _upper: Loop upper bound */
         /* int _step: Loop step */
         /* int _chunk: Slice chunk */
         /* int _sign: Loop sign 1 ascendant, -1 descendant */
      private:
         /*! \brief SlicerDataFor default constructor (disabled)
          */
         SlicerDataFor ();
         /*! \brief SlicerDataFor copy constructor (disabled)
          */
         SlicerDataFor ( const SlicerDataFor &sdf );
         /*! \brief SlicerDataFor copy assignment operator (disabled)
          */
         SlicerDataFor& operator= ( const SlicerDataFor &sdf );
      public:
         /*! \brief SlicerDataFor constructor
          */
         SlicerDataFor ( int lower, int upper, int step, int chunk = 1 )
         {
            _lower = lower;
            _upper = upper;
            _step = step;
            _chunk = chunk; 
            _sign = ( step < 0 ) ? -1 : +1;
         }
         /*! \brief SlicerDataFor destructor
          */
         ~SlicerDataFor ( ) { }
         /*! \brief Set lower bound
          */
         void setLower ( int n ) { _lower = n; }
         /*! \brief Set upper bound
          */
         void setUpper ( int n ) { _upper = n; }
         /*! \brief Set step
          */
         void setStep  ( int n ) {  _step = n; }
         /*! \brief Set chunk size
          */
         void setChunk ( int n ) { _chunk = n; }
         /*! \brief Set increment sign
          */
         void setSign  ( int n ) { _sign = n; }
         /*! \brief Get lower bound
          */
         int getLower ( void ) const { return _lower; }
         /*! \brief Get upper bound
          */
         int getUpper ( void ) const { return _upper; }
         /*! \brief Get step
          */
         int getStep  ( void ) const { return _step; }
         /*! \brief Get chunk size
          */
         int getChunk ( void ) const { return _chunk; }
         /*! \brief Get increment sign
          */
         int getSign  ( void ) const { return _sign; }
   };

};

#endif

