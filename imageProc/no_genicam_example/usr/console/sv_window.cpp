#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#include <stdio.h>
#include <stdlib.h>

#include "svgige.h"
#include "sv_window.h"
#include "sv_gec.h"

/* initializations */

// variables for GTK controls
GtkWidget *wnd;
GtkWidget *drawingarea;

// image buffer
unsigned char *buffer;
unsigned int imgWidth;
unsigned int imgHeight;

// internal states
int windowOpen;
int imageAcquisitionRunning;

// 0 for grayscale (1 Byte per pixel), else for RGB (3 Byte pre pixel)
bool isColor;

/// set format of image
/**
 * set format of image:
 * 0  : grayscale
 * >0 : rgb
 */
void setImageFormat(bool color)
{
    isColor = color;
}

/// is the window open?
/**
 * 
 */
int displayWindowOpen() 
{
	return windowOpen;
}

/// status of image acquisition
/**
 * 
 */
void setImageAcquisitionRunning(int value) 
{
	imageAcquisitionRunning = value;
	
}

/// event for destroying window
/**
 * 
 * \param wdgt widget
 * \param data data
 */
void e_destroy(GtkWidget *wdgt, gpointer data) 
{
	windowOpen = FALSE;
	gtk_widget_destroy(drawingarea);
	gtk_widget_destroy(wnd);
	g_print(" > Window being forcefully terminated!\n");
	wnd = NULL;
	drawingarea = NULL;
	gtk_main_quit();
}

/// event for updating image
/**
 * 
 * \param wdgt widget
 * \param data data
 */
void e_draw_image(GtkWidget *widget, gpointer data) 
{
	if (imageAcquisitionRunning == TRUE) 
	{
        if (isColor == false)
		{
			gdk_draw_gray_image(drawingarea->window, drawingarea->style->fg_gc[GTK_STATE_NORMAL], 0, 0, imgWidth, imgHeight, GDK_RGB_DITHER_NONE, buffer, imgWidth);
		} 
		else 
		{
			gdk_draw_rgb_image(drawingarea->window, drawingarea->style->fg_gc[GTK_STATE_NORMAL], 0, 0, imgWidth, imgHeight, GDK_RGB_DITHER_NONE, buffer, imgWidth * 3);
		}
	}
}

/// close window
/**
 * 
 */
void closeWindow() 
{
	windowOpen = FALSE;
	gtk_widget_destroy(drawingarea);
	gtk_widget_destroy(wnd);
	gtk_main_quit();
}

/// draw buffer to drawing area
/**
 * 
 */
void drawImage() 
{
	if (imageAcquisitionRunning == TRUE) 
	{
		gdk_threads_enter();
		
		if (drawingarea != NULL) 
		{
            if (isColor == false)
			{
				gdk_draw_gray_image(drawingarea->window, drawingarea->style->fg_gc[GTK_STATE_NORMAL], 0, 0, imgWidth, imgHeight, GDK_RGB_DITHER_NONE, buffer, imgWidth);
			} 
			else 
			{
				gdk_draw_rgb_image(drawingarea->window, drawingarea->style->fg_gc[GTK_STATE_NORMAL], 0, 0, imgWidth, imgHeight, GDK_RGB_DITHER_NONE, buffer, imgWidth * 3);
			}
		}
		
		gdk_threads_leave();
	}
}

/// assigns buffer data to local variables
/**
 * 
 * \param buf image buffer
 * \param width of buffer
 * \param height of buffer
 */
void setBuffer(unsigned char* buf, unsigned int width, unsigned int height) 
{
	imgWidth = width;
	imgHeight = height;
	buffer = buf;
	
	if( windowOpen == TRUE )
 		gtk_widget_set_size_request(drawingarea, imgWidth, imgHeight);
}

/// creates window and controls
/**
 * main functions, creates window, adds controls and assigns signals
 * 
 * \return 
 * \param argc argument count (ignored)
 * \param argv argument vector (ignored)
 */
int openWindow(int argc, char **argv, int ImgWidth, int ImgHeight) 
{
	windowOpen = TRUE;
	
	//gtk_init(&argc, &argv);
	gtk_init(0, NULL);
	
	buffer = NULL;
	
	if (!g_thread_supported ()) 
	{
		g_thread_init(NULL);
	}
	gdk_threads_init();

	//gdk_rgb_set_verbose(TRUE);
	gdk_rgb_init();
	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	gtk_widget_set_default_visual(gdk_rgb_get_visual());
	
	//GtkWidget *wnd;
	wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_border_width(GTK_CONTAINER(wnd), 15); /* 15px padding */
	gtk_window_set_title(GTK_WINDOW(wnd), "GTK GUI example");
	gtk_window_resize(GTK_WINDOW(wnd), ImgWidth + 20, ImgHeight + 20);
	gtk_signal_connect(GTK_OBJECT(wnd), "destroy", GTK_SIGNAL_FUNC(e_destroy), NULL);
	
	drawingarea = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(drawingarea), "expose_event", GTK_SIGNAL_FUNC(e_draw_image), NULL);
	gtk_widget_set_size_request(drawingarea, ImgWidth, ImgHeight);
	gtk_widget_set_double_buffered(drawingarea, TRUE);
	gtk_widget_show(drawingarea);
	
	GtkWidget *vbox;
	vbox = gtk_vbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox),drawingarea, FALSE, FALSE, 0);
	//gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(wnd), vbox);
	gtk_widget_show(vbox);

	
	gtk_widget_show(wnd);

	gdk_rgb_init();
	
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	
	return 0;
}
