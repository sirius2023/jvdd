//#include <SFML/Graphics.hpp>
//Ver 4: add clipping
#include <cmath>
#include <png.h>
#include <allegro.h>
#include "loadpng.h"
#include "judas.h"
#include "timer.h"
//using namespace sf;
typedef BITMAP* Sprite;
typedef BITMAP* Texture;
typedef unsigned int Color;
typedef BITMAP RenderWindow;
BITMAP *bgscr;
BITMAP *textbuf;
BITMAP *scoreBoardBuf;

/* Sound Related */
#if 0
//allegro sound
SAMPLE *menumove;
SAMPLE *menuselect;
SAMPLE *tune1;
SAMPLE *fart;
#endif

JUDASSAMPLE *menumove;
JUDASSAMPLE *menuselect;
JUDASSAMPLE *tune1, *tune2;
JUDASSAMPLE *fart;

struct 
{
   int width;
   int height;
   int carX, carY, carW, carH;
}screenResolData[3] =
      {
        { 640, 480, 170, 310, 280, 170 },
	{ 800, 600, 200, 390, 350, 210 },
	{ 1024, 768, 270, 498, 450, 270}
      };
int screenResolId = 0;

int pan = 128;
int pitch = 1000;

int width = 640;
int height = 480;
//int width = 1024;
//int height = 768;
int roadW = 1200;
int segL = 200; //segment length
float camD = 0.84; //camera depth
float myGCamY;
int tmpSprtCrdFg = 0;
void draw_screen_elements(BITMAP *app);
void draw_animation(BITMAP *app, int a = 0 , struct Line *ln = NULL );
int getScrWidthForElement(int);
int getScrHeightForElement(int);
void drawDebugBackg()
{
	int points[8];
    Color backg  = makecol(16,200,16);
	    //:makecol(0,154,0);
	points[0] = 0; points[1] = 80;
	points[2] = 0; points[3] = 100;
	points[4] = 640; points[5] = 100;
	points[6] = 640; points[7] = 80;
    polygon(screen, 4, points, backg);
}
void drawQuad(BITMAP *w, Color c, int x1,int y1,int w1,int x2,int y2,int w2)
{
    if ( ( (x1 - w1 >= 0) && (x1 - w1 <= screenResolData[screenResolId].width) )
	|| ( (x2 - w2 >= 0) && (x2 - w2 <= screenResolData[screenResolId].width) )
	   || ( (x1 + w1 >= 0) && (x1 + w1 <= screenResolData[screenResolId].width) )
	      || ( (x2 + w2 >= 0) && (x2 + w2 <= screenResolData[screenResolId].width) ) )
    {
	int points[8];
	points[0] = x1-w1; points[1] = y1;
	points[2] = x2-w2; points[3] = y2;
	points[4] = x2+w2; points[5] = y2;
	points[6] = x1+w1; points[7] = y1;
        //polygon(screen, 4, points, c);
        polygon(w, 4, points, c);
        //floodfill(w, points[0] + 8, points[1]-3,c);
    }
}
struct Line
{
  float x,y,z; //3d center of line
  float X,Y,W; //screen coord
  float curve,spriteX,clip,scale,myCamY;
  Sprite sprite;
  int spriteType, spriteDispFg;

  Line()
  {spriteX=curve=x=y=z=spriteType=0;spriteDispFg=1;sprite = NULL;}

  void project(int camX,int camY,int camZ)
  {
    scale = camD/(z-camZ);

    X = (1 + scale*(x - camX)) * screenResolData[screenResolId].width/2;
    Y = (1 - scale*(y - camY)) * screenResolData[screenResolId].height/2;
    W = scale * roadW  * screenResolData[screenResolId].width/2;
    /*
    X = (1 + scale*(x - camX)) * width/2;
    Y = (1 - scale*(y - camY)) * height/2;
    W = scale * roadW  * width/2;*/
    if (myGCamY > Y)
	    myGCamY = Y;
    myCamY = myGCamY;
  }

  void drawSprite(RenderWindow *app)
  {
    Sprite s = sprite;
    if (s == NULL) return;
    int w = s->w;
    int h = s->h;

    float destX = X + scale * spriteX * screenResolData[screenResolId].width/2;
    //float destX = X + scale * spriteX * width/2;
    float destY = Y + 4;
    float destW  = w * W / 266;
    float destH  = h * W / 266;

    destX += destW * spriteX; //offsetX
    destY += destH * (-1);    //offsetY

    //float clipH = destY+destH-clip;
    float clipH = clip - destY;
    //float clipH = destY+destH-((1-scale*(clip-myCamY))*height/2);
    //if (clipH<0) clipH=0;
    //if (clipH<0)  return;//everything clipped

    //if (clipH>=h) clipH = h;
    if (destY >= myCamY) return;
    float projH = myCamY - destY;
    if (projH > destH)
	    projH = destH;

    masked_stretch_blit(s, app, 0, 0,  s->w, s->h * projH/destH, destX, destY, destW, projH);
    //masked_stretch_blit(s, app, 0, 0,  s->w, s->h, destX, destY, destW, clipH * W / 266);
    //if (tmpSprtCrdFg)
      //textprintf_ex(screen, font, 0, 90, -1, -1, "destX: %f, destY: %f, destW: %f, destH: %f, myCamY: %f", destX, destY, destW, destH, myCamY);

    /*
    s.setTextureRect(IntRect(0,0,w,h-h*clipH/destH));
    s.setScale(destW/w,destH/h);
    s.setPosition(destX, destY);
    app.draw(s);*/
    }
  void drawExtSprite(RenderWindow *app, BITMAP *sp)
  {
    Sprite s = sp;
    if (s == NULL) return;
    int w = s->w;
    int h = s->h;

    float destX = X + scale * spriteX * screenResolData[screenResolId].width/2;
    //float destX = X + scale * spriteX * width/2;
    float destY = Y + 4;
    //float destW  = w * W / 266;
    //float destH  = h * W / 266;
    //To make explosion bigger
    float destW  = w * W / 100 ;
    float destH  = h * W / 100;

    destX += destW * spriteX; //offsetX
    destY += destH * (-1);    //offsetY

    //float clipH = destY+destH-clip;
    float clipH = clip - destY;
    //float clipH = destY+destH-((1-scale*(clip-myCamY))*height/2);
    //if (clipH<0) clipH=0;
    //if (clipH<0)  return;//everything clipped

    //if (clipH>=h) clipH = h;
    if (destY >= myCamY) return;
    float projH = myCamY - destY;
    if (projH > destH)
	    projH = destH;

    masked_stretch_blit(s, app, 0, 0,  s->w, s->h * projH/destH, destX, destY, destW, projH);
  }
};
int speed = 0, curBgX, curBgW;
float playerX = 0;
int k; //keypress
int tempkey, tempkey2; //keypress
int printDebugFg = 0;//debug printing
int faceLogoIdx = 0; //face logo idx
int menukey_handler(int key)
{ 
#if 0
  if ((k & 0xff00) == 0x5300) { playerX+=0.2; curBgX += 10; if (curBgX >= 640) curBgX -= 640; }
  if ((k & 0xff00) == 0x5200) { playerX-=0.2; curBgX -= 10; if (curBgX < 0) curBgX += 640; }
  if ((k & 0xff00) == 0x5400) speed+=200;
  if ((k & 0xff00) == 0x5500) speed-=200;
#endif
  return key;
}
END_OF_FUNCTION(menukey_handler);

