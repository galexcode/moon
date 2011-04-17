/*
 * pipeline-vda.cpp: Video Decode Acceleration Framework related parts of the pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

/*
 *	FFmpegDecoder
 */

#include <config.h>

#include <stdlib.h>
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "pipeline-vda.h"
#include "pipeline.h"
#include "mp3.h"
#include "clock.h"
#include "debug.h"
#include "deployment.h"

namespace Moonlight {

void
register_vda ()
{
	Media::RegisterDecoder (new MoonVDADecoderInfo ());
}

/*
 * MoonVDADecoder
 */

MoonVDADecoder::MoonVDADecoder (Media* media, IMediaStream* stream)
	: IMediaDecoder (Type::MOONVDADECODER, media, stream)
{
	this->decoder = NULL;
}

void
MoonVDADecoder::InputEnded ()
{
	GetStream ()->SetOutputEnded (true);
}

static void
VDADecoderCallback (void *decompressionOutputRefCon, CFDictionaryRef frameInfo, OSStatus status, uint32_t infoFlags, CVImageBufferRef imageBuffer)
{
	MoonVDADecoder *decoder = (MoonVDADecoder *) decompressionOutputRefCon;

	if (imageBuffer == NULL) {
		return;
	}

	OSType format_type = CVPixelBufferGetPixelFormatType(imageBuffer);
	if (format_type != kCVPixelFormatType_422YpCbCr8) {
		g_warning ("Mismatched format in VDA");
		return;
	}

	g_warning ("Decoder: %p\n", decoder);
}

static OSStatus
CreateDecoder (SInt32 inHeight, SInt32 inWidth, OSType inSourceFormat, CFDataRef inAVCCData, MoonVDADecoder *userData, VDADecoder *decoderOut)
{
	decoderOut = NULL;

	OSStatus status;

	CFMutableDictionaryRef decoderConfiguration = NULL;
	CFMutableDictionaryRef destinationImageBufferAttributes = NULL;
	CFDictionaryRef emptyDictionary;

	CFNumberRef height = NULL;
	CFNumberRef width= NULL;
	CFNumberRef sourceFormat = NULL;
	CFNumberRef pixelFormat = NULL;

	// source must be H.264
	if (inSourceFormat != 'avc1') {
		fprintf (stderr, "Source format is not H.264!\n");
		return paramErr;
	}

	// the avcC data chunk from the bitstream must be present
	if (inAVCCData == NULL) {
		fprintf (stderr, "avc1 decoder configuration data cannot be NULL!\n");
		return paramErr;
	}

	decoderConfiguration = CFDictionaryCreateMutable (kCFAllocatorDefault, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	height = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &inHeight);
	width = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &inWidth);
	sourceFormat = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &inSourceFormat);

	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_Height, height);
	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_Width, width);
	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_SourceFormat, sourceFormat);
	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_avcCData, inAVCCData);

	destinationImageBufferAttributes = CFDictionaryCreateMutable (kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	OSType cvPixelFormatType = kCVPixelFormatType_422YpCbCr8;
	pixelFormat = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &cvPixelFormatType);
	emptyDictionary = CFDictionaryCreate (kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDictionarySetValue (destinationImageBufferAttributes, kCVPixelBufferPixelFormatTypeKey, pixelFormat);
	CFDictionarySetValue (destinationImageBufferAttributes, kCVPixelBufferIOSurfacePropertiesKey, emptyDictionary);

	status = VDADecoderCreate (decoderConfiguration, destinationImageBufferAttributes, (VDADecoderOutputCallback*) VDADecoderCallback, (void *) userData, decoderOut);

	if (kVDADecoderNoErr != status) {
		fprintf (stderr, "VDADecoderCreate failed. err: %i\n", (int) status);
	}

	if (decoderConfiguration) CFRelease (decoderConfiguration);
	if (destinationImageBufferAttributes) CFRelease (destinationImageBufferAttributes);
	if (emptyDictionary) CFRelease (emptyDictionary);

	return status;
}

void
MoonVDADecoder::OpenDecoderAsyncInternal ()
{
	IMediaStream *stream = GetStream ();
	VideoStream *vs = (VideoStream *) stream;
	VDADecoder hardwareDecoder = NULL;
	int format = 'avc1';

	CFDataRef avcCData = CFDataCreate (kCFAllocatorDefault, (const uint8_t*) stream->GetExtraData (), stream->GetExtraDataSize ());
	OSStatus status = CreateDecoder ((SInt32) vs->GetHeight (), (SInt32) vs->GetWidth (), (OSType) format, avcCData, this, &hardwareDecoder);

	if (avcCData) CFRelease (avcCData);
}

void
MoonVDADecoder::Dispose ()
{
	g_assert_not_reached ();
}

void
MoonVDADecoder::Cleanup (MediaFrame *frame)
{
	g_assert_not_reached ();
}

void
MoonVDADecoder::CleanState ()
{
	g_assert_not_reached ();
}

void
MoonVDADecoder::DecodeFrameAsyncInternal (MediaFrame *mf)
{
	g_assert_not_reached ();
}

/*
 * VDADecoderInfo
 */

bool
MoonVDADecoderInfo::Supports (const char* codec)
{
	return codec != NULL && strcmp (codec, "h264") == 0;
}

IMediaDecoder*
MoonVDADecoderInfo::Create (Media* media, IMediaStream* stream)
{
	return new MoonVDADecoder (media, stream);
}

};
