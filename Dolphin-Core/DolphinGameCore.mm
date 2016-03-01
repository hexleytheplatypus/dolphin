/*
 Copyright (c) 2013, OpenEmu Team

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//  Had to rename /Common/Assert.h to /Common/AssertInt.h
//  Changed <al*> includes to <OpenAL/al*>
//  Updated to Dolphin Git Source 28 Feb 2016

#import "DolphinGameCore.h"
#include "Dolphin-Core/DolHost.h"
#import <OpenEmuBase/OERingBuffer.h>

#import <AppKit/AppKit.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <string>
#include <unistd.h>

#include "Common/GL/GLInterface/AGL.h"

#define SAMPLERATE 448000
#define SIZESOUNDBUFFER 448000 / 60 * 4
#define OpenEmu 1

@interface DolphinGameCore () <OEGCSystemResponderClient>
{
    DolHost *gc_host;
    NSView* dolView;
                          
    uint16_t *_soundBuffer;
    bool _isInitialized;
    bool _shouldReset;
    float _frameInterval;
}
@property (copy) NSString *filePath;
@end

__weak DolphinGameCore *_current = 0;

@implementation DolphinGameCore
- (id)init
{
    if(self = [super init])
        gc_host = DolHost::GetInstance();


    _current = self;
    return self;
}

- (void)dealloc
{
    delete gc_host;
    free(_soundBuffer);
}

# pragma mark - Execution
- (BOOL)loadFileAtPath:(NSString *)path
{

    NSString *resourcePath = [[[self owner] bundle] resourcePath];
    NSString *supportDirectoryPath = [self supportDirectoryPath];
    
    gc_host->Init([supportDirectoryPath UTF8String] );
    self.filePath = path;
    
    return YES;
}

- (void)setPauseEmulation:(BOOL)flag
{
    gc_host->Pause(flag);
}

- (void)stopEmulation
{
    gc_host->RequestStop();
    
    [super stopEmulation];
}

- (void) startEmulation
{
    [NSThread detachNewThreadSelector:@selector(runDolphinThread) toTarget:self withObject:nil];
    [super startEmulation];
}

- (void)resetEmulation
{
    gc_host->RequestReset();
}

- (void)executeFrame
{
    if(!_isInitialized)
    {
        const char * cpath = [[self filePath] cStringUsingEncoding:NSUTF8StringEncoding];
    
        if(gc_host->LoadFileAtPath(cpath))
            _isInitialized=true;
    }
    gc_host->UpdateFrame();
}

# pragma  CoreThread
- (void)runDolphinThread
{
    @autoreleasepool
    {
        OESetThreadRealtime(1. / 50, .007, .03); // guessed from bsnes
        
        //Set this thread as the render delegate so we have access to the Alternate Context
        [self.renderDelegate willRenderFrameOnAlternateThread];
        
        //Get the id of the Render buffer in OpenEmu and pass it to the GC_host
        gc_host->SetPresentationFBO((int)[[self.renderDelegate presentationFramebuffer] integerValue]);
        
        //Start the Core Thread of Dolphin
        gc_host->RunCore();
        
        [super stopEmulation];
    }
}

# pragma Core Emulator Callbacks
- (void)makeCurrent
{
    //This will make the OE Alternate Context the current OGL Context
    [self.renderDelegate willRenderFrameOnAlternateThread];
}

- (void)swapBuffers
{
    //This will render the Dolphin FBO frame
    [self.renderDelegate didRenderFrameOnAlternateThread];
}

# pragma mark - Video
- (OEGameCoreRendering)gameCoreRendering
{
    return OEGameCoreRenderingOpenGL3Video;
}

- (BOOL)hasAlternateRenderingThread
{
    return YES;
}

- (BOOL)needsDoubleBufferedFBO
{
    return YES;
}

- (const void *)videoBuffer
{
    return NULL;
}

- (NSTimeInterval)frameInterval
{
    return _frameInterval ?: 60;
}

- (OEIntSize)bufferSize
{
    return OEIntSizeMake(640, 480);
}

- (OEIntSize)aspectSize
{
    return OEIntSizeMake(4, 3);
}

- (GLenum)pixelFormat
{
    return GL_RGBA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_BYTE;
}

- (GLenum)internalPixelFormat
{
    return GL_RGBA;
}

# pragma mark - Audio
- (NSUInteger)channelCount
{
    return 2;
}

- (double)audioSampleRate
{
    return SAMPLERATE;
}

# pragma mark - Save States
- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    block(gc_host->SaveState([fileName UTF8String]),nil);
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
     block(gc_host->LoadState([fileName UTF8String]),nil);
}

# pragma mark - Input
- (oneway void)didMoveGCJoystickDirection:(OEGCButton)button withValue:(CGFloat)value forPlayer:(NSUInteger)player
{
    gc_host->SetAxis(button, value, (int)player);
}

- (oneway void)didPushGCButton:(OEGCButton)button forPlayer:(NSUInteger)player
{
    gc_host->SetButtonState(button, 1, (int)player);
}

- (oneway void)didReleaseGCButton:(OEGCButton)button forPlayer:(NSUInteger)player
{
    gc_host->SetButtonState(button, 0, (int)player);
}
@end
