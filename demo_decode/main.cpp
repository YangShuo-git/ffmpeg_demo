#include <iostream>
#include "demo_decode.h"
using std::cout;

int main(int argc, const char * argv[]) {
    std::string curFile(__FILE__);
    cout << curFile << "\n";

    #if 0
    unsigned long pos = curFile.find("demo-mac");
    if (pos == string::npos) {
        return -1;
    }

    string resourceDir = curFile.substr(0, pos)+"filesources/";
    
    int test_use = 3;
    if (test_use == 7) {
        string srcPath = resourceDir + "test_1280x720_3.mp4";
        SoftDecoder decoder;
        decoder.decodeFormat(srcPath);
    } else if (test_use == 8) {
        string srcPath = resourceDir + "test_441_f32le_2.aac";
        SoftDecoder decoder;
        decoder.decodeStream(srcPath);
    } else if (test_use == 9) {
        string srcPath = resourceDir + "test_640x360_yuv420p.yuv";
        string dstPath = resourceDir+ "1-abc-test.h264";
        SoftDecoder decoder;
        decoder.encodeAsStream(srcPath,dstPath);
    }
    // } else if (test_use == 10) {
    //     string srcPath = resourceDir + "test_1280x720_3.mp4";
    //     HardEnDecoder decoder;
    //     decoder.decodeFormat(srcPath);
    // } else if (test_use == 11) {
    //     string srcPath = resourceDir + "test_640x360_yuv420p.yuv";
    //     string dstPath = resourceDir + "3-test.h264";
    //     HardEnDecoder decoder;
    //     decoder.doEncode(srcPath,dstPath);

    #endif
    return 0;
}
