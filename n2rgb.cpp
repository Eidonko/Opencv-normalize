#include <string.h>
#include <iostream>
#include <fstream>
#include <map>
#include <utility>              // std::pair
#include <iomanip>              // std::setprecision
#include <limits>               // std::numeric_limits
#include <vector>
#include <algorithm>    // std::max
#include <queue>

// OpenCV includes
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace cv;
using namespace std;


#define GSLOG_USR_ERROR(s) fprintf(stderr, "%s\n", s)

void fromRGB2Mat(Mat& img, unsigned char **r, unsigned char **g, unsigned char **b);
void fromMat2RGB(Mat& img, unsigned char **r, unsigned char **g, unsigned char **b);

void badswitch(int c) {
	fprintf(stderr, "Bad switch: %c\n", c);
	exit(1);
}

bool MiniMax(unsigned char const *r, int rocols, int *rMin, int *rMax)
{
  int min=256;
  int max= -1;
  while (rocols > 0)
  {
	  rocols--;
	  if ( r[rocols] < min ) min = r[rocols];
	  if ( r[rocols] > max ) max = r[rocols];
  }
  *rMin = min, *rMax = max;
  return true;
}

int main(int argc, char *argv[])
{
	bool theSuccess;
	int verbose = 0;
	bool computeStatistics = false;
	unsigned int options;
	bool jointNormalization = false;
	int progressCounter;
	bool multiplex = false;
	int rMin, rMax, gMin, gMax, bMin, bMax;
	void (*badSwitch)(int) = badswitch;
	

	Mat src, dst;

	options = 0;

	string oFileName; // output grid
	string iFileName; // input grid
	string tFileName; // clock ticks


	// manage command-line args
	if (argc > 1)
		for (int i = 1; i < argc; i++)
			if (argv[i][0] == '-')
				switch (argv[i][1]) {
				case 'v':	verbose = 1; break;
				case 'o':	if (i >= argc - 1) {
								(*badSwitch)(argv[i][1]);
								return false;
						}
						oFileName = argv[++i]; break;
				case 'i':	if (i >= argc - 1) {
								(*badSwitch)(argv[i][1]);
								return false;
						}
						iFileName = argv[++i]; break;
				case 'm':	computeStatistics = true; break;
				case 'j':	jointNormalization = true; break;
				case '3':	multiplex = true; break;
				default:	(*badSwitch)('*');
						return false;
				}


	
	
	if (iFileName.empty()) {
		GSLOG_USR_ERROR("normalize2Rgb: option -i input-image must be specified.");
		return false;
	}
	if (oFileName.empty()) {
		GSLOG_USR_ERROR("normalize2Rgb: option -o output-image must be specified.");
		return false;
	}

	std::cerr << "Opening input file " << iFileName << '\n';
	src = imread(iFileName, cv::IMREAD_COLOR);
	namedWindow("Input image", CV_WINDOW_AUTOSIZE );
	imshow("Input image", src );

	src.copyTo(dst);

	std::cout << "Image " << iFileName << " consists of " << src.channels() << " channels and "
                << src.cols << "x" << src.rows << " pixels.\n";


	const int rows = src.rows;
	const int cols = src.cols;

        unsigned char **r, **g, **b;
        unsigned char *buffr, *buffg, *buffb;

	int rocol = rows*cols;

        r = new unsigned char *[rows];
        g = new unsigned char *[rows];
        b = new unsigned char *[rows];

        buffr = new unsigned char[ rocol ];
        buffg = new unsigned char[ rocol ];
        buffb = new unsigned char[ rocol ];

        for (int i=0, disp=0; i<rows; i++, disp += cols)
                r[i] = & buffr[disp];
        for (int i=0, disp=0; i<rows; i++, disp += cols)
                g[i] = & buffg[disp];
        for (int i=0, disp=0; i<rows; i++, disp += cols)
                b[i] = & buffb[disp];

        fromMat2RGB(dst, r, g, b);

	MiniMax(buffr, rocol, &rMin, &rMax);
	MiniMax(buffg, rocol, &gMin, &gMax);
	MiniMax(buffb, rocol, &bMin, &bMax);
	printf("r: [%d, %d]\n", rMin, rMax);
	printf("g: [%d, %d]\n", gMin, gMax);
	printf("b: [%d, %d]\n", bMin, bMax);

	// Length of the R, G, and B domains
	int rDomain = rMax - rMin;
	int gDomain = gMax - gMin;
	int bDomain = bMax - bMin;
	int Domain = std::max(rDomain, std::max(gDomain, bDomain));

	double t; // NOTE: THIS MUST BE A float OR A double !
	unsigned char *pri, *pgi, *pbi;
	pri = buffr, pgi = buffg, pbi = buffb;
        for (int i=0; i<rocol; i++, pri++, pgi++, pbi++)
	{
		t = *pri -rMin;
		*pri = int(std::round((t / Domain) * 255.0)); 
		t = *pgi -gMin;
		*pgi = int(std::round((t / Domain) * 255.0)); 
		t = *pbi -bMin;
		*pbi = int(std::round((t / Domain) * 255.0)); 
	}

	fromRGB2Mat(dst, r, g, b);
	namedWindow( "Normalized image", CV_WINDOW_AUTOSIZE );
	imshow("Normalized image", dst );

	imwrite(oFileName, dst );

        delete r, delete g, delete b;
        delete buffr, delete buffg, delete buffb;

	waitKey(0);

	return 0;

}
void fromRGB2Mat(Mat& img, unsigned char **r, unsigned char **g, unsigned char **b) {
	for(int y = 0; y < img.rows; y++){
		for(int x = 0; x < img.cols; x++){
			Vec3b& intensity = img.at<Vec3b>(y, x);
			uchar& blu = intensity.val[0];
			uchar& gre = intensity.val[1];
			uchar& red = intensity.val[2];

			blu = b[y][x];
			gre = g[y][x];
			red = r[y][x];
		}
	}
}

void fromMat2RGB(Mat& img, unsigned char **r, unsigned char **g, unsigned char **b) {
	for(int y = 0; y < img.rows; y++){
		for(int x = 0; x < img.cols; x++){
			Vec3b& intensity = img.at<Vec3b>(y, x);
			uchar& blu = intensity.val[0];
			uchar& gre = intensity.val[1];
			uchar& red = intensity.val[2];

			b[y][x] = blu;
			g[y][x] = gre;
			r[y][x] = red;
		}
	}
}

// EoM.N O R M A L I Z E 2 R G B
