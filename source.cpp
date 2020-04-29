//Credit to Adam Sampson for mandelbrot algorithm

//Original timing = 81615ms
//Split up time = 20778ms

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <complex>
#include <fstream>
#include <iostream>
#include <iostream>
#include <thread>
#include <vector>

//Import things we need from the standard library

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::complex;
using std::cout;
using std::endl;
using std::vector;
using std::ofstream;
using std::this_thread::sleep_for;
using std::thread;

// Define the alias "the_clock" for the clock type we're going to use.
typedef std::chrono::steady_clock the_clock;

//Setting the number of threads to be used in the program.
unsigned int NUM_THREADS = 0;

const int WIDTH = 480;
const int HEIGHT = 320;

// The number of times to iterate before we assume that a point isn't in the
// Mandelbrot set.
// (You may need to turn this up if you zoom further into the set.)
const int MAX_ITERATIONS = 500;

// The image data.
// Each pixel is represented as 0xRRGGBB.
uint32_t image[HEIGHT][WIDTH];

int firstColour = 0;
int secondColour = 0;

int i = 0;
int sectionSize = 0;
int threadID = 0;
int threadNumber = 0;

// Write the image to a TGA file with the given name.
// Format specification: http://www.gamers.org/dEngine/quake3/TGA.txt
void fileWriteThread(const char* filename)
{
	ofstream outfile(filename, ofstream::binary);

	uint8_t header[18] = {
		0, // no image ID
		0, // no colour map
		2, // uncompressed 24-bit image
		0, 0, 0, 0, 0, // empty colour map specification
		0, 0, // X origin
		0, 0, // Y origin
		WIDTH & 0xFF, (WIDTH >> 8) & 0xFF, // width
		HEIGHT & 0xFF, (HEIGHT >> 8) & 0xFF, // height
		24, // bits per pixel
		0, // image descriptor
	};
	outfile.write((const char*)header, 18);

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			uint8_t pixel[3] = {
				image[y][x] & 0xFF, // blue channel
				(image[y][x] >> 8) & 0xFF, // green channel
				(image[y][x] >> 16) & 0xFF, // red channel
			};
			outfile.write((const char*)pixel, 3);
		}
	}

	outfile.close();
	if (!outfile)
	{
		// An error has occurred at some point since we opened the file.
		cout << "Error writing to " << filename << endl;
		exit(1);
	}
}

// Render the Mandelbrot set into the image array.
// The parameters specify the region on the complex plane to plot.
void compute_mandelbrot(double left, double right, double top, double bottom, int threadID)
{
	sectionSize = HEIGHT / NUM_THREADS;

	int y = threadID * sectionSize;
	int y_stop = y + sectionSize;

	for (y; y < y_stop; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.
			complex<double> c(left + (x * (right - left) / WIDTH),
				top + (y * (bottom - top) / HEIGHT));

			// Start off z at (0, 0).
			complex<double> z(0.0, 0.0);

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			int iterations = 0;
			while (abs(z) < 2.0 && iterations < MAX_ITERATIONS)
			{
				z = (z * z) + c;

				++iterations;
			}

			if (iterations == MAX_ITERATIONS)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				image[y][x] = firstColour; // black
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				image[y][x] = secondColour; // white
			}
		}
	}

	return;
}

int main(int argc, char* argv[])
{
	
	cout << "Please enter your first colour choice." << endl;
	cout << "Enter 1 for red, 2 for yellow, 3 for orange." << endl;

	std::cin >> firstColour;

	switch (firstColour)
	{
	case 1: firstColour = 0xFF0000; // prints "1"
		break;       // and exits the switch
	case 2: firstColour = 0xFFFF00;
		break;
	case 3: firstColour = 0xFFA500;
		break;
	}

	cout << "Please enter your second colour choice." << endl;
	cout << "Enter 1 for blue, 2 for green and 3 for purple ." << endl;

	std::cin >> secondColour;

	switch (secondColour)
	{
	case 1: secondColour = 0x0000FF; //blue
		break;
	case 2: secondColour = 0x00FF00; //green
		break;
	case 3: secondColour = 0x6a0dad; //purple
		break;
	}

	cout << "How many threads would you like to run concurrently? Your machine can run up to a maximum of: " << std::thread::hardware_concurrency() << " threads." << endl;
	std::cin >> NUM_THREADS;

	while (NUM_THREADS > std::thread::hardware_concurrency())
	{
		cout << "Please enter a valid number of threads." << endl;
		cout << "How many threads would you like to run concurrently? Your machine can run up to a maximum of: " << std::thread::hardware_concurrency() << " threads." << endl;

		std::cin >> NUM_THREADS;
	}

	cout << "Please wait..." << endl;



	// Start timing
	the_clock::time_point start = the_clock::now();

	// This shows the whole set.
	//for (int y = 0; y < HEIGHT; y += HEIGHT / NUM_THREADS)
	//{
		//compute_mandelbrot(-2.0, 1.0, 1.125, -1.125, y, y + HEIGHT / thread);

		// This zooms in on an interesting bit of detail.
	//	compute_mandelbrot(-0.751085, -0.734975, 0.118378, 0.134488, y, y + HEIGHT / NUM_THREADS);

//	}

	for (i = 0; i < NUM_THREADS; i++)
	{
		cout << "Creating thread: " << i << endl;
		threadID = i;

		compute_mandelbrot(-2.0, 1.0, 1.125, -1.125, threadID);
		//th[i].compute_mandelbrot
	}

	thread writeToFile(fileWriteThread, "output.tga");

	writeToFile.join();
	// Stop timing
	the_clock::time_point end = the_clock::now();

	// Compute the difference between the two times in milliseconds
	auto time_taken = duration_cast<milliseconds>(end - start).count();
	cout << "Computing the Mandelbrot set took " << time_taken << " ms." << endl;

	return 0;
}
