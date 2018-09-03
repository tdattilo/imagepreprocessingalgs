#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <tiffio.h>
#include <tiff.h>

#define KN .3
#define KS .5
#define RS 128

using namespace std;

struct stats{
	double mean;
	double stdev;
};

struct tiffData{
	long imageWidth;
	long imageLength;
	char **data;
	uint32 xres;
	uint32 yres;
	uint32 resunit;
};
//getImage and makeImage largely derived from Gleicher. LibTiff package used.
void getImage(tiffData&, int);
void makeImage(tiffData&, char, char**, int);
stats statsCalc(char*, char*, char*);
char niblack(stats, char);
char sauvola(stats, char);
unsigned char bernsen(char*, char*, char*);
unsigned char bernsenCorner(char*, char*, int, int);
stats cornerStatsCalc(char*, char*, int);
unsigned char bernsenEdge(char*, char*, char*, int, int);
stats edgeStatsCalc(char*, char*, char*, int, int);


int main(){
	for (int z=1; z<60; z++){
	tiffData inData;
	char ** niblackData;
	char ** sauvolaData;
	char ** bernsenData;
	getImage(inData, z);
	
	niblackData=new char*[inData.imageLength];
	sauvolaData=new char*[inData.imageLength];
	bernsenData=new char*[inData.imageLength];
	for(int j=0; j<inData.imageLength; j++){
		niblackData[j]=new char[inData.imageWidth+1];
		sauvolaData[j]=new char[inData.imageWidth+1];
		bernsenData[j]=new char[inData.imageWidth+1];
	}
	for(int l=0; l<(inData.imageLength); l++){
		for(int k=0; k<(inData.imageWidth); k++){
			if((l==0||l==(inData.imageLength-1))
			&&(k==0||k==(inData.imageWidth-1))){
				if(l==(inData.imageLength-1)){
				stats test=cornerStatsCalc(inData.data[l-1]+k, 
					inData.data[l]+k, k);
				bernsenData[l][k]=bernsenCorner(inData.data[l-1]+k, 
					inData.data[l]+k, k, l);
				niblackData[l][k]=niblack(test, inData.data[l][k]);
				sauvolaData[l][k]=sauvola(test, inData.data[l][k]);
				}
				else{
					stats test=cornerStatsCalc(inData.data[l]+k, 
						inData.data[l+1]+k, k);
					bernsenData[l][k]=bernsenCorner(inData.data[l]+k, 
						inData.data[l+1]+k, k, l);
					niblackData[l][k]=niblack(test, 
						inData.data[l][k]);
					sauvolaData[l][k]=sauvola(test, 
						inData.data[l][k]);
				}

			}
			else if((l==0)||(l==(inData.imageLength-1))){
				if(l==0){
					stats test=edgeStatsCalc(inData.data[l]+k, 
						inData.data[l+1]+k, inData.data[l+2]+k, k, l);
					bernsenData[l][k]=bernsenEdge(inData.data[l]+k, 
						inData.data[l+1]+k, inData.data[l+2]+k, k, l);
					niblackData[l][k]=niblack(test, inData.data[l][k]);
					sauvolaData[l][k]=sauvola(test, inData.data[l][k]);
				}
				else{
					stats test=edgeStatsCalc(inData.data[l-1]+k, 
						inData.data[l]+k, inData.data[l]+k, k, l);
					bernsenData[l][k]=bernsenEdge(inData.data[l-1]+k, 
						inData.data[l]+k, inData.data[l]+k, k, l);
					niblackData[l][k]=niblack(test, inData.data[l][k]);
					sauvolaData[l][k]=sauvola(test, inData.data[l][k]);
				}

			}
			else if((k==0)||(k==(inData.imageWidth-1))){
				stats test=edgeStatsCalc(inData.data[l-1]+k, 
					inData.data[l]+k, inData.data[l+1]+k, k, l);
				niblackData[l][k]=niblack(test, inData.data[l][k]);
				sauvolaData[l][k]=sauvola(test, inData.data[l][k]);
				bernsenData[l][k]=bernsenEdge(inData.data[l-1]+k, 
					inData.data[l]+k, inData.data[l+1]+k, k, l);
			}
			else{
				stats test=statsCalc((inData.data[l-1])+k-1, 
					(inData.data[l])+k-1, (inData.data[l+1])+k-1);
				niblackData[l][k]=niblack(test, inData.data[l][k]);
				sauvolaData[l][k]=sauvola(test, inData.data[l][k]);
				bernsenData[l][k]=bernsen((inData.data[l-1])+k-1, 
					(inData.data[l])+k-1, (inData.data[l+1])+k-1);
			}
		}
	}
	makeImage(inData, 'n', niblackData, z);
	makeImage(inData, 's', sauvolaData, z);
	makeImage(inData, 'b', bernsenData, z);
	
	for(int j=0; j<inData.imageLength; j++){
		delete[] niblackData[j];
		delete[] sauvolaData[j];
		delete[] bernsenData[j];
	}
	delete[] niblackData;
	delete[] sauvolaData;
	delete[] bernsenData;
	}
	return 0;
}

