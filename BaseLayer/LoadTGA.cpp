/////////////////////////////////////////////////////////////////////////////////////
// Routines to load the texture map
/////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <windows.h>	// Normal Windows stuff
#include <stdio.h>
#include "loadtga.h"
#include <malloc.h>


///////////////////////////////////////////////////////////////////////////////
// This stuff forward is the TGA Loading Routines
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
// Name: LoadTGAFile()
// Desc: Given a filename of a TGA file, it loads the image in BITMAP form
//-----------------------------------------------------------------------
unsigned char *LoadTGAFile( char * strFilename,	tTGAHeader_s *header)
{
/// Local Variables ///////////////////////////////////////////////////////////
	short			BitsPerPixel;
	unsigned char	*buffer;
	int bitsize;		/* Total size of bitmap */
	unsigned char	*newbits;		/* New RGB bits */
	unsigned char	*from, *to;		/* RGB looping vars */
	int		i, j,			/* Looping vars */
	width;			/* Aligned width of bitmap */
    FILE* file;
    long dwWidth, dwHeight;
///////////////////////////////////////////////////////////////////////////////

    // Open the file and read the header
	errno_t err;
	err = fopen_s(&file, strFilename, "rb");
    if( err )
        return NULL;

    if ( fread( header, sizeof( tTGAHeader_s ), 1, file ) != 1 )
    {
        fclose( file );
        return NULL;
    }

    // Parse the TGA header
	dwWidth = (long)header->d_width;
	dwHeight = (long)header->d_height;
	BitsPerPixel = (short)header->d_pixel_size;          // 16, 24, or 32

    // Create a bitmap to load the data into

	bitsize = dwWidth * dwHeight * (BitsPerPixel/8);
	if ((newbits = (unsigned char *)calloc(bitsize, 1)) == NULL)
	{
        fclose( file );
        return NULL;
	}
 	buffer = (unsigned char *)malloc(dwWidth*dwHeight*(BitsPerPixel / 8));
    if ( fread( buffer, dwWidth*dwHeight*(BitsPerPixel / 8), 1, file ) != 1 )
	{
        fclose( file );
		free(buffer);
		free(newbits);
        return NULL;
	}

	width   = (BitsPerPixel / 8) * dwWidth;

    for (i = 0; i < dwHeight; i ++)
		for (j = 0, from = ((unsigned char *)buffer) + i * width,
	        to = newbits + i * width;
			j < dwWidth;
			j ++, from += (BitsPerPixel / 8), to += (BitsPerPixel / 8))
        {
				if (BitsPerPixel == 24)
				{
					to[0] = from[2];
					to[1] = from[1];
					to[2] = from[0];
				}
				else
				{
					to[0] = from[2];
					to[1] = from[1];
					to[2] = from[0];
					to[3] = from[3];
				}
        };
	free(buffer);
    fclose( file );

    return newbits;
}