int direction_handler(int key)
{ k = key;
  //if ((k & 0xff00) == 0x5300) { playerX+=0.2; if (playerX >  16) playerX = 6;  else { curBgX += 10; if (curBgX >= screenResolData[screenResolId].width) curBgX -= screenResolData[screenResolId].width; } }
  //if ((k & 0xff00) == 0x5200) { playerX-=0.2; if (playerX < -16) playerX = -6; else { curBgX -= 10; if (curBgX < 0) curBgX += screenResolData[screenResolId].width; } }
  //if ((k & 0xff00) == 0x5400) speed+=20;
  //if ((k & 0xff00) == 0x5500) speed-=10;
  //if (speed > 300) speed = 300;
  if (speed < -200) speed = -200;
  //if((k & 0xFF) == 27) exit(0);
//return 0;
return key;
}
END_OF_FUNCTION(direction_handler);

//Global switch for volume
int musicvol;
int sfxvol;
int soundSwitch;
int musicSwitch;

void judas_test(void)
{
unsigned mixratetable[] =
{
        5513,
        8269,
        11025,
        16538,
        22050,
        33075,
        44100
};
int mixrate;
int mixer;
int interpolation;
//int musicvol;
//int sfxvol;

        mixrate = 4;
        mixer = FASTMIXER;
        interpolation = 0;
        musicvol = 60;
        sfxvol = 60;
        soundSwitch = 1;
        musicSwitch = 1;

        atexit(judas_uninit);
        atexit(timer_uninit);

        judas_config();
        if (!judas_init(mixratetable[mixrate], mixer, SIXTEENBIT | STEREO, interpolation))
        {
                printf("JUDAS ERROR: %s\n", judas_errortext[judas_error]);
                //return 666;
                return ;
        }

        /* Exit if in no sound mode */
        if (judas_device == DEV_NOSOUND)
        {
          printf("ERROR: Sound card not detected!\n");
          exit (0);
	}

        //kbd_init();
        timer_init(0x4280, judas_update);

        judas_setmusicmastervolume(4, musicvol);
        judas_setsfxmastervolume(4, sfxvol);

        //judas_loadxm("tune1.xm");
        //judas_playxm(0);
	//sleep(1);

}
void exitgame(void)
{   volatile int i = 10000, j = 1000;
    fart = judas_loadwav("fart.wav");
    judas_playsample(fart, 5, 22050, 64*256, LEFT);
    judas_playsample(fart, 6, 22050, 64*256, RIGHT);
    //fart = load_sample("fart.wav");
    //play_sample(fart, 255, pan, pitch, FALSE);
    while (i--)
    {
	j = 1000;
        while (j--);
    }
}
void cr_delay(int d)
{
	volatile int i = d * 1000;
	while (i--);
}
BITMAP *menuParts, *carPart = NULL;
int getScrWidthForMenuElement(int width)
{
    return (int)((1.0 * width * screenResolData[screenResolId].width)/1024);
}
int getScrHeightForMenuElement(int height)
{
    return (int)((1.0 * height * screenResolData[screenResolId].height)/768);
}
int dummyScrMgr(BITMAP *scr, int keyFg)
{
	return 0;
}
int mainMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *setScrPart = NULL;
	PALETTE pal;
	//static int vertStartx = 194, vertStarty = 135, vertDisp = 50;
	static int vertStartx = 310, vertStarty = 216, vertDisp = 80;

	if (setScrPart == NULL)
	    setScrPart = load_bmp("bgt.bmp", pal);
	    //setScrPart = load_png("mainbg.png", pal);
	if (carPart == NULL)
	    carPart = load_png("cariconp.png", pal);
        if (!setScrPart || !carPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading png file for main menu.\n" );
	    exit (1);
        }
        stretch_blit(setScrPart, scr, 0, 0, setScrPart->w, setScrPart->h, getScrWidthForMenuElement(304), getScrHeightForMenuElement(208), getScrWidthForMenuElement(setScrPart->w), getScrHeightForMenuElement(setScrPart->h));//304,208  120, 208
        //blit(setScrPart, scr, 0, 0, 190, 130, setScrPart->w, setScrPart->h);
        masked_stretch_blit(carPart, scr, 0, 0, carPart->w, carPart->h, getScrWidthForMenuElement(vertStartx) , getScrHeightForMenuElement(vertStarty + (keyFg * vertDisp)), getScrWidthForMenuElement(carPart->w), getScrHeightForMenuElement(carPart->h));
        //masked_blit(carPart, scr, 0, 0, vertStartx , vertStarty + (keyFg * vertDisp), carPart->w, carPart->h);
	return 0;
}
int settingMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *setScrPart = NULL;
	PALETTE pal;
	static int vertStartx = 191, vertStarty = 218, vertDisp = 80;

	if (setScrPart == NULL)
	    setScrPart = load_png("setngbg.png", pal);
	if (carPart == NULL)
	    carPart = load_png("cariconp.png", pal);
        if (!setScrPart || !carPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading png file for settings menu.\n" );
	    exit (1);
        }
        stretch_blit(setScrPart, scr, 0, 0, setScrPart->w, setScrPart->h, getScrWidthForMenuElement(190), getScrHeightForMenuElement(216), getScrWidthForMenuElement(setScrPart->w), getScrHeightForMenuElement(setScrPart->h));//304,208  120, 208
        //stretch_blit(setScrPart, scr, 0, 0, setScrPart->w, setScrPart->h, 222, 134, 68, 187);
        //blit(setScrPart, scr, 0, 0, 349, 213, setScrPart->w, setScrPart->h);
        //masked_blit(carPart, scr, 0, 0, vertStartx , vertStarty + (keyFg * vertDisp), carPart->w, carPart->h);
        masked_stretch_blit(carPart, scr, 0, 0, carPart->w, carPart->h, getScrWidthForMenuElement(vertStartx) , getScrHeightForMenuElement(vertStarty + (keyFg * vertDisp)), getScrWidthForMenuElement(carPart->w), getScrHeightForMenuElement(carPart->h));

	return 0;
}
int setMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *setScrPart = NULL;
	PALETTE pal;
	static int vertStartx = 354, vertStarty = 214, vertDisp = 88;

	if (setScrPart == NULL)
	    setScrPart = load_png("soundbg.png", pal);
	if (carPart == NULL)
	    carPart = load_png("cariconp.png", pal);
        if (!setScrPart || !carPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading png file for sound menu.\n" );
	    exit (1);
        }
        stretch_blit(setScrPart, scr, 0, 0, setScrPart->w, setScrPart->h, getScrWidthForMenuElement(352), getScrHeightForMenuElement(212), getScrWidthForMenuElement(setScrPart->w), getScrHeightForMenuElement(setScrPart->h));//304,208  120, 208
        //stretch_blit(setScrPart, scr, 0, 0, setScrPart->w, setScrPart->h, 222, 134, 68, 187);
        //blit(setScrPart, scr, 0, 0, 349, 213, setScrPart->w, setScrPart->h);
        //masked_blit(carPart, scr, 0, 0, vertStartx , vertStarty + (keyFg * vertDisp), carPart->w, carPart->h);
        masked_stretch_blit(carPart, scr, 0, 0, carPart->w, carPart->h, getScrWidthForMenuElement(vertStartx) , getScrHeightForMenuElement(vertStarty + (keyFg * vertDisp)), getScrWidthForMenuElement(carPart->w), getScrHeightForMenuElement(carPart->h));

	return 0;
}
int exitMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *setScrPart = NULL;
	PALETTE pal;
	static int vertStartx = 306, vertStarty = 314, vertDisp = 232;

	if (setScrPart == NULL)
	    setScrPart = load_png("exitBg.png", pal);
	if (carPart == NULL)
	    carPart = load_png("cariconp.png", pal);
        if (!setScrPart || !carPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading png file for exit menu.\n" );
	    exit (1);
        }
        masked_stretch_blit(setScrPart, scr, 0, 0, setScrPart->w, setScrPart->h, getScrWidthForMenuElement(vertStartx), getScrHeightForMenuElement(vertStarty), getScrWidthForMenuElement(setScrPart->w), getScrHeightForMenuElement(setScrPart->h));
        //blit(setScrPart, scr, 0, 0, 349, 213, setScrPart->w, setScrPart->h);
        masked_stretch_blit(carPart, scr, 0, 0, carPart->w, carPart->h, getScrWidthForMenuElement(vertStartx + (keyFg * vertDisp)), getScrHeightForMenuElement(vertStarty), getScrWidthForMenuElement(carPart->w), getScrHeightForMenuElement(carPart->h));

	return 0;
}
int volMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *volScrPart = NULL, *offPart = NULL, *onPart = NULL;
	PALETTE pal;
	static int volLevel = 3, vertStartx = 221, vertStarty = 134, vertDisp = 55;
	int butonOffset[] = { 1, 15, 28, 41, 54, 67, 80 }, i;

	if (volScrPart == NULL)
	    volScrPart = load_png("bas4butn.png", pal);
	if (offPart == NULL)
	    offPart = load_png("offbuton.png", pal);
	if (onPart == NULL)
	    onPart = load_png("onbuton.png", pal);
        if (!volScrPart || !offPart || !onPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading image file for volume.\n");
	    exit (1);
        }
        volLevel += keyFg;
	if (volLevel >= 7)
		volLevel = 6;
	if (volLevel < 0)
		volLevel = 0;

	musicvol = volLevel * 15;
	sfxvol   = volLevel * 15;

        stretch_blit(volScrPart, scr, 0, 0, volScrPart->w, volScrPart->h, getScrWidthForMenuElement(678), getScrHeightForMenuElement(309), getScrWidthForMenuElement(volScrPart->w), getScrHeightForMenuElement(volScrPart->h));
        //blit(volScrPart, scr, 0, 0, 425, 194, volScrPart->w, volScrPart->h);
        //masked_blit(carPart, scr, 0, 0, vertStartx , vertStarty + (keyFg * vertDisp), carPart->w, carPart->h);
	for (i = 0; i <=  volLevel; i++)
            //blit(onPart, scr, 0, 0, 425 + (butonOffset[i]), 197, onPart->w, onPart->h);
            stretch_blit(onPart, scr, 0, 0, onPart->w, onPart->h, getScrWidthForMenuElement(680 + (i * 21)), getScrHeightForMenuElement(314), getScrWidthForMenuElement(onPart->w), getScrHeightForMenuElement(onPart->h));
	for (i = volLevel + 1; i <  7; i++)
            //blit(offPart, scr, 0, 0, 425 + (butonOffset[i]), 197, offPart->w, offPart->h);
            stretch_blit(offPart, scr, 0, 0, onPart->w, onPart->h, getScrWidthForMenuElement(680 + (i * 21)), getScrHeightForMenuElement(314), getScrWidthForMenuElement(offPart->w), getScrHeightForMenuElement(offPart->h));

        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);

        judas_setmusicmastervolume(4, musicSwitch?musicvol:0);
        judas_setsfxmastervolume(4, soundSwitch?sfxvol:0);

	return 0;
}
int sndMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *offPart = NULL, *onPart = NULL;
	PALETTE pal;

	if (offPart == NULL)
	    offPart = load_png("offword.png", pal);
	if (onPart == NULL)
	    onPart = load_png("onword.png", pal);
        if (!offPart || !onPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading image file for background sound.\n");
	    exit (1);
        }

        if (keyFg > 0)
	    soundSwitch = 1;
	else if (keyFg < 0)
	    soundSwitch = 0;

	if (soundSwitch)
            //blit(onPart, scr, 0, 0, onPart->w, onPart->h);
            masked_stretch_blit(onPart, scr, 0, 0, onPart->w, onPart->h, getScrWidthForMenuElement(677), getScrHeightForMenuElement(379), getScrWidthForMenuElement(onPart->w), getScrHeightForMenuElement(onPart->h));
	else
            //blit(offPart, scr, 0, 0, 425, 238, offPart->w, offPart->h);
            masked_stretch_blit(offPart, scr, 0, 0, offPart->w, offPart->h, getScrWidthForMenuElement(677), getScrHeightForMenuElement(379), getScrWidthForMenuElement(offPart->w), getScrHeightForMenuElement(offPart->h));

        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);

        judas_setsfxmastervolume(4, soundSwitch?sfxvol:0);

	return 0;
}
int musicMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *offPart = NULL, *onPart = NULL;
	PALETTE pal;

	if (offPart == NULL)
	    offPart = load_png("offword.png", pal);
	if (onPart == NULL)
	    onPart = load_png("onword.png", pal);
        if (!offPart || !onPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading image file for music.\n");
	    exit (1);
        }

        if (keyFg > 0)
	    musicSwitch = 1;
	else if (keyFg < 0)
	    musicSwitch = 0;

	if (musicSwitch)
            //blit(onPart, scr, 0, 0, 425 , 291, onPart->w, onPart->h);
            masked_stretch_blit(onPart, scr, 0, 0, onPart->w, onPart->h, getScrWidthForMenuElement(677), getScrHeightForMenuElement(463), getScrWidthForMenuElement(offPart->w), getScrHeightForMenuElement(offPart->h));
	else
            //blit(offPart, scr, 0, 0, 425, 291, offPart->w, offPart->h);
            masked_stretch_blit(offPart, scr, 0, 0, offPart->w, offPart->h, getScrWidthForMenuElement(677), getScrHeightForMenuElement(463), getScrWidthForMenuElement(offPart->w), getScrHeightForMenuElement(offPart->h));

        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);

        judas_setmusicmastervolume(4, musicSwitch?musicvol:0);

	return 0;
}
int scrResMenuMgr(BITMAP *scr, int keyFg)
{
	static BITMAP *setScrPart = NULL;
	PALETTE pal;
	static int vertStartx = 190, vertStarty = 217, vertDisp = 85;

	if (setScrPart == NULL)
	    setScrPart = load_png("setngbg.png", pal);
	if (carPart == NULL)
	    carPart = load_png("cariconp.png", pal);
        if (!setScrPart || !carPart) {
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	    allegro_message("Error loading png file for settings menu.\n" );
	    exit (1);
        }
        stretch_blit(setScrPart, scr, 0, 0, setScrPart->w, setScrPart->h, getScrWidthForMenuElement(190), getScrHeightForMenuElement(216), getScrWidthForMenuElement(setScrPart->w), getScrHeightForMenuElement(setScrPart->h));//304,208  120, 208
        masked_stretch_blit(carPart, scr, 0, 0, carPart->w, carPart->h, getScrWidthForMenuElement(vertStartx) , getScrHeightForMenuElement(vertStarty + (keyFg * vertDisp)), getScrWidthForMenuElement(carPart->w), getScrHeightForMenuElement(carPart->h));

}
int setScrResol(int keyFg)
{
	switch(keyFg)
	{
	   case 1:
               set_color_depth(32);
   	       if (set_gfx_mode(GFX_VESA3, 640, 480, 0, 0) < 0) {
	         if (set_gfx_mode(GFX_VESA2L, 640, 480, 0, 0) < 0) {
	            set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	            allegro_message("Error setting video mode (640x480).\n");
	            return 1;
		 }
               }
               screenResolId = 0;
	       break;
	   case 2:
               set_color_depth(32);
   	       if (set_gfx_mode(GFX_VESA3, 800, 600, 0, 0) < 0) {
	         if (set_gfx_mode(GFX_VESA2L, 800, 600, 0, 0) < 0) {
	            set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	            allegro_message("Error setting video mode (800x600).\n");
	            return 1;
		 }
               }
               screenResolId = 1;
	       break;
	   case 3:
               set_color_depth(16);
   	       if (set_gfx_mode(GFX_VESA3, 1024, 768, 0, 0) < 0) {
                 if (set_gfx_mode(GFX_VESA2L, 1024, 768, 0, 0) < 0) {
	             set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	             allegro_message("Error setting video mode (1024x768).\n");
		     return 1;
		 }
	       }
               screenResolId = 2;
	       break;
	}
}
struct menuet
{ 
    const char  *scrImg;
    int   mvDir; //direction of movement 0 -vert, 1- hor
    int   t; //top
    int   l; //left
    int   w; //width cordinate
    int   h; //height cordinate
    int   ds; //delta for movement
    int   fh; // first position
    int   lh; // last position
    int   (*manageScr)(BITMAP *scr, int keyMove);
};
struct menuet menuValStack[4];
int menuValStackIdx = 0;
int screens(int stage)
{
    int scrX = 0, scrY = 0;
    BITMAP *fScr, *menuCanvas;PALETTE pal;int scrKey;
    struct menuet menutab, menuentry[] = { 
                                           {"UI_MAIN.png", 0, 424, 192, 728, 272, 80, 192, 312 , mainMenuMgr},
                                           {"UI_Cntrl.png", 0, 275, 125, 430, 165, 50, 125, 125, dummyScrMgr },
                                           {"UI_Sound.png", 0, 464, 200, 668, 280, 80, 200, 440, setMenuMgr },
                                           {"UI_Exit.png", 1, 408, 304, 528, 368, 240, 408, 648, exitMenuMgr },
                                           {"UI_Sound.png", 0, 668, 280, 864, 360, 125, 280, 280, volMenuMgr },
                                           {"UI_Sound.png", 0, 668, 360, 896, 440, 125, 360, 360, sndMenuMgr },
                                           {"UI_Sound.png", 0, 668, 440, 896, 520, 125, 440, 440, musicMenuMgr },
                                           {"UI_Setng.png", 0, 301, 202, 843, 264, 80, 202, 522, settingMenuMgr },
                                           {"UI_Scrn.png", 0, 301, 202, 843, 264, 85, 202, 457, scrResMenuMgr }
                                         };

    menutab = menuentry[stage];

    int boxcol = makecol(255,255,255);
    int optionSel = 0;

    scrX = screenResolData[screenResolId].width;
    scrY = screenResolData[screenResolId].height;

    menuCanvas = create_bitmap(scrX, scrY);
    //menuParts  = create_bitmap(200, 310);

    //fScr = load_png("UI_MAIN.png", pal);
    fScr = load_png(menutab.scrImg, pal);
    clear_keybuf();
    stretch_blit(fScr, menuCanvas, 0, 0, fScr->w, fScr->h, 0, 0, scrX, scrY);
    do
    {
      if (2 == stage)
      {//should take fromm table but for now...
	      //below calls set the pics as per current settings of vol etc.
	      volMenuMgr(menuCanvas, 0);
	      sndMenuMgr(menuCanvas, 0);
	      musicMenuMgr(menuCanvas, 0);

      }
      //Draw on the screen and draw rectangle
      blit(menuCanvas, screen, 0, 0, 0, 0, menuCanvas->w, menuCanvas->h);
      rect(screen, getScrWidthForMenuElement(menutab.t), getScrHeightForMenuElement(menutab.l), getScrWidthForMenuElement(menutab.w), getScrHeightForMenuElement(menutab.h), boxcol);
      scrKey = readkey();

      if (((scrKey & 0xFF00) == 0x5400) && (menutab.mvDir == 0) )
      {
             judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
             //play_sample(menumove, 255, pan, pitch, FALSE);
	     if (menutab.l > menutab.fh) 
		     { menutab.l -= menutab.ds; menutab.h -= menutab.ds; optionSel--; menutab.manageScr(menuCanvas, optionSel); }
      }
      else if (((scrKey & 0xFF00) == 0x5500) && (menutab.mvDir == 0) )
      {
             judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
             //play_sample(menumove, 255, pan, pitch, FALSE);
	     if (menutab.l < menutab.lh) 
	     {
		     { menutab.l += menutab.ds; menutab.h += menutab.ds; optionSel++; menutab.manageScr(menuCanvas, optionSel); }
	     }
      }
      else if (((scrKey & 0xFF00) == 0x5200) && (menutab.mvDir == 1) )
      {
             judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
             //play_sample(menumove, 255, pan, pitch, FALSE);
	     if (menutab.t > menutab.fh) 
		     { menutab.t -= menutab.ds; menutab.w -= menutab.ds; optionSel--; menutab.manageScr(menuCanvas, optionSel); } 
      }
      else if (((scrKey & 0xFF00) == 0x5300) && (menutab.mvDir == 1) )
      {
             judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
             //play_sample(menumove, 255, pan, pitch, FALSE);
	     if (menutab.t < menutab.lh) 
		     { menutab.t += menutab.ds; menutab.w += menutab.ds; optionSel++; menutab.manageScr(menuCanvas, optionSel); } 
      }
      else if ( (stage == 2) && ((scrKey & 0xFF00) == 0x4300) )//0x4300 is Enter key
      {
          if (optionSel == 1)
	  {
	      stage = 4;
	      menuValStack[menuValStackIdx++] = menutab;
              menutab = menuentry[stage];
              menutab.manageScr(menuCanvas, 0);
              optionSel = 0;
	      scrKey = 0;
	  }
	  else if (optionSel == 2)
	  {
	      stage = 5;
	      menuValStack[menuValStackIdx++] = menutab;
              menutab = menuentry[stage];
              menutab.manageScr(menuCanvas, 0);
              optionSel = 0;
	      scrKey = 0;
	  }
	  else if (optionSel == 3)
	  {
	      stage = 6;
	      menuValStack[menuValStackIdx++] = menutab;
              menutab = menuentry[stage];
              menutab.manageScr(menuCanvas, 0);
              optionSel = 0;
	      scrKey = 0;
	  }
      }
      else if (stage == 4) 
      {
	      if ((scrKey & 0xFF00) == 0x4300) 
	      {
	      stage = 2;
	      menutab = menuValStack[--menuValStackIdx];
              //menutab = menuentry[stage];
              optionSel = 1;
	      scrKey = 0;
	      }
	      else if ((scrKey & 0xFF00) == 0x5300)
	      {
                      optionSel++;
		      menutab.manageScr(menuCanvas, 1);
	      }
	      else if ((scrKey & 0xFF00) == 0x5200) 
	      {
                      optionSel--;
		      menutab.manageScr(menuCanvas, -1);
	      }
      }
      else if (stage == 5) 
      {
	      if ((scrKey & 0xFF00) == 0x4300) 
	      {
	      stage = 2;
	      menutab = menuValStack[--menuValStackIdx];
              //menutab = menuentry[stage];
              optionSel = 2;
	      scrKey = 0;
	      }
	      else if ((scrKey & 0xFF00) == 0x5200)
	      {
                      optionSel++;
		      menutab.manageScr(menuCanvas, 1);
	      }
	      else if ((scrKey & 0xFF00) == 0x5300) 
	      {
                      optionSel--;
		      menutab.manageScr(menuCanvas, -1);
	      }
      }
      else if (stage == 6) 
      {
	      if ((scrKey & 0xFF00) == 0x4300) 
	      {
	      stage = 2;
	      menutab = menuValStack[--menuValStackIdx];
              //menutab = menuentry[stage];
              optionSel = 3;
	      scrKey = 0;
	      }
	      else if ((scrKey & 0xFF00) == 0x5200)
	      {
                      optionSel++;
		      menutab.manageScr(menuCanvas, 1);
	      }
	      else if ((scrKey & 0xFF00) == 0x5300) 
	      {
                      optionSel--;
		      menutab.manageScr(menuCanvas, -1);
	      }
      }
      else if (stage == 8) 
      {
	      if ((scrKey & 0xFF00) == 0x4300) 
	      {
		      if (optionSel != 0)
		          setScrResol(optionSel);
			  /*
		      else
		      {
	      stage = 7;
	      menutab = menuValStack[--menuValStackIdx];
              //menutab = menuentry[stage];
              optionSel = 0;
	      scrKey = 0;
		      }*/
	      }
      }
    }while((scrKey & 0xFF00) != 0x4300);
    judas_playsample(menuselect, 4, 22050, 64*256, MIDDLE);
    //play_sample(menuselect, 255, pan, pitch, FALSE);
    //textprintf_ex(screen, font, 100, 100, makecol(255, 100, 200), -1,  "top  : %d, left %d", menuentry[0].t, menuentry[0].l);
    //readkey();
    //optionSel = ((menutab.l - menutab.fh) / menutab.ds);
    destroy_bitmap(menuCanvas);
    destroy_bitmap(fScr);
    return optionSel;
}