stats statsCalc(char* first, char* second, char* third){
	double sum = 0;
	sum+=*(first)+*(first+1)+*(first+2);
	sum+=*(second)+(second+1)+(second+2);
	sum+=*(third)+*(third+1)+*(third+2);
	double mean=sum/9;
	double variance=0;
	variance+=(*(first)-mean)*(*(first)-mean)+(*(first+1)-mean)*(*(first+1)-mean)+(*(first+2)-mean)*(*(first+2)-mean)
			+(*(second)-mean)*(*(second)-mean)+(*(second+1)-mean)*(*(second+1)-mean)+(*(second+2)-mean)*(*(second+2)-mean)
			+(*(third)-mean)*(*(third)-mean)+(*(third+1)-mean)*(*(third+1)-mean)+(*(third+2)-mean)*(*(third+2)-mean);

	variance=variance/8;
	stats retVals;
	retVals.mean=mean;
	retVals.stdev=sqrt(variance);
	return retVals;
}

char niblack(stats calcStats, char val){
	int retVal = calcStats.mean + KN * calcStats.stdev;
	if (retVal>val) retVal = 0;
	else retVal=255;
	return retVal;
}
char sauvola(stats calcStats, char val){
	int retVal = calcStats.mean * (1-KS *(1-(calcStats.stdev/RS)));
	if (retVal>val) retVal = 0;
	else retVal=255;
	return retVal;
}
unsigned char bernsen(char* first, char* second, char* third){
	unsigned char max, min;
	max=0;
	min=255;
	if(*(first)>max) max=*(first);
	if(*(first)<min) min=*(first);
	if(*(first+1)>max) max=*(first+1);
	if(*(first+1)<min) min=*(first+1);
	if(*(first+2)>max) max=*(first+2);
	if(*(first+2)<min) min=*(first+2);
	if(*(second)>max) max=*(second);
	if(*(second)<min) min=*(second);
	if(*(second+1)>max) max=*(second+1);
	if(*(second+1)<min) min=*(second+1);
	if(*(second+2)>max) max=*(second+2);
	if(*(second+2)<min) min=*(second+2);
	if(*(third)>max) max=*(third);
	if(*(third)<min) min=*(third);
	if(*(third+1)>max) max=*(third+1);
	if(*(third+1)<min) min=*(third+1);
	if(*(third+2)>max) max=*(third+2);
	if(*(third+2)<min) min=*(third+2);
	if(*(second+1)>(max + min)/2) return 255;
	else return 0;
}

