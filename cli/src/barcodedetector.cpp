#include "barcodedetector.h"
//#include "ImageReaderSource.h"
//#include <zxing/common/Counted.h>
//#include <zxing/Binarizer.h>
//#include <zxing/MultiFormatReader.h>
//#include <zxing/Result.h>
//#include <zxing/ReaderException.h>
//#include <zxing/common/GlobalHistogramBinarizer.h>
//#include <zxing/common/HybridBinarizer.h>
//#include <exception>
//#include <zxing/Exception.h>
//#include <zxing/common/IllegalArgumentException.h>
//#include <zxing/BinaryBitmap.h>
//#include <zxing/DecodeHints.h>
//
//#include <zxing/qrcode/QRCodeReader.h>
//#include <zxing/multi/qrcode/QRCodeMultiReader.h>
//#include <zxing/multi/ByQuadrantReader.h>
//#include <zxing/multi/MultipleBarcodeReader.h>
//#include <zxing/multi/GenericMultipleBarcodeReader.h>
//#include <opencv2\highgui\highgui.hpp>
//#include <opencv2\core\core.hpp>
//#include "zxing\common\GreyscaleLuminanceSource.h"
//#include "zxing\common\Array.h"
//#include "OpenCVBitmapSource.h"
//
//using namespace std;
//using namespace zxing;
//using namespace zxing::multi;
//using namespace zxing::qrcode;
//using namespace cv;

//BarcodeDetector::BarcodeDetector()
//{
//}
//
//BarcodeDetector::~BarcodeDetector()
//{
//}

//ZXingBarcodeDetector::ZXingBarcodeDetector()
//{
//
//}
//
//ZXingBarcodeDetector::~ZXingBarcodeDetector()
//{
//
//}
//
//bool ZXingBarcodeDetector::Detect(cv::Mat image, std::string& result)
//{
//	if (image.data == NULL)
//		return false;
//
//	Ref<OpenCVBitmapSource> source(new OpenCVBitmapSource(image));
//
//	int hresult = 1;
//	bool use_hybrid = true; // default
//	std::vector<string> codes;
//	hresult = read_image(source, use_hybrid, codes);
//	if (hresult != 0) { // try using global mode if hybrid failed
//		hresult = read_image(source, false, codes);
//		if (hresult != 0) {
//			cout << "decoding failed" << std::endl;
//		}
//	}
//	if (hresult == 0) // decode success
//		result = codes[0];
//	return hresult == 0;
//}
//
//int ZXingBarcodeDetector::read_image(Ref<LuminanceSource> source, bool hybrid,
//	std::vector<std::string>& codes)
//{
//	std::vector<Ref<Result> > results;
//	std::string cell_result;
//	int res = -1;
//
//	try {
//		Ref<Binarizer> binarizer;
//		if (hybrid) {
//			binarizer = new HybridBinarizer(source);
//		}
//		else {
//			binarizer = new GlobalHistogramBinarizer(source);
//		}
//		DecodeHints hints(DecodeHints::DEFAULT_HINT);
//		//DecodeHints hints(DecodeHints::ONED_HINT);
//		//DecodeHints hints;
//		// try_harder: spend more time to try to find a barcode
//		bool try_harder = true;
//		hints.setTryHarder(try_harder);
//		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
//		//	results = decode_multi(binary, hints); // multi-barcode one pic
//		results = decode(binary, hints);
//		res = 0;
//	}
//	catch (const ReaderException& e) {
//		cell_result = "zxing::ReaderException: " + std::string(e.what());
//		res = -2;
//	}
//	catch (const zxing::IllegalArgumentException& e) {
//		cell_result = "zxing::IllegalArgumentException: " + std::string(e.what());
//		res = -3;
//	}
//	catch (const zxing::Exception& e) {
//		cell_result = "zxing::Exception: " + std::string(e.what());
//		res = -4;
//	}
//	catch (const std::exception& e) {
//		cell_result = "std::exception: " + std::string(e.what());
//		res = -5;
//	}
//
//	if (res != 0) {
//		cout << (hybrid ? "Hybrid" : "Global")
//			<< " binarizer failed: " << cell_result << std::endl;
//	}
//	else  {
//		cout << (hybrid ? "Hybrid" : "Global")
//			<< " binarizer succeeded: " << std::endl;
//		for (size_t i = 0; i < results.size(); i++) {
//			cout << "  Format: "
//				<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
//				<< std::endl;
//			for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
//				cout << "  Point[" << j << "]: "
//					<< results[i]->getResultPoints()[j]->getX() << " "
//					<< results[i]->getResultPoints()[j]->getY() << std::endl;
//			}
//			cout << "    ";
//			cout << results[i]->getText()->getText() << std::endl;
//			codes.push_back(results[i]->getText()->getText());
//		}
//	}
//
//	return res;
//}