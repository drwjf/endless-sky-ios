/* Music.cpp
Copyright (c) 2016 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Music.h"
using namespace std;

namespace {
    // How many samples to put in each output block. Because the output is in
    // stereo, the duration of the sample is half this amount:
    const size_t OUTPUT_CHUNK = 32768;
}

void Music::Init(const vector<string> &sources)
{
}



// Music constructor, which starts the decoding thread. Initially, the thread
// has no file to read, so it will sleep until a file is specified.
Music::Music()
: silence(OUTPUT_CHUNK, 0)
{
}



// Destructor, which waits for the thread to stop.
Music::~Music()
{
}



// Set the source of music. If the path is empty, this music will be silent.
void Music::SetSource(const string &name)
{
}



// Get the name of the current music source playing.
const string &Music::GetSource() const
{
	return currentSource;
}