unsigned char bernsenCorner(char* first, char* second, 
	int k, int l){
	unsigned char max, min;
	max=0;
	min=255;
	if(k<l){
		if(*(first)>max) max=*(first);
		if(*(first)<min) min=*(first);
		if(*(first+1)>max) max=*(first+1);
		if(*(first+1)<min) min=*(first+1);
		if(*(second)>max) max=*(second);
		if(*(second)<min) min=*(second);
		if(*(second+1)>max) max=*(second+1);
		if(*(second+1)<min) min=*(second+1);
		if(*(second)>(max + min)/2) return 255;
		else return 0;
	}
	else if (k>l){
		if(*(first)>max) max=*(first);
		if(*(first)<min) min=*(first);
		if(*(first-1)>max) max=*(first-1);
		if(*(first-1)<min) min=*(first-1);
		if(*(second)>max) max=*(second);
		if(*(second)<min) min=*(second);
		if(*(second-1)>max) max=*(second-1);
		if(*(second-1)<min) min=*(second-1);
		if(*(first)>(max + min)/2) return 255;
		else return 0;
	}
	else{
		if(l==0){
			if(*(first)>max)max=*(first);
			if(*(first)<min) min=*(first);
			if(*(first+1)>max)max=*(first+1);
			if(*(first+1)<min) min=*(first+1);
			if(*(second)>max) max=*(second);
			if(*(second)<min) min=*(second);
			if(*(second+1)>max) max=*(second+1);
			if(*(second+1)<min) min=*(second+1);
			if(*(first)>(max + min)/2) return 255;
			else return 0;
		}
		else{
			if(*(first)>max)max=*(first);
			if(*(first)<min) min=*(first);
			if(*(first-1)>max)max=*(first-1);
			if(*(first-1)<min) min=*(first-1);
			if(*(second)>max) max=*(second);
			if(*(second)<min) min=*(second);
			if(*(second-1)>max) max=*(second-1);
			if(*(second-1)<min) min=*(second-1);
			if(*(second)>(max + min)/2) return 255;
			else return 0;
		}
	
	}
}
stats cornerStatsCalc(char* first, char* second, int k){
	double sum = 0;
	int lrbox=1;
	if(k>0)lrbox*=-1;
	for (int j=0; j<2; j++) sum+= *(first+j*lrbox)+*(second+j*lrbox);
	double mean=sum/4;
	double variance=0;
	for (int i=0; i<2; i++){
		variance+=(*(first+i*lrbox)-mean)*(*(first+i*lrbox)-mean)
			+(*(second+i*lrbox)-mean)*(*(second+i*lrbox)-mean);
	}
	variance=variance/3;
	stats retVals;
	retVals.mean=mean;
	retVals.stdev=sqrt(variance);
	return retVals;
}
unsigned char bernsenEdge(char* first, char* second, 
	char* third, int k, int l){
	unsigned char max, min;
	max=0;
	min=255;
	
	if(k==0){
		for(int i=0; i<2; i++){
			if(*(first+i)>max)max=*(first+i);
			if(*(first+i)<min) min=*(first+i);
			if(*(second+i)>max) max=*(second+i);
			if(*(second+i)<min) min=*(second+i);
			if(*(third+i)>max) max=*(third+i);
			if(*(third+i)<min) min=*(third+i);
		}
		if(*(second)>(max + min)/2) return 255;
		else return 0;
	}
	
	else if (l==0){
		for(int i=0; i<3; i++){
			if(*(first+i)>max)max=*(first+i);
			if(*(first+i)<min) min=*(first+i);
			if(*(second+i)>max) max=*(second+i);
			if(*(second+i)<min) min=*(second+i);
		}
		if(*(first+1)>(max + min)/2) return 255;
		else return 0;
	}
	else if (k<l){
		for(int i=0; i<3; i++){
			if(*(first+i)>max)max=*(first+i);
			if(*(first+i)<min) min=*(first+i);
			if(*(second+i)>max) max=*(second+i);
			if(*(second+i)<min) min=*(second+i);
		}
		if(*(second+1)>(max + min)/2) return 255;
		else return 0;
	}
	else{
			for(int i=0; i<2; i++){
			if(*(first-i)>max)max=*(first-i);
			if(*(first-i)<min) min=*(first-i);
			if(*(second-i)>max) max=*(second-i);
			if(*(second-i)<min) min=*(second-i);
			if(*(third-i)>max) max=*(third-i);
			if(*(third-i)<min) min=*(third-i);
		}
		if(*(second)>(max + min)/2) return 255;
		else return 0;
	}
	
}
stats edgeStatsCalc(char* first, char* second, 
	char* third, int k, int l){
	double sum = 0;
	double mean;
	double variance;
	if(k==0){
		for (int j=0; j<2; j++) sum+= *(first+j)+*(second+j)+*(third+j);
		mean=sum/6;
		variance=0;
		for (int i=0; i<2; i++){
			variance+=(*(first+i)-mean)*(*(first+i)-mean)
				+(*(second+i)-mean)*(*(second+i)-mean)+(*(third+i)-mean)
					*(*(third+i)-mean);
		}
		variance=variance/5;
	}
	else if(l==0){
		for (int j=0; j<3; j++) sum+= *(first+j)+*(second+j);
		mean=sum/6;
		variance=0;
		for (int i=0; i<3; i++){
			variance+=(*(first+i)-mean)*(*(first+i)-mean)
				+(*(second+i)-mean)*(*(second+i)-mean);
		}
		variance=variance/5;	
	}
	else if(k<l){
		for (int j=0; j<3; j++) sum+= *(first-j)+*(second-j);
		mean=sum/6;
		variance=0;
		for (int i=0; i<3; i++){
			variance+=(*(first-i)-mean)*(*(first-i)-mean)
				+(*(second-i)-mean)*(*(second-i)-mean);
		}
		variance=variance/5;	
	}
	else{
		for (int j=0; j<2; j++) sum+= *(first-j)+*(second-j)
			+*(third-j);
		mean=sum/6;
		variance=0;
		for (int i=0; i<2; i++){
			variance+=(*(first-i)-mean)*(*(first-i)-mean)
				+(*(second-i)-mean)*(*(second-i)-mean)
					+(*(third-i)-mean)*(*(third-i)-mean);
		}
		variance=variance/5;
	}
	stats retVals;
	retVals.mean=mean;
	retVals.stdev=sqrt(variance);
	return retVals;
}

