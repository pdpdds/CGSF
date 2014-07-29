typedef struct
{
   unsigned char  d_iif_size;            // IIF size (after header), usually 0
   unsigned char  d_cmap_type;           // ignored
   unsigned char  d_image_type;          // should be 2
   unsigned char  pad[5];

   unsigned short d_x_origin;
   unsigned short d_y_origin;
   unsigned short d_width;
   unsigned short d_height;

   unsigned char  d_pixel_size;          // 16, 24, or 32
   unsigned char  d_image_descriptor;    // Bits 3-0: size of alpha channel
                                         // Bit 4: must be 0 (reserved)
                                         // Bit 5: should be 0 (origin)
                                         // Bits 6-7: should be 0 (interleaving)
} tTGAHeader_s;

#define RGB16(r,g,b)   ( ((r>>3) << 10) + ((g>>3) << 5) + (b >> 3) )
#define RGB24(r,g,b)   ( ((r) << 16) + ((g) << 8) + (b) )
#define GET16R(v)   (v >> 10)
#define GET16G(v)   ((v >> 5) & 0x1f)
#define GET16B(v)   (v & 0x1f)

unsigned char *LoadTGAFile( char* strFilename,tTGAHeader_s *header);

