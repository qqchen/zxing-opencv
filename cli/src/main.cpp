// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Copyright 2010-2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <fstream>
#include <string>
#include "ImageReaderSource.h"
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
#include "zxing\common\GreyscaleLuminanceSource.h"
#include "zxing\common\Array.h"
#include "OpenCVBitmapSource.h"

using namespace std;
using namespace zxing;
using namespace zxing::multi;
using namespace zxing::qrcode;
using namespace cv;

namespace {

	bool more = false;
	bool test_mode = false;
	bool try_harder = false;
	bool search_multi = false;
	bool use_hybrid = false;
	bool use_global = false;
	bool verbose = false;

}

vector<Ref<Result> > decode(Ref<BinaryBitmap> image, DecodeHints hints) {
	Ref<Reader> reader(new MultiFormatReader);
	return vector<Ref<Result> >(1, reader->decode(image, hints));
}

vector<Ref<Result> > decode_multi(Ref<BinaryBitmap> image, DecodeHints hints) {
	MultiFormatReader delegate;
	GenericMultipleBarcodeReader reader(delegate);
	return reader.decodeMultiple(image, hints);
}

int read_image(Ref<LuminanceSource> source, bool hybrid, string expected) {
	vector<Ref<Result> > results;
	string cell_result;
	int res = -1;

	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		}
		else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		if (search_multi) {
			results = decode_multi(binary, hints);
		}
		else {
			results = decode(binary, hints);
		}
		res = 0;
	}
	catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + string(e.what());
		res = -2;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + string(e.what());
		res = -3;
	}
	catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + string(e.what());
		res = -4;
	}
	catch (const std::exception& e) {
		cell_result = "std::exception: " + string(e.what());
		res = -5;
	}

	if (test_mode && results.size() == 1) {
		std::string result = results[0]->getText()->getText();
		if (expected.empty()) {
			cout << "  Expected text or binary data for image missing." << endl
				<< "  Detected: " << result << endl;
			res = -6;
		}
		else {
			if (expected.compare(result) != 0) {
				cout << "  Expected: " << expected << endl
					<< "  Detected: " << result << endl;
				cell_result = "data did not match";
				res = -6;
			}
		}
	}

	if (res != 0 && (verbose || (use_global ^ use_hybrid))) {
		cout << (hybrid ? "Hybrid" : "Global")
			<< " binarizer failed: " << cell_result << endl;
	}
	else if (!test_mode) {
		if (verbose) {
			cout << (hybrid ? "Hybrid" : "Global")
				<< " binarizer succeeded: " << endl;
		}
		for (size_t i = 0; i < results.size(); i++) {
			if (more) {
				cout << "  Format: "
					<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
					<< endl;
				for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
					cout << "  Point[" << j << "]: "
						<< results[i]->getResultPoints()[j]->getX() << " "
						<< results[i]->getResultPoints()[j]->getY() << endl;
				}
			}
			if (verbose) {
				cout << "    ";
			}
			cout << results[i]->getText()->getText() << endl;
		}
	}

	return res;
}

