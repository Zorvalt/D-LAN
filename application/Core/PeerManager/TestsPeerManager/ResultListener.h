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
  
#ifndef TESTS_PEERMANAGER_RESULTLISTENER_H
#define TESTS_PEERMANAGER_RESULTLISTENER_H

#include <QObject>
#include <QSharedPointer>

#include <Protos/core_protocol.pb.h>

#include <Common/Hash.h>

#include <ISocket.h>
using namespace PM;

class ResultListener : public QObject
{
   Q_OBJECT
public:
   ResultListener();

   QList<Protos::Core::GetEntriesResult> getEntriesResultList() const;
   int getNbEntriesResultReceived(int n) const;

public slots:
   void entriesResult(const Protos::Core::GetEntriesResult& result);

   void result(const Protos::Core::GetHashesResult& result);
   void nextHash(const Common::Hash& hash);

   void result(const Protos::Core::GetChunkResult& result);
   void stream(QSharedPointer<PM::ISocket> socket);
   void getChunk(Common::Hash hash, int offset, QSharedPointer<PM::ISocket> socket);

private:
   QList<Protos::Core::GetEntriesResult> entriesResultList;
   int nbHashes;
   int currentHash;
};

#endif
