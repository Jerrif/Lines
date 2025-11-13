/* Converts an input image to lines by randomly selecting
 * a position and color to draw.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>
//#include <ctype.h> // Don't actually need these? It's all just isalnum() islower() toupper() etc.
#include <windows.h> // for FindFirstFile
#include <process.h> // for threads
#include "rgb_struct.h" // defines RGBVALUES and BYTE
#include "linked_lists_hash_tables.h" // When testing, remember to copy this from the testing project into this one!
#include "hash_tables.h" // This is just one function. Maybe I can just move it into here or a different file?
#include "lodepng.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define RAND() (rand()&0x7ffff) /* ensure only 15-bits. For func randGen */
//#define MAX_THREADS  4
//#define max_threads  4

// void primaryFunction(char *active_file);
void primaryFunction(void *argss);

struct func_args // yeah yeah, I know this is verbose and dumb but I just want this to work for now
{
	char file_to_pass[180];
	uint16_t array_index;
};
struct func_args args;
struct func_args *args_ptr = &args;

struct timeval  tv1, tv4; // found this online for timing stuff

// this function is unnecessary, you can just use the function inside it
void encodeOneStep(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  /*Encode the image*/
  unsigned error = lodepng_encode24_file(filename, image, width, height);

  /*if there's an error, display it*/
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

/* I found this solution online. rand() on my machine was only
 * generating 16 bit (65535) numbers, but I want 32 bits
 * I at least mostly understand what's going on here
 */
uint32_t randGen()
{
	uint64_t n = 0;
	n+=RAND();
	n+=(((uint32_t)RAND()) << 15);
	n+=(((uint32_t)RAND()) << 30);
	return n;
}

/*	I think a better random generator
	The previous one (randGen()) was not
	generating a new number for simultaneous threads
	*/
uint32_t next_g = 1; // global version of "next" variable

int myrand1(uint32_t seed)
{
	uint32_t next = seed;
	next = next * 1103515245 + 12345;
	return next;
}

// calculate euclidian distance between two points
int32_t e_dist(int32_t r1, int32_t r2, int32_t g1, int32_t g2, int32_t b1, int32_t b2)
{
	int32_t red = abs(r1 - r2);
	int32_t green = abs(g1 - g2);
	int32_t blue = abs(b1 - b2);
//	int32_t d = sqrt((red * red) + (green * green) + (blue * blue));
	int32_t d = (red * red) + (green * green) + (blue * blue);
	return d;
}

int max_threads = 4;
int f = 0;
int file_count = 0;
int locked = 0;
uint16_t threadNr = 0;
HANDLE *thread_array;
unsigned threadID;
/* command line arguments:
 * -a animation mode = False
 */

int16_t size = 40;
int32_t iterations = 1000000;
float slope = 0.0;
uint8_t slope_flag = 0;
uint8_t bg_flag = 0;
uint8_t out_flag = 0;
uint8_t out_min_flag = 0;
uint8_t anim_flag = 0;
uint8_t rng_seed_flag = 0;
uint32_t rng_seed = 0;
uint8_t color_distribution_flag = 0;
uint16_t o = 0; // outfile count for the anim mode

int main(int argc, char *argv[])
{
	/* parse the command line arguments */
	char *cvalue = NULL;
    int opt;
    char *usage_text = "[OPTION]...\n"
    				   "Converts an image to lines by randomly selecting"
					   " a position and color to draw.\n"
    				   "Requires two directories: '\\input' and '\\output'.";
    char *options_text="  -s [40]          Length of the line in pixels.\n"
    				   "  -i [1000000]     Number of iterations to use.\n"
    				   "  -m [random]      Set slope to use between -s and s (float). Random by default.\n"
    				   "  -t [4]           Number of threads to use.\n"
    				   "  -r [random]      Sets the RNG seed to use. Use a 32 bit value (between 0 and ~4.29 billion).\n"
    				   "  -a               Animation mode. Creates new outputs at various stages\n"
    				   "                   which can be combined to make a video. Outputs ~150 images.\n"
    				   "                   Do not use with -o or -p.\n"
    				   "  -b               Use a black background instead of white.\n"
    		       "  -c               Use an even distribution for each color instead of weighted.\n"
    				   "  -p               Strip size and iteration information from output filename.\n"
    			     "  -o               Output filename will be output.\n"
    				   "  -h               Show this help text.";
    while ((opt = getopt (argc, argv, "s:i:m:t:r:bocpah?")) != -1)
    {
      switch (opt)
        {
        case 's':
        	cvalue = optarg;
        	size = atoi(cvalue);
        	if(size < 1)
        	{
        		printf("Size cannot be less than 1\n");
        		return 0;
        	}
        	break;
        case 'i':
        	cvalue = optarg;
        	iterations = atol(cvalue);
        	if(iterations < 1)
        	{
        		printf("Iterations cannot be less than 1\n");
        		return 0;
        	}
        	break;
        case 'b':
        	bg_flag = 1;
        	break;
        case 'm':
        	slope_flag = 1;
        	cvalue = optarg;
        	slope = atof(cvalue);
        	break;
        case 't':
        	cvalue = optarg;
        	max_threads = atoi(cvalue);
        	if(max_threads < 1)
        	{
        		printf("Threads cannot be less than 1\n");
        		return 0;
        	}
        	break;
        case 'r':
        	rng_seed_flag = 1;
        	cvalue = optarg;
        	rng_seed = atol(cvalue);
        	break;
        case 'c':
        	color_distribution_flag = 1;
        	break;
        case 'o':
        	out_flag = 1;
        	break;
        case 'p':
        	out_min_flag = 1;
        	break;
        case 'a':
        	anim_flag = 1;
        	break;
        case 'h': // fallthrough is intentional
        case '?':
        	printf("Usage: %s %s\n\n%s\n", argv[0], usage_text, options_text);
        	return 1;
        default:
        	printf("I don't know what you did, but you shouldn't be here\n");
        	break;
        }
    }

    gettimeofday(&tv1, NULL); // start the global timer

    if(max_threads == 1)
    	printf("Using %d thread\n", max_threads);
    else
    	printf("Using %d threads\n", max_threads);
    thread_array = (HANDLE *)malloc(max_threads * sizeof(HANDLE));
    memset(thread_array, 0, max_threads * sizeof(HANDLE));

//    for(int c=0; c < max_threads; c++)
//    {
//    	printf("thread_array[%d]: %s\n", c, thread_array[c]);
//    }

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
//    char (*file_list[10])[180];
//    char *file_list[20] = {0}; // an array of 20 pointers to char* s of variable length
//    char **file_list; // an array of 20 pointers to char* s of variable length

    hFind = FindFirstFile("input\\*", &FindFileData);
    do
    { // I don't know how big to allocate the file list array yet, so I'm just going to do this first pass to count the files
       if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
       {
    	   file_count++;
       }
    } while (FindNextFile(hFind, &FindFileData) != 0);
    char *file_list[file_count];
    hFind = FindFirstFile("input\\*", &FindFileData);
    int z = 0; // index for file list
    do // but I do it again over here to actually store the filenames now that I know how many there are
    {
       if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
       {
//    	   file_list = (char **)malloc(sizeof(char *)*z);
//    	   file_list[z] = malloc(500);
    	   file_list[z] = malloc(strlen(FindFileData.cFileName) + 1); // allocate the memory for the file name
    	   strcpy(file_list[z], FindFileData.cFileName);
    	   z++;
       }
    }
    while (FindNextFile(hFind, &FindFileData) != 0);
//    fprintf(debug_file, "Total Files: %i\n", file_count); // DEBUGGING

    double tt = 0.0;
    uint8_t mgmt_array[file_count];

    memset(mgmt_array, 0, sizeof(uint8_t) * file_count);

    printf("File count: %d\n", file_count);
    uint8_t done = 0;
//	f_mutex = CreateMutex(NULL, TRUE, NULL);
//    for(int c=0; c < max_threads; c++)
//    {
//    	printf("thread_array[%d]: %p\n", c, thread_array[c]);
//    }
//    for(uint16_t f=0; f<file_count; f++) /* loop through each file in input directory */
//    {
	while(done != 1)
	{
//		WaitForSingleObject(f_mutex, INFINITE);
		if(mgmt_array[f] != 1) // pic f is not being worked on
		{
			while(threadNr >= max_threads)
			{
				Sleep(10);
			}
			for(uint16_t h=0; h<max_threads; h++)
			{
				while(locked)
				{
//					printf(".....waiting for unlock.....\n");
					Sleep(10);
				}
				if(thread_array[h] == 0) // if the thread is free
				{
                    locked = 1;
					mgmt_array[f] = 1;
					strcpy(args.file_to_pass, file_list[f]);
					args.array_index = h;
					threadNr++;
					thread_array[h] = (HANDLE)_beginthreadex(NULL, 0, (void*)primaryFunction, (void*)args_ptr, 0, &threadID);
					f++; // maybe take this out? Put it somewhere else?
					break;
				}
				while(locked)
				{
//					printf(".....waiting for unlock 1.....\n");
                    Sleep(10);
				}
			}
		}
		else // if pic f is being worked on, try the next pic
		{
            f++;
		}

		if(f == file_count)
		{
            done = 1;
		}
    }

	Sleep(200);
    printf("\nWaiting for threads to finish\n\n");
	WaitForMultipleObjects(max_threads, thread_array, TRUE, INFINITE);
    while(threadNr > 0)
    {
//    	printf("\n\nWaiting on %d threads\n\n", threadNr);
        Sleep(100);
    }

	gettimeofday(&tv4, NULL);
	tt = (double) (tv4.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv4.tv_sec - tv1.tv_sec);
	printf("\n\n%d image(s) processed in %.2f seconds\n", file_count, tt);
	printf("Average %.2f seconds/image\n\n", tt/file_count);
    printf("Iterations: %d\nSize: %d\n", iterations, size);
    if(rng_seed_flag)
    {
        printf("Seed: %u\n", rng_seed);
    }

    for(int i=0; i<file_count; i++)
    {
        free(file_list[i]);
    }
    free(thread_array);
	return 0;
}


// void primaryFunction(void *active_file)
void primaryFunction(void *argss)
{
	uint32_t next = 1; // Used for the myrand() function.
	int myrand(void) // nested function? I bet you didn't think I could.
	{
		if(rng_seed_flag) // So, if you have set an RNG seed, use the local version of "next". This allows the RNG to still change for each call to myrand() within the function (thread)
		{
			next = next * 1103515245 + 12345; // but each thread will initialize this variable to the same starting point (rng_seed)
			return((unsigned)next);
		}
		else // If you haven't set a seed, use the global "next" variable. This way, each individual call to myrand() will give a completely different number, persisting through threads and function calls.
		{
			next_g = next_g * 1103515245 + 12345; // change the global rng.
			return((unsigned)next_g);
		}
	}

	// mysrand is a seed function for myrand()
	void mysrand(unsigned seed)
	{
	    next_g = seed;
	}
	    if(rng_seed_flag)
	    	mysrand(rng_seed);
	    else
	    	mysrand(tv1.tv_usec); // Seed for the myrand() RNG. Uses the current clock microsecond.


	char z_file[180];
	strcpy(z_file, args.file_to_pass);
	uint16_t r = args.array_index;

	struct timeval  tv2, tv3; // found this online for timing stuff

    uint32_t l = 100; // index for the anim mode
    uint16_t k = 0; // index for the anim mode
    uint32_t hard_code = iterations*0.9; // this is pretty bad

	gettimeofday(&tv3, NULL); // time each image
	char fn[140] = "input\\";
	strcat(fn, z_file);
	char op[140] = "output\\";
	if(!out_flag && !anim_flag)
	{
		if(!out_min_flag && !anim_flag)
			sprintf(op, "%si=%.4gm s=%d ", op, (float)iterations/1000000, size);
		strcat(op, z_file);
	}
	else if(out_flag && !anim_flag)
	{
		strcat(op, "output.png");
	}

	locked = 0; // unlock my ghetto makeshift mutex

	char *infile = fn;
	char *outfile = op;

	uint32_t drawn = 0;
	uint32_t rejected = 0;
	char *filename = (strrchr(infile, '\\')) ? strrchr(infile, '\\') + 1 : infile;
	// pointer to the image input_img
	BYTE* input_img;
	BYTE* output_img;
	// image width and height, and #of components (1= gray scale, 4 = rgba)
	int width, height, components;

	input_img = stbi_load(infile, &width, &height, &components, 3);

	if((input_img == 0) || (width == 0) || (height == 0))
	{
		printf("Error loading image %s. Reason: %s\n", filename, stbi_failure_reason());
//		exit(1);
		return;
	}

	uint32_t width_times_height = width * height; // used a lot, so let's just calculate it once

	output_img = malloc(width_times_height * 3);
	if(!bg_flag)
		memset(output_img, 255, width_times_height * 3); // makes the image base white instead of black

	RGBVALUES draw_color;
	RGBVALUES orig_color;
	RGBVALUES outp_color;

	uint32_t total_colors = 0;
	RGBVALUES *color_array = malloc(sizeof(RGBVALUES) * (255*255*255)); // An array large enough for every single unique color.
	if(color_distribution_flag)
	{
//		uint32_t total_colors = 0;
		RGBVALUES current_px; // Pixel to compare.
		uint32_t key;
		uint32_t table_size = 255 * 255;
		node* hash_table[table_size]; // A hash table. An array of linked lists!

		for(uint32_t init=0; init < table_size; init++) // Initialize hash table
		{
			hash_table[init] = NULL; // Seems to have no performance impact at all?
		}

		for(uint32_t px=0; px < width_times_height; px++)
		{

			memcpy(&current_px, input_img + (px * 3), 3);
			key = hash(current_px);

			if(!find_number(current_px, hash_table[key])) // If the actual value is NOT found at the key location,
			{
				hash_table[key] = prepend(current_px, hash_table[key]); // Add it to the hash table
				memcpy(color_array + total_colors, &current_px, 3); // Add it to the color array
				total_colors++;
			}
		}
		printf("Unique colors: %u\n", total_colors);

		color_array = realloc(color_array, total_colors * 3); // Resize the color array to exactly the size we need.
//		for(int ii=0; ii < total_colors; ii++)
//		{
//			printf("Color[%u]: %u, %u, %u\n", ii, color_array[ii].red, color_array[ii].green, color_array[ii].blue);
//		}

//		uint32_t none = 0;
//		uint32_t totl = 0;
//		for(uint32_t w=0; w < table_size; w++)
//		{
//			uint32_t nodes = count_nodes(hash_table[w]);
////			if(nodes > 10)
////				printf("Nodes in Bucket[%u]: %u\n", w, nodes);
//			if(!nodes)
//				none++;
//			totl += nodes;
//		}
//		printf("Unused nodes: %u\nAverage node count: %f\n", none, (double)totl/(double)table_size);

    for(int i=0; i<table_size; i++)
    {
        free_list(hash_table[i]);
    }
		free(hash_table);
	}

	printf("Processing image: %s\n", filename);

	for(uint32_t i=0; i < iterations; i++) // main loop
	{

		int32_t old_dist = 0;
		int32_t new_dist = 0;
		int32_t loc = 0;
		int32_t px_col = 0;

		if(color_distribution_flag)
		{ // Pick a random color from the array of all the colors
			px_col = abs(myrand() % total_colors);
			memcpy(&draw_color, color_array + px_col, 3);
		}
		else
		{ // For a weighted color distribution, we simply pick a pixel on the image to sample the color from
			px_col = abs(myrand() % (width_times_height)); // randomly select a pixel to get the color from
//			printf("Color pixel:    %d\n", px_col);
			memcpy(&draw_color, input_img + (px_col*3), 3); // px_col*3 because each pixel in the image is 3 bytes (R,G,B)
		}

		// randomly select a starting location for the line/dot
		if(!slope_flag || (slope_flag && slope == 0))
		{
			loc = abs(myrand() % (width_times_height)); // uint32_t maybe?
		}
		else // if slope is set, allow a loc slightly above or below image borders
		{
			if(slope > 0) // positive slope draws downwards (oops)
				loc = ((abs(myrand())) % ((width_times_height)+(size/2 * width))) - (size/2 * width);
			else
				loc = ((abs(myrand())) % ((width_times_height)+(size/2 * width))) + (size/2 * width);
		}

		if(!slope_flag)
		{
		// get random line slope
			int16_t numerator = abs(myrand()) % (size * 2 + 1);
			int16_t denominator = abs(myrand()) % (size * 2) + 1; // no zeros allowed
			if(numerator > size)
				numerator = size - numerator;
			if(denominator > size)
				denominator = size - denominator;
			slope = (float)numerator / (float)denominator;
		}

		// get points in line
		int32_t points[size];
		float rise = 1.0;
		float run = 1.0;
		if(slope > 1.0 || slope < -1.0)
		{
			run = fabs(1.0/slope);
			if(slope < -1.0)
				rise = -1.0;
		}
		else
		{
			rise = slope;
		}
		for(uint16_t p=0; p<size; p++)
		{
			int16_t x_run = run * p;
			int16_t x_rise = rise * p;
			int32_t point_to_draw = loc + x_run + x_rise * width;
			if(point_to_draw < 0 || point_to_draw >= width_times_height)
			{
				points[p] = -1; // is this an okay way to do this?
			}
			else
			{
				points[p] = point_to_draw;
				memcpy(&orig_color, input_img+(point_to_draw*3), 3);
				memcpy(&outp_color, output_img+(point_to_draw*3), 3);

				old_dist += e_dist(orig_color.red, outp_color.red,
								   orig_color.green, outp_color.green,
								   orig_color.blue, outp_color.blue);
				new_dist += e_dist(orig_color.red, draw_color.red,
								   orig_color.green, draw_color.green,
								   orig_color.blue, draw_color.blue);
			}
		}

		if(new_dist < old_dist)
		{
			for(uint16_t p=0; p<size; p++)
			{
				if(points[p] >= 0)
				{
					output_img[points[p]*3] = draw_color.red;
					output_img[points[p]*3 + 1] = draw_color.green;
					output_img[points[p]*3 + 2] = draw_color.blue;
				}
			}
			drawn++;
		}
		else
		{
			rejected++;
		}

		if(anim_flag) // animation outputs. This whole thing is a mess
		{
			uint8_t foo = 0;
			if(i < 1000 && i > 0)
			{
				if((1024%i == 0) || (1536%i == 0))
				{
					foo = 1;
				}
			}
			if(i % l == 0 || i == hard_code || foo == 1)
			{
				foo = 0;
				o++;
				char op[140] = "output\\";
				sprintf(op, "%s%04d.png", op, o);
				outfile = op;
				encodeOneStep(outfile, output_img, width, height);
				k++;
			}
			if(k == 9 || i == iterations*0.7)
			{
				if(i >= iterations*0.7 && i < iterations*0.9)
				{
					l = l / 2;
				}
				else
				{
					l = l * 2;
				}
				k = 0;
			}
			if(i == hard_code)
			{
				hard_code = hard_code + iterations*0.01;
			}
		}
	}

	if(anim_flag) // iterate the filename one last time to draw the finished image
	{
		o++;
		sprintf(op, "%s%04d.png", op, o);
		outfile = op;
	}

  free(color_array);

	encodeOneStep(outfile, output_img, width, height);
	free(input_img);
	free(output_img);
	gettimeofday(&tv2, NULL);
	printf("Completed image: %s in %.2f seconds\n", filename,
		 (double) (tv2.tv_usec - tv3.tv_usec) / 1000000 +
		 (double) (tv2.tv_sec - tv3.tv_sec));

	thread_array[r]=0;
	threadNr--;
	_endthreadex(0);
}
