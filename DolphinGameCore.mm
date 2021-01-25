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

/*
    What doesn't work:
          I got everything in GC working

 */

//  Changed <al*> includes to <OpenAL/al*>
//  Added iRenderFBO to Videoconfig, OGL postprocessing and renderer
//  Added SetState to device.h for input and FullAnalogControl
//  Added Render on alternate thread in Core.cpp in EmuThread() Video Thread
//  Added Render on alternate thread in Cope.cpp in CPUThread() to support single thread mode CPU/GPU

#import "DolphinGameCore.h"
#include "DolHost.h"
#include "AudioCommon/SoundStream.h"
#include "OpenEmuAudioStream.h"
#include <stdatomic.h>

#import <AppKit/AppKit.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#define SAMPLERATE 48000
#define SIZESOUNDBUFFER 48000 / 60 * 4
#define OpenEmu 1

@interface DolphinGameCore () <OEGCSystemResponderClient>
@property (copy) NSString *filePath;
@end

DolphinGameCore *_current = 0;

extern std::unique_ptr<SoundStream> g_sound_stream;

@implementation DolphinGameCore
{
    DolHost *dol_host;

    uint16_t *_soundBuffer;
    bool _isWii;
    atomic_bool _isInitialized;
    float _frameInterval;

    NSString *autoLoadStatefileName;
    NSString *_dolphinCoreModule;
    OEIntSize _dolphinCoreAspect;
    OEIntSize _dolphinCoreScreen;
    
    NSMutableArray <NSMutableDictionary <NSString *, id> *> *_advancedMenus;
}

- (instancetype)init
{
    if(self = [super init]){
        dol_host = DolHost::GetInstance();
    }

    _current = self;

    return self;
}

- (void)dealloc
{
    delete dol_host;
    free(_soundBuffer);
}

# pragma mark - Execution
- (BOOL)loadFileAtPath:(NSString *)path
{
    self.filePath = path;

    if([[self systemIdentifier] isEqualToString:@"openemu.system.gc"])
    {
        _dolphinCoreModule = @"gc";
        _isWii = false;
        _dolphinCoreAspect = OEIntSizeMake(4, 3);
        _dolphinCoreScreen = OEIntSizeMake(640, 480);
    }
    else
    {
        _dolphinCoreModule = @"Wii";
        _isWii = true;
        _dolphinCoreAspect = OEIntSizeMake(16,9);
        _dolphinCoreScreen = OEIntSizeMake(854, 480);
    }

    dol_host->Init([[self supportDirectoryPath] fileSystemRepresentation], [path fileSystemRepresentation] );

    usleep(5000);
    return YES;
}

- (void)setPauseEmulation:(BOOL)flag
{
    dol_host->Pause(flag);
    
    [super setPauseEmulation:flag];
}

- (void)stopEmulation
{
    _isInitialized = false;
    
    dol_host->RequestStop();

    [super stopEmulation];
}

- (void)startEmulation
{
    if (!_isInitialized)
    {
        [self.renderDelegate willRenderFrameOnAlternateThread];

        dol_host->SetPresentationFBO((int)[[self.renderDelegate presentationFramebuffer] integerValue]);

        if(dol_host->LoadFileAtPath())
            _isInitialized = true;
        
        _frameInterval = dol_host->GetFrameInterval();
        
        if (_isWii)
            [self changeAdvancedMenuOption:@"Wiimote" menuID:@"Controller1"];
        
    }
    [super startEmulation];

    //Disable the OE framelimiting
    [self.renderDelegate suspendFPSLimiting];
}

- (void)resetEmulation
{
     dol_host->Reset();
}

- (void)executeFrame
{
   if (![self isEmulationPaused])
    {
        if(!dol_host->CoreRunning()) {
        dol_host->Pause(false);
        }
    
      dol_host->UpdateFrame();
    }
}

# pragma mark - Nand directory Callback
- (const char *)getBundlePath
{
    NSBundle *coreBundle = [NSBundle bundleForClass:[self class]];
    const char *dataPath;
    dataPath = [[coreBundle resourcePath] fileSystemRepresentation];

    return dataPath;
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
    return NO;
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
    return _dolphinCoreScreen;
}

- (OEIntSize)aspectSize
{
    return _dolphinCoreAspect;
}

- (void) SetScreenSize:(int)width :(int)height
{
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
    return OE_SAMPLERATE;
}

