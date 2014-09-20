/*
 * prueba.c
 *
 *  Created on: May 8, 2014
 *      Author: joel
 */

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <unistd.h>



typedef int bool;
#define true 1
#define false 0

//Lights
#define GREEN 1
#define YELLOW 2
#define RED 3


/*Apuntador a los elementos del entrenamiento de imagenes. */
CvHaarClassifierCascade *cascade;
/*Apuntador al bloque de memoria a utilizar durante el codigo. */
CvMemStorage            *storage;

/*Apuntador al archivo de video a analizar. */
CvCapture 				*capture1;
/*Apuntador a los frames del video 1*/
IplImage  				*frame1;
/*Apuntador a region de interes del video 1 */
IplImage  				*roi1Image;
/*Apuntador a la imagen despues de ajusar el tamaño*/
IplImage  				*roi1Adj;
/* Apuntador a la imagen de semaforo 1*/
IplImage  				*semaphore1;
//Puntos para ROI de imagen 1
CvPoint orig1;
CvPoint dest1;

/*Apuntador al archivo de video a analizar. */
CvCapture 				*capture2;
/*Apuntador a los frames del video 2*/
IplImage  				*frame2;
/*Apuntador a region del video 2 */
IplImage  				*roi2Image;
/*Apuntador a la imagen despues de ajusar el tamaño*/
IplImage  				*roi2Adj;
/* Apuntador a la imagen de semaforo 2*/
IplImage  				*semaphore2;
//Puntos para ROI de imagen 2
CvPoint orig2;
CvPoint dest2;

//Percent resize
int input_resize_percent = 60;

int detect(IplImage *img); // Declaracion de funcion detect
void changeSemaphore(IplImage *sem, int light, int semNum);
void mouseVideo1Callback(int event, int x, int y, int flags, void* param)
{
    static bool clicked=false;
    IplImage *tmpImage;
	switch (event)
	{
		case CV_EVENT_LBUTTONDOWN:

					tmpImage=cvClone(frame1);
					printf("Mouse X, Y: %d, %d \n", x, y);
					clicked=true;
					orig1=cvPoint(x,y);
					break;
		 case CV_EVENT_MOUSEMOVE:
			 if (clicked)
				 {
					 tmpImage=cvClone(frame1);
					 dest1=cvPoint(x,y);

					 cvCircle(tmpImage,orig1, 3, CV_RGB(0,255,0),-1,8,0);
					 cvCircle(tmpImage,dest1,3, CV_RGB(0,255,0),-1,8,0);
					 cvRectangle(tmpImage, orig1, dest1, CV_RGB(0,255,0),1,8,0);
					 cvShowImage("video1", tmpImage);
				  }
			 break;
		 case CV_EVENT_LBUTTONUP:
				 clicked=false;
				 break;
		}
}

void mouseVideo2Callback(int event, int x, int y, int flags, void* param)
{
    static bool clicked=false;
    IplImage *tmpImage;
	switch (event)
	{
		case CV_EVENT_LBUTTONDOWN:

					tmpImage=cvClone(frame2);
					printf("  Mouse X, Y: %d, %d \n", x, y);
					clicked=true;
					orig2=cvPoint(x,y);
					break;
		 case CV_EVENT_MOUSEMOVE:
			 if (clicked)
				 {
					 tmpImage=cvClone(frame2);
					 dest2=cvPoint(x,y);

					 cvCircle(tmpImage,orig2, 3, CV_RGB(0,255,0),-1,8,0);
					 cvCircle(tmpImage,dest2,3, CV_RGB(0,255,0),-1,8,0);
					 cvRectangle(tmpImage, orig2, dest2, CV_RGB(0,255,0),1,8,0);
					 cvShowImage("video2", tmpImage);
				  }
			 break;
		 case CV_EVENT_LBUTTONUP:
				 clicked=false;
				 break;
		}
}

bool changing=false;
/* Variables de control*/
bool analiza1, analiza2;


