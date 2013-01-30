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


#ifndef _NANOX_NETWORK
#define _NANOX_NETWORK

#include <string>
#include "requestqueue_decl.hpp"
#include "networkapi.hpp"

namespace nanos {
         class SendDataRequest {
            protected:
            NetworkAPI *_api;
            void *_origAddr;
            void *_destAddr;
            std::size_t _len;
            std::size_t _count;
            std::size_t _ld;
            public:
            SendDataRequest( NetworkAPI *api, void *origAddr, void *destAddr, std::size_t len, std::size_t count, std::size_t ld );
            virtual ~SendDataRequest();
            void doSend();
            void *getOrigAddr() const;
            virtual void doSingleChunk() = 0;
            virtual void doStrided( void *localAddr ) = 0;
         };

   class Network
   {
      private:
         unsigned int _numNodes;
         NetworkAPI *_api; 
         unsigned int _nodeNum;

         //std::string _masterHostname;
         char * _masterHostname;
         bool _checkForDataInOtherAddressSpaces;

         class ReceivedWDData {
            private:
               struct recvDataInfo {
                  recvDataInfo() : _wd( NULL ), _count( 0 ), _expected( 0 ) { }
                  WorkDescriptor *_wd;
                  std::size_t _count;
                  std::size_t _expected;
               };
               std::map< unsigned int, struct recvDataInfo > _recvWdDataMap;
               Lock _lock;
               Atomic<unsigned int> _receivedWDs;
            public:
            ReceivedWDData();
            ~ReceivedWDData();
            void addData( unsigned int wdId, std::size_t size );
            void addWD( unsigned int wdId, WorkDescriptor *wd, std::size_t expectedData );
            unsigned int getReceivedWDsCount() const;
         };

         class SentWDData {
            private:
               std::map< unsigned int, std::size_t > _sentWdDataMap;
               Lock _lock;
            public:
            SentWDData();
            ~SentWDData();
            void addSentData( unsigned int wdId, std::size_t sentData );
            std::size_t getSentData( unsigned int wdId );
         };

         ReceivedWDData _recvWdData;
         SentWDData _sentWdData;

         std::list< std::pair< unsigned int, std::pair<WD *, std::size_t> > > _deferredWorkReqs;
         Lock _deferredWorkReqsLock;
         Atomic<unsigned int> _recvSeqN;

         void checkDeferredWorkReqs();

         Lock _waitingPutRequestsLock;
         std::set< void * > _waitingPutRequests;
         std::set< void * > _receivedUnmatchedPutRequests;


      public:
         static const unsigned int MASTER_NODE_NUM = 0;
         typedef struct {
            int complete;
            void * resultAddr;
         } mallocWaitObj;


         std::list< SendDataRequest * > _delayedPutReqs;
         Lock _delayedPutReqsLock;

         RequestQueue< SendDataRequest > _dataSendRequests;

         // constructor

         Network ();
         ~Network ();

         void setAPI ( NetworkAPI *api );
         NetworkAPI *getAPI ();
         void setNumNodes ( unsigned int numNodes );
         unsigned int getNumNodes () const;
         void setNodeNum ( unsigned int nodeNum );
         unsigned int getNodeNum () const;
         void notifyWorkDone ( unsigned int nodeNum, void *remoteWdAddr, int peId );
         void notifyMalloc ( unsigned int nodeNum, void * result, mallocWaitObj *waitObjAddr );

         void initialize ( void );
         void finalize ( void );
         void poll ( unsigned int id );
         void sendExitMsg( unsigned int nodeNum );
         void sendWorkMsg( unsigned int dest, void ( *work ) ( void * ), unsigned int arg0, unsigned int arg1, unsigned int numPe, std::size_t argSize, char * arg, void ( *xlate ) ( void *, void * ), int arch, void *remoteWdAddr );
         bool isWorking( unsigned int dest, unsigned int numPe ) const;
         void sendWorkDoneMsg( unsigned int nodeNum, void *remoteWdaddr, int peId );
         void put ( unsigned int remoteNode, uint64_t remoteAddr, void *localAddr, std::size_t size, unsigned int wdId, WD const &wd );
         void putStrided1D ( unsigned int remoteNode, uint64_t remoteAddr, void *localAddr, void *localPack, std::size_t size, std::size_t count, std::size_t ld, unsigned int wdId, WD const &wd );
         void get ( void *localAddr, unsigned int remoteNode, uint64_t remoteAddr, std::size_t size, volatile int *req );
         void getStrided1D ( void *packedAddr, unsigned int remoteNode, uint64_t remoteTag, uint64_t remoteAddr, std::size_t size, std::size_t count, std::size_t ld, volatile int *req );
         void *malloc ( unsigned int remoteNode, std::size_t size );
         void memFree ( unsigned int remoteNode, void *addr );
         void memRealloc ( unsigned int remoteNode, void *oldAddr, std::size_t oldSize, void *newAddr, std::size_t newSize );
         void nodeBarrier( void );

         void setMasterHostname( char *name );
         //const std::string & getMasterHostname( void ) const;
         const char * getMasterHostname( void ) const;
         void sendRequestPut( unsigned int dest, uint64_t origAddr, unsigned int dataDest, uint64_t dstAddr, std::size_t len, unsigned int wdId, WD const &wd );
         void sendRequestPutStrided1D( unsigned int dest, uint64_t origAddr, unsigned int dataDest, uint64_t dstAddr, std::size_t len, std::size_t count, std::size_t ld, unsigned int wdId, WD const &wd );
         //void setNewMasterDirectory(NewRegionDirectory *dir);
         std::size_t getTotalBytes();
         void mallocSlaves ( void **addresses, std::size_t size );

         static Lock _nodeLock;
         static Atomic<uint64_t> _nodeRCAaddr;
         static Atomic<uint64_t> _nodeRCAaddrOther;

	 void enableCheckingForDataInOtherAddressSpaces();
	 bool doIHaveToCheckForDataInOtherAddressSpaces() const;

         SimpleAllocator *getPackerAllocator() const;
         std::size_t getMaxGetStridedLen() const;

         void *allocateReceiveMemory( std::size_t len );
         void freeReceiveMemory( void * addr );

         void notifyWork( std::size_t expectedData, WD *delayedWD, unsigned int delayedSeq);
         void notifyPut( unsigned int from, unsigned int wdId, std::size_t totalLen, uint64_t realTag );
         void notifyWaitRequestPut( void *addr );
         void notifyRequestPut( SendDataRequest *req );
         void notifyGet( SendDataRequest *req );
void invalidateDataFromDevice( uint64_t addr, std::size_t len );
void getDataFromDevice( uint64_t addr, std::size_t len );
   };
}

#endif
