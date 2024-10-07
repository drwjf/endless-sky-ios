/* Audio.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Audio.h"

#include "../Files.h"
#include "../Logger.h"
#include "Music.h"
#include "../Point.h"
#include "../Random.h"
#include "Sound.h"

/* WJF
#include <AL/al.h>
#include <AL/alc.h>
*/

#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace std;

namespace {
	// This class represents a new sound source that is queued to be added. This
	// is so that any thread can add a sound, but the audio thread can control
	// when those sounds actually start playing.
	class QueueEntry {
	public:
		void Add(Point position);
		void Add(const QueueEntry &other);

		Point sum;
		double weight = 0.;
	};

	// OpenAL only allows a certain number of distinct sound sources. To work
	// around that limitation, multiple instances of the same sound playing at
	// the same time will be "coalesced" into a single source, and sources will
	// be recycled once they are no longer playing.
	class Source {
	public:
		Source(const Sound *sound, unsigned source);

		void Move(const QueueEntry &entry) const;
		unsigned ID() const;
		const Sound *GetSound() const;

	private:
		const Sound *sound = nullptr;
		unsigned source = 0;
	};



	// Mutex to make sure different threads don't modify the audio at the same time.
	mutex audioMutex;

    /* WJF
	// OpenAL settings.
	ALCdevice *device = nullptr;
	ALCcontext *context = nullptr;
     */
	bool isInitialized = false;
	double volume = .125;

	// This queue keeps track of sounds that have been requested to play. Each
	// added sound is "deferred" until the next audio position update to make
	// sure that all sounds from a given frame start at the same time.
	map<const Sound *, QueueEntry> soundQueue;
	map<const Sound *, QueueEntry> deferred;
	thread::id mainThreadID;

	// Sound resources that have been loaded from files.
	map<string, Sound> sounds;
	// OpenAL "sources" available for playing sounds. There are a limited number
	// of these, so they must be reused.
	vector<Source> sources;
	vector<unsigned> recycledSources;
	vector<unsigned> endingSources;
	unsigned maxSources = 255;

	// Queue and thread for loading sound files in the background.
	map<string, string> loadQueue;
	thread loadThread;

	// The current position of the "listener," i.e. the center of the screen.
	Point listener;

	// MP3 streaming:
	unsigned musicSource = 0;
	const size_t MUSIC_BUFFERS = 3;
	unsigned musicBuffers[MUSIC_BUFFERS];
	shared_ptr<Music> currentTrack;
	shared_ptr<Music> previousTrack;
	int musicFade = 0;
	vector<int16_t> fadeBuffer;
}



// Begin loading sounds (in a separate thread).
void Audio::Init(const vector<string> &sources)
{
    isInitialized = true;
}



void Audio::CheckReferences()
{
}



// Report the progress of loading sounds.
double Audio::GetProgress()
{
	unique_lock<mutex> lock(audioMutex);

	if(loadQueue.empty())
		return 1.;

	double done = sounds.size();
	double total = done + loadQueue.size();
	return done / total;
}



// Get the volume.
double Audio::Volume()
{
	return volume;
}



// Set the volume (to a value between 0 and 1).
void Audio::SetVolume(double level)
{
	volume = min(1., max(0., level));
}



// Get a pointer to the named sound. The name is the path relative to the
// "sound/" folder, and without ~ if it's on the end, or the extension.
const Sound *Audio::Get(const string &name)
{
	unique_lock<mutex> lock(audioMutex);
	return &sounds[name];
}



// Set the listener's position, and also update any sounds that have been
// added but deferred because they were added from a thread other than the
// main one (the one that called Init()).
void Audio::Update(const Point &listenerPosition)
{
	if(!isInitialized)
		return;

	listener = listenerPosition;

	for(const auto &it : deferred)
		soundQueue[it.first].Add(it.second);
	deferred.clear();
}



// Play the given sound, at full volume.
void Audio::Play(const Sound *sound)
{
	Play(sound, listener);
}



// Play the given sound, as if it is at the given distance from the
// "listener". This will make it softer and change the left / right balance.
void Audio::Play(const Sound *sound, const Point &position)
{
	if(!isInitialized || !sound || !volume)
		return;

	// Place sounds from the main thread directly into the queue. They are from
	// the UI, and the Engine may not be running right now to call Update().
	if(this_thread::get_id() == mainThreadID)
		soundQueue[sound].Add(position - listener);
	else
	{
		unique_lock<mutex> lock(audioMutex);
		deferred[sound].Add(position - listener);
	}
}



// Play the given music. An empty string means to play nothing.
void Audio::PlayMusic(const string &name)
{
	if(!isInitialized)
		return;

	// Skip changing music if the requested music is already playing.
	if(name == currentTrack->GetSource())
		return;

	// Don't worry about thread safety here, since music will always be started
	// by the main thread.
	musicFade = 65536;
	swap(currentTrack, previousTrack);
	// If the name is empty, it means to turn music off.
	currentTrack->SetSource(name);
}



// Begin playing all the sounds that have been added since the last time
// this function was called.
void Audio::Step()
{
}



// Shut down the audio system (because we're about to quit).
void Audio::Quit()
{
	// First, check if sounds are still being loaded in a separate thread, and
	// if so interrupt that thread and wait for it to quit.
	unique_lock<mutex> lock(audioMutex);
	if(!loadQueue.empty())
		loadQueue.clear();
	if(loadThread.joinable())
	{
		lock.unlock();
		loadThread.join();
		lock.lock();
	}
}



namespace {
	// Add a new source to this queue entry. Sources are weighted based on their
	// position, and multiple sources can be added together in the same entry.
	void QueueEntry::Add(Point position)
	{
		// A distance of 500 counts as 1 OpenAL unit of distance.
		position *= .002;
		// To avoid having sources at a distance of 0 be infinitely loud, have
		// the minimum distance be 1 unit away.
		double d = 1. / (1. + position.Dot(position));
		sum += d * position;
		weight += d;
	}



	// Combine two queue entries.
	void QueueEntry::Add(const QueueEntry &other)
	{
		sum += other.sum;
		weight += other.weight;
	}



	// This is a wrapper for an OpenAL audio source.
	Source::Source(const Sound *sound, unsigned source)
		: sound(sound), source(source)
	{
	}



	// Reposition this source based on the given entry in a sound queue.
	void Source::Move(const QueueEntry &entry) const
	{
		Point angle = entry.sum / entry.weight;
		// The source should be along the vector (angle.X(), angle.Y(), 1).
		// The length of the vector should be sqrt(1 / weight).
		double scale = sqrt(1. / (entry.weight * (angle.LengthSquared() + 1.)));
	}



	// Get the OpenAL ID for this source.
	unsigned Source::ID() const
	{
		return source;
	}



	// Get the sound resource currently being played by this source.
	const Sound *Source::GetSound() const
	{
		return sound;
	}



}