int my_read_image(Ref<LuminanceSource> source, bool hybrid, vector<string>& codes) {
	vector<Ref<Result> > results;
	string cell_result;
	int res = -1;

	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		}
		else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		try_harder = true;
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		search_multi = false;
		if (search_multi) {
			results = decode_multi(binary, hints);
		}
		else {
			results = decode(binary, hints);
		}
		res = 0;
	}
	catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + string(e.what());
		res = -2;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + string(e.what());
		res = -3;
	}
	catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + string(e.what());
		res = -4;
	}
	catch (const std::exception& e) {
		cell_result = "std::exception: " + string(e.what());
		res = -5;
	}

	//if (test_mode && results.size() == 1) {
	//	std::string result = results[0]->getText()->getText();
	//	if (expected.empty()) {
	//		cout << "  Expected text or binary data for image missing." << endl
	//			<< "  Detected: " << result << endl;
	//		res = -6;
	//	}
	//	else {
	//		if (expected.compare(result) != 0) {
	//			cout << "  Expected: " << expected << endl
	//				<< "  Detected: " << result << endl;
	//			cell_result = "data did not match";
	//			res = -6;
	//		}
	//	}
	//}

	if (res != 0 && (verbose || (use_global ^ use_hybrid))) {
		cout << (hybrid ? "Hybrid" : "Global")
			<< " binarizer failed: " << cell_result << endl;
	}
	else if (!test_mode) {
		if (verbose) {
			cout << (hybrid ? "Hybrid" : "Global")
				<< " binarizer succeeded: " << endl;
		}
		for (size_t i = 0; i < results.size(); i++) {
			if (more) {
				cout << "  Format: "
					<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
					<< endl;
				for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
					cout << "  Point[" << j << "]: "
						<< results[i]->getResultPoints()[j]->getX() << " "
						<< results[i]->getResultPoints()[j]->getY() << endl;
				}
			}
			if (verbose) {
				cout << "    ";
			}
			cout << results[i]->getText()->getText() << endl;
		}
	}

	return res;
}

string read_expected(string imagefilename) {
	string textfilename = imagefilename;
	string::size_type dotpos = textfilename.rfind(".");

	textfilename.replace(dotpos + 1, textfilename.length() - dotpos - 1, "txt");
	ifstream textfile(textfilename.c_str(), ios::binary);
	textfilename.replace(dotpos + 1, textfilename.length() - dotpos - 1, "bin");
	ifstream binfile(textfilename.c_str(), ios::binary);
	ifstream *file = 0;
	if (textfile.is_open()) {
		file = &textfile;
	}
	else if (binfile.is_open()) {
		file = &binfile;
	}
	else {
		return std::string();
	}
	file->seekg(0, ios_base::end);
	size_t size = size_t(file->tellg());
	file->seekg(0, ios_base::beg);

	if (size == 0) {
		return std::string();
	}

	char* data = new char[size + 1];
	file->read(data, size);
	data[size] = '\0';
	string expected(data);
	delete[] data;

	return expected;
}

int read_image(Ref<LuminanceSource> source, bool hybrid,
	std::vector<std::string>& codes);

int recognizeBarcode(const std::string& filename,
	cv::Rect rect, std::vector<std::string>& codes);

void decode_image(Reader *reader, cv::Mat &image);

void CheckCode(string pathfile, Rect roi);

int main()
{
	const string filename = "test.png";
	Rect roi(1090, 370, 454, 210);
	//vector<string> codes;
	//recognizeBarcode(filename, roi, codes);
	//for (size_t i = 0; i < codes.size(); i++)
	//{
	//	cout << codes[i] << endl;
	//}

	CheckCode("./filepath.txt", roi);

	//Mat image = imread(filename, 0);
	//Ref<Reader> reader(new MultiFormatReader);
	//decode_image(reader, image);

	return 1;
}