//thread for change  Red to Green Light
void *GreentoRed1(void *arg)
{
	changing=true;
	IplImage *sem=(IplImage *)arg;
	changeSemaphore(sem, YELLOW, 2);
	sleep(1);
	changeSemaphore(sem, RED, 2);
	changeSemaphore(sem, GREEN, 1);
	sleep(1);

	changing=false;

	return NULL;
}
void *GreentoRed2(void *arg)
{
	changing=true;
	IplImage *sem=(IplImage *)arg;
	changeSemaphore(sem, YELLOW, 1);
	sleep(1);
	changeSemaphore(sem, RED, 1);
	changeSemaphore(sem, GREEN, 2);
	sleep(1);

	changing=false;

	return NULL;
}

bool timeout=false;
bool reset=false;
void *timeOut(void *arg)
{


	while(1)
	{
	sleep(10);
		timeout=true;
     	if (reset)
     	{
     		timeout=false;
     		reset=false;
     	}
	}

return NULL;

}

void changeSemaphore(IplImage *sem, int light, int semNum)
{
	IplImage* lights;
	lights=cvClone(semaphore1);

	switch(light)
	{
	case GREEN:
		 cvCircle(lights,cvPoint(200,398), 48, CV_RGB(0,255,0),-1,8,0);
		break;
	case YELLOW:
		cvCircle(lights,cvPoint(200,255), 48, CV_RGB(255,255,0),-1,8,0);
		break;
	case RED:
		 cvCircle(lights,cvPoint(200,112),48, CV_RGB(255,0,0),-1,8,0);
		break;
	}
	if (semNum==1)
		cvShowImage("semaforo 1", lights);
	else if(semNum==2)
		cvShowImage("semaforo 2", lights);
}


