/* Sprite.cpp
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

/* WJF */
#define Point ELSPoint
#import <UIKit/UIKit.h>
#undef Point
#import "ELSImageManager.h"
/* WJF */

#include "Sprite.h"

#include "ImageBuffer.h"
#include "../Preferences.h"
#include "../Screen.h"

UIImage *spriteImage(const Sprite *sprite);

#include <algorithm>

using namespace std;



Sprite::Sprite(const string &name)
	: name(name)
{
    if (name.empty()) {
        return;
    }
    
#if 0
    UIImage *image = spriteImage(this);
    width = (int)image.size.width;
    height = (int)image.size.height;
    frames = 1;
#endif
    NSDictionary<NSString *, NSNumber *> *value = spriteWHF(this);
#if DEBUG
    if (!value) {
        NSLog(@"!!!!! Unknown sprite map for sprite: %@", @(Name().c_str()));
    }
#endif
    width = value[@"w"].intValue;
    height = value[@"h"].intValue;
    frames = value[@"f"].intValue;
}



const string &Sprite::Name() const
{
	return name;
}



// Add the given frames, optionally uploading them. The given buffer will be cleared afterwards.
void Sprite::AddFrames(ImageBuffer &buffer, bool is2x)
{
	// If this is the 1x image, its dimensions determine the sprite's size.
	if(!is2x)
	{
		width = buffer.Width();
		height = buffer.Height();
		frames = buffer.Frames();
	}
}



// Upload the given frames. The given buffer will be cleared afterwards.
void Sprite::AddSwizzleMaskFrames(ImageBuffer &buffer, bool is2x)
{
}



// Free up all textures loaded for this sprite.
void Sprite::Unload()
{
	width = 0.f;
	height = 0.f;
	frames = 0;
}



// Get the width, in pixels, of the 1x image.
float Sprite::Width() const
{
	return width;
}



// Get the height, in pixels, of the 1x image.
float Sprite::Height() const
{
	return height;
}



// Get the number of frames in the animation.
int Sprite::Frames() const
{
	return frames;
}



// Get the offset of the center from the top left corner; this is for easy
// shifting of corner to center coordinates.
Point Sprite::Center() const
{
	return Point(.5 * width, .5 * height);
}



// Get the texture index, based on whether the screen is high DPI or not.
uint32_t Sprite::Texture() const
{
	return Texture(Screen::IsHighResolution());
}



// Get the index of the texture for the given high DPI mode.
uint32_t Sprite::Texture(bool isHighDPI) const
{
	return (isHighDPI && texture[1]) ? texture[1] : texture[0];
}



// Get the texture index, based on whether the screen is high DPI or not.
uint32_t Sprite::SwizzleMask() const
{
	return SwizzleMask(Screen::IsHighResolution());
}



// Get the index of the texture for the given high DPI mode.
uint32_t Sprite::SwizzleMask(bool isHighDPI) const
{
	return (isHighDPI && swizzleMask[1]) ? swizzleMask[1] : swizzleMask[0];
}
