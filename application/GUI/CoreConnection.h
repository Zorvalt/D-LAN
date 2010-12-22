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
  
#ifndef GUI_CORECONNECTION_H
#define GUI_CORECONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QHostInfo>
#include <QSharedPointer>
#include <QProcess>

#include <Protos/gui_protocol.pb.h>
#include <Protos/common.pb.h>

#include <Common/Timeoutable.h>
#include <Common/Network.h>
#include <Common/LogManager/IEntry.h>

namespace GUI
{
   class CoreConnection;

   class IBrowseResult : public Common::Timeoutable
   {
      Q_OBJECT
   protected:
      IBrowseResult(int time) : Common::Timeoutable(time) {}

   public:
      virtual ~IBrowseResult() {}
      virtual void start() = 0;

   signals:
      void result(const google::protobuf::RepeatedPtrField<Protos::Common::Entries>&);
   };

   class ISearchResult : public Common::Timeoutable
   {
      Q_OBJECT
   protected:
      ISearchResult(int time) : Common::Timeoutable(time) {}

   public:
      virtual ~ISearchResult() {}
      virtual void start() = 0;

   signals:
      void result(const Protos::Common::FindResult&);
   };

   class BrowseResult : public IBrowseResult
   {
      Q_OBJECT
   public:
      BrowseResult(CoreConnection* coreConnection, const Common::Hash& peerID);
      BrowseResult(CoreConnection* coreConnection, const Common::Hash& peerID, const Protos::Common::Entry& entry);
      BrowseResult(CoreConnection* coreConnection, const Common::Hash& peerID, const Protos::Common::Entries& entries, bool withRoots = true);
      void start();
      void setTag(quint64 tag);

   private slots:
      void browseResult(const Protos::GUI::BrowseResult& browseResult);

   private:
      void init(CoreConnection* coreConnection);

      CoreConnection* coreConnection;
      const Common::Hash peerID;
      Protos::GUI::Browse browseMessage;
      quint64 tag;
   };

   class SearchResult : public ISearchResult
   {
      Q_OBJECT
   public:
      SearchResult(CoreConnection* coreConnection, const QString& terms);
      void start();
      void setTag(quint64 tag);

   private slots:
      void searchResult(const Protos::Common::FindResult& findResult);

   private:
      CoreConnection* coreConnection;
      const QString terms;
      quint64 tag;
   };

   class CoreConnection : public QObject
   {
      Q_OBJECT
   public:
      CoreConnection();
      ~CoreConnection();

      Common::Hash getOurID() const;
      void sendChatMessage(const QString& message);
      void setCoreSettings(const Protos::GUI::CoreSettings settings);

      QSharedPointer<IBrowseResult> browse(const Common::Hash& peerID);
      QSharedPointer<IBrowseResult> browse(const Common::Hash& peerID, const Protos::Common::Entry& entry);
      QSharedPointer<IBrowseResult> browse(const Common::Hash& peerID, const Protos::Common::Entries& entries, bool withRoots = true);

      QSharedPointer<ISearchResult> search(const QString& terms);

      void download(const Common::Hash& peerID, const Protos::Common::Entry& entry);
      void cancelDownloads(const QList<quint64>& downloadIDs);
      void moveDownloads(quint64 downloadIDRef, const QList<quint64>& downloadIDs, bool moveBefore = true);
      void refresh();

      bool isConnected();

   public slots:
      void connectToCore();

   signals:
      void coreConnected();
      void coreDisconnected();

      void newState(const Protos::GUI::State&);
      void newChatMessage(const Common::Hash& peerID, const QString& message);
      void newLogMessage(QSharedPointer<const LM::IEntry> entry);
      void browseResult(const Protos::GUI::BrowseResult& browseResult);
      void searchResult(const Protos::Common::FindResult& findResult);

   private slots:
      void stateChanged(QAbstractSocket::SocketState socketState);
      void dataReceived();
      void adressResolved(QHostInfo hostInfo);
      void connected();
      void disconnected();

   private:
      friend class BrowseResult;
      friend class SearchResult;

      void tryToConnectToTheNextAddress();

      void send(Common::Network::GUIMessageType type);
      void send(Common::Network::GUIMessageType type, const google::protobuf::Message& message);
      bool readMessage();

      QTcpSocket socket;
      Common::Hash ourID;
      Common::Network::MessageHeader<Common::Network::GUIMessageType> currentHeader;

      int currentHostLookupID;

      QList<QHostAddress> addressesToTry; // When a name is resolved many addresses can be returned, we will try all of them until a connection is successfuly established.

      QList< QSharedPointer<BrowseResult> > browseResultsWithoutTag;
      QList< QSharedPointer<SearchResult> > searchResultsWithoutTag;
      bool authenticated;
   };
}

#endif
