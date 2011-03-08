/**
  * Aybabtu - A decentralized LAN file sharing software.
  * Copyright (C) 2010-2011 Greg Burri <greg.burri@gmail.com>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
  
#ifndef UPLOADMANAGER_UPLOADER_H
#define UPLOADMANAGER_UPLOADER_H

#include <QThread>
#include <QSharedPointer>
#include <QTimer>
#include <QMutex>

#include <Common/Uncopyable.h>
#include <Common/TransferRateCalculator.h>
#include <Core/FileManager/IChunk.h>
#include <Core/FileManager/IDataReader.h>
#include <Core/PeerManager/ISocket.h>

#include <IUpload.h>

namespace UM
{
   class Uploader : public QThread, public IUpload, Common::Uncopyable
   {
      Q_OBJECT
      static quint64 currentID; ///< Used to generate the new upload ID.

   public:
      Uploader(QSharedPointer<FM::IChunk> chunk, int offset, QSharedPointer<PM::ISocket> socket, Common::TransferRateCalculator& transferRateCalculator);
      ~Uploader();

      quint64 getID() const;
      Common::Hash getPeerID() const;
      int getProgress() const;
      QSharedPointer<FM::IChunk> getChunk() const;
      QSharedPointer<PM::ISocket> getSocket() const;
      void startTimer();

   signals:
      void uploadFinished(bool error);
      void uploadTimeout();

   protected:
      void run();

   private:
      const quint64 ID; ///< Each uploader has an ID to identified it.
      QSharedPointer<FM::IChunk> chunk; ///< The chunk uploaded.
      int offset; ///< The current offset into the chunk.
      QSharedPointer<PM::ISocket> socket; ///< The socket to send data.
      QTimer timer; ///< Timer to enable a timeout for the uploader. See the settings "upload_live_time".
      mutable QMutex mutex; ///< A mutex to protect the 'offset' data member.
      Common::TransferRateCalculator& transferRateCalculator; /// To compute the transfer rate.
      QThread* mainThread;
   };
}
#endif
