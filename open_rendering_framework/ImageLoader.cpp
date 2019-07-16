
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include "ImageLoader.h"
#include "PPMLoader.h"
#include "HDRLoader.h"
#include <fstream>


//-----------------------------------------------------------------------------
//  
//  Utility functions 
//
//-----------------------------------------------------------------------------

optix::TextureSampler loadTexture( optix::Context context,
                                            const std::string& filename,
                                            const optix::float3& default_color )
{
  bool IsHDR = false;
  size_t len = filename.length();
  if(len >= 3) {
    IsHDR = (filename[len-3] == 'H' || filename[len-3] == 'h') &&
      (filename[len-2] == 'D' || filename[len-2] == 'd') &&
      (filename[len-1] == 'R' || filename[len-1] == 'r');
  }
  if(IsHDR)
    return loadHDRTexture(context, filename, default_color);
  else
    return loadPPMTexture(context, filename, default_color);
}

unsigned char* bufferToImage(RTformat format, unsigned int width, unsigned int height, void* data)
{
	unsigned int pixel_size = 4;
	unsigned char* pix = new unsigned char[width * height * pixel_size];

	switch (format) {
	case RT_FORMAT_UNSIGNED_BYTE4:
		// Data is BGRA and upside down, so we need to swizzle to RGB
		for (int j = height - 1; j >= 0; --j) {
			unsigned char *dst = &pix[0] + (pixel_size * width*(height - 1 - j));
			unsigned char *src = ((unsigned char*)data) + (4 * width*j);
			for (int i = 0; i < width; i++) {
				*dst++ = *(src + 2);
				*dst++ = *(src + 1);
				*dst++ = *(src + 0);

				if (pixel_size == 4) {
					*dst++ = static_cast<unsigned char>(0xff);
				}
				src += 4;
			}
		}
		break;

	case RT_FORMAT_FLOAT:
		// This buffer is upside down
		for (int j = height - 1; j >= 0; --j) {
			unsigned char *dst = &pix[0] + pixel_size*width*(height - 1 - j);
			float* src = ((float*)data) + ( width*j);
			for (int i = 0; i < width; i++) {
				int P = static_cast<int>((*src++) * 255.0f);
				unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;

				// write the pixel to all 3 channels
				*dst++ = static_cast<unsigned char>(Clamped);
				*dst++ = static_cast<unsigned char>(Clamped);
				*dst++ = static_cast<unsigned char>(Clamped);
				*dst++ = static_cast<unsigned char>(0xff);
			}
		}
		break;

	case RT_FORMAT_FLOAT3:
		// This buffer is upside down
		for (int j = height - 1; j >= 0; --j) {
			unsigned char *dst = &pix[0] + (pixel_size * width*(height - 1 - j));
			float* src = ((float*)data) + (3 * width*j);
			for (int i = 0; i < width; i++) {
				for (int elem = 0; elem < 3; ++elem) {
					int P = static_cast<int>((*src++) * 255.0f);
					unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;
					*dst++ = static_cast<unsigned char>(Clamped);
				}
				if (pixel_size == 4) {
					*dst++ = static_cast<unsigned char>(0xff);
				}
			}
		}
		break;

	case RT_FORMAT_FLOAT4:
		// This buffer is upside down
		for (int j = height - 1; j >= 0; --j) {
			unsigned char *dst = &pix[0] + (pixel_size * width*(height - 1 - j));
			float* src = ((float*)data) + (4 * width*j);
			for (int i = 0; i < width; i++) {
				for (int elem = 0; elem < 3; ++elem) {
					int P = static_cast<int>((*src++) * 255.0f);
					unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;
					*dst++ = static_cast<unsigned char>(Clamped);
				}
				
				
				if (pixel_size == 4) {
					int P = static_cast<int>((*src) * 255.0f);
					unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;
					*dst++ = static_cast<unsigned char>(Clamped);
				}
				src++;

				// skip alpha
				
			}
		}
		break;

	default:
		fprintf(stderr, "Unrecognized buffer data type or format.\n");
		exit(2);
		break;
	}
	return pix;
}
