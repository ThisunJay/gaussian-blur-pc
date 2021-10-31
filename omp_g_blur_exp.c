# include <stdio.h>
# include <stdlib.h>
# include <malloc.h>
# include <stdint.h>
# include <time.h>
# include <math.h>
# include <sys/time.h>
# include <omp.h>

# define IMAGESIZE 54

# pragma pack(push, 2)          
    typedef struct {
        char sign;
        int size;
        int notused;
        int data;
        int headwidth;
        int width;
        int height;
        short numofplanes;
        short bitpix;
        int method;
        int arraywidth;
        int horizresol;
        int vertresol;
        int colnum;
        int basecolnum;
    } img;
# pragma pop

char* openImageFile(img* bmp);
void generateOutputImage(char* imageDataChar, img* bmp);
int setBoundaries(int i , int minLen , int maxLen);

int main(){
    unsigned char* imageDataChar;
    img* bmp = (img*) malloc (IMAGESIZE);

    int radius = 4;

    imageDataChar = openImageFile(bmp);

    int width = bmp->width;
    int height = bmp->height;
    printf("Image Width = %d Image Width = %d\n", width, height);
    
    int i, j;       
    int rgb_width =  width * 3 ;
    if ((width * 3  % 4) != 0) {
       rgb_width += (4 - (width * 3 % 4));  
    }

    unsigned char* red;
    unsigned char* green;
    unsigned char* blue;
    red = (unsigned char*) malloc(width*height);
    green = (unsigned char*) malloc(width*height);
    blue = (unsigned char*) malloc(width*height);
	
    int pos = 0; 
    for (i = 0; i < height; i++) {
        for (j = 0; j < width * 3; j += 3, pos++){
            red[pos] = imageDataChar[i * rgb_width + j];
            green[pos] = imageDataChar[i * rgb_width + j + 1];
            blue[pos] = imageDataChar[i * rgb_width + j + 2];
        }
    }

    struct timeval algo_start_time, algo_stop_time, algo_elapsed_time; 
   
 	gettimeofday(&algo_start_time,NULL);
        #pragma omp parallel for schedule(static,100) private(j)
	    for( i = 0 ; i < height; i++){
            for(j = 0 ; j < width ; j++) {
                int row;
                int col;
                double ImageRedSum = 0;
                double ImageGreenSum = 0;
                double ImageBlueSum = 0;
                double ImageWeightSum = 0;
            
                for(row = i-radius; row <= i + radius; row++){
                    for(col = j-radius; col<= j + radius; col++) {
                        int x = setBoundaries(col,0,width-1);
                        int y = setBoundaries(row,0,height-1);
                        int tempPos = y * width + x;
                        double square = (col-j)*(col-j)+(row-i)*(row-i);
                        double sigma = radius*radius;
                        double weight = exp(-square / (2*sigma)) / (3.14*2*sigma);
                        ImageRedSum += red[tempPos] * weight;
                        ImageGreenSum += green[tempPos] * weight;
                        ImageBlueSum += blue[tempPos] * weight;
                        ImageWeightSum += weight;
                    }    
                }
                red[i*width+j] = round(ImageRedSum/ImageWeightSum);
                green[i*width+j] = round(ImageGreenSum/ImageWeightSum);
                blue[i*width+j] = round(ImageBlueSum/ImageWeightSum);
                ImageRedSum = 0;
                ImageGreenSum = 0;
                ImageBlueSum = 0;
                ImageWeightSum = 0;
            }
	    }  
    gettimeofday(&algo_stop_time,NULL);
    timersub(&algo_stop_time, &algo_start_time, &algo_elapsed_time); 

    printf("Time Taken ----> %f \n", algo_elapsed_time.tv_sec+algo_elapsed_time.tv_usec/1000000.0);

    pos = 0;
    for (i = 0; i < height; i++ ) {
        for (j = 0; j < width* 3 ; j += 3 , pos++) {
            imageDataChar [i * rgb_width  + j] = red[pos];
            imageDataChar [i * rgb_width  + j + 1] = green[pos];
            imageDataChar [i * rgb_width  + j + 2] = blue[pos];
        }
    }

    generateOutputImage(imageDataChar, bmp);
    free(red);
    free(green);
    free(blue);
    free(bmp); 
    return 0;
}


char* openImageFile(img* in) {
    char inPutFileNameBuffer[32];
    sprintf(inPutFileNameBuffer, "500.bmp");

    FILE* file;
    if (!(file = fopen(inPutFileNameBuffer, "rb"))) {
        printf("No File Found Named 500.bmp");
        free(in);
        exit(1);
    }
    fread(in, 54, 1, file);

    char* data = (char*) malloc (in->arraywidth);
    fseek(file, in->data, SEEK_SET);
    fread(data, in->arraywidth, 1, file);
    fclose(file);
    return data;
}
void generateOutputImage(char* imageDataChar , img* out) {
    FILE* file;
    time_t now;
    time(&now);
    char fileNameBuffer[32];
    sprintf(fileNameBuffer, "sample_parallel.bmp");
    file = fopen(fileNameBuffer, "wb");
    fwrite(out, IMAGESIZE, 1, file);
    fseek(file, out->data, SEEK_SET);
    fwrite(imageDataChar, out->arraywidth, 1, file);
    fclose(file);
}


int setBoundaries(int i , int minLen , int maxLen){
    if( i < minLen) return minLen;
    else if( i > maxLen ) return maxLen;
    return i;  
}


