#include "sv_debayer.h"
#include "svgige.h"

#define CLIP(in, out)\
   in = in < 0 ? 0 : in;\
   in = in > 255 ? 255 : in;\
   out=in;

void ClearBorders(unsigned char *rgb, int sx, int sy, int w) {
   int i, j;
   // black edges are added with a width w:
   i = 3 * sx * w - 1;
   j = 3 * sx * sy - 1;
   while (i >= 0) {
      rgb[i--] = 0;
      rgb[j--] = 0;
   }
   i = sx * (sy - 1) * 3 - 1 + w * 3;
   while (i > sx) {
      j = 6 * w;
      while (j > 0) {
         rgb[i--] = 0;
         j--;
      }
      i -= (sx - 2 * w) * 3;
   }
}

void simple(const unsigned char *bayer, unsigned char *rgb, int sx, int sy, int tile) {
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;

    int blue = tile == GVSP_PIX_BAYBG8|| tile ==  GVSP_PIX_BAYGB8 || tile ==  GVSP_PIX_BAYBG12_PACKED || tile == GVSP_PIX_BAYGB12_PACKED ? -1 : 1;
    int start_with_green = tile == GVSP_PIX_BAYGB8 || tile ==GVSP_PIX_BAYGR8 || tile ==GVSP_PIX_BAYGB12_PACKED || tile ==GVSP_PIX_BAYGR12_PACKED;

    int i, imax, iinc;

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
        rgb[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
        rgb[i++] = 0;
        rgb[i++] = 0;
        rgb[i++] = 0;
    }

    rgb += 1;
    width -= 1;
    height -= 1;

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
        const unsigned char *bayerEnd = bayer + width;

        if (start_with_green) {
            rgb[-blue] = bayer[1];
            rgb[0] = (bayer[0] + bayer[bayerStep + 1] + 1) >> 1;
            rgb[blue] = bayer[bayerStep];
            bayer++;
            rgb += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[-1] = bayer[0];
                rgb[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
                rgb[1] = bayer[bayerStep + 1];

                rgb[2] = bayer[2];
                rgb[3] = (bayer[1] + bayer[bayerStep + 2] + 1) >> 1;
                rgb[4] = bayer[bayerStep + 1];
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[1] = bayer[0];
                rgb[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
                rgb[-1] = bayer[bayerStep + 1];

                rgb[4] = bayer[2];
                rgb[3] = (bayer[1] + bayer[bayerStep + 2] + 1) >> 1;
                rgb[2] = bayer[bayerStep + 1];
            }
        }

        if (bayer < bayerEnd) {
            rgb[-blue] = bayer[0];
            rgb[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
            rgb[blue] = bayer[bayerStep + 1];
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }
}

void HQLinear( unsigned char const * bayer, unsigned char *rgb, int sx, int sy, int tile) {
   const int bayerStep = sx;
   const int rgbStep = 3 * sx;
   int width = sx;
   int height = sy;

   int blue = tile == GVSP_PIX_BAYBG8|| tile ==  GVSP_PIX_BAYGB8 || tile ==  GVSP_PIX_BAYBG12_PACKED || tile == GVSP_PIX_BAYGB12_PACKED ? -1 : 1;
   int start_with_green = tile == GVSP_PIX_BAYGB8 || tile ==GVSP_PIX_BAYGR8 || tile ==GVSP_PIX_BAYGB12_PACKED || tile ==GVSP_PIX_BAYGR12_PACKED;

   ClearBorders(rgb, sx, sy, 2);
   rgb += 2 * rgbStep + 6 + 1;
   height -= 4;
   width -= 4;

   /* We begin with a (+1 line,+1 column) offset with respect to bilinear decoding, so start_with_green is the same, but blue is opposite */
   blue = -blue;

   for (; height--; bayer += bayerStep, rgb += rgbStep) {
      int t0, t1;
       unsigned char const* bayerEnd = bayer + width;
      const int bayerStep2 = bayerStep * 2;
      const int bayerStep3 = bayerStep * 3;
      const int bayerStep4 = bayerStep * 4;

      if (start_with_green) {
         /* at green pixel */
         rgb[0] = bayer[bayerStep2 + 2];
         t0 = rgb[0] * 5
               + ((bayer[bayerStep + 2] + bayer[bayerStep3 + 2]) << 2)
               - bayer[2]
               - bayer[bayerStep + 1]
               - bayer[bayerStep + 3]
               - bayer[bayerStep3 + 1]
               - bayer[bayerStep3 + 3]
               - bayer[bayerStep4 + 2]
               + ((bayer[bayerStep2] + bayer[bayerStep2 + 4] + 1) >> 1);
         t1 = rgb[0] * 5 +
               ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 3]) << 2)
               - bayer[bayerStep2]
               - bayer[bayerStep + 1]
               - bayer[bayerStep + 3]
               - bayer[bayerStep3 + 1]
               - bayer[bayerStep3 + 3]
               - bayer[bayerStep2 + 4]
               + ((bayer[2] + bayer[bayerStep4 + 2] + 1) >> 1);
         t0 = (t0 + 4) >> 3;
         CLIP(t0, rgb[-blue]);
         t1 = (t1 + 4) >> 3;
         CLIP(t1, rgb[blue]);
         bayer++;
         rgb += 3;
      }

      if (blue > 0) {
         for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
            /* B at B */
            rgb[1] = bayer[bayerStep2 + 2];
            /* R at B */
            t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                   bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
                  -
                  (((bayer[2] + bayer[bayerStep2] +
                     bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                     2]) * 3 + 1) >> 1)
                  + rgb[1] * 6;
            /* G at B */
            t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                   bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2]) << 1)
                  - (bayer[2] + bayer[bayerStep2] +
                     bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                  + (rgb[1] << 2);
            t0 = (t0 + 4) >> 3;
            CLIP(t0, rgb[-1]);
            t1 = (t1 + 4) >> 3;
            CLIP(t1, rgb[0]);
            /* at green pixel */
            rgb[3] = bayer[bayerStep2 + 3];
            t0 = rgb[3] * 5
                  + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
                  - bayer[3]
                  - bayer[bayerStep + 2]
                  - bayer[bayerStep + 4]
                  - bayer[bayerStep3 + 2]
                  - bayer[bayerStep3 + 4]
                  - bayer[bayerStep4 + 3]
                  +
                  ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
                    1) >> 1);
            t1 = rgb[3] * 5 +
                  ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
                  - bayer[bayerStep2 + 1]
                  - bayer[bayerStep + 2]
                  - bayer[bayerStep + 4]
                  - bayer[bayerStep3 + 2]
                  - bayer[bayerStep3 + 4]
                  - bayer[bayerStep2 + 5]
                  + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
            t0 = (t0 + 4) >> 3;
            CLIP(t0, rgb[2]);
            t1 = (t1 + 4) >> 3;
            CLIP(t1, rgb[4]);
         }
      } else {
         for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
            /* R at R */
            rgb[-1] = bayer[bayerStep2 + 2];
            /* B at R */
            t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                   bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
                  -
                  (((bayer[2] + bayer[bayerStep2] +
                     bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                     2]) * 3 + 1) >> 1)
                  + rgb[-1] * 6;
            /* G at R */
            t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                   bayer[bayerStep2 + 3] + bayer[bayerStep * 3 +
                   2]) << 1)
                  - (bayer[2] + bayer[bayerStep2] +
                     bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                  + (rgb[-1] << 2);
            t0 = (t0 + 4) >> 3;
            CLIP(t0, rgb[1]);
            t1 = (t1 + 4) >> 3;
            CLIP(t1, rgb[0]);

            /* at green pixel */
            rgb[3] = bayer[bayerStep2 + 3];
            t0 = rgb[3] * 5
                  + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
                  - bayer[3]
                  - bayer[bayerStep + 2]
                  - bayer[bayerStep + 4]
                  - bayer[bayerStep3 + 2]
                  - bayer[bayerStep3 + 4]
                  - bayer[bayerStep4 + 3]
                  +
                  ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
                    1) >> 1);
            t1 = rgb[3] * 5 +
                  ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
                  - bayer[bayerStep2 + 1]
                  - bayer[bayerStep + 2]
                  - bayer[bayerStep + 4]
                  - bayer[bayerStep3 + 2]
                  - bayer[bayerStep3 + 4]
                  - bayer[bayerStep2 + 5]
                  + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
            t0 = (t0 + 4) >> 3;
            CLIP(t0, rgb[4]);
            t1 = (t1 + 4) >> 3;
            CLIP(t1, rgb[2]);
         }
      }

      if (bayer < bayerEnd) {
         /* B at B */
         rgb[blue] = bayer[bayerStep2 + 2];
         /* R at B */
         t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
               -
               (((bayer[2] + bayer[bayerStep2] +
                  bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                  2]) * 3 + 1) >> 1)
               + rgb[blue] * 6;
         /* G at B */
         t1 = (((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                 bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2])) << 1)
               - (bayer[2] + bayer[bayerStep2] +
                  bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
               + (rgb[blue] << 2);
         t0 = (t0 + 4) >> 3;
         CLIP(t0, rgb[-blue]);
         t1 = (t1 + 4) >> 3;
         CLIP(t1, rgb[0]);
         bayer++;
         rgb += 3;
      }

      bayer -= width;
      rgb -= width * 3;

      blue = -blue;
      start_with_green = !start_with_green;
   }
}
