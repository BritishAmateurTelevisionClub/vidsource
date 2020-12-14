//#define VERSION "tcanim1v15"

#define VERSION "tcprog (2020-10-28)" 

// particles.c - Simple particles example using the OpenVG Testbed
// via Nick Williams (github.com/nilliams)
// https://gist.githubusercontent.com/nilliams/7705819/raw/9cbb5a1298d6eef858639095148ede2c33cb6d40/particles.c
//
// Usage: ./particles [OPTIONS]
//
// Options:
//  -t  show trails
//
//  -r  right-to-left only  (direction alternates by default)
//  -l  left-to-right only
//
// Modified by Brian Jordan, G4EWJ


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
//#include "openvg.h"
//#include "vgu.h"
#include "shapes.h"
#include <dirent.h>
#include <fnmatch.h>
#include <math.h>

typedef unsigned char uchar ;
typedef unsigned short uint16 ;

#define NUM_PARTICLES 		100
#define CYCLEFIELDS			15000

	int 			angle ;
	int 			np ;
	int				changes ;
	struct timeval	now ;
	int 			cycles ;
	int				activecount ;
	int				xoffset ;
	int 			yoffset ;
	char			balloontext [6] ;
	char			filename [1024] ;
	int				frames ;
	int				banneractive ;
	int 			bannertoggle ;
	int				balloonsactive ;
	char			bannertext [65005] ;
	int				counter1, counter2 ;
	int 			W, H ;
	float			PI ;

#define	MAXNAMES	1000
	VGImage			save [MAXNAMES] ;
    struct dirent 	**namelist ;
	int				namecount ;
	int				nameindex ;
	int				nameseconds ;
	int				namefields ;

typedef struct particle 
{
	int				active ;
	float			x ;
	float	        y ;
	int 			vx ;
	int				vy ;
	int 			r, g, b;
	int				radius;
	int				cycles ;	
} particle_t;

	particle_t 	particles[NUM_PARTICLES] ;
	int 		showTrails 		= 0 ;
	int 		directionRTL 	= 1 ;
	int 		alternate 		= 1 ;
	float 		gravity 		= 0.28 ;
	
	int			scan			(void) ;
	int			filter			(const struct dirent *) ;
	void		makeImage		(void) ;


// Initialize _all_ the particles

void initParticles (int W, int H) 
{
	int i;

	for (i = 0; i < NUM_PARTICLES; i++) 
	{
		particle_t *p = &particles[i];
		p->active = 0 ;
		p->x = 0;
		p->y = 0;
		p->vx = 10000 * ((rand() % 10) + 11) ;;
		p->vy = 10000 * ((rand() % 17) + 20) ;
		p->vy = 10000 ;
		p->r = rand() % 256;
		p->g = rand() % 256;
		p->b = rand() % 256;
		p->radius = (rand() % 20) + 30;
		if (directionRTL) 
		{
			p->vx *= -1;
			p->x = 10000 * W ;
		}
		p->cycles = 1 ;
	}
}

