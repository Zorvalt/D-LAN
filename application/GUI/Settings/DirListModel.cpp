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
  
#include <Settings/DirListModel.h>
using namespace GUI;

void DirListModel::setDirs(const QStringList& dirs)
{
   QStringList dirsToRemove = this->dirs;

   for (QStringListIterator i(dirs); i.hasNext();)
   {
      QString dir = i.next();
      if (this->dirs.contains(dir))
      {
         dirsToRemove.removeOne(dir);
      }
      else
      {
         this->beginInsertRows(QModelIndex(), this->dirs.size(), this->dirs.size());
         this->dirs << dir;
         this->endInsertRows();
      }
   }

   for (QStringListIterator i(dirsToRemove); i.hasNext();)
   {
      int j = this->dirs.indexOf(i.next());
      if (j != -1)
      {
         this->beginRemoveRows(QModelIndex(), j, j);
         this->dirs.removeAt(j);
         this->endRemoveRows();
      }
   }
}

void DirListModel::addDir(const QString& dir)
{
   if (this->dirs.contains(dir))
      return;

   this->beginInsertRows(QModelIndex(), this->dirs.size(), this->dirs.size());
   this->dirs << dir;
   this->endInsertRows();
}

void DirListModel::addDirs(const QStringList& dirs)
{
   foreach (QString dir, dirs)
      this->addDir(dir);
}

void DirListModel::rmDir(int row)
{
   if (row >= this->dirs.size())
      return;

   this->beginRemoveRows(QModelIndex(), row, row);
   this->dirs.removeAt(row);
   this->endRemoveRows();
}

const QStringList& DirListModel::getDirs() const
{
   return this->dirs;
}

int DirListModel::rowCount(const QModelIndex& parent) const
{
   return this->dirs.size();
}

QVariant DirListModel::data(const QModelIndex& index, int role) const
{
   if (role != Qt::DisplayRole || index.row() >= this->dirs.size())
      return QVariant();
   return this->dirs[index.row()];
}