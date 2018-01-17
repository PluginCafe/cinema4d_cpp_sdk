#ifndef PAINTCHANNELS_H__
#define PAINTCHANNELS_H__

#include "c4d.h"
#include "paintbrushbase.h"

class BrushDabData;

struct TriangleData
{
	Vector32 destBitmapCoords[3];
	Vector32 sourceBitmapCoords[3];
	Vector points3D[3];
};

struct PaintChannels
{
	PaintLayerBmp *channel;
	Vector fgColor;
	TriangleData triangle[2];

	Float strength;
	Bool useStencil;
	Bool useStamp;
	Bool fillTool;
	Int32 minmax[4];

	PaintChannels();
	~PaintChannels();

	Bool Init();
	void UpdateBitmaps();

	void SetupPoly_Pixel(BrushDabData *dab, const CPolygon &p, const UVWStruct &polyUVs, const Vector *polyPoints);
	void SetupPoly_Bary(BrushDabData *dab, const CPolygon &p, const UVWStruct &polyUVs, const Vector *polyPoints);
};


inline Bool GetChannelInfo(PaintLayerBmp *paintBmp, int &bitdepth, int &numChannels)
{
	if (!paintBmp) 
		return false;
	Bool supported = false;

	COLORMODE colorMode = (COLORMODE)paintBmp->GetColorMode();
	switch (colorMode)
	{
		// 8Bit
		case COLORMODE_ALPHA:	// only alpha channel
			bitdepth = 8;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE_GRAY:
			bitdepth = 8;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE_AGRAY:
			bitdepth = 8;
			numChannels = 2;
			supported = true;
			break;
		case COLORMODE_RGB:
			bitdepth = 8;
			numChannels = 3;
			supported = true;
			break;
		case COLORMODE_ARGB:
			bitdepth = 8;
			numChannels = 4;
			supported = true;
			break;
		case COLORMODE_CMYK:
			bitdepth = 8;
			numChannels = 4;
			break;
		case COLORMODE_ACMYK:
			bitdepth = 8;
			numChannels = 5;
			break;
		case COLORMODE_MASK: // gray map as mask
			bitdepth = 8;
			numChannels = 1;
			break;
		case COLORMODE_AMASK: // gray map as mask
			bitdepth = 8;
			numChannels = 2;
			break;

		// 16 bit modes
		case COLORMODE_GRAYw:
			bitdepth = 16;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE_AGRAYw:
			bitdepth = 16;
			numChannels = 2;
			supported = true;
			break;
		case COLORMODE_RGBw:
			bitdepth = 16;
			numChannels = 3;
			supported = true;
			break;
		case COLORMODE_ARGBw:
			bitdepth = 16;
			numChannels = 4;
			supported = true;
			break;
		case COLORMODE_MASKw:
			bitdepth = 16;
			numChannels = 1;
			break;

		// 32 bit modes
		case COLORMODE_GRAYf:
			bitdepth = 32;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE_AGRAYf:
			bitdepth = 32;
			numChannels = 2;
			supported = true;
			break;
		case COLORMODE_RGBf:
			bitdepth = 32;
			numChannels = 3;
			supported = true;
			break;
		case COLORMODE_ARGBf:
			bitdepth = 32;
			numChannels = 4;
			supported = true;
			break;
		case COLORMODE_MASKf:
			bitdepth = 32;
			numChannels = 1;
			break;
	}

	return supported;
}

