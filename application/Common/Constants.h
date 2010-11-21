#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include <QDir>

namespace Common
{
   const QString APPLICATION_FOLDER_NAME(".aybabtu");
   const QString APPLICATION_FOLDER_PATH(QDir::homePath() + '/' + APPLICATION_FOLDER_NAME);
   const QString LOG_FOLDER_NAME("log");
   const QString FILE_CACHE("cache.bin"); ///< The name of the file cache saved in the home directory.
   const QString FILE_QUEUE("queue.bin"); ///< This file contains the current downloads.

   const QString SERVICE_NAME("AybabtuCore");

   const int PROTOBUF_STREAMING_BUFFER_SIZE(4 * 1024); ///< 4kB.

   const QString BINARY_PREFIXS[] = {"B", "KiB", "MiB", "GiB", "TiB", "Pi", "Ei", "Zi"};
}

#endif
