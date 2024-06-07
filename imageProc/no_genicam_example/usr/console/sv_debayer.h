#ifndef SV_DEBAYER_H
#define SV_DEBAYER_H

void simple(const unsigned char *bayer, unsigned char *rgb, int sx, int sy, int tile) ;
void HQLinear( unsigned char const* bayer, unsigned char *rgb, int sx, int sy, int tile);
void ClearBorders( unsigned char *rgb, int sx, int sy, int w);


#endif // SV_DEBAYER_H
