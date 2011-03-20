#include <allegro.h>
#include <winalleg.h>
#include <stdio.h>
#include <string>
//#include <vector>
#include <png.h>
#include <loadpng.h>
#include <pthread.h>

#define WINW 512
#define WINH 512

#define NEWVIEWTHRESHOLD 256			

#define TILESIZE 256
#define VIEWTILES 2
#define MAXZOOMLEVEL 2

using namespace std; 

typedef struct Tile {					// tile object
	unsigned int x, y, z;
	BITMAP *image;
	PALETTE pal;
	string filename;
} Tile_t;

// easy fix: make a temp file called "fixme.bat"
// contents of which will be "del /s /q [zoomlevel_dir]"
// that should fix the offending pngs in that zoom level
// on next reload, which will call fixme.bat if it exists
// if the program terminates normally, fixme.bat will be
// deleted

// ----------------------------------------------- global var def --------------

BITMAP *screenbuf, *viewport;

int Mx=0, My=0, Mr=4, Mz=0;				// old mouse coords
int Vx=0, Vy=0;							// 
int Gx=0, Gy=0, Gz=0;

int serverNo=0;							// rotating server# for the tile URL
int updateView=1;						// flag to update the display
//vector<Tile_t> tileCache;
FONT *mapFont;

// table of the maxXY values (20 values for the 20 zooms)
const unsigned int pow2[]={
	1, 2, 4, 8, 16,
	32, 64, 128, 256, 512,
	1024, 2048, 4096, 8192,
	16384, 32768, 65536, 131072, 262144
};

// ------------------------------------------------ prototype def --------------

bool FileExists(const char *f);
void GetTile(Tile_t &a);
void GetViewTiles();
void *LoadDisplayTiles(void *b);
void pruneTileCache();


void *LoadDisplayTile(void *b){
	char imageFILE[64]; 
	Tile_t a;
	int dx=((int)b)%VIEWTILES;			// position in the view buffer
	int dy=(((int)b)-dx)/VIEWTILES;
	
	a.x=Gx+dx, a.y=Gy+dy, a.z=Gz;

	if(a.x<pow2[Gz] && a.y<pow2[Gz]){
		
		sprintf(imageFILE,"%d\\%d_%d.png",Gz,a.x,a.y);
		if(!FileExists(imageFILE)) GetTile(a);
		a.image=load_png(imageFILE,a.pal);
//		set_palette(a.pal);
		blit(a.image, viewport, 0, 0, dx*256, dy*256, 256, 256);
		destroy_bitmap(a.image);
		a.image=NULL;
	}
}

void GetViewTiles(){
	pthread_t tid[VIEWTILES*VIEWTILES];
	clear(viewport);
	for(int i=0;i<VIEWTILES*VIEWTILES;i++){
		pthread_create(&tid[i], NULL, LoadDisplayTile, (void *)i);
	}
	updateView=0,Vx=0,Vy=0;
}

void Mouse(){
	char CMD[64];
	int dx=0,dy=0,dz=mouse_z-Mz;
	if (mouse_b & 2) updateView=1;			// RMB for tile reload
	if (mouse_b & 1){
		dx=mouse_x-Mx;
		dy=mouse_y-My;
		Vx+=dx;
		Vy+=dy;
	}
	if(dz){
		if(!(dz<0 && !Gz)){
			Mr=(dz>0)?Mr*2:Mr/2;
			if(Mr>256) Mr=4;				// correct mouse circle cursor size
			if(Mr<4) Mr=256;
			Gz+=dz;
			Vx-=dz*mouse_x*2, Vy-=dz*mouse_y*2;
			if(dz) Gx=(dz>0)?Gx*2:Gx/2, Gy=(dz>0)?Gy*2:Gy/2;
			updateView=1;

			sprintf(CMD,"echo @del /s /q %d > fixme.bat",Gz);
			system(CMD);					// easyfix
		}	
	}
	Mz=mouse_z;
	Mx=mouse_x;
	My=mouse_y;			// update "old" mouse coords

	
	// take care of throwing up new views
	if(Vx>NEWVIEWTHRESHOLD){	Vx=0; Gx--; updateView=1;}
	if(Vy>NEWVIEWTHRESHOLD){	Vy=0; Gy--; updateView=1;}
	if(Vx<-NEWVIEWTHRESHOLD){	Vx=0; Gx++; updateView=1;}
	if(Vy<-NEWVIEWTHRESHOLD){	Vy=0; Gy++; updateView=1;}

	clear(screenbuf);
	blit(viewport, screenbuf, 0, 0, Vx, Vy, viewport->w, viewport->h);

//	stretch_blit(viewport, screenbuf, 0, 0, WINW, WINH, Vx, Vy, (Gz+1)*WINW, (Gz+1)*WINH);
	acquire_screen(); 
	blit(screenbuf, screen, 0, 0, 0, 0, WINW, WINH);
	circle(screen, mouse_x, mouse_y, Mr, makecol(255,0,0));		// draw cursor
	textprintf_ex(screen, mapFont, 4, 4, makecol(200, 0, 128), -1, "Zoom level %d", Gz);
	release_screen();	
}

int main(void){
	system("fixme");		// easyfix
	// make sure we have all the directories before we do anything, otherwise
	// FileExists will fail and its result will crash the app:
	// we have a maxzoom of 19, so 20 folders needed
	int j=19; char mdTMP[16]=""; string mdCMD; do{
		if(j){sprintf(mdTMP,"md %d >nul | ",j);}
		else{ sprintf(mdTMP,"md %d >nul",j);}
		mdCMD+=mdTMP;
	}while(j--); system(mdCMD.c_str()); system("cls");
	system("type readme.txt");
	
	allegro_init();
	screenbuf=create_bitmap(WINW,WINH);
	viewport=create_bitmap(TILESIZE*VIEWTILES,TILESIZE*VIEWTILES);

	install_mouse(); install_keyboard(); register_png_file_type();
	set_color_depth(16); set_gfx_mode(GFX_AUTODETECT_WINDOWED, WINW, WINH, 0, 0);

	PALETTE r;
	if(FileExists("6x8.pcx")) mapFont=load_font("6x8.pcx",r,NULL);
	else mapFont=font;

	while(!keypressed()){
		Mouse();
		if(updateView) GetViewTiles(); 
		rest(10);
	}
	
	system("del fixme.bat");			// easyfix:

return 0;}END_OF_MAIN();

// ------------------------------------- miscellaneous helpers -----------------

bool FileExists(const char *f){
	if(access(f, F_OK)!=-1) return true;
	return false;
}

void GetTile(Tile_t &a){
	// sample: http://mt1.google.com/vt/v=w2.104&hl=en&x=3&y=5&z=4&s=Galile
	char curlCMD[1024];
	sprintf(curlCMD, "curl -s \"http://mt%d.google.com/vt/v=w2.104&hl=en&x=%d&y=%d&z=%d&s=%s\" > %d\\%d_%d.png ",
			serverNo++, a.x, a.y, a.z,
			((string) "Galileo").substr(0,AL_RAND()%7+1).c_str(),
			a.z, a.x, a.y);
	if(serverNo>3) serverNo=0;
	system(curlCMD);
}