- (id<OEAudioBuffer>)audioBufferAtIndex:(NSUInteger)index
{
    return self;
}

- (NSUInteger)length
{
    return OE_SIZESOUNDBUFFER;
}

- (NSUInteger)read:(void *)buffer maxLength:(NSUInteger)len
{
    if (_isInitialized && g_sound_stream)
        return static_cast<OpenEmuAudioStream*>(g_sound_stream.get())->readAudio(buffer, (int)len);
    return 0;
}

- (NSUInteger)write:(const void *)buffer maxLength:(NSUInteger)length
{
    return 0;
}

# pragma mark - Save States
- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    // we need to make sure we are initialized before attempting to save a state
    while (! _isInitialized)
        usleep (1000);

    block(dol_host->SaveState([fileName UTF8String]),nil);

}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    if (!_isInitialized)
    {
        //Start a separate thread to load
        autoLoadStatefileName = fileName;
        
        [NSThread detachNewThreadSelector:@selector(autoloadWaitThread) toTarget:self withObject:nil];
        block(true, nil);
    } else {
        block(dol_host->LoadState([fileName UTF8String]),nil);
    }
}

- (void)autoloadWaitThread
{
    @autoreleasepool
    {
        //Wait here until we get the signal for full initialization
        while (!_isInitialized)
            usleep (100);
        
        dol_host->LoadState([autoLoadStatefileName UTF8String]);
    }
}

# pragma mark - Input GC
- (oneway void)didMoveGCJoystickDirection:(OEGCButton)button withValue:(CGFloat)value forPlayer:(NSUInteger)player
{
    if(_isInitialized)
    {
        dol_host->SetAxis(button, value, (int)player);
    }
}

- (oneway void)didPushGCButton:(OEGCButton)button forPlayer:(NSUInteger)player
{
    if(_isInitialized)
    {
        dol_host->setButtonState(button, 1, (int)player);
    }
}

- (oneway void)didReleaseGCButton:(OEGCButton)button forPlayer:(NSUInteger)player
{
    if(_isInitialized)
    {
        dol_host->setButtonState(button, 0, (int)player);
    }
}

# pragma mark - Input Wii
- (oneway void)didMoveWiiJoystickDirection:(OEWiiButton)button withValue:(CGFloat)value forPlayer:(NSUInteger)player
{
    if(_isInitialized)
    {
        dol_host->SetAxis(button, value, (int)player);
    }
}

- (oneway void)didPushWiiButton:(OEWiiButton)button forPlayer:(NSUInteger)player
{
    if(_isInitialized)
    {
        if (button > OEWiiButtonCount) {
            dol_host->processSpecialKeys(button , (int)player);
        } else {
            dol_host->setButtonState(button, 1, (int)player);
        }
    }
}

- (oneway void)didReleaseWiiButton:(OEWiiButton)button forPlayer:(NSUInteger)player
{
    if(_isInitialized && button != OEWiimoteSideways && button != OEWiimoteUpright)
    {
        dol_host->setButtonState(button, 0, (int)player);
    }
}

//- (oneway void) didMoveWiiAccelerometer:(OEWiiAccelerometer)accelerometer withValue:(CGFloat)X withValue:(CGFloat)Y withValue:(CGFloat)Z forPlayer:(NSUInteger)player
//{
//    if(_isInitialized)
//    {
//        if (accelerometer == OEWiiNunchuk)
//        {
//            dol_host->setNunchukAccel(X,Y,Z,(int)player);
//        }
//        else
//        {
//            dol_host->setWiimoteAccel(X,Y,Z,(int)player);
//        }
//    }
//}

//- (oneway void)didMoveWiiIR:(OEWiiButton)button IRinfo:(OEwiimoteIRinfo)IRinfo forPlayer:(NSUInteger)player
//{
//    if(_isInitialized)
//    {
//        dol_host->setIRdata(IRinfo ,(int)player);
//    }
//}

- (oneway void)didChangeWiiExtension:(OEWiimoteExtension)extension forPlayer:(NSUInteger)player
{
    if(_isInitialized)
    {
        dol_host->changeWiimoteExtension(extension, (int)player);
    }
}