//int main(int argc, char** argv) {
//	if (argc <= 1) {
//		cout << "Usage: " << argv[0] << " [OPTION]... <IMAGE>..." << endl
//			<< "Read barcodes from each IMAGE file." << endl
//			<< endl
//			<< "Options:" << endl
//			<< "  (-h|--hybrid)             use the hybrid binarizer (default)" << endl
//			<< "  (-g|--global)             use the global binarizer" << endl
//			<< "  (-v|--verbose)            chattier results printing" << endl
//			<< "  --more                    display more information about the barcode" << endl
//			<< "  --test-mode               compare IMAGEs against text files" << endl
//			<< "  --try-harder              spend more time to try to find a barcode" << endl
//			<< "  --search-multi            search for more than one bar code" << endl
//			<< endl
//			<< "Example usage:" << endl
//			<< "  zxing --test-mode *.jpg" << endl
//			<< endl;
//		return 1;
//	}
//
//	int total = 0;
//	int gonly = 0;
//	int honly = 0;
//	int both = 0;
//	int neither = 0;
//
//	for (int i = 1; i < argc; i++) {
//		string filename = argv[i];
//		if (filename.compare("--verbose") == 0 ||
//			filename.compare("-v") == 0) {
//			verbose = true;
//			continue;
//		}
//		if (filename.compare("--hybrid") == 0 ||
//			filename.compare("-h") == 0) {
//			use_hybrid = true;
//			continue;
//		}
//		if (filename.compare("--global") == 0 ||
//			filename.compare("-g") == 0) {
//			use_global = true;
//			continue;
//		}
//		if (filename.compare("--more") == 0) {
//			more = true;
//			continue;
//		}
//		if (filename.compare("--test-mode") == 0) {
//			test_mode = true;
//			continue;
//		}
//		if (filename.compare("--try-harder") == 0) {
//			try_harder = true;
//			continue;
//		}
//		if (filename.compare("--search-multi") == 0){
//			search_multi = true;
//			continue;
//		}
//
//		if (filename.length() > 3 &&
//			(filename.substr(filename.length() - 3, 3).compare("txt") == 0 ||
//			filename.substr(filename.length() - 3, 3).compare("bin") == 0)) {
//			continue;
//		}
//
//		if (!use_global && !use_hybrid) {
//			use_global = use_hybrid = true;
//		}
//
//		if (test_mode) {
//			cerr << "Testing: " << filename << endl;
//		}
//
//		Ref<LuminanceSource> source;
//		try {
//			//source = ImageReaderSource::create(filename);
//			Mat image = imread(filename);
//			source = Ref<OpenCVBitmapSource>(new OpenCVBitmapSource(image));
//
//		}
//		catch (const zxing::IllegalArgumentException &e) {
//			cerr << e.what() << " (ignoring)" << endl;
//			continue;
//		}
//
//		string expected = read_expected(filename);
//
//		int gresult = 1;
//		int hresult = 1;
//		if (use_hybrid) {
//			hresult = read_image(source, true, expected);
//		}
//		if (use_global && (verbose || hresult != 0)) {
//			gresult = read_image(source, false, expected);
//			if (!verbose && gresult != 0) {
//				cout << "decoding failed" << endl;
//			}
//		}
//		gresult = gresult == 0;
//		hresult = hresult == 0;
//		gonly += gresult && !hresult;
//		honly += hresult && !gresult;
//		both += gresult && hresult;
//		neither += !gresult && !hresult;
//		total = total + 1;
//	}
//
//	if (test_mode) {
//		cout << endl
//			<< "Summary:" << endl
//			<< " " << total << " images tested total," << endl
//			<< " " << (honly + both) << " passed hybrid, " << (gonly + both)
//			<< " passed global, " << both << " pass both, " << endl
//			<< " " << honly << " passed only hybrid, " << gonly
//			<< " passed only global, " << neither << " pass neither." << endl;
//	}
//
//	return 0;
//}


int read_image(Ref<LuminanceSource> source, bool hybrid,
	std::vector<std::string>& codes)
{
	std::vector<Ref<Result> > results;
	std::string cell_result;
	int res = -1;

	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		}
		else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		//DecodeHints hints(DecodeHints::ONED_HINT);
		//DecodeHints hints;
		// try_harder: spend more time to try to find a barcode
		bool try_harder = true;
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		//	results = decode_multi(binary, hints); // multi-barcode one pic
		results = decode(binary, hints);
		res = 0;
	}
	catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + std::string(e.what());
		res = -2;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + std::string(e.what());
		res = -3;
	}
	catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + std::string(e.what());
		res = -4;
	}
	catch (const std::exception& e) {
		cell_result = "std::exception: " + std::string(e.what());
		res = -5;
	}

	if (res != 0) {
		cout << (hybrid ? "Hybrid" : "Global")
			<< " binarizer failed: " << cell_result << std::endl;
	}
	else  {
		cout << (hybrid ? "Hybrid" : "Global")
			<< " binarizer succeeded: " << std::endl;
		for (size_t i = 0; i < results.size(); i++) {
			cout << "  Format: "
				<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
				<< std::endl;
			for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
				cout << "  Point[" << j << "]: "
					<< results[i]->getResultPoints()[j]->getX() << " "
					<< results[i]->getResultPoints()[j]->getY() << std::endl;
			}
			cout << "    ";
			cout << results[i]->getText()->getText() << std::endl;
			codes.push_back(results[i]->getText()->getText());
		}
	}

	return res;
}

