#include <stdio.h>
#include <stdint.h>
#include <string>

using namespace std;

typedef struct ti {	unsigned long int x, y; } tit;

int main(void){
	int z, a=0;					// zoom, flag to capture first (max/min)(x/y)
	unsigned long int bw=0, bh=0,
	minx, miny,
	maxx,maxy,
	cx, cy,
	i, j;
	
	char CMD[256], tileFile[256];
	string HTML, goo;
	FILE *flist;
	uint8_t *mapMatrix;
	
	printf("AutoTiler for Atlas by JSYang ~ mie_686@yahoo.com\n");
	printf("Zoom level to scan: "); scanf("%d",&z);

//	printf("\nWidth bound (0 for unbounded): "); scanf("%d",&bw);
//	printf("\nHeight bound(0 for unbounded): "); scanf("%d",&bh);

	printf("\nScanning for map tiles...\n");
	sprintf(CMD,"dir %d /OS /B > tiler.tmp",z); system(CMD);

	flist=fopen("tiler.tmp","r"); 
	while(fgets(tileFile,256,flist)){
		goo=tileFile;
		goo=goo.substr(0,goo.find('.',0));
		cx=atoi(goo.substr(0,goo.find('_',0)).c_str());
		cy=atoi(goo.substr(goo.find('_',0)+1).c_str());
		if(a){
			if(cx<minx) minx=cx; if(cx>maxx) maxx=cx;
			if(cy<miny) miny=cy; if(cy>maxy) maxy=cy;			
		}else{ 	minx=maxx=cx, miny=maxy=cy, a++; }

	}
	fclose(flist);

//	if(bw+bh){
//		mapMatrix=malloc(8*bw*bh); i=bw*bh;
//	else mapMatrix=malloc(8*(maxx-minx)*(maxy-miny));

	sprintf(tileFile,"AutoTiler result for zoom %d",z);
	HTML="<html><title>"+(string) tileFile+"</title><body bgcolor=#000><table cellpadding=0 cellspacing=0>";
	cx=maxx-minx, cy=maxy-miny;j=0;
	while(j<=cy){
		HTML+="<tr>";
		i=0;while(i<=cx){
			sprintf(tileFile,"<td><img src=%d/%d_%d.png></td>",z,minx+i,miny+j);
			HTML+=tileFile; i++;
		}
		HTML+="</tr>"; j++;
	}
	HTML+="</table></body></html>";
	flist=fopen("map.html","w+"); fputs(HTML.c_str(),flist); fclose(flist);
	system("del tiler.tmp");
	system("map.html");
	
return 0;}