void getImage(tiffData& datastruct, int file){
	stringstream stream;
	stream<<"Original Images/"<<file<<".tiff";
	string s;
	string t;
	stream>>s;
	stream>>t;
	s+=" ";
	s+=t;
    TIFF* tif = TIFFOpen(s.c_str(), "r");
    if (tif) {
        uint32 imagelength;
        uint32 imagewidth;
        tdata_t buf;
        uint32 row;
        char* temp;
        uint32 xres;
        uint32 yres;
        uint32 resunit;
        
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);
        datastruct.imageLength=imagelength;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imagewidth);
        TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xres);
        TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yres);
        TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT, &resunit);
        datastruct.imageWidth=imagewidth;
        datastruct.xres=xres;
        datastruct.yres=yres;
        datastruct.resunit=resunit;
        datastruct.data=new char*[imagelength];
        buf = _TIFFmalloc(TIFFScanlineSize(tif));
        for (row = 0; row < imagelength; row++){
        	datastruct.data[row]=new char[imagewidth];
            TIFFReadScanline(tif, buf, row);
            temp=static_cast<char*>(buf);
				for(int w=0; w<imagewidth; w++){
					datastruct.data[row][w]=*(temp+w);
				}
        }
        _TIFFfree(buf);    
        TIFFClose(tif);
    }
}


void makeImage(tiffData& datastruct, char ch, char** data, int file){
	TIFF* tif;
	stringstream stream;
	string s;
	if(ch=='n'){
		stream<<"Niblack/";
		stream<<file;
		stream<<".tiff";
	}
	if(ch=='b'){
		stream<<"Bernsen/";
		stream<<file;
		stream<<".tiff";
	}
	if(ch=='s'){
		stream<<"Sauvola/";
		stream<<file;
		stream<<".tiff";
	}
	stream>>s;
	tif=TIFFOpen(s.c_str(), "w");
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, datastruct.imageWidth);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, datastruct.imageLength);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tif, TIFFTAG_COMPRESSION,1);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, 0);
	TIFFSetField(tif, TIFFTAG_XRESOLUTION, datastruct.xres);
	TIFFSetField(tif, TIFFTAG_YRESOLUTION, datastruct.yres);
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, datastruct.resunit);

	unsigned char *buf = NULL;
	if (TIFFScanlineSize(tif)==datastruct.imageWidth)
		buf =(unsigned char*)_TIFFmalloc(datastruct.imageWidth);
	else
		buf = (unsigned char*)_TIFFmalloc(TIFFScanlineSize(tif));

	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 
		datastruct.imageWidth));

	for (uint32 row = 0; row < datastruct.imageLength; row++)
	{
 	   memcpy(buf, data[row], datastruct.imageWidth);
 	   if (TIFFWriteScanline(tif, buf, row, 0) < 0)
 	   break;
	}
	TIFFClose(tif);
	if(buf) _TIFFfree(buf);


}
