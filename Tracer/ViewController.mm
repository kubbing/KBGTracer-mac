//
//  ViewController.m
//  Tracer
//
//  Created by Jakub Hladík on 02.01.15.
//  Copyright (c) 2015 jakubhladik.pro. All rights reserved.
//

#import "ViewController.h"

#include "Renderer.h"

#import <Quartz/Quartz.h>

@interface ViewController ()

@property (strong, nonatomic) NSTimer *timer;
@property (strong, nonatomic) NSValue *rendererValue;

@end


@implementation ViewController

- (void)dealloc
{
    Renderer *renderer = (Renderer *)self.rendererValue.pointerValue;
    if (renderer) {
        delete renderer;
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self initRenreder];
    
    self.timer = [NSTimer scheduledTimerWithTimeInterval:(1/1.0) target:self selector:@selector(timerFired:) userInfo:nil repeats:YES];
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

#pragma mark - Timer

- (void)timerFired:(NSTimer *)timer
{
    Renderer *renderer = (Renderer *)self.rendererValue.pointerValue;
    unsigned width, height;
    float progress;
    unsigned char*data = renderer->getImageData(&width, &height, &progress);
    
    [self updateImageViewWithImageData:data width:width height:height progress:progress];
}

#pragma mark - Actions

- (void)initRenreder
{
    Renderer *r = new Renderer();
    self.rendererValue = [NSValue valueWithPointer:r];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^{
        r->render();
    });
}

- (void)updateImageViewWithImageData:(unsigned char*)data width:(CGFloat)width height:(CGFloat)height progress:(CGFloat)progress
{
    size_t bufferLength = width * height * 4;
    CGDataProviderRef providerRef = CGDataProviderCreateWithData(NULL, data, bufferLength, NULL);
    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = 32;
    size_t bytesPerRow = 4 * width;

    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaLast;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;

    CGImageRef imageRef = CGImageCreate(width,
                                        height,
                                        bitsPerComponent,
                                        bitsPerPixel,
                                        bytesPerRow,
                                        colorSpaceRef,
                                        bitmapInfo,
                                        providerRef,   // data provider
                                        NULL,       // decode
                                        YES,        // should interpolate
                                        renderingIntent);
    
    NSImage *image = [[NSImage alloc] initWithCGImage:imageRef
                                                 size:NSMakeSize(width / 2.0, height / 2.0)];
    
    CFRelease(providerRef);
    CFRelease(colorSpaceRef);
    
    self.imageView.image = image;
    
    if (progress == 1.0) {
        NSBitmapImageRep *newRep = [[NSBitmapImageRep alloc] initWithCGImage:imageRef];
        [newRep setSize:[image size]];
        NSData *pngData = [newRep representationUsingType:NSPNGFileType properties:nil];
        
        NSString *documentsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        
        NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
        formatter.dateFormat = @"yyyy-MM-dd\'T\'HH-mm-ss";
        NSString *timeString = [formatter stringFromDate:[NSDate date]];
        
        NSString *filePath = [NSMutableString stringWithFormat:@"%@/tracer %@.png", documentsPath, timeString];
        NSError *error;
        [pngData writeToURL:[NSURL fileURLWithPath:filePath] options:NSDataWritingAtomic error:&error];
        if (error) {
            NSLog(@"%@", error);
        }
        else {
            NSLog(@"Saved to %@", filePath);
        }
        
        [self.timer invalidate];
        self.timer = nil;
    }
    
    CFRelease(imageRef);
}

@end
