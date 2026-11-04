/***************************************************************************************
 *
 * ImageSerializerJPG.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 5/24/2004: Gary Hinger: created file
 *
 ***************************************************************************************/

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/stdafx.h"
#include <cstdio>
#include <algorithm>
#include "WT/Image/Image.h"
#include "jpeglib.h"
#include "jerror.h"

using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CImageSerializerJPG )

/*virtual*/ bool CImageSerializerJPG::Validate( CStream* pStream ) const
{
    uint8 const KJPGHeaderByte0 = 0xffu;
    uint8 const KJPGHeaderByte1 = 0xd8u;

	if (pStream == NULL)
    {
        return false;
    }

    int32 const kHeaderSize = 2; //Check 2 bytes
    uint8 Header[kHeaderSize];

    int32 ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &Header ), sizeof Header );
	pStream->Seek( -ActualBytesRead, SeekOrigin::Current );
    if (ActualBytesRead != sizeof Header)
    {
        //Incomplete header
        return false;
    }

    if (Header[0] != KJPGHeaderByte0 ||
        Header[1] != KJPGHeaderByte1)
    {
        //Not a JPG file
        return false;
    }

    return true;
}

float32 CImageSerializerJPG::GetQuality() const
{
    return m_Quality;
}

bool CImageSerializerJPG::SetQuality(float32 Quality)
{
    if (Quality < 0.0f || Quality > 1.0f)
    {
        return false;
    }

    m_Quality = Quality;
    return true;
}

CImageSerializerJPG::CImageSerializerJPG()
{
    m_Quality = 1.0f;
}

namespace
{
//I/O and error-handling routines and structures for libjpeg

int32 const OUTPUT_BUF_SIZE = 4096; //Choose an efficiently CStream::Write()'able size
int32 const INPUT_BUF_SIZE = 4096; //Choose an efficiently CStream::Read()'able size

//Augmented data destination object for Stream output
struct my_destination_mgr
{
    jpeg::jpeg_destination_mgr pub; //public fields