- (oneway void)IRMovedAtPoint:(int)X withValue:(int)Y
{
//    if (_isInitialized)
//    {
//        int dX = (1023.0 / 854.0) * X;
//        int dY =  (767.0 / 480.0) * Y;
//
////        dol_host->DisplayMessage([[NSString stringWithFormat:@"X: %d, Y: %d",dX,dY ] UTF8String]);
//
//       dol_host->SetIR(0, dX,dY);
//    }
}

# pragma mark - Cheats
- (void)setCheat:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled
{
    dol_host->SetCheat([code UTF8String], [type UTF8String], enabled);
}

# pragma mark - Advanced Menu Functions (do not change)
- (void)changeAdvancedMenuOption:(NSString *)advancedMenuName menuID:(NSString *)menuID
{
    if (_advancedMenus.count == 0)
        [self advancedMenu];

    // First check if Menu Option is toggleable and grab its preference key
    BOOL isMenuToggleable = NO;
    BOOL isValidMenu = NO;
    BOOL menuState = NO;
    NSString *menuPrefKey;
    NSString *menuIDKey;

    for (NSDictionary *menuDict in _advancedMenus)
    {
        if (menuDict[OEGameCoreAdvancedMenuGroupNameKey]){
            NSDictionary *subMenuDict = [self findAdvancedSubMenu:menuDict menuName:advancedMenuName menuID:menuID];
            if (subMenuDict){
                menuState = [subMenuDict[OEGameCoreAdvancedMenuStateKey] boolValue];
                menuPrefKey = subMenuDict[OEGameCoreAdvancedMenuPrefKeyNameKey];
                menuIDKey = subMenuDict[OEGameCoreAdvancedMenuGroupIDKey];
                isMenuToggleable = [subMenuDict[OEGameCoreAdvancedMenuAllowsToggleKey] boolValue];
                isValidMenu = YES;
                break;
            }
        }
        if ([menuDict[OEGameCoreAdvancedMenuNameKey] isEqualToString:advancedMenuName])
        {
            if ([menuDict[OEGameCoreAdvancedMenuGroupIDKey] isEqualToString:menuID])
            {
                menuState = [menuDict[OEGameCoreAdvancedMenuStateKey] boolValue];
                menuPrefKey = menuDict[OEGameCoreAdvancedMenuPrefKeyNameKey];
                menuIDKey = menuDict[OEGameCoreAdvancedMenuGroupIDKey];
                isMenuToggleable = [menuDict[OEGameCoreAdvancedMenuAllowsToggleKey] boolValue];
                isValidMenu = YES;
                break;
            }
        }
    }

    if (!isValidMenu)
        return;
    
    // Handle option state changes
    for (NSMutableDictionary *menuDict in _advancedMenus)
    {
        if (menuDict[OEGameCoreAdvancedMenuGroupNameKey]){
            [self processAdvancedSubMenu:menuDict menuName:advancedMenuName menuID:menuID menuState:menuState menuPrefKey:menuPrefKey menuToggle:isMenuToggleable];
        }
    }

    //send the menuname and menuID for actions to be taken
    [self advancedMenuAction:advancedMenuName menuID:menuID];
}

- (void) processAdvancedSubMenu:(NSMutableDictionary *)parentMenuDict menuName:(NSString *)menuName menuID:(NSString *)menuID menuState:(BOOL)menuState menuPrefKey:(NSString *)menuPrefKey menuToggle:(BOOL)menuToggle;
{
    for (NSMutableDictionary *subMenuDict in parentMenuDict[OEGameCoreAdvancedMenuGroupItemsKey])
    {
        //This function iterates through the menu items and sets the option and clears others in the same prefs group or toggles the menu item
        NSString *curMenuID = subMenuDict[OEGameCoreAdvancedMenuGroupIDKey];
        
        if (subMenuDict[OEGameCoreAdvancedMenuGroupNameKey]){
            [self processAdvancedSubMenu:subMenuDict menuName:menuName menuID:menuID menuState:menuState menuPrefKey:menuPrefKey menuToggle:menuToggle];
        }
        else if ( [curMenuID isEqualToString:menuID] ){
            NSString *curMenuName = subMenuDict[OEGameCoreAdvancedMenuNameKey];
            NSString *curMenuKey  = subMenuDict[OEGameCoreAdvancedMenuPrefKeyNameKey];
            
            if (!curMenuName)
                continue;
            // Mutually exclusive option state change
            else if ([curMenuName isEqualToString:menuName] && !menuToggle)
                subMenuDict[OEGameCoreAdvancedMenuStateKey] = @YES;
            // Reset mutually exclusive options that are the same prefs group
            else if (!menuToggle && [curMenuKey isEqualToString:menuPrefKey])
                subMenuDict[OEGameCoreAdvancedMenuStateKey] = @NO;
            // Toggleable option state change
            else if ([curMenuName isEqualToString:menuName]  && menuToggle)
                subMenuDict[OEGameCoreAdvancedMenuStateKey] = @(!menuState);
        }
    }
}

