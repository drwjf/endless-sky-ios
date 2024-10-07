/* ImageSet.cpp
Copyright (c) 2017 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ImageSet.h"

#include "../GameData.h"
#include "ImageBuffer.h"
#include "../Logger.h"
#include "Mask.h"
#include "MaskManager.h"
#include "Sprite.h"

#include <algorithm>
#include <cassert>

using namespace std;

// Check if the given path is to an image of a valid file type.
bool ImageSet::IsImage(const filesystem::path &path)
{
	filesystem::path ext = path.extension();
	return (ext == ".png" || ext == ".jpg" || ext == ".PNG" || ext == ".JPG");
}



// Get the base name for the given path. The path should be relative to one
// of the source image directories, not a full filesystem path.
string ImageSet::Name(const filesystem::path &path)
{
    /*WJF Change to load lazily by demand */
    return path.string(); //WJF: .substr(0, NameEnd(path));
}



// Determine whether the given path or name is for a sprite whose loading
// should be deferred until needed.
bool ImageSet::IsDeferred(const filesystem::path &path)
{
	if(path.empty())
		return false;
	return *path.begin() == "land";
}



ImageSet::ImageSet(string name)
	: name(std::move(name))
{
}



// Get the name of the sprite for this image set.
const string &ImageSet::Name() const
{
	return name;
}



// Whether this image set is empty, i.e. has no images.
bool ImageSet::IsEmpty() const
{
    /*WJF Change to load lazily by demand */
    return false;
}



// Add a single image to this set. Assume the name of the image has already
// been checked to make sure it belongs in this set.
void ImageSet::Add(filesystem::path path)
{
    /*WJF Change to load lazily by demand */
}



// Reduce all given paths to frame images into a sequence of consecutive frames.
void ImageSet::ValidateFrames() noexcept(false)
{
    /*WJF Change to load lazily by demand */
}



// Load all the frames. This should be called in one of the image-loading
// worker threads. This also generates collision masks if needed.
void ImageSet::Load() noexcept(false)
{
    /*WJF Change to load lazily by demand */
}



// Create the sprite and optionally upload the image data to the GPU. After this is
// called, the internal image buffers and mask vector will be cleared, but
// the paths are saved in case the sprite needs to be loaded again.
void ImageSet::Upload(Sprite *sprite, bool enableUpload)
{
    /*WJF No OpenGL GPU uploading */
}