int recognizeBarcode(const std::string& filename,
	cv::Rect rect, std::vector<std::string>& codes)
{
	cv::Mat tmpl = cv::imread(filename, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	// bar region
	cv::Mat newImage = tmpl(rect);
	//cv::Mat newImage = tmpl;

	// bar image name
	size_t extPos = filename.find_last_of(".");
	if (extPos == std::string::npos) {
		cout << "[RecognizeBarCode] Invalid filename" << std::endl;
		return 0;
	}
	std::string extension = filename.substr(extPos);
	std::string barImageName = filename.substr(0, extPos) + "_barImage" + extension;
	//std::string barImageName = filename.substr(0, extPos) + "_barImage.png";
	cv::imwrite(barImageName, newImage);


	//Ref<LuminanceSource> source(
	//	new GreyscaleLuminanceSource(
	//	ArrayRef<char>((char *)newImage.data, newImage.step * newImage.rows),
	//	newImage.step, newImage.rows, 0, 0, newImage.cols, newImage.rows));
	Ref<OpenCVBitmapSource> source(new OpenCVBitmapSource(newImage));

	//// recognize
	//Ref<LuminanceSource> source;
	//try {
	//	source = ImageReaderSource::create(barImageName);
	//}
	//catch (const zxing::IllegalArgumentException &e) {
	//	std::cerr << e.what() << " (ignoring)" << std::endl;
	//	return FAIL;
	//}

	int hresult = 1;
	bool use_hybrid = true; // default
	hresult = read_image(source, use_hybrid, codes);
	if (hresult != 0) { // try using global mode if hybrid failed
		hresult = read_image(source, false, codes);
		if (hresult != 0) {
			cout << "decoding failed" << std::endl;
		}
	}

	return hresult;
}

void decode_image(Reader *reader, cv::Mat &image)
{
	try
	{
		Ref<OpenCVBitmapSource> source(new OpenCVBitmapSource(image));
		Ref<Binarizer> binarizer(new GlobalHistogramBinarizer(source));
		Ref<BinaryBitmap> bitmap(new BinaryBitmap(binarizer));
		Ref<Result> result(reader->decode(bitmap, DecodeHints(DecodeHints::TRYHARDER_HINT)));//+DecodeHints::DEFAULT_HINT)));
		cout << result->getText()->getText() << endl;
		//Export the read barcode here
	}
	catch (zxing::Exception& e)
	{
		//Export your failure to read the code here
		cerr << "Error: " << e.what() << endl;
	}
}

string GetFileNameInDir(const char* fileDir)
{
	string fileName;
	string dir(fileDir);
	int len = strlen(fileDir);
	for (int i = len - 1; i >= 0; i--)
	{
		if (fileDir[i] == '\\')
		{
			fileName = dir.substr(i + 1, len - i - 1);
			break;
		}
	}
	return fileName;
}


void CheckCode(string pathfile, Rect roi)
{
	ifstream ifs;
	ifs.open(pathfile.c_str());
	ofstream ofs;
	ofs.open("data.txt");

	string line;
	while (true)
	{
		char buf[1024];
		ifs.getline(buf, 1024);
		string line(buf);
		if (line.empty())
			break;

		string file = GetFileNameInDir(line.c_str());
		cout << " image : " << file << endl;
		ofs << file << endl;

		// 识别客观题区
		vector<string> codes;
		recognizeBarcode(line, roi, codes);
		for (size_t i = 0; i < codes.size(); i++)
		{
			cout << "reusult : " << codes[i] << endl;
			ofs << codes[i] << endl;
		}

	}
	ifs.close();
	ofs.close();
}