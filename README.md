#Logging

A very simple program written in Qt to demonstrate how logging changes to items kept in buffer could be implemented.

## Transaction

Before you can store or access any information you have to start a transaction. After starting the transaction, you can:
* Read one page. This will return the last written value by you (even if not committed) or else the current value. Two consecutive calls could return different data if you haven't written to the page.
* Write one page. This change is temporary unless you commit.
* Commit. This will end your transaction. All temporary data is permanent after the commit
* Rollback. This will end your transaction. All temporary data will be erased and will no longer be available.

After you end one transaction, you have to start a new one before you can access pages again.

## Log

The log consists of three phases:
1. Temporary memory. This consists of pages not yet committed and will be lost on application termination
2. Permanent data not written to page files. This data is located in "./log/" and has to be read from the latest logs to get the latest version.
3. Permanent data written to page file. These files can be found at "./pages/". They may not contain the latest data but only a snapshot at one point in time. This is to speed up the reconstruction from the "./log/" files

## Paths

All paths are relative to your current paths
* "./log/" contains all log files enumerated from 0 to the latest file. "last_log.dat" always contains the number of the latest log file.
* "./pages/" contain the page files which were written. Even if no file exists for a page, the page might have been stored in the logs (in this case, you have to traverse through all log files in chronological order).

## License

Copyright (C) 2016 Marcus Soll

Logger is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Logger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Logger. If not, see <http://www.gnu.org/licenses/>.