void draw (int w, int h) 
{
	int 		i;
	particle_t 	*p;
	int			fontsize ;
	int			tempf ;
	int 		temp ;
	
	vgSetPixels (0,0,save[nameindex],0,0,w,h) ;
	if (banneractive == 1)
	{
		Fill (0,0,0,1) ;
		if (bannertoggle & 1)
		{
			Fill (128,0,192,1) ;
		}
		else
		{
			Fill (128,0,192,1) ;
		}
		Fill (255,255,255,1) ;
		Text (w-frames*2,28,bannertext,SansTypeface,19) ;
		frames++ ;
		Fill (0,0,0,1) ;
		temp = TextWidth (bannertext,SansTypeface,19) ;
		if (temp - frames * 2 < - W)
		{
			frames = 0 ;
			bannertoggle++ ;
			if (CYCLEFIELDS && cycles >= CYCLEFIELDS)
			{
				banneractive = 1 ;
			}
		}
	}

	activecount = 0 ;
	for (i = 0; i < NUM_PARTICLES ; i++) 
	{
		p = &particles[i];
		if (p->active)
		{
			activecount++ ;
			Fill(p->r, p->g, p->b, 1.0);
			Circle(p->x/10000, p->y/10000, p->radius);

			tempf = (299 * p->r + 587 * p->g + 114 * p->b) / 1000 ;
			if (tempf >= 128)
			{
				Fill (0,0,0,1) ;
			}
			else
			{
				Fill (255,255,255,1) ;
			}
			fontsize = (14+2*strlen(balloontext)) * p->radius / TextWidth(balloontext,SansTypeface,25) ;	
			TextMid (p->x/10000,p->y/10000-fontsize/2,balloontext,SansTypeface,fontsize) ;	

// Apply the velocity

			p->x += p->vx / 5 ;
			p->y += p->vy / 5 ;

// Gravity

			p->vy -= 10 * 10000 * gravity / 12 ;

// Stop particles leaving the canvas  

			if (p->x / 10000 < -50)	p->x = (w + 50) * 10000 ;
			if (p->x / 10000 > w + 50) p->x = -50 * 10000 ;

// When particle reaches the bottom of screen reset velocity & start posn
			
			if (p->y < -50 * 10000) 
			{
				p->cycles++ ;
				p->x = 0;
				p->y = 0;
				p->vx = 10000 * ((rand() % 20) + 10) ;
				p->vy = 10000 * ((rand() % 12) + 25) ;
	
				if (directionRTL) 
				{
					p->vx *= -1;
					p->x = w * 10000 ;
				}
				rand() ;
				p->r = rand() % 256;
				p->g = rand() % 256;
				p->b = rand() % 256;
				p->radius = (rand() % 30) + 40 ;
				if (CYCLEFIELDS && cycles >= CYCLEFIELDS && banneractive == 2)
				{
					p->active = 0 ;
				}
			}

			if (p->y / 10000 > h + 50) p->y = -50 * 10000;
		}
	}
}


void makeImage ()
{
	for (nameindex = 0 ; nameindex < namecount ; nameindex++)
	{
		if (save[nameindex] != 0)
		{
			vgDestroyImage (save[nameindex]) ;
		}
		Start (W, H);
		save [nameindex] = vgCreateImage (VG_lRGBX_8888,W,H,VG_IMAGE_QUALITY_NONANTIALIASED) ;
		Fill (0,0,0,1);
		Rect (0, 0, W, H);
		Image (xoffset,yoffset,W-xoffset,H-yoffset,namelist[nameindex]->d_name) ;
		
	  	if (strlen(bannertext))
	  	{	
			Fill (128,0,192,1) ;
			Rect (0,20,W,34) ;
	  	}
   	    Fill (0,0,0,1) ;
	    vgFinish() ;
		vgGetPixels (save[nameindex],0,0,0,0,W,H) ;
	}
}