    CStream* pOutStream; //Target stream
    jpeg::JOCTET* buffer; //Start of buffer
};

//Initialize destination --- called by jpeg_start_compress
//before any data is actually written.
void init_destination(jpeg::j_compress_ptr cinfo)
{
    my_destination_mgr* dest = reinterpret_cast<my_destination_mgr*>(cinfo->dest);

    //Allocate the output buffer --- it will be released when done with image
    dest->buffer = (jpeg::JOCTET *)
        (*cinfo->mem->alloc_small) ((jpeg::j_common_ptr) cinfo, JPOOL_IMAGE,
				    OUTPUT_BUF_SIZE * sizeof (jpeg::JOCTET));

    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

//Empty the output buffer --- called whenever buffer fills up.
jpeg::boolean empty_output_buffer(jpeg::j_compress_ptr cinfo)
{
    my_destination_mgr* dest = reinterpret_cast<my_destination_mgr*>(cinfo->dest);

    dest->pOutStream->Write(reinterpret_cast<uint8*>( dest->buffer ), OUTPUT_BUF_SIZE);
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
    return TRUE;
}

//Terminate destination --- called by jpeg_finish_compress
//after all data has been written.  Usually needs to flush buffer.

void term_destination(jpeg::j_compress_ptr cinfo)
{
    my_destination_mgr* dest = reinterpret_cast<my_destination_mgr*>(cinfo->dest);
    size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

    /* Write any data remaining in the buffer */
    if (datacount > 0) 
    {
        dest->pOutStream->Write( reinterpret_cast<uint8*>( dest->buffer ),
			static_cast<int32>( datacount ) );
    }
}

//Prepare for output to a Stream.
void jpeg_Stream_dest (jpeg::j_compress_ptr cinfo, CStream* pOutStream)
{
  if (cinfo->dest == NULL) { //first time for this JPEG object?
    cinfo->dest = (jpeg::jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((jpeg::j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(my_destination_mgr));
  }

  my_destination_mgr* dest = reinterpret_cast<my_destination_mgr*>(cinfo->dest);
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->pOutStream = pOutStream;
}

//Augmented data source object for libjpeg
struct my_source_mgr
{
    jpeg::jpeg_source_mgr pub; //Public fields

    CStream* pInStream; //Source stream
    jpeg::JOCTET* buffer; //Start of buffer
    jpeg::boolean start_of_file; //Have we gotten any data yet?
};

//Initialize source --- called by jpeg_read_header
//before any data is actually read.
void init_source(jpeg::j_decompress_ptr cinfo)
{
    my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);

    //We reset the empty-input-file flag for each image,
    //but we don't clear the input buffer.
    //This is correct behavior for reading a series of images from one source.
    src->start_of_file = TRUE;
}

/*
Fill the input buffer --- called whenever buffer is emptied.

In typical applications, this should read fresh data into the buffer
(ignoring the current state of next_input_byte & bytes_in_buffer),
reset the pointer & count to the start of the buffer, and return TRUE
indicating that the buffer has been reloaded.  It is not necessary to
fill the buffer entirely, only to obtain at least one more byte.

There is no such thing as an EOF return.  If the end of the file has been
reached, the routine has a choice of ERREXIT() or inserting fake data into
the buffer.  In most cases, generating a warning message and inserting a
fake EOI marker is the best course of action --- this will allow the
decompressor to output however much of the image is there.  However,
the resulting error message is misleading if the real problem is an empty
input file, so we handle that case specially.

In applications that need to be able to suspend compression due to input
not being available yet, a FALSE return indicates that no more data can be
obtained right now, but more may be forthcoming later.  In this situation,
the decompressor will return to its caller (with an indication of the
number of scanlines it has read, if any).  The application should resume
decompression after it has loaded more data into the input buffer.  Note
that there are substantial restrictions on the use of suspension --- see
the documentation.

When suspending, the decompressor will back up to a convenient restart point
(typically the start of the current MCU). next_input_byte & bytes_in_buffer
indicate where the restart point will be if the current call returns FALSE.
Data beyond this point must be rescanned after resumption, so move it to
the front of the buffer rather than discarding it.
*/
jpeg::boolean fill_input_buffer(jpeg::j_decompress_ptr cinfo)
{
    my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);

    uint32 nbytes = static_cast<int32>( src->pInStream->Read( reinterpret_cast<uint8*>( src->buffer ), INPUT_BUF_SIZE) );

    if (nbytes <= 0) {
        if (src->start_of_file)
        {
            //Treat empty input file as fatal error
            ERREXIT(cinfo, JERR_INPUT_EMPTY);
        }
        WARNMS(cinfo, JWRN_JPEG_EOF);
        //Insert a fake EOI marker
        src->buffer[0] = (jpeg::JOCTET)0xFF;
        src->buffer[1] = (jpeg::JOCTET)JPEG_EOI;
        nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = FALSE;
    return TRUE;
}

//Skip data --- used to skip over a potentially large amount of
//uninteresting data (such as an APPn marker).
//
//Writers of suspendable-input applications must note that skip_input_data
//is not granted the right to give a suspension return.  If the skip extends
//beyond the data currently in the buffer, the buffer can be marked empty so
//that the next read will cause a fill_input_buffer call that can suspend.
//Arranging for additional bytes to be discarded before reloading the input
//buffer is the application writer's problem.
void skip_input_data(jpeg::j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);

    if (num_bytes > 0) 
    {
        while (num_bytes > (long) src->pub.bytes_in_buffer) 
        {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            fill_input_buffer(cinfo);
            //note we assume that fill_input_buffer will never return FALSE,
            //so suspension need not be handled.
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

//An additional method that can be provided by data source modules is the
//resync_to_restart method for error recovery in the presence of RST markers.
//For the moment, this source module just uses the default resync method
//provided by the JPEG library.  That method assumes that no backtracking
//is possible.

//Terminate source --- called by jpeg_finish_decompress
//after all data has been read.  Often a no-op.
//
//NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
//application must deal with any cleanup that should happen even
//for error exit.
void term_source(jpeg::j_decompress_ptr cinfo)
{
    //no work necessary here
}

//Prepare for input from a Stream.
//The caller must have already opened the stream, and is responsible
//for closing it after finishing decompression.
void jpeg_Stream_src(jpeg::j_decompress_ptr cinfo, CStream* pInStream)
{
    //The source object and input buffer are made permanent so that a series
    //of JPEG images can be read from the same file by calling jpeg_Stream_src
    //only before the first one.  (If we discarded the buffer at the end of
    //one image, we'd likely lose the start of the next one.)
    //This makes it unsafe to use this manager and a different source
    //manager serially with the same JPEG object.  Caveat programmer.

    if (cinfo->src == NULL) //first time for this JPEG object?
    {	
        cinfo->src = (jpeg::jpeg_source_mgr*)
            (*cinfo->mem->alloc_small) ((jpeg::j_common_ptr) cinfo, JPOOL_PERMANENT,
			sizeof (my_source_mgr));
        my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);
        src->buffer = (jpeg::JOCTET *)
            (*cinfo->mem->alloc_small) ((jpeg::j_common_ptr) cinfo, JPOOL_PERMANENT,
            INPUT_BUF_SIZE * sizeof (jpeg::JOCTET));
    }

    my_source_mgr* src = reinterpret_cast<my_source_mgr*>(cinfo->src);
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg::jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term_source;
    src->pInStream = pInStream;
    src->pub.bytes_in_buffer = 0; //forces fill_input_buffer on first read
    src->pub.next_input_byte = NULL; //until buffer loaded
}

//JPEG error handling routines

struct my_error_mgr 
{
    jpeg::jpeg_error_mgr pub; //"public" fields
};

//Here's the routine that will replace the standard error_exit method:
void my_error_exit(jpeg::j_common_ptr cinfo)
{
  //cinfo->err really points to a my_error_mgr struct, so coerce pointer
  //my_error_mgr* myerr = reinterpret_cast<my_error_mgr*>(cinfo->err);

  //Don't display the message.
  //We could postpone this until after returning, if we chose.
  //(*cinfo->err->output_message) (cinfo);

  throw false;
}

} //anonymous namespace

void CImageSerializerJPG::Decode(CStream* pStream, CImage* pImage) const
{
    if (pImage == NULL || pStream == NULL)
    {
		throw std::invalid_argument( "NULL pointer" );
    }

    //This struct contains the JPEG decompression parameters and pointers to
    //working space (which is allocated as needed by the JPEG library).
    jpeg::jpeg_decompress_struct cinfo;

    //We use our private extension JPEG error handler.
    //Note that this struct must live as long as the main JPEG parameter
    //struct, to avoid dangling-pointer problems.
    my_error_mgr jerr;

    //Allocate and initialize JPEG decompression object

    //We set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg::jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

	std::vector<jpeg::JSAMPLE> Row; //Output row buffer

    try
    {
        //Now we can initialize the JPEG decompression object.
        jpeg_create_decompress(&cinfo);

        //Specify data source
        jpeg_Stream_src(&cinfo, pStream);

        //Read file parameters with jpeg_read_header()
        //We can ignore the return value from jpeg_read_header since
        //  (a) suspension is not possible with the data source, and
        //  (b) we passed TRUE to reject a tables-only JPEG file as an error.
        //See libjpeg.doc for more info.
        jpeg::jpeg_read_header(&cinfo, TRUE);

        //Set parameters for decompression
        //We don't need to change any of the defaults set by
        //jpeg_read_header(), so we do nothing here.
        
        //Start decompressor
        //We can ignore the return value since suspension is not possible
        //with the data source.
        jpeg::jpeg_start_decompress(&cinfo);

        //After jpeg_start_decompress() we have the correct scaled
        //output image dimensions available, as well as the output colormap
        //if we asked for color quantization.

        CImage::Desc Desc;

        if (cinfo.out_color_space == jpeg::JCS_GRAYSCALE &&
            cinfo.output_components == 1)
        {
            Desc.Format = EPixelFormat::L8;
        } else if (cinfo.out_color_space == jpeg::JCS_RGB &&
            cinfo.output_components == 3) {
            Desc.Format = EPixelFormat::X8R8G8B8;
        } else {
            //Unrecognized combination of color space and number of components
            throw false;
        }

        Desc.Width = cinfo.output_width;
        Desc.Height = cinfo.output_height;
		Desc.Depth = 1;
        Desc.PaletteSize = 0;

        pImage->Init(Desc);

        //Get the image description again, including updated pitch information
        Desc = pImage->GetDesc();

        //Lock image data
        HImageLock hLock = const_cast<CImage*>( pImage )->Lock( EGraphicsResourceLock::ReadOnly );
        uint8 *pData = hLock->GetDataPtr();
	    //HImageLock hLockPal = const_cast<CImage*>( pImage )->LockPalette( EGraphicsResourceLock::ReadOnly );
        //uint8 *pPal = hLockPal->GetDataPtr();

        //Make an output work buffer of the right size.
        Row.resize( cinfo.output_width * cinfo.output_components );

        jpeg::JSAMPROW a[1];
        a[0] = &Row[0];

		jpeg::JSAMPLE* pSourceRow = &Row[0];

        //Read scanlines
        //Here we use the library's state variable cinfo.output_scanline as the
        //loop counter, so that we don't have to keep track ourselves.
        while (cinfo.output_scanline < cinfo.output_height) 
        {
            uint8* pDestRow = pData + cinfo.output_scanline * Desc.RowPitch;

            //jpeg_read_scanlines expects an array of pointers to scanlines.
            //Here the array is only one element long, but you could ask for
            //more than one scanline at a time if that's more convenient.
            jpeg::jpeg_read_scanlines(&cinfo, a, 1);

            switch (Desc.Format)
            {
            default:
                assert( false );
                throw false;
            case EPixelFormat::L8:
                std::copy(&pSourceRow[0], &pSourceRow[Desc.Width], pDestRow);
                break;
            case EPixelFormat::X8R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    pDestRow[j * 4 + 0] = pSourceRow[j * 3 + 2]; //B
                    pDestRow[j * 4 + 1] = pSourceRow[j * 3 + 1]; //G
                    pDestRow[j * 4 + 2] = pSourceRow[j * 3 + 0]; //R
                }
                break;
            }
        }

		Row.clear();

        //Finish decompression
        //We can ignore the return value since suspension is not possible
        //with the data source.
        jpeg::jpeg_finish_decompress(&cinfo);

    } catch (...) {

        //If we get here, the JPEG code has signaled an error.
        //We need to clean up the JPEG object and return.
        jpeg::jpeg_destroy_decompress(&cinfo);
		throw;
    }

    //Release JPEG decompression object
    //This is an important step since it will release a good deal of memory.
    jpeg::jpeg_destroy_decompress(&cinfo);

    //At this point you may want to check to see whether any corrupt-data
    //warnings occurred (test whether jerr.pub.num_warnings is nonzero).
}

int32 CImageSerializerJPG::Encode( CStream* pStream, CImage const* pImage ) const
{
    //If image pointer is NULL, then fail
    if (pImage == NULL || pStream == NULL)
    {
        return 0;
    }
    
	CImage::Desc Desc = pImage->GetDesc();

    //If image is locked or has the wrong format, then fail
    if (pImage->IsLocked() || pImage->IsPaletteLocked() ||
		Desc.Depth != 1)
    {
        return 0;
    }

	switch (Desc.Format)
	{
        case EPixelFormat::P8_R8G8B8:
        case EPixelFormat::X8R8G8B8:
        case EPixelFormat::A8R8G8B8:
		case EPixelFormat::R32F:
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
        case EPixelFormat::L8:
			break;
		default:
			return 0; //Unsupported format
	}

    uint32 OrigPosition = pStream->GetPosition();

    //JPEG compression object
    jpeg::jpeg_compress_struct cinfo;

    //JPEG error handler
    my_error_mgr jerr;

    //Row of pixel data
	std::vector<jpeg::JSAMPLE> Row;

    //We set up the normal JPEG error routines, then override error_exit.
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    //Be prepared to catch exceptions produced by my_error_exit
    try
    {
        //Now we can initialize the JPEG compression object.
        jpeg::jpeg_create_compress(&cinfo);

        //Specify data destination
        jpeg_Stream_dest(&cinfo, pStream);

        //Set parameters for compression
        //First we supply a description of the input image.
        //Four fields of the cinfo struct must be filled in:
        cinfo.image_width = Desc.Width; //image width and height, in pixels
        cinfo.image_height = Desc.Height;
        switch (Desc.Format)
        {  
        default:
            assert( false );
        case EPixelFormat::P8_R8G8B8:
        case EPixelFormat::X8R8G8B8:
        case EPixelFormat::A8R8G8B8:
		case EPixelFormat::R32F:
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
            cinfo.input_components = 3;		//# of color components per pixel
            cinfo.in_color_space = jpeg::JCS_RGB; 	//colorspace of input image
            break;
        case EPixelFormat::L8:
            cinfo.input_components = 1;
            cinfo.in_color_space = jpeg::JCS_GRAYSCALE;
            break;
        }

        //Set default compression parameters.
        jpeg::jpeg_set_defaults(&cinfo);

        //Choose quality between 0 and 100.
        int32 const Quality = Float32ToInt32(100 * m_Quality);
        assert( Quality >= 0 && Quality <= 100 );
        jpeg::jpeg_set_quality(&cinfo, Quality, TRUE);

        //Start compressor
        //TRUE ensures that we will write a complete interchange-JPEG file.
        //Pass TRUE unless you are very sure of what you're doing.
        jpeg::jpeg_start_compress(&cinfo, TRUE);

        //Lock image data
	    HImageLock hLock = const_cast<CImage*>( pImage )->Lock( EGraphicsResourceLock::ReadOnly );
        uint8 *pData = hLock->GetDataPtr();
		HImageLock hLockPal = const_cast<CImage*>( pImage )->LockPalette( EGraphicsResourceLock::ReadOnly );
        uint8 *pPal = hLockPal->GetDataPtr();

        //Write scanlines
        Row.resize( cinfo.image_width * cinfo.num_components );

        jpeg::JSAMPROW a[1];
        a[0] = &Row[0];

        while (cinfo.next_scanline < cinfo.image_height) 
        {
            uint8 const* pSrcRow = pData + cinfo.next_scanline * Desc.RowPitch;

            switch (Desc.Format)
            {
            case EPixelFormat::L8:
                std::copy(pSrcRow, pSrcRow + Desc.Width, &Row[0]);
                break;
            case EPixelFormat::X8R8G8B8:
            case EPixelFormat::A8R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    Row[j * 3 + 0] = pSrcRow[j * 4 + 2]; //R
                    Row[j * 3 + 1] = pSrcRow[j * 4 + 1]; //G
                    Row[j * 3 + 2] = pSrcRow[j * 4 + 0]; //B
                }
                break;
            case EPixelFormat::P8_R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    uint32 Index = pSrcRow[j];
                    Row[j * 3 + 0] = pPal[Index * 3 + 0]; //R
                    Row[j * 3 + 1] = pPal[Index * 3 + 1]; //G
                    Row[j * 3 + 2] = pPal[Index * 3 + 2]; //B
                }
                break;
			case EPixelFormat::R32F:
				for (int32 j = 0; j < Desc.Width; ++j)
				{
					int32 r = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[j] );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					Row[j * 3 + 0] = static_cast<uint8>( r ); //R
					Row[j * 3 + 1] = 0xffu; //G
					Row[j * 3 + 2] = 0xffu; //B
				}
				break;
			case EPixelFormat::A32B32G32R32F:
				for (int32 j = 0; j < Desc.Width; ++j)
				{
					int32 r = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[4 * j + 0] );
					int32 g = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[4 * j + 1] );
					int32 b = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[4 * j + 2] );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					Row[j * 3 + 0] = static_cast<uint8>( r ); //R
					Row[j * 3 + 1] = static_cast<uint8>( g ); //G
					Row[j * 3 + 2] = static_cast<uint8>( b ); //B
				}
				break;
			case EPixelFormat::A16B16G16R16F:
				for (int32 j = 0; j < Desc.Width; ++j)
				{
					int32 r = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pSrcRow )[4 * j + 0] ) );
					int32 g = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pSrcRow )[4 * j + 1] ) );
					int32 b = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pSrcRow )[4 * j + 2] ) );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					Row[j * 3 + 0] = static_cast<uint8>( r ); //R
					Row[j * 3 + 1] = static_cast<uint8>( g ); //G
					Row[j * 3 + 2] = static_cast<uint8>( b ); //B
				}
				break;
            }

            jpeg::jpeg_write_scanlines(&cinfo, a, 1);
        }

		Row.clear();

        //Finish compression
        jpeg_finish_compress(&cinfo);

        //Release JPEG compression object
        jpeg_destroy_compress(&cinfo);

        return pStream->GetPosition() - OrigPosition;
    } catch (...) {

        //If we get here, the JPEG code has signaled an error.
        //We need to clean up the JPEG object and return.
        jpeg::jpeg_destroy_compress(&cinfo);

		throw;
    }
}

} //namespace Graphics
} //namespace WT