- (NSDictionary *) findAdvancedSubMenu:(NSDictionary *)parentMenuDict menuName:(NSString *)menuName menuID:(NSString *)menuID
{
    //This function iterates through the menu items and returns the dictionary item if found
    NSDictionary *testSubMenuDict;
    
    for (NSDictionary *subMenuDict in parentMenuDict[OEGameCoreAdvancedMenuGroupItemsKey])
    {
        if (subMenuDict[OEGameCoreAdvancedMenuGroupNameKey])
        {
            testSubMenuDict = [self findAdvancedSubMenu:subMenuDict menuName:menuName menuID:menuID];
            
            if (testSubMenuDict)
            {
                if ([testSubMenuDict[OEGameCoreAdvancedMenuNameKey] isEqualToString:menuName])
                {
                    if ([testSubMenuDict[OEGameCoreAdvancedMenuGroupIDKey] isEqualToString:menuID])
                    {
                        return testSubMenuDict;
                    }
                }
            }
        }
        if ([subMenuDict[OEGameCoreAdvancedMenuNameKey] isEqualToString:menuName])
        {
            if ([subMenuDict[OEGameCoreAdvancedMenuGroupIDKey] isEqualToString:menuID])
            {
                return subMenuDict;
            }
        }
    }
    return nil;
}

# pragma mark - Advanced Menu Creation