int main(int argc, char **argv) 
{
	int 		i ;
	int			x ;
	int 		temp ;
	int			errors ;
	char		*p ;	

	init (&W, &H);

///	H = 720 ; W = 576 ;		// uncomment this line to force 720x576

	i = 0 ;
	PI = 4 * atan (1) ;

	counter1 = 0 ; counter2 = 0 ;

	errors = 0 ;
	if (argc != 6)
	{
		errors++ ;
	}
	else
	{
		strncpy (filename,argv[1],sizeof(filename)-5) ;
		p = strchr (filename,'*') ;
		if (p)
		{
			nameseconds = atoi (p+1) ;
			if (nameseconds == 0)
			{
				nameseconds = 1 ;
			}
			*p = 0 ;
			p = strstr (filename,".jpg") ;
			if (p)
			{
				*p = 0 ;
			}
			strcat (filename,"*.jpg") ;	
		}
		p = strstr (filename,".jpg") ;
		if (p == 0)
		{
			strcat (filename,".jpg") ;
		}
		namecount = scan() ;
		if (namecount > MAXNAMES)
		{
			namecount = MAXNAMES ;
		}
		if (namecount == 0)
		{
			printf ("Cannot find %s\n\n",filename) ;
			exit (1) ;
		}
		xoffset = atoi (argv[2]) ;
		if (abs(xoffset) > W)
		{
			errors++ ;
		}
		yoffset = atoi (argv[3]) ;
		if (abs(yoffset) > H)
		{
			errors++ ;
		}
		strncpy (balloontext,argv[4],sizeof(balloontext)-1) ;
		strncpy (bannertext,argv[5],sizeof(bannertext)-5) ;
		if (strlen(bannertext))
		{
			strcat (bannertext,"    ") ;
		}
	}
//	printf ("\n") ;
	if (errors)
	{
		printf ("Usage: %s \"FILENAME(S)\" XOFFSET YOFFSET \"BALLOONTEXT\" \"BANNERTEXT\" \n",VERSION) ;
		printf ("FILENAME(S) may be a single file or FILENAMES*n - it is not necessary to specify the .jpg extension\n") ;
		printf ("FILENAMES*n displays all files that start with FILENAMES in alpha order changing every n seconds\n") ;
		printf ("FILENAME(S) should be enclosed in double quotes\n") ;
		printf ("\n") ;
		exit (2) ;
	}

//	printf ("\n%s: Test Card Animator\n",VERSION) ;
	if (namecount > 1)
	{
		printf ("Using image files in this order: \n") ;
		for (nameindex = 0 ; nameindex < namecount ; nameindex++)
		{
			printf ("%s\n",namelist[nameindex]->d_name) ;
		}
		printf ("\n") ;
	}

	memset (save,0,sizeof(save)) ;

	makeImage() ;	
	
	Start (W, H) ;
	vgSetPixels (0, 0, save[0], 0, 0, W, H) ;
	End() ;

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);	
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);	
	vgLoadIdentity() ;

	namefields = 0 ;
	nameindex = 0 ;
	
    while (1)
    {
		counter1++ ;
		memset (particles,0,sizeof(particles)) ;
		frames = 0 ;
		cycles = 0 ;
		changes = 0 ;
		angle = 0 ;
		directionRTL = 1 ;
		alternate = 1 ;

		if (strlen(balloontext) > 0 )
		{
			np = 4 ;
		}
		else
		{
			np = 0 ;
		}
		
		srand(time(NULL));

		initParticles(W, H);
		for (x = 0 ; x < np ; x++)
		{
			particles[x].active = 1 ;		
		}


		if (strlen(bannertext) > 0)
		{
			banneractive = 1 ;
		}
		else
		{
			banneractive = 1 ;
		}

		Start (W,H) ;
		End() ;

		Start (W,H) ;
		End() ;
			
		balloonsactive = 0 ;
		while (1) 
		{
			Start (W, H);
			counter2++ ;
			draw (W, H);
			Fill (255,255,255,1) ;
			End() ;
			cycles++ ;

			namefields++ ;
			if (namefields >= nameseconds * 50)
			{
				nameindex++ ;
				namefields = 0 ;
			}
			if (nameindex >= namecount)
			{
				nameindex = 0 ;
			}

			if (activecount == 0 && CYCLEFIELDS && cycles >= CYCLEFIELDS && frames == 0)
			{			
				banneractive = 1 ;
				Start (W, H);
				counter2++ ;
				draw (W, H);		
				End() ;
				break ;
			}

			temp = 1 ;
			for (x = 0 ; x < np ; x++)
			{
				temp &= particles[x].cycles ;
			}

			temp = 0 ; 

			if (temp && banneractive == 0 && strlen(bannertext))
			{
				frames = 0 ;
				banneractive = 1 ;
			}

			gettimeofday (&now,0) ;
			x = now.tv_sec * 1000 + now.tv_usec / 1000 ;

			// Change launch direction every 25 draws
			i++;
			if (alternate && i == 25) 
			{
				changes++ ;
				directionRTL = directionRTL ? 0 : 1;
				i = 0;
			}
		}
    }
}

int filter (const struct dirent *sdx)
{
	if (fnmatch(filename,sdx->d_name,0)==0)
	{
		return (1) ;
	}
	else
	{
		return (0) ;
	}
}

int scan()
{    
	int 	n ;

    	n = scandir(".", &namelist, filter, alphasort) ;
    	if (n < 0)
    	{
			perror("scandir") ;
    	}
	return (n) ;
}



