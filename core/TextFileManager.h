/*
 * PtokaX - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2004-2015  Petr Kozelka, PPK at PtokaX dot org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
#ifndef TextFileManagerH
#define TextFileManagerH
//---------------------------------------------------------------------------
struct User;
//---------------------------------------------------------------------------

class clsTextFilesManager {
private:
    struct TextFile {
    	TextFile * pPrev, * pNext;

        char * sCommand, * sText;

        TextFile();
        ~TextFile();

        TextFile(const TextFile&);
        const TextFile& operator=(const TextFile&);
    };

	TextFile * pTextFiles;

    clsTextFilesManager(const clsTextFilesManager&);
    const clsTextFilesManager& operator=(const clsTextFilesManager&);
public:
    static clsTextFilesManager * mPtr;

	clsTextFilesManager();
	~clsTextFilesManager();

	bool ProcessTextFilesCmd(User * u, char * cmd, bool fromPM = false) const;
	void RefreshTextFiles();
};
//---------------------------------------------------------------------------

#endif