inline void SetPixel(COLORMODE colorMode, UInt &idx, UChar *pBuffer, Float32 falloff, UChar *colBuffer, const BaseBitmap *pBitmap = nullptr, int sourceX = 0, int sourceY = 0)
{
	if (falloff <= 0)
	{
		switch (colorMode)
		{
			// 8Bit
			case COLORMODE_ALPHA:	// only alpha channel
				idx += 1;
				break;
			case COLORMODE_GRAY:
				idx += 1;
				break;
			case COLORMODE_AGRAY:
				idx += 2;
				break;
			case COLORMODE_RGB:
				idx += 3;
				break;
			case COLORMODE_ARGB:
				idx += 4;
				break;
			case COLORMODE_CMYK:
				break;
			case COLORMODE_ACMYK:
				break;
			case COLORMODE_MASK: // gray map as mask
				idx += 1;
				break;
			case COLORMODE_AMASK: // gray map as mask
				idx += 2;
				break;

			// 16 bit modes
			case COLORMODE_GRAYw:
				idx += 2;
				break;
			case COLORMODE_AGRAYw:
				idx += 4;
				break;
			case COLORMODE_RGBw:
				idx += 6;
				break;
			case COLORMODE_ARGBw:
				idx += 8;
				break;
			case COLORMODE_MASKw:
				idx += 2;
				break;

			// 32 bit modes
			case COLORMODE_GRAYf:
				idx += 4;
				break;
			case COLORMODE_AGRAYf:
				idx += 8;
				break;
			case COLORMODE_RGBf:
				idx += 12;
				break;
			case COLORMODE_ARGBf:
				idx += 16;
				break;
			case COLORMODE_MASKf:
				idx += 4;
				break;
		}

		return;
	}

	Float32 falloffInterp = 1.0f - falloff;
	switch (colorMode)
	{
		// 8Bit
		case COLORMODE_ALPHA:	// only alpha channel
		{
			idx += 1;
			break;
		}
		case COLORMODE_GRAY:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 1, COLORMODE_GRAY, PIXELCNT_0);
			pBuffer[idx] = (UChar)(pBuffer[idx] * (falloffInterp) + colBuffer[0] * falloff);
			idx += 1;
			break;
		}
		case COLORMODE_AGRAY:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 1, COLORMODE_AGRAY, PIXELCNT_0);
			pBuffer[idx] = (UChar)(pBuffer[idx] * (falloffInterp) + colBuffer[0] * falloff);
			pBuffer[idx+1] = (UChar)(pBuffer[idx+1] * (falloffInterp) + colBuffer[1] * falloff);
			idx += 2;
			break;
		}
		case COLORMODE_RGB:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 3, COLORMODE_RGB, PIXELCNT_0);
			pBuffer[idx] = (UChar)(pBuffer[idx] * (falloffInterp) + colBuffer[0] * falloff);
			pBuffer[idx+1] = (UChar)(pBuffer[idx+1] * (falloffInterp) + colBuffer[1] * falloff);
			pBuffer[idx+2] = (UChar)(pBuffer[idx+2] * (falloffInterp) + colBuffer[2] * falloff);
			idx += 3;
			break;
		}
		case COLORMODE_ARGB:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 3, COLORMODE_ARGB, PIXELCNT_0);
			pBuffer[idx] = (UChar)(pBuffer[idx] * (falloffInterp) + colBuffer[0]* falloff);
			pBuffer[idx+1] = (UChar)(pBuffer[idx+1] * (falloffInterp) + colBuffer[1] * falloff);
			pBuffer[idx+2] = (UChar)(pBuffer[idx+2] * (falloffInterp) + colBuffer[2] * falloff);
			pBuffer[idx+3] = (UChar)(pBuffer[idx+3] * (falloffInterp) + colBuffer[3] * falloff);
			idx += 4;
			break;
		}
		case COLORMODE_CMYK:
			break;
		case COLORMODE_ACMYK:
			break;
		case COLORMODE_AMASK:
			break;

		// 16 bit modes
		case COLORMODE_GRAYw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 2, COLORMODE_GRAYw, PIXELCNT_0);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(destBuf[0] * (falloffInterp) + colBuf[0] * falloff);
			idx += 2;
			break;
		}
		case COLORMODE_AGRAYw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 2, COLORMODE_AGRAYw, PIXELCNT_0);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(destBuf[0] * (falloffInterp) + colBuf[0] * falloff);
			destBuf[1] = (UInt16)(destBuf[1] * (falloffInterp) + colBuf[1] * falloff);
			idx += 4;
			break;
		}
		case COLORMODE_RGBw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 6, COLORMODE_RGBw, PIXELCNT_0);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(destBuf[0] * (falloffInterp) + colBuf[0] * falloff);
			destBuf[1] = (UInt16)(destBuf[1] * (falloffInterp) + colBuf[1] * falloff);
			destBuf[2] = (UInt16)(destBuf[2] * (falloffInterp) + colBuf[2] * falloff);
			idx += 6;
			break;
		}
		case COLORMODE_ARGBw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 6, COLORMODE_ARGBw, PIXELCNT_0);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(destBuf[0] * (falloffInterp) + colBuf[0] * falloff);
			destBuf[1] = (UInt16)(destBuf[1] * (falloffInterp) + colBuf[1] * falloff);
			destBuf[2] = (UInt16)(destBuf[2] * (falloffInterp) + colBuf[2] * falloff);
			destBuf[3] = (UInt16)(destBuf[3] * (falloffInterp) + colBuf[3] * falloff);
			idx += 8;
			break;
		}
		case COLORMODE_MASK:
			break;
		case COLORMODE_MASKw:
			break;

		// 32 bit modes
		case COLORMODE_GRAYf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 4, COLORMODE_GRAYf, PIXELCNT_0);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = destBuf[0] * (falloffInterp) + colBuf[0] * falloff;
			idx += 4;
			break;
		}
		case COLORMODE_AGRAYf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 4, COLORMODE_AGRAYf, PIXELCNT_0);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = destBuf[0] * (falloffInterp) + colBuf[0] * falloff;
			destBuf[1] = destBuf[1] * (falloffInterp) + colBuf[1] * falloff;
			idx += 8;
			break;
		}
		case COLORMODE_RGBf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 12, COLORMODE_RGBf, PIXELCNT_0);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = destBuf[0] * (falloffInterp) + colBuf[0] * falloff;
			destBuf[1] = destBuf[1] * (falloffInterp) + colBuf[1] * falloff;
			destBuf[2] = destBuf[2] * (falloffInterp) + colBuf[2] * falloff;
			idx += 12;
			break;
		}
		case COLORMODE_ARGBf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 12, COLORMODE_ARGBf, PIXELCNT_0);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = destBuf[0] * (falloffInterp) + colBuf[0] * falloff;
			destBuf[1] = destBuf[1] * (falloffInterp) + colBuf[1] * falloff;
			destBuf[2] = destBuf[2] * (falloffInterp) + colBuf[2] * falloff;
			destBuf[3] = destBuf[3] * (falloffInterp) + colBuf[3] * falloff;
			idx += 16;
			break;
		}
		case COLORMODE_MASKf:
			break;
	}
}

#endif // PAINTCHANNELS_H__

