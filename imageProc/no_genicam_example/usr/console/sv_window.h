#ifndef SV_WINDOW_H
#define SV_WINDOW_H

int openWindow(int argc, char **argv, int ImgWidth, int ImgHeight);
void drawImage();
void setBuffer(unsigned char* buf, unsigned int width, unsigned int height);
void closeWindow();
int displayWindowOpen();
void setImageAcquisitionRunning(int value);
void setImageFormat(bool color);

#endif // SV_WINDOW_H