int main(int argc, char *argv[])
{
    RenderWindow *app;int depth = 16;PALETTE pal;
    int highestScor = 990, currentScor = 0;
    int testresol, speedMaintCycles = 0;;
    time_t t;
    
    /*RenderWindow app(VideoMode(width, height), "Outrun Racing!");
    app.setFramerateLimit(60);*/

    /* Initialise Allegro. */
    allegro_init();
    install_keyboard();
    //keyboard_callback = direction_handler;
#if 0
    /* install a digital sound driver */
    if (install_sound(DIGI_AUTODETECT, MIDI_NONE, "/") != 0) {
       allegro_message("Error initialising sound system\n%s\n", allegro_error);
       return 1;
    }
#endif
    judas_test();

    /* read in the WAV file */
    //menumove = load_sample(argv[1]);
    menumove = judas_loadwav("MENUMOVE.WAV");
    menuselect = judas_loadwav("MENUSEL.WAV");
    //menumove = load_sample("MENUMOVE.WAV");
    //menuselect = load_sample("MENUSEL.WAV");
    //tune1 = load_sample("tune1.wav");
    //if (!menumove || !menuselect || !tune1) {
    if (!menumove || !menuselect) {
       allegro_message("Error reading WAV file \n");
       return 1;
    }
 
    if (argc < 2)
    {   
//#if 0	
    screenResolId = 0;
    set_color_depth(depth);
    if (set_gfx_mode(GFX_VESA3, 640, 480, 0, 0) < 0) {
    if (set_gfx_mode(GFX_VESA2L, 640, 480, 0, 0) < 0) {
    //if ((set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) < 0) &&
        //(set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) < 0)) { // }
	allegro_message("Error setting video mode (640x480x%d).\n", depth);
	return 1;
    }
    }
//#else
    }
    else
    {
	testresol = atoi(argv[1]);
        set_color_depth(16);
        if (testresol == 1)
	{
            screenResolId = 1;
    if (set_gfx_mode(GFX_VESA3, 800, 600, 0, 0) < 0) {
    if (set_gfx_mode(GFX_VESA2L, 800, 600, 0, 0) < 0) {
	allegro_message("Error setting video mode (800x600x%d).\n", depth);
	return 1;
    }}
        }
	else 
	{
            screenResolId = 2;
    if (set_gfx_mode(GFX_VESA3, 1024, 768, 0, 0) < 0) {
    if (set_gfx_mode(GFX_VESA2L, 1024, 768, 0, 0) < 0) {
	allegro_message("Error setting video mode (1024x768x%d).\n", depth);
	return 1;
    }}
        }
    }
//#endif

    /* start up the sample */
    judas_loadxm("tune1.xm");
    judas_playxm(0);
    //tune1 = judas_loadwav("TUNE1.WAV");
    //judas_playsample(tune1, 7, 22050, 64*256, MIDDLE);
    //judas_playsample(tune1, 7, 44100, 64*256, MIDDLE);
    //menumove = load_sample("MENUMOVE.WAV");
    //play_sample(tune1, 255, pan, pitch, FALSE);

    int scrId = 0, opt;
    int nextScr[][5] = { 
	                   {1, 7, 3, -1, -1},
	                   {-1, -1, -1, -1},
	                   {0, 4, 0, 0, -1},
	                   {-1, 0, -1, -1, -1},

	                   {-1, 0, -1, -1, -1},
	                   {-1, 0, -1, -1, -1},
	                   {-1, 0, -1, -1, -1},

	                   {8, 2, 1, 0 , -1},
	                   {7, 0, 0, 0 , -1},
			};
    srand((unsigned) time(&t));

    do
    {   
        opt = screens(scrId);
        if (scrId == 3 && opt == 0)
	{
	    exitgame();
	    return 1;
	}
	scrId = nextScr[scrId][opt];
    }while(scrId != -1);

  //screens(3);
    judas_stopxm();
    //judas_stopsample(7);
    //judas_freesample(tune1);
    //destroy_sample(tune1);

    keyboard_callback = direction_handler;
    app = create_bitmap(1024, 768);
    //app = create_bitmap(640, 480);

    textbuf = create_bitmap(640, 80);
    clear_to_color(textbuf, makecol(255,0,255));
    //textout_ex(textbuf, font, "Ravi the warrior", 0, 10, -1, -1);
    
    scoreBoardBuf = create_bitmap(640, 80);
    clear_to_color(scoreBoardBuf, makecol(255,0,255));

    Sprite object[7];
    object[4] = load_png("4P.png", pal);
    object[6] = load_png("6P.png", pal);
    object[1] = load_png("KANGAROO.png", pal);
    object[2] = load_png("SENPEDES.png", pal);
    /*
    Texture t[50];
    Sprite object[50];
    for(int i=1;i<=7;i++)
     {
       t[i].loadFromFile("images/"+std::to_string(i)+".png");
       t[i].setSmooth(true);
       object[i].setTexture(t[i]);
     }

    Texture bg;
    bg.loadFromFile("images/bg.png");
    bg.setRepeated(true);
    Sprite sBackground(bg);
    sBackground.setTextureRect(IntRect(0,0,5000,411));
    sBackground.setPosition(-2000,0);*/
        /* Load the PNG into a BITMAP structure. */
    Texture bg, car;
    bg = load_png("BG.png", pal);
    if (!bg) {
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	allegro_message("Error loading file `%s'.\n", "BG.DAT");
	return 1;
    }
    curBgX = 0;
    curBgW = bg->w - 100;
    
    bgscr = create_bitmap(2048, 384);
    stretch_blit(bg, bgscr, 0, 0, bg->w, bg->h, 0, 0, screenResolData[screenResolId].width, screenResolData[screenResolId].height / 2);
    stretch_blit(bg, bgscr, 0, 0, bg->w, bg->h, screenResolData[screenResolId].width, 0, screenResolData[screenResolId].width, screenResolData[screenResolId].height / 2);
    //stretch_blit(bg, bgscr, 0, 0, bg->w, bg->h, 640, 0, 640, 240);
    /*
    bgscr = create_bitmap(1280, 240);
    stretch_blit(bg, bgscr, 0, 0, bg->w, bg->h, 0, 0, 640, 240);
    stretch_blit(bg, bgscr, 0, 0, bg->w, bg->h, 640, 0, 640, 240);
    */
    destroy_bitmap(bg);

    //textprintf_ex(textbuf, font, 0, 10, -1, -1, "Dim %d %d", bg->w, bg->h);
    car = load_png("CAR.png", pal);
    if (!car) {
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	allegro_message("Error loading file `%s'.\n", "CAR.png");
	return 1;
    }
    

    //Sprite object[5];//delete
    //std::vector<Line> lines;
    Line lines[1700];
    float Cxpos = 0;

//allegro_message(" Lines created \n");
    for(int i=0;i<1700;i++)
     {
       Line line;
       line.z = i*segL;
       line.x += Cxpos; //ravi

       if (i>300 && i<700) { Cxpos += 20; line.x += 20; line.curve=0.5;}
       if (i>1100 && i<1500) { Cxpos -= 20; line.x -= 20; line.curve=-0.7;}

       if (i>300 && i%40==0) {line.spriteX=-2.7; line.sprite=object[4]; line.spriteType=4;}
       if (i%57==0)          {line.spriteX=2.0; line.sprite=object[6]; line.spriteType=6;}
       if ((i-2)%227==0)          {line.spriteX=-0.5*(i%3); line.sprite=object[1]; line.spriteType=1;}
       if ((i-2)%343==0)          {line.spriteX=-0.6*(i%3); line.sprite=object[2]; line.spriteType=2;}
       //if (i>80 && i<300 && i%40==0) {line.spriteX=-2.5; line.sprite=object[0];}
       /*
       if (i<300 && i%20==0) {line.spriteX=-2.5; line.sprite=object[5];}
       if (i%17==0)          {line.spriteX=2.0; line.sprite=object[6];}
       if (i>300 && i%20==0) {line.spriteX=-0.7; line.sprite=object[4];}
       if (i>800 && i%20==0) {line.spriteX=-1.2; line.sprite=object[1];}
       if (i==400)           {line.spriteX=-1.2; line.sprite=object[7];}
       */

       if (i>750 && i < 1603) line.y = sin(i/30.0)*1500;

       lines[i] = line;//lines.push_back(line);
     }

   //int N = 1600;//lines.size();
   int N = 1700;//lines.size();
   //float playerX = 0;
   int pos = 0;
   int H = 1500;
   //int k; //keypress
/*
    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }*/
  //int speed=0;
    judas_loadxm("tune2.xm");
    judas_playxm(0);
    //tune2 = judas_loadwav("TUNE2.WAV");
    //judas_playsample(tune2, 8, 44100, 64*256, MIDDLE);
    do
    {

    //clear_to_color(textbuf, makecol(255,0,255));
    //textprintf_ex(textbuf, font, 0, 0, -1, -1, "CurDim %d %d", curBgX, curBgW);
    currentScor += speed / 200;
    clear_to_color(scoreBoardBuf, makecol(255,0,255));
    textprintf_ex(scoreBoardBuf, font, 0, 0, -1, -1,  "High  : %d", highestScor);
    textprintf_ex(scoreBoardBuf, font, 0, 10, -1, -1, "Score : %d", currentScor);
  //int speed=0;
/*
  if ((k & 0xff00) == 0x5300) playerX+=0.1;
  if ((k & 0xff00) == 0x5200) playerX-=0.1;
  if ((k & 0xff00) == 0x5400) speed=200;
  if ((k & 0xff00) == 0x5500) speed=-200;
 
  if (Keyboard::isKeyPressed(Keyboard::Tab)) speed*=3;
  if (Keyboard::isKeyPressed(Keyboard::W)) H+=100;
  if (Keyboard::isKeyPressed(Keyboard::S)) H-=100;
*/
  //if (speed >= 100) speed -= 10;
  if (speed < 0)
	  speed = 0;
  pos+=speed;
  while (pos >= N*segL) pos-=N*segL;
  while (pos < 0) pos += N*segL;

  //app.clear(Color(105,205,4));
  //app.draw(sBackground);
  clear_to_color(app,makecol(105,205,4));
  //stretch_blit(bgscr, app, curBgX, 0, 640, bgscr->h, 0, 0, 640, 240); //Ravi can be changed to simple blit
  stretch_blit(bgscr, app, curBgX, 0, screenResolData[screenResolId].width, screenResolData[screenResolId].height / 2, 0, 0, screenResolData[screenResolId].width, screenResolData[screenResolId].height / 2);

  int startPos = pos/segL;
  int camH = lines[startPos].y + H;
  //if (speed>0) sBackground.move(-lines[startPos].curve*2,0);
  //if (speed<0) sBackground.move( lines[startPos].curve*2,0);

  //int maxy = height;  //height represent the downmost point
  int maxy = screenResolData[screenResolId].height;  //height represent the downmost point
  float x=0,dx=0;
  myGCamY = screenResolData[screenResolId].height;
  int printRoadDimFg = 0;
  ///////draw road////////
  for(int n = startPos; n<startPos+300; n++)  
   {
    Line &l = lines[n%N];
    l.project(playerX*roadW-x, camH, startPos*segL - (n>=N?N*segL:0));
    x+=dx;
    dx+=l.curve;

    l.clip=maxy;
    if (l.Y>=maxy) continue;
    maxy = l.Y;

    Color grass  = (n/3)%2?makecol(16,200,16):makecol(0,154,0);
    Color rumble = (n/3)%2?makecol(255,255,255):makecol(0,0,0);
    Color road   = (n/3)%2?makecol(107,107,107):makecol(105,105,105);

    Line p = lines[(n-1)%N]; //previous line

    //if (printRoadDimFg == 2)
    {
    drawQuad(app, grass, 0, p.Y, screenResolData[screenResolId].width, 0, l.Y, screenResolData[screenResolId].width);
    //drawQuad(app, grass, 0, p.Y, width, 0, l.Y, width);
    drawQuad(app, rumble,p.X, p.Y, p.W*1.2, l.X, l.Y, l.W*1.2);
    drawQuad(app, road,  p.X, p.Y, p.W, l.X, l.Y, l.W);

     // clear_to_color(textbuf, makecol(255,0,255));
      //textprintf_ex(textbuf, font, 0, 10, -1, -1, "Dim: %d %d %d - %d %d %d : %d", (int)p.X, (int)p.Y, (int)p.W, (int)l.X, (int)l.Y, (int)l.W, (int)n);
     // printRoadDimFg++;
    }
    //else
    {
      //printRoadDimFg++;
    }
#if 0
    if (printDebugFg == 1)
    {
      //clear_to_color(textbuf, makecol(255,0,255));
      if (p.Y < 480 || l.Y < 480 && ((p.X - p.W) < 640 || (p.X + p.W) < 640))
			     // || l.Y < 480)
      {
	      drawDebugBackg();
	      readkey();
      textprintf_ex(screen, font, 0, 80, -1, -1, "Road dim: %d %d %d - %d %d %d : %d", (int)p.X, (int)p.Y, (int)p.W, (int)l.X, (int)l.Y, (int)l.W, (int)playerX);
      textprintf_ex(screen, font, 0, 88, -1, -1, "Road Dim: %d %d  %d %d ", n, startPos, (int)x, (int)camH);
    //drawQuad(screen, road,  p.X, p.Y, p.W*1.2, l.X, l.Y, l.W*1.2);
    //drawQuad(screen, grass,  9464, 456, 230, 8325, 429, 201);
    //drawQuad(screen, grass,  8020,456,210,7439,408,179);
    drawQuad(screen, rumble,  -8401,493,322 * 1.2,-6953,443,268 * 1.2);
      tempkey = k; 
      tempkey2 = readkey();
        if ((tempkey2 & 0xFF) == 97)
		printDebugFg = 0;
      k = tempkey;
      }
    }
#endif
   }
   //readkey();

    ////////draw objects////////
    for(int n=startPos+300; n>startPos && ((k & 0xFF) != 27); n--)
    {
      //if (startPos == 730)
      //{tmpSprtCrdFg = 1;
      //readkey();
      //textprintf_ex(screen, font, 0, 80, -1, -1, "Clip: %f", lines[n%N].clip);
      //}
      if (n - startPos > 200)
      {//To reenable the disp that may have been switched off due to collision
	  lines[n%N].spriteDispFg = 1;
      }
      if (lines[n%N].spriteDispFg)//if the flag hasn't been switched off due to collision
      {
            //Detect collision, can be moved to another function
	    if ( (n - startPos < 10) && (lines[n%N].spriteType == 1) )
            {
  	        lines[n%N].spriteDispFg = 0;
                draw_animation(app, 1, &lines[n%N]);
            }
         lines[n%N].drawSprite(app);
      }
    }
    /*
    //Detect collision, can be moved to another function
    if (lines[(startPos+10)%N].spriteType == 1)
    {
	lines[(startPos+10)%N].spriteDispFg = 0;
        draw_animation(app, 1, &lines[(startPos+10)%N]);
    }
    */

    draw_animation(app);
    //tmpSprtCrdFg = 0;
//allegro_message(" KeyPress %x \n", k);
    //app.display();
    //The car
    masked_stretch_blit(car, app, 0, 0,  car->w, car->h, screenResolData[screenResolId].carX, screenResolData[screenResolId].carY, screenResolData[screenResolId].carW, screenResolData[screenResolId].carH);
    //masked_blit(car, app, 0, 0, 20,20, car->w, car->h);
    masked_blit(textbuf, app, 0, 0, 0, 100, 320, 240);
    //masked_blit(scoreBoardBuf, app, 0, 0, 520, 80, 320, 240);
    masked_blit(scoreBoardBuf, app, 0, 0, getScrWidthForElement(853), getScrHeightForElement(130), 320, 240);
    draw_screen_elements(app);
    //blit(app, screen, 0, 0, 0, 0, app->w, app->h);
    blit(app, screen, 0, 0, 0, 0, screenResolData[screenResolId].width, screenResolData[screenResolId].height);

    //textprintf_ex(screen, font, 0, 100, -1, -1, "StartPos : %d", startPos);
      //if (startPos > 450)
      //readkey();
    //k = readkey();
    //cr_delay(1500);
        if ((k & 0xFF) == 27)
	{
	   if (screens(3) == 1)
	      k = 0;
	   else
	      k = 27;
	}
        if (key[KEY_LSHIFT])
        {
           //textprintf(screen, font, 8, -1, -1, "received");
	   faceLogoIdx++;
	   if (faceLogoIdx > 2)
		faceLogoIdx = 1;
        } 
        if (key[KEY_LEFT])
        {
           playerX-=0.2; if (playerX < -16) playerX = -16; else { curBgX -= 10; if (curBgX < 0) curBgX += screenResolData[screenResolId].width; } 
	   speed -= 20;
	}
        if (key[KEY_RIGHT])
        {
           playerX+=0.2; if (playerX >  16) playerX = 16;  else { curBgX += 10; if (curBgX >= screenResolData[screenResolId].width) curBgX -= screenResolData[screenResolId].width; }
	   speed -= 20;
	}
        if (key[KEY_UP])
        {
           if (speed < 400)
	   speed += 5;
	}
	else
	{
	if (speed)
	   speed -= 5;
	}
        if ((k & 0xFF) == 32)
	{
	   printDebugFg = 1;
	}
        if (speed >= 400)
	{
	   speedMaintCycles++;
	   if(speedMaintCycles % 10 == 0)
	     speed += ((rand() % 3) - 1);
	   if (speed > 405) speed = 405;
	   if (speed < 400) speed = 400;
	}
	else
	{
	   speedMaintCycles = 0;
	}

    }while((k & 0xFF) != 27);
    destroy_bitmap(bgscr);
    destroy_bitmap(textbuf);
    destroy_bitmap(scoreBoardBuf);
    destroy_bitmap(app);

    judas_stopxm();
    exitgame();

    return 0;
}