- (NSArray <NSDictionary <NSString *, id> *> *)advancedMenu
{
    //Create your menu array here.  4 levels deep at most
    if (_advancedMenus.count == 0)
    {
        _advancedMenus = [NSMutableArray array];

        NSArray <NSDictionary <NSString *, id> *> *advancedMenuWithDefault =
        @[ // Level 1
//            @{  OEGameCoreAdvancedMenuGroupNameKey      : @"Controllers",
//                OEGameCoreAdvancedMenuGroupIDKey        : @"Controllers",
//                OEGameCoreAdvancedMenuGroupItemsKey     : @[
//                    // Level 2
                    @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Controller 1",
                       OEGameCoreAdvancedMenuGroupIDKey        : @"Controller1",
                       OEGameCoreAdvancedMenuGroupItemsKey     : @[
                              // Level 3
                              OEAdvancedMenu_OptionDefault(@"Wiimote", @"Controller1", @"controller1"),
                              OEAdvancedMenu_Option(@"Sideways Wiimote", @"Controller1", @"controller1"),
                              OEAdvancedMenu_SeparatorItem(),
                              @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Wiimote Attachment",
                                 OEGameCoreAdvancedMenuGroupIDKey        : @"Controller1",
                                 OEGameCoreAdvancedMenuGroupItemsKey     : @[
                                      // Level 4
                                      OEAdvancedMenu_OptionIndentedDefault(@"None", @"Controller1", @"attachment1"),
                                      OEAdvancedMenu_OptionIndented(@"Nunckuk", @"Controller1", @"attachment1"),
                                      OEAdvancedMenu_OptionIndented(@"Classic Controller", @"Controller1", @"attachment1"),
                                 ]
                              },
                       ]
                    },
                    // Level 2
                    @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Controller 2",
                       OEGameCoreAdvancedMenuGroupIDKey        : @"Controller2",
                       OEGameCoreAdvancedMenuGroupItemsKey     : @[
                               // Level 3
                               OEAdvancedMenu_OptionDefault(@"Wiimote", @"Controller2", @"controller2"),
                               OEAdvancedMenu_Option(@"Sideways Wiimote", @"Controller2", @"controller2"),
                               OEAdvancedMenu_SeparatorItem(),
                               @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Wiimote Attachment",
                                  OEGameCoreAdvancedMenuGroupIDKey        : @"Controller2",
                                  OEGameCoreAdvancedMenuGroupItemsKey     : @[
                                       // Level 4
                                       OEAdvancedMenu_OptionIndentedDefault(@"None", @"Controller2", @"attachment2"),
                                       OEAdvancedMenu_OptionIndented(@"Nunckuk", @"Controller2", @"attachment2"),
                                       OEAdvancedMenu_OptionIndented(@"Classic Controller", @"Controller2", @"attachment2"),
                                  ]
                               },
                       ]
                    },
                    // Level 2
                    @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Controller 3",
                       OEGameCoreAdvancedMenuGroupIDKey        : @"Controller3",
                       OEGameCoreAdvancedMenuGroupItemsKey     : @[
                               // Level 3
                               OEAdvancedMenu_OptionDefault(@"Wiimote", @"Controller3", @"controller3"),
                               OEAdvancedMenu_Option(@"Sideways Wiimote", @"Controller3", @"controller3"),
                               OEAdvancedMenu_SeparatorItem(),
                               @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Wiimote Attachment",
                                  OEGameCoreAdvancedMenuGroupIDKey        : @"Controller3",
                                  OEGameCoreAdvancedMenuGroupItemsKey     : @[
                                          // Level 4
                                       OEAdvancedMenu_OptionIndentedDefault(@"None", @"Controller3", @"attachment3"),
                                       OEAdvancedMenu_OptionIndented(@"Nunckuk", @"Controller3", @"attachment3"),
                                       OEAdvancedMenu_OptionIndented(@"Classic Controller", @"Controller3", @"attachment3"),
                                  ]
                               },
                       ]
                    },
                    // Level 2
                    @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Controller 4",
                       OEGameCoreAdvancedMenuGroupIDKey        : @"Controller4",
                       OEGameCoreAdvancedMenuGroupItemsKey     : @[
                               // Level 3
                               OEAdvancedMenu_OptionDefault(@"Wiimote", @"Controller4", @"controller4"),
                               OEAdvancedMenu_Option(@"Sideways Wiimote", @"Controller4", @"controller4"),
                               OEAdvancedMenu_SeparatorItem(),
                               @{ OEGameCoreAdvancedMenuGroupNameKey      : @"Wiimote Attachment",
                                  OEGameCoreAdvancedMenuGroupIDKey        : @"Controller4",
                                  OEGameCoreAdvancedMenuGroupItemsKey     : @[
                                          // Level 4
                                       OEAdvancedMenu_OptionIndentedDefault(@"None", @"Controller4", @"attachment4"),
                                       OEAdvancedMenu_OptionIndented(@"Nunckuk", @"Controller4", @"attachment4"),
                                       OEAdvancedMenu_OptionIndented(@"Classic Controller", @"Controller4", @"attachment4"),
                                  ]
                               },
                       ]
//                    },
//               ]
            },
          ];

        // Deep mutable copy
        _advancedMenus = (NSMutableArray *)CFBridgingRelease(CFPropertyListCreateDeepCopy(kCFAllocatorDefault, (CFArrayRef)advancedMenuWithDefault, kCFPropertyListMutableContainers));
    }
    return [_advancedMenus copy];
}

# pragma mark - Advanced Menu Actions Processing

- (void)advancedMenuAction:(NSString *)advancedMenuName menuID:(NSString *)menuID
{
    //Write your code to process Clicked menu items here
    int player = 0;
    
    if ([menuID isEqualToString:@"Controller1"]){
        player = 1;
    }
    if ([menuID isEqualToString:@"Controller2"]){
        player = 2;
    }
    if ([menuID isEqualToString:@"Controller3"]){
        player = 3;
    }
    if ([menuID isEqualToString:@"Controller4"]){
        player = 4;
    }
    
    if (player){
        if ([advancedMenuName isEqualToString:@"Wiimote"])
        {
            dol_host->setWiimoteSideways(player, false);
            return;
        }
        if ([advancedMenuName isEqualToString:@"Sideways Wiimote"])
        {
            dol_host->setWiimoteSideways(player, true);
            return;
        }
        
        if ([advancedMenuName isEqualToString:@"none"])
        {
            dol_host->changeWiimoteExtension(0, player);
            return;
        }
        if ([advancedMenuName isEqualToString:@"Nunchuk"])
        {
            dol_host->changeWiimoteExtension(1, player);
            return;
        }
        if ([advancedMenuName isEqualToString:@"Classic Controller"])
        {
            dol_host->changeWiimoteExtension(2, player);
            return;
        }
    }
}
@end
