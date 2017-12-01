//
//  ViewController.m
//  PitchDetection
//
//  Created by apple on 2017/2/17.
//  Copyright © 2017年 xiaokai.zhan. All rights reserved.
//

#import "ViewController.h"
#import "CommonUtil.h"
#import "pitch_detection.hpp"
#include <vector>

using namespace std;

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}
- (IBAction)detectPitch:(id)sender {
    NSLog(@"detectPitch...");
    double startEncodeTimeMills = CFAbsoluteTimeGetCurrent() * 1000;
    NSString* pcmFilePath = [CommonUtil bundlePath:@"111.pcm"];
    NSString* resultFilePath = [CommonUtil documentsPath:@"result.pcm"];
    const char* filePath = [pcmFilePath cStringUsingEncoding:NSUTF8StringEncoding];
    const char* outputFilePath = [resultFilePath cStringUsingEncoding:NSUTF8StringEncoding];
    int win_s = 4096;
    int hop_s = 1024;
    CPitchDetection* PitchDetectionObject = new CPitchDetection(win_s, hop_s, 44100);
    int len = 2048;
    FILE* inputPCMFile = fopen(filePath, "rb");
    short s_buf[len];
    size_t actualLen = -1;
    while((actualLen = fread(s_buf, sizeof(short), len, inputPCMFile)) > 0) {
        float f_buf[len];
        for (int i=0; i<len; i++) {
            f_buf[i] = float(s_buf[i]);
        }
        PitchDetectionObject->Process(f_buf, len);
    }
    fclose(inputPCMFile);
    // Output
    FILE* outputFile = fopen(outputFilePath, "wb+");
    int write_s = hop_s;
    short tmpBuf_out[write_s];
    for(int i=0; i<write_s; i++) {
        tmpBuf_out[i] = 0;
    }
    vector<PitchElement> pitchTrack = PitchDetectionObject->GetPitchData();
    for(vector<PitchElement>::iterator it = pitchTrack.begin();
        it != pitchTrack.end();
        it++
        )
    {
        float fPitch = it->freq;
        float fConf = it->conf;
        if(fPitch > 0 && fConf > 0.9){
            for (int i=0; i<write_s; i++)
            {
                tmpBuf_out[i] = short(fPitch);
            }
        }
        fwrite(tmpBuf_out, sizeof(short), write_s, outputFile);
        for(int i=0; i<write_s; i++) {
            tmpBuf_out[i] = 0;
        }
        
    }
    fclose(outputFile);
    double wasteTimeMills = CFAbsoluteTimeGetCurrent() * 1000 - startEncodeTimeMills;
    NSLog(@"wasteTimeMills is %lf", wasteTimeMills);
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
