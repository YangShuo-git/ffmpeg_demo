#include <iostream>
#include "demo_decode.h"
using std::cout;

int main(int argc, const char * argv[]) {
    std::string curFile(__FILE__);
    cout << curFile << "\n";

    #if 1
    unsigned long pos = curFile.find("demo_decode");
    if (pos == std::string::npos) {
        return -1;
    }

    std::string resourceDir = curFile.substr(0, pos)+"assets/";
    cout << resourceDir << "\n";
    
    int test_use = 3;
    SoftDecoder decoder;

    if (test_use == 3) {
        std::string srcPath = resourceDir + "2_audio.mp4";
        decoder.decodeFormat(srcPath);
    } else if (test_use == 8) {
        std::string srcPath = resourceDir + "test_441_f32le_2.aac";
        // SoftDecoder decoder;
        decoder.decodeStream(srcPath);
    } else if (test_use == 9) {
        std::string srcPath = resourceDir + "test_640x360_yuv420p.yuv";
        std::string dstPath = resourceDir+ "1-abc-test.h264";
        // SoftDecoder decoder;
        decoder.encodeAsStream(srcPath,dstPath);
    }
    // } else if (test_use == 10) {
    //     std::string srcPath = resourceDir + "test_1280x720_3.mp4";
    //     HardEnDecoder decoder;
    //     decoder.decodeFormat(srcPath);
    // } else if (test_use == 11) {
    //     std::string srcPath = resourceDir + "test_640x360_yuv420p.yuv";
    //     std::string dstPath = resourceDir + "3-test.h264";
    //     HardEnDecoder decoder;
    //     decoder.doEncode(srcPath,dstPath);

    #endif
    return 0;
}