int main()
{
  int input_resize_percent = 90;
  char *filename = "/usr/share/opencv/samples/xml/cars3.xml";

  pthread_t cambioSem;
  pthread_t timeOuts;
  //Para contar el numero de carros
  int Ncarros;

  cascade = (CvHaarClassifierCascade*) cvLoad(filename, 0, 0, 0);
  storage = cvCreateMemStorage(0);
  capture1 = cvCreateFileCapture("/usr/share/opencv/samples/videos/video1.avi");
  capture2 = cvCreateFileCapture("/usr/share/opencv/samples/videos/video2.avi");
  assert(cascade && storage && capture1); // assert if error exists

  cvNamedWindow("video1", 0);
  frame1 = cvQueryFrame(capture1);

  cvNamedWindow("video2", 0);
  frame2 = cvQueryFrame(capture2);


  if(!frame1)
    return 0;

  cvShowImage("video1", frame1);
  cvSetMouseCallback("video1", mouseVideo1Callback,NULL);

  cvShowImage("video2", frame2);
  cvSetMouseCallback("video2", mouseVideo2Callback,NULL);

  const int KEY_SPACE  = 32;
  const int KEY_ESC    = 27;
  int key=0;

  semaphore1 = cvLoadImage("/usr/share/opencv/samples/images/semaphore.png",CV_LOAD_IMAGE_COLOR);
  semaphore2 = cvLoadImage("/usr/share/opencv/samples/images/semaphore.png",CV_LOAD_IMAGE_COLOR);

  if(!(semaphore1->imageData))      // Check for invalid input
     {
        printf("Could not open or find the image\n");
         return -1;
     }

  //Init
  analiza1=true;
  analiza2=false;
  changeSemaphore(semaphore1, RED, 1);
  changeSemaphore(semaphore2, GREEN, 2);
/*
  while(1)
		{
		  key=cvWaitKey(0);
		  if (key==KEY_SPACE)
			break;
		  if (key==KEY_ESC)
			  return 0;
		}
*/
char corre;
corre=getchar();
  
pthread_create(&timeOuts,NULL,timeOut,NULL);

		  do
		  {

			 //Process 1:

			frame1 = cvQueryFrame(capture1); // pointer to a cvCapture structure

			if(!frame1)
			  break;
			//cvShowImage("video1", frame1);

			roi1Image=cvCloneImage(frame1);
			if ((orig1.x != dest1.x) && (orig1.y != dest1.y))
			{
			cvSetImageROI(roi1Image, cvRect(orig1.x<dest1.x?orig1.x:dest1.x, orig1.y<dest1.y?orig1.y:dest1.y,
						abs(dest1.x-orig1.x),abs(dest1.y-orig1.y)));
			roi1Adj = cvCreateImage(cvSize(abs(dest1.x-orig1.x)*input_resize_percent/100,
												abs(dest1.y-orig1.y)*input_resize_percent/100),
												roi1Image->depth, roi1Image->nChannels);
			}
			else
			{	cvSetImageROI(roi1Image,cvRect(0,0,frame1->width,frame1->height));

			roi1Adj = cvCreateImage(cvSize((int)((frame1->width*input_resize_percent)/100) , (int)((frame1->height*input_resize_percent)/100)),
					frame1->depth, frame1->nChannels);
			}

			cvResize(roi1Image, roi1Adj, CV_INTER_LINEAR);

			if (!changing && !analiza2)
				{
					Ncarros=detect(roi1Adj);
					
					if (Ncarros>=3)
					{
				printf("Proceso 1 Numero de Carros: %d \n", Ncarros);
							reset=true;
							  pthread_create(&cambioSem,NULL,GreentoRed1,semaphore1);
							  analiza2=true;
							  analiza1=false;
					}
				}


			if (timeout && !analiza2)
			{
				timeout=false;
				pthread_create(&cambioSem,NULL,GreentoRed1,semaphore1);
											  analiza2=true;
											  analiza1=false;
			}



			cvShowImage("video1", roi1Adj);


			//Proceso 2:
			frame2 = cvQueryFrame(capture2); // pointer to a cvCapture structure

			if(!frame2)
			  break;

			//cvShowImage("video2", frame2);
			roi2Image=cvCloneImage(frame2);

			if ((orig2.x != dest2.x) && (orig2.y != dest2.y))
			{	cvSetImageROI(roi2Image, cvRect(orig2.x<dest2.x?orig2.x:dest2.x, orig2.y<dest2.y?orig2.y:dest2.y,
						abs(dest2.x-orig2.x),abs(dest2.y-orig2.y)));
			roi2Adj = cvCreateImage(cvSize(abs(dest2.x-orig2.x)*input_resize_percent/100,
												abs(dest2.y-orig2.y)*input_resize_percent/100),
												roi1Image->depth, roi1Image->nChannels);
			}
			else
			{
				cvSetImageROI(roi2Image,cvRect(0,0,frame2->width,frame2->height));
			roi2Adj = cvCreateImage(cvSize((int)((frame2->width*input_resize_percent)/100) , (int)((frame2->height*input_resize_percent)/100)),
					frame2->depth, frame2->nChannels);
			}
			cvResize(roi2Image, roi2Adj, CV_INTER_LINEAR);


			if (!changing && !analiza1)
			{
				Ncarros=detect(roi2Adj);

				if (Ncarros>= 4)
					{
					printf("Proceso 2 Numero de Carros: %d \n", Ncarros);
							reset=true;
							  pthread_create(&cambioSem,NULL,GreentoRed2,semaphore2);
							  analiza2=false;
							  analiza1=true;
					}
		}

		if (timeout && !analiza1)
			{
				timeout=false;
				pthread_create(&cambioSem,NULL,GreentoRed2,semaphore2);
				analiza2=false;
				analiza1=true;
				}
			cvShowImage("video2", roi2Adj);



			key = cvWaitKey(2);
			if(key == KEY_SPACE)
			  key = cvWaitKey(0);

			if(key == KEY_ESC)
			  break;

	sleep(1);

			  }while(1);

				  cvDestroyAllWindows();
				  pthread_cancel(timeOuts);
				  pthread_cancel(cambioSem);
				  cvReleaseImage(&frame1);
				  cvReleaseImage(&roi1Image);
				  cvReleaseImage(&roi1Adj);
				  cvReleaseImage(&semaphore1);
				  cvReleaseCapture(&capture1);
				  cvReleaseImage(&frame2);
				  cvReleaseImage(&roi2Image);
				  cvReleaseImage(&roi2Adj);
				  cvReleaseImage(&semaphore2);
				  cvReleaseCapture(&capture2);
				  cvReleaseHaarClassifierCascade(&cascade);
				  cvReleaseMemStorage(&storage);

				  return 0;
}

