#include <priv/ChunkDownload.h>
using namespace DM;

#include <windows.h>
#include <winsock2.h>

#include <QMutexLocker>

#include <Common/Settings.h>
#include <Core/FileManager/Exceptions.h>
#include <Core/PeerManager/IPeer.h>

#include <priv/Log.h>

ChunkDownload::ChunkDownload(QSharedPointer<PM::IPeerManager> peerManager, OccupiedPeers& occupiedPeersDownloadingChunk, Common::Hash chunkHash)
   : peerManager(peerManager), occupiedPeersDownloadingChunk(occupiedPeersDownloadingChunk), chunkHash(chunkHash), socket(0), downloading(false)
{
   connect(this, SIGNAL(finished()), this, SLOT(downloadingEnded()), Qt::QueuedConnection);
   this->mainThread = QThread::currentThread();
}

Common::Hash ChunkDownload::getHash()
{
   return this->chunkHash;
}

void ChunkDownload::setPeerIDs(const QList<Common::Hash>& peerIDs)
{
   this->peers.clear();
   for (QListIterator<Common::Hash> i(peerIDs); i.hasNext();)
   {
      PM::IPeer* peer = this->peerManager->getPeer(i.next());
      if (peer)
      {
         this->peers << peer;
         this->occupiedPeersDownloadingChunk.newPeer(peer);
      }
   }
}

void ChunkDownload::setChunk(QSharedPointer<FM::IChunk> chunk)
{
   this->chunk = chunk;
   this->chunk->setHash(this->chunkHash);
}

void ChunkDownload::setPeerSource(PM::IPeer* peer)
{
   if (!this->peers.contains(peer))
   {
      this->peers << peer;
      this->occupiedPeersDownloadingChunk.newPeer(peer);
   }
}

/**
  * To be ready :
  * - It must be have at least one peer.
  * - It isn't finished.
  * - It isn't currently downloading.
  */
bool ChunkDownload::isReadyToDownload()
{
   if (this->peers.isEmpty() || this->downloading || (!this->chunk.isNull() && this->chunk->isComplete()))
      return false;

   this->currentDownloadingPeer = this->getTheFastestFreePeer();

   return this->currentDownloadingPeer != 0;
}

/**
  * Tell the chunkDownload to download the chunk from one of its peer.
  */
void ChunkDownload::startDownloading()
{
   this->downloading = true;
   this->occupiedPeersDownloadingChunk.setPeerAsOccupied(this->currentDownloadingPeer);

   Protos::Core::GetChunk getChunkMess;
   getChunkMess.mutable_chunk()->set_hash(this->chunkHash.getData(), Common::Hash::HASH_SIZE);
   getChunkMess.set_offset(this->chunk->getKnownBytes());
   this->getChunkResult = this->currentDownloadingPeer->getChunk(getChunkMess);
   connect(this->getChunkResult.data(), SIGNAL(result(const Protos::Core::GetChunkResult&)), this, SLOT(result(const Protos::Core::GetChunkResult&)));
   connect(this->getChunkResult.data(), SIGNAL(stream(PM::ISocket*)), this, SLOT(stream(PM::ISocket*)));

   this->getChunkResult->start();
}

void ChunkDownload::run()
{
   QSharedPointer<FM::IDataWriter> writer;

   try
   {
      L_DEBU(QString("Starting downloading a chunk : %1").arg(this->chunk->toStr()));

      QSharedPointer<FM::IDataWriter> writer = this->chunk->getDataWriter();

      const int BUFFER_SIZE = SETTINGS.getUInt32("buffer_size");
      char buffer[BUFFER_SIZE];

      int bytesRead = 0;
      int bytesReadTotal = 0;
      int socketDescriptor = socket->getQSocket()->socketDescriptor();

      //while (bytesRead = recv(socketDescriptor, buffer, BUFFER_SIZE, 0x8))
      forever
      {
         socket->getQSocket()->waitForReadyRead();
         bytesRead = socket->getQSocket()->read(buffer, BUFFER_SIZE);

         if (bytesRead == -1) //SOCKET_ERROR)
         {
            L_ERRO(QString("Socket : cannot receive data"));
            break;
         }
         bytesReadTotal += bytesRead;

         //L_WARN(QString(QByteArray(buffer, bytesRead).toHex()));
         writer->write(buffer, bytesRead);

         if (bytesReadTotal >= this->chunkSize)
            break;
      }

      //connect(this->writer.data(), SIGNAL(writeFinished(FM::IDataWriter::Status)), this, SLOT(writeFinished(FM::IDataWriter::Status)), Qt::QueuedConnection);
      /*connect(this->socket->getQSocket(), SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
      connect(this->socket->getQSocket(), SIGNAL(disconnected()), this, SLOT(socketDisconnected()));*/
   }
   catch(FM::UnableToOpenFileInWriteModeException)
   {
      L_ERRO("UnableToOpenFileInWriteModeException");
   }
   catch(FM::IOErrorException&)
   {
      L_ERRO("IOErrorException");
   }
   catch (FM::ChunkDeletedException&)
   {
      L_ERRO("ChunkDeletedException");
   }
   catch (FM::TryToWriteBeyondTheEndOfChunkException&)
   {
      L_ERRO("TryToWriteBeyondTheEndOfChunkException");
   }
   this->socket->getQSocket()->moveToThread(this->mainThread);
}

void ChunkDownload::result(const Protos::Core::GetChunkResult& result)
{
   if (result.status() != Protos::Core::GetChunkResult_Status_OK)
   {
      L_WARN(QString("Status error from GetChunkResult : %1. Download aborted.").arg(result.status()));
      this->downloadingEnded();
   }
   else
   {
      if (!result.has_chunk_size())
      {
         L_ERRO(QString("Message 'GetChunkResult' doesn't contain the size of the chunk : %1. Download aborted.").arg(this->chunk->getHash().toStr()));
         this->downloadingEnded();
      }
      else
         this->chunkSize = result.chunk_size();
   }
}

void ChunkDownload::stream(PM::ISocket* socket)
{
   this->socket = socket;
   this->socket->getQSocket()->moveToThread(this);
   this->start();
}

/*
void ChunkDownload::socketReadyRead()
{
   this->downloadNextData();
}

void ChunkDownload::socketDisconnected()
{
   this->downloadingEnded();
}*/

void ChunkDownload::downloadingEnded()
{
   L_DEBU(QString("Downloading ended, chunk : %1").arg(this->chunk->toStr()));
   if (this->socket)
   {
      // Empty the socket.
      while (!this->socket->getQSocket()->readAll().isNull());
      this->socket->getQSocket()->moveToThread(QThread::currentThread());

      /*disconnect(this->socket->getQSocket(), SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
      disconnect(this->socket->getQSocket(), SIGNAL(disconnected()), this, SLOT(socketDisconnected()));*/
      this->socket->finished();
      this->socket = 0;
   }

   this->getChunkResult.clear();
   this->downloading = false;
   emit downloadFinished();
   this->occupiedPeersDownloadingChunk.setPeerAsFree(this->currentDownloadingPeer);
   this->currentDownloadingPeer = 0;
}

PM::IPeer* ChunkDownload::getTheFastestFreePeer() const
{
   PM::IPeer* current = 0;
   foreach (PM::IPeer* peer, this->peers)
   {
      if (this->occupiedPeersDownloadingChunk.isPeerFree(peer) && (!current || peer->getSpeed() > current->getSpeed()))
         current = peer;
   }
   return current;
}
