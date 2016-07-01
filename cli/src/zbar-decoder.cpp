#include "zbar-decoder.h"
#include <iostream>
#include "ZBarWin64\include\zbar.h"
#include <opencv2\opencv.hpp>

using namespace std;
//using namespace zbar;
using namespace cv;

bool DecodeByZBar(Mat& img, string& code)
{
	if (img.data == NULL)
		return false;

	// create a reader
	zbar::ImageScanner scanner;

	// configure the reader
	scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

	const void *raw = img.data;
	zbar::Image image(img.cols, img.rows, "Y800", raw, img.cols * img.rows);

	// scan the image for barcodes
	int n = scanner.scan(image);
	bool success = false;
	// extract results
	for (zbar::Image::SymbolIterator symbol = image.symbol_begin();
		symbol != image.symbol_end();
		++symbol) {
		// do something useful with results
		cout << "decoded " << symbol->get_type_name()
			<< " symbol \"" << symbol->get_data() << '"' << endl;
		code = string(symbol->get_data());
		success = true;
	}

	// clean up
	image.set_data(NULL, 0);
	return success;
}