/**
 *
 */

/**
 @name void detect(IplImage *img)
 @brief Detecta y muestra los objetos a identificar mediante el entrenamiento de imagenes previamente definido.

  Esta funcion analiza frame por frame del video para encontrar regiones rectangulares que son candidatas a contener objetos que el clasificador ha sido entrenado para encontrar. El clasificador devuelve los candidatos positivos como una secuencia de rectangulos.

 @param[in] IplImage* img
 @returns Video donde muestra los candidatos positivos encerrados en rectangulos.
 */

/**
 *
 */

/** @name cvHaarDetectObjects()
      @brief Busca posibles candidatos que sean objetos a seguir.

  		    Encontrar regiones rectangulares que son candidatas a contener objetos que el clasificador ha sido entrenado para encontrar. El clasificador devuelve los candidatos positivos como un objeto.

  		   @param[in] image (IntPtr) Entrenamiento de imagenes que el clasificador utilizara. En este caso es un documento XML.
  		   @param[in] storage (IntPtr)Bloque dinamico de almacenamiento donde se guardaran los objetos rectangulos candidatos a ser clasificados como objetos positivos.
  		   @param[in] scaleFactor (IntPtr) El factor por el cual se escalara la ventana de busqueda de objetos durante cada escaneo de la imagen. Por ejemplo, 1.1 significa que la ventana de busqueda se amplia un 10%.
  		   @param[in] minimumNeighbours (Double) Es el numero minimo de rectangulos adjuntos para clasificar un objeto. Todos los grupos de menor numero a este parametro son rechazados como objetos. Si el parametro es 0, la funcion no analiza los vecinos de nadie y toma todos los rectangulos como posibles candidatos.
  		   @param[in] flags (Int32) Especifica el modo de operacion. Actualmente solo existe una bandera para esto: CV_HAAR_DO_CANNY_PRUNING. Si se define, la funcion utiliza un detector de bordes Canny para rechazar regiones de la imagen que contiene muy pocos o muchos bordes que dificultan la busqueda de objetos.
  		   @param[in] minimumSize (MCvSize) Tamano minimo de la ventana de busqueda.
  		   @param[in] maximumSize (MCvSize) Tamano maximo de la ventana de busqueda.

  		   @returns[out] Candidatos positivos encerrados en rectangulos.
*/

int detect(IplImage *img)
{
	int total;
  CvSize img_size = cvGetSize(img);
  CvSeq *object = cvHaarDetectObjects(
    img,
    cascade,
    storage,
    1.1, //1.1,//1.5, //-------------------SCALE FACTOR
    2, //2        //------------------MIN NEIGHBOURS
    0, //CV_HAAR_DO_CANNY_PRUNING
    cvSize(0,0),//cvSize( 30,30), // ------MINSIZE
    img_size //cvSize(70,70)//cvSize(640,480)  //---------MAXSIZE
    );

  int i;
  for(i=0;i<(object ? object->total :0);i++)
  {
    CvRect *r = (CvRect*)cvGetSeqElem(object, i); // Finds the element with the given index in the sequence and returns the pointer to it.
    cvRectangle(img,
      cvPoint(r->x, r->y),
      cvPoint(r->x + r->width, r->y + r->height),
      CV_RGB(255, 0, 0), 2, 8, 0);
  }
  if (changing>=true)
	  total=0;
  else
	  total=(object->total);
  return total;
}