int getElemXYpos(int elementId, int *x, int *y)
{
    struct XY_t
    {
	int x;
	int y;
    };
    static XY_t elemXYlist[] =
         {  
	     {52,617},
	     {900,671},
	     {185,62},
	     {393,62},
	     {647,62},
	     {852,62},
	     {262,62},
	     {480,62},
	     {738,62},
	     {955,62},
	     { 51,24},
	     {0,0}
	 };
    static int bottomMesgBar[3] = { 360, 477, 610 };

    
    *x = (int)((1.0 * elemXYlist[elementId].x * screenResolData[screenResolId].width)/1050);
    *y = ((int)((1.0 * elemXYlist[elementId].y * screenResolData[screenResolId].height)/780));
    if (elementId < 2)
      *y -= bottomMesgBar[screenResolId];

}
int getDigitWidth(int d)
{
    static int dWidth[10] = { 37, 18, 37, 35, 34, 33, 35, 29, 37, 35};

    return dWidth[d];
}
int getScrWidthForElement(int width)
{
    return (int)((1.0 * width * screenResolData[screenResolId].width)/1050);
}
int getScrHeightForElement(int height)
{
    return (int)((1.0 * height * screenResolData[screenResolId].height)/780);
}
void draw_screen_elements(BITMAP *app)
{
    static BITMAP *scrTopMesgBar, *scrBotMesgBar, *logoElems, *faceLogo[3], *meterDigit;
    static int loadedFg = 0, fl_idx = 0, speedt = 123;
    BITMAP *pngImg;
    PALETTE pal;
    int pinkColor, digit, digitFg = 0;
    int scrW = 0, scrH = 0, yPos = 0, scrWTemp = 0, xPos = 0;


    if (0 == loadedFg)
    {
    //scrTopMesgBar = create_bitmap(640, 120);
    scrTopMesgBar = create_bitmap(1024, 120);
    clear_to_color(scrTopMesgBar, makecol(255,0,255));

    //scrBotMesgBar = create_bitmap(640, 120);
    scrBotMesgBar = create_bitmap(1024, 120);
    clear_to_color(scrBotMesgBar, makecol(255,0,255));
//#if 0
    faceLogo[0] = load_png("FcNormal.png", pal);
    faceLogo[1] = load_png("FcDrunk.png", pal);
    faceLogo[2] = load_png("FcWasted.png", pal);

    scrW = getScrWidthForElement(faceLogo[0]->w); scrH = getScrHeightForElement(faceLogo[0]->h);//sz of all 3 are same
    getElemXYpos(10, &xPos, &yPos);
    masked_stretch_blit(faceLogo[fl_idx], scrTopMesgBar, 0, 0, faceLogo[fl_idx]->w, faceLogo[fl_idx]->h, xPos, yPos, scrW, scrH);
    //masked_blit(faceLogo[fl_idx], scrTopMesgBar, 0, 0, 30, 20, faceLogo[fl_idx]->w, faceLogo[fl_idx]->h);
//#endif
    pngImg = load_png("TimeLogo.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(2, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 110, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);

    pngImg = load_png("CashLogo.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(3, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 240, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);

    pngImg = load_png("BeerLogo.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(4, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 400, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);

    pngImg = load_png("SmokeLgo.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(5, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 520, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);



    //To be replaced with value based print.
    pngImg = load_png("Timenum.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(6, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 157, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);

    pngImg = load_png("Cashnum.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(7, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 292, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);

    pngImg = load_png("Beernum.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(8, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 452, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);

    pngImg = load_png("Smokenum.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(9, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrTopMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    //masked_blit(pngImg, scrTopMesgBar, 0, 0, 582, 40, pngImg->w, pngImg->h);
    destroy_bitmap(pngImg);



    //Now drawing bottom screen elements
    /*
    pngImg = load_png("Speed.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(0, &xPos, &yPos);
    masked_stretch_blit(pngImg, scrBotMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    destroy_bitmap(pngImg);
    */
    scrW = getScrWidthForElement(37 * 3); scrH = getScrHeightForElement(64);
    getElemXYpos(0, &xPos, &yPos);
    meterDigit = load_png("Meter.png", pal);

    pngImg = load_png("SpeedUnt.png", pal);
    yPos += scrH; scrWTemp = scrW;
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    //xPos -= ((scrW - scrWTemp)/2); 
    masked_stretch_blit(pngImg, scrBotMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    destroy_bitmap(pngImg);

    pngImg = load_png("Level.png", pal);
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    getElemXYpos(1, &xPos, &yPos);
    //masked_blit(pngImg, scrBotMesgBar, 0, 0, 552, 55, pngImg->w, pngImg->h);
    masked_stretch_blit(pngImg, scrBotMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    destroy_bitmap(pngImg);

    pngImg = load_png("Levelnum.png", pal);
    xPos += scrW; yPos += scrH;
    scrW = getScrWidthForElement(pngImg->w); scrH = getScrHeightForElement(pngImg->h);
    yPos -= scrH; //Base are aligned
    //masked_blit(pngImg, scrBotMesgBar, 0, 0, 607, 45, pngImg->w, pngImg->h);
    masked_stretch_blit(pngImg, scrBotMesgBar, 0, 0, pngImg->w, pngImg->h, xPos, yPos, scrW, scrH);
    destroy_bitmap(pngImg);

    loadedFg = 1;
    }

    //Printing elements which change with every page
    if (fl_idx != faceLogoIdx)
    {
    fl_idx = faceLogoIdx;
    pinkColor = makecol(255,0,255);
    scrW = getScrWidthForElement(faceLogo[0]->w); scrH = getScrHeightForElement(faceLogo[0]->h);//sz of all 3 are same
    getElemXYpos(10, &xPos, &yPos);
    rectfill(scrTopMesgBar, xPos, yPos, xPos + scrW, yPos + scrH, pinkColor);
    masked_stretch_blit(faceLogo[fl_idx], scrTopMesgBar, 0, 0, faceLogo[fl_idx]->w, faceLogo[fl_idx]->h, xPos, yPos, scrW, scrH);
    }


#define MeterXsz 37
	    //Change in bottom bar
    //meterDigit = load_png("Meter.png", pal);
    pinkColor = makecol(255,0,255);
    //pinkColor = makecol(255,0,0);
    scrW = getScrWidthForElement(MeterXsz); scrH = getScrHeightForElement(64);
    scrWTemp = scrW;
    getElemXYpos(0, &xPos, &yPos);
    rectfill(scrBotMesgBar, xPos, yPos, xPos + (scrW * 3), yPos + scrH, pinkColor);
    speedt = speed;
    digitFg = 0;
    digit = speedt / 100;
    if ( digit != 0)
    {
    masked_stretch_blit(meterDigit, scrBotMesgBar, digit * MeterXsz, 0, MeterXsz, 64, xPos, yPos, scrW, scrH);//100 place digit
    digitFg = 1;
    scrWTemp = getScrWidthForElement(getDigitWidth(digit));
    }
    digit = (speedt % 100 ) / 10;
    if ( digitFg || digit != 0)
    {
    xPos += scrWTemp;
    masked_stretch_blit(meterDigit, scrBotMesgBar, digit * MeterXsz, 0, MeterXsz, 64, xPos, yPos, scrW, scrH);
    scrWTemp = getScrWidthForElement(getDigitWidth(digit));
    }
    digit = speedt % 10;
    xPos += scrWTemp;
    masked_stretch_blit(meterDigit, scrBotMesgBar, digit * MeterXsz, 0, MeterXsz, 64, xPos, yPos, scrW, scrH);
    //scrWTemp = getScrWidthForElement(getDigitWidth(digit));


    switch(screenResolId) //800X600  1024X768
    {
	case 0:
    		masked_blit(scrTopMesgBar, app, 0, 0, 0, 0, 640, 120);
    		masked_blit(scrBotMesgBar, app, 0, 0, 0, 360, 640, 120);
		break;
	case 1:
    		masked_blit(scrTopMesgBar, app, 0, 0, 0, 0, 800, 120);
    		masked_blit(scrBotMesgBar, app, 0, 0, 0, 477, 800, 120);
		break;
	case 2:
    		masked_blit(scrTopMesgBar, app, 0, 0, 0, 0, 1024, 120);
    		masked_blit(scrBotMesgBar, app, 0, 0, 0, 610, 1024, 120);
		break;
    }

}
void draw_animation(BITMAP *app, int a , struct Line *ln )
{
    static BITMAP *ani1[17];
    static int initFg = 0, sprtIdx = -1, lineId = 0;
    static struct Line line;
    PALETTE pal;

    if (a == 1)
    {
	 sprtIdx = 16;
	 line = *ln;
         return;
    }

    if (initFg == 0)
    {
	 ani1[0] = load_png("EXPLOSN1\\EXPLOS~1.png", pal);
	 ani1[1] = load_png("EXPLOSN1\\EXPLOS~2.png", pal);
	 ani1[2] = load_png("EXPLOSN1\\EXPLOS~3.png", pal);
	 ani1[3] = load_png("EXPLOSN1\\EXPLOS~4.png", pal);
	 ani1[4] = load_png("EXPLOSN1\\EXPLOS~5.png", pal);
	 ani1[5] = load_png("EXPLOSN1\\EXPLOS~6.png", pal);
	 ani1[6] = load_png("EXPLOSN1\\EXPLOS~7.png", pal);
	 ani1[7] = load_png("EXPLOSN1\\EXPLOS~8.png", pal);
	 ani1[8] = load_png("EXPLOSN1\\EXPLOS~9.png", pal);
	 ani1[9] = load_png("EXPLOSN1\\EXPLO~10.png", pal);
	 ani1[10] = load_png("EXPLOSN1\\EXPLO~11.png", pal);
	 ani1[11] = load_png("EXPLOSN1\\EXPLO~12.png", pal);
	 ani1[12] = load_png("EXPLOSN1\\EXPLO~13.png", pal);
	 ani1[13] = load_png("EXPLOSN1\\EXPLO~14.png", pal);
	 ani1[14] = load_png("EXPLOSN1\\EXPLO~15.png", pal);
	 ani1[15] = load_png("EXPLOSN1\\EXPLO~16.png", pal);
	 ani1[16] = load_png("EXPLOSN1\\EXPLO~17.png", pal);

	 initFg = 1;
    }
    if (sprtIdx >= 12)
    {
	    //clear_keybuf();
	    //readkey();
	 line.drawExtSprite(app, ani1[sprtIdx--]);
    }


}
END_OF_FUNCTION(main);
