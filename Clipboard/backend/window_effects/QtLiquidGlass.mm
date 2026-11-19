#ifdef PLATFORM_OSX
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <map>

// -----------------------------------------------------------------------------
// Internal Registry & Types
// -----------------------------------------------------------------------------

struct GlassContext {
    NSView* glassView;       // The visual effect view
    NSView* effectContainerView; // Clips the effect to the requested Qt/window frame
    NSBox* backgroundView;   // The optional opaque backing layer
    NSView* hostView;        // The Qt Native View we attached to
    NSView* containerView;   // The parent we injected into (NSThemeFrame for root windows)
    NSObject* frameObserver; // Keeps glass geometry valid while Qt animates window size
    double requestedCornerRadius;
    int id;
};

static std::map<int, GlassContext> g_registry;
static int g_nextViewId = 1; // Start at 1

// Keys for objc-associated objects (to find ID from View)
static const void *kGlassContextIdKey = &kGlassContextIdKey;
static const CGFloat kMinGlassEffectHeight = 160.0;

#define RUN_ON_MAIN(block)                                  \
  if ([NSThread isMainThread]) {                            \
    block();                                                \
  } else {                                                  \
    dispatch_sync(dispatch_get_main_queue(), block);        \
  }

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

static NSRect FrameForGlassContext(const GlassContext& ctx) {
    if (!ctx.hostView || !ctx.containerView) return NSZeroRect;

    if (ctx.containerView == ctx.hostView) {
        return [ctx.hostView bounds];
    }

    if ([ctx.hostView superview] == ctx.containerView) {
        return [ctx.hostView frame];
    }

    return [ctx.containerView bounds];
}

static void ApplyCornerRadiusToView(NSView *view, double cornerRadius) {
    if (!view) return;

    SEL setCR = sel_registerName("setCornerRadius:");
    if ([view respondsToSelector:setCR]) {
        ((void (*)(id, SEL, double))objc_msgSend)(view, setCR, cornerRadius);
    } else {
        view.layer.cornerRadius = cornerRadius;
        view.layer.masksToBounds = (cornerRadius > 0);
    }
}

static double VisibleCornerRadius(NSRect frame, double requestedRadius) {
    if (requestedRadius <= 0.0) return 0.0;

    double minSide = MIN(NSWidth(frame), NSHeight(frame));
    if (minSide <= 0.0) return 0.0;

    return MIN(requestedRadius, minSide * 0.5);
}

static void SyncGlassGeometryAndRadius(GlassContext& ctx) {
    NSRect frame = FrameForGlassContext(ctx);
    NSRect clippedBounds = NSMakeRect(0, 0, NSWidth(frame), NSHeight(frame));
    NSRect glassFrame = clippedBounds;
    glassFrame.size.height = MAX(NSHeight(clippedBounds), kMinGlassEffectHeight);
    glassFrame.origin.y = (NSHeight(clippedBounds) - NSHeight(glassFrame)) * 0.5;
    double visibleCornerRadius = VisibleCornerRadius(frame, ctx.requestedCornerRadius);

    if (ctx.effectContainerView) {
        [ctx.effectContainerView setFrame:frame];
        ctx.effectContainerView.layer.cornerRadius = visibleCornerRadius;
        ctx.effectContainerView.layer.masksToBounds = (visibleCornerRadius > 0);
    }
    if (ctx.glassView) {
        [ctx.glassView setFrame:glassFrame];
    }
    if (ctx.backgroundView) {
        [ctx.backgroundView setFrame:clippedBounds];
    }

    // The visible rounded shape belongs to the clipping wrapper. Keeping a
    // radius on the taller inner glass view makes its clipped sides bulge.
    ApplyCornerRadiusToView(ctx.glassView, 0.0);

    if (ctx.backgroundView) {
        ctx.backgroundView.layer.cornerRadius = 0.0;
        ctx.backgroundView.layer.masksToBounds = NO;
    }
}

// Injects an NSGlassEffectView (or NSVisualEffectView fallback) behind the
// given native view. Handles root windows (sibling injection into NSThemeFrame),
// frameless windows (content swap), and child widgets. Returns a registry ID.
extern "C" {
  int AddGlassEffectView(void* nativeViewPtr, bool opaque);
  void ConfigureGlassView(int viewId, double cornerRadius, double r, double g, double b, double a);
  void RemoveGlassEffectView(int viewId);

  int AddGlassEffectView(void* nativeViewPtr, bool opaque) {
    if (!nativeViewPtr) return -1;

    __block int resultId = -1;

    RUN_ON_MAIN(^{
      NSView *rootView = reinterpret_cast<NSView *>(nativeViewPtr);
      if (!rootView) return;

      // Remove existing glass to prevent stacking duplicates
      NSNumber *existingId = objc_getAssociatedObject(rootView, kGlassContextIdKey);
      if (existingId) {
          RemoveGlassEffectView([existingId intValue]);
      }

      NSWindow *win = [rootView window];
      bool isRoot = (win && [win contentView] == rootView);

      NSView *container = nil;

      if (isRoot) {
          // Root window: force transparency and inject into the frame
          [win setOpaque:NO];
          [win setBackgroundColor:[NSColor clearColor]];
          win.styleMask |= NSWindowStyleMaskFullSizeContentView;
          win.titlebarAppearsTransparent = YES;
          
          // Inject into NSThemeFrame's content slot, or swap if needed
          if ([rootView superview]) {
              container = [rootView superview];
          } else {
              // Frameless: wrap rootView in a new container
              NSRect frame = [rootView frame];
              NSView *newContainer = [[NSView alloc] initWithFrame:frame];
              newContainer.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
              newContainer.wantsLayer = YES;
              
              [win setContentView:newContainer];
              
              [rootView setFrame:newContainer.bounds];
              [rootView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
              [newContainer addSubview:rootView];
              
              container = newContainer;
          }
      } else {
          // Child widget: inject inside its native view
          container = rootView;
      }

      NSRect frameRect = (container == rootView) ? [rootView bounds] : [rootView frame];
      if (isRoot && container != [rootView superview]) {
          frameRect = [container bounds];
      }

      NSView *effectContainer = [[NSView alloc] initWithFrame:frameRect];
      effectContainer.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
      effectContainer.wantsLayer = YES;
      effectContainer.layer.masksToBounds = YES;

      // Optional opaque backing layer
      NSBox *backgroundView = nil;
      if (opaque) {
          backgroundView = [[NSBox alloc] initWithFrame:[effectContainer bounds]];
          backgroundView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
          backgroundView.boxType = NSBoxCustom;
          backgroundView.borderType = NSNoBorder;
          backgroundView.fillColor = [NSColor windowBackgroundColor];
          backgroundView.wantsLayer = YES;
      }

      NSView *glass = nil;
      Class glassCls = NSClassFromString(@"NSGlassEffectView");
      NSRect glassFrame = [effectContainer bounds];
      glassFrame.size.height = MAX(NSHeight(glassFrame), kMinGlassEffectHeight);
      glassFrame.origin.y = (NSHeight([effectContainer bounds]) - NSHeight(glassFrame)) * 0.5;
      if (glassCls) {
          glass = [[glassCls alloc] initWithFrame:glassFrame];
      } else {
          // Fallback to NSVisualEffectView on older macOS
          NSVisualEffectView *visual = [[NSVisualEffectView alloc] initWithFrame:glassFrame];
          visual.blendingMode = NSVisualEffectBlendingModeBehindWindow;
          visual.material = NSVisualEffectMaterialUnderWindowBackground;
          visual.state = NSVisualEffectStateActive;
          glass = visual;
      }
      glass.autoresizingMask = NSViewWidthSizable;
      glass.wantsLayer = YES;

      if (backgroundView) {
          [effectContainer addSubview:backgroundView positioned:NSWindowBelow relativeTo:nil];
          [effectContainer addSubview:glass positioned:NSWindowAbove relativeTo:backgroundView];
      } else {
          [effectContainer addSubview:glass];
      }

      // Subview order: [Background] -> [Glass] -> [Qt Content]
      if (container == rootView) {
          [container addSubview:effectContainer positioned:NSWindowBelow relativeTo:nil];
      } else {
          [container addSubview:effectContainer positioned:NSWindowBelow relativeTo:rootView];
      }

      // Register context and associate the ID with the host view
      int id = g_nextViewId++;
      GlassContext ctx;
      ctx.id = id;
      ctx.glassView = glass;
      ctx.effectContainerView = effectContainer;
      ctx.backgroundView = backgroundView;
      ctx.hostView = rootView;
      ctx.containerView = container;
      ctx.frameObserver = nil;
      ctx.requestedCornerRadius = 0.0;

      g_registry[id] = ctx;
      objc_setAssociatedObject(rootView, kGlassContextIdKey, @(id), OBJC_ASSOCIATION_RETAIN);

      [rootView setPostsFrameChangedNotifications:YES];
      [container setPostsFrameChangedNotifications:YES];

      __block int observerId = id;
      NSObject *observer = [[NSNotificationCenter defaultCenter]
          addObserverForName:NSViewFrameDidChangeNotification
                      object:nil
                       queue:[NSOperationQueue mainQueue]
                  usingBlock:^(NSNotification *note) {
                      auto it = g_registry.find(observerId);
                      if (it == g_registry.end()) return;

                      GlassContext& observedCtx = it->second;
                      NSView *changedView = (NSView *)[note object];
                      if (changedView == observedCtx.hostView || changedView == observedCtx.containerView) {
                          SyncGlassGeometryAndRadius(observedCtx);
                      }
                  }];

      g_registry[id].frameObserver = observer;
      
      resultId = id;
    });

    return resultId;
  }

  // Sets corner radius (native setCornerRadius: or layer fallback) and tint color.
  // Also rounds the opaque backing layer and container (NSThemeFrame) to prevent
  // rectangular backgrounds from bleeding through transparent glass materials.
  void ConfigureGlassView(int viewId, double cornerRadius, double r, double g, double b, double a) {
    RUN_ON_MAIN(^{
      auto it = g_registry.find(viewId);
      if (it == g_registry.end()) return;
      GlassContext& ctx = it->second;
      ctx.requestedCornerRadius = cornerRadius;

      if (ctx.glassView) {
          SyncGlassGeometryAndRadius(ctx);

          // Tint: use native setTintColor: if available, otherwise layer bg
          NSColor* c = [NSColor colorWithRed:r green:g blue:b alpha:a];
          if (c && [ctx.glassView respondsToSelector:@selector(setTintColor:)]) {
              [(id)ctx.glassView setTintColor:c];
          } else if (c) {
              ctx.glassView.layer.backgroundColor = c.CGColor;
          }
      }

    });
  }
}
// Detaches the glass and backing views, clears the associated object, and
// removes the context from the registry.
extern "C" void RemoveGlassEffectView(int viewId) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    GlassContext& ctx = it->second;

    // Detach views and clear the associated object on the host
    if (ctx.frameObserver) [[NSNotificationCenter defaultCenter] removeObserver:ctx.frameObserver];
    if (ctx.effectContainerView) [ctx.effectContainerView removeFromSuperview];
    if (ctx.hostView) {
        objc_setAssociatedObject(ctx.hostView, kGlassContextIdKey, nil, OBJC_ASSOCIATION_ASSIGN);
    }
    g_registry.erase(it);
  });
}

// -----------------------------------------------------------------------------
// Setters
// -----------------------------------------------------------------------------

// Sets the private _variant property that controls the glass style (e.g. 16=sidebar, 2=dock).
extern "C" void SetGlassViewVariant(int viewId, int variant) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    SEL sel = sel_registerName("set_variant:");
    if ([glass respondsToSelector:sel]) {
      ((void (*)(id, SEL, long long))objc_msgSend)(glass, sel, (long long)variant);
    }
  });
}

// Only applies to the NSVisualEffectView fallback path
extern "C" void SetGlassViewMaterial(int viewId, int material) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    if ([glass isKindOfClass:[NSVisualEffectView class]]) {
        [(NSVisualEffectView*)glass setMaterial:(NSVisualEffectMaterial)material];
    }
  });
}

// Sets blending mode: 0=BehindWindow (blur desktop), 1=WithinWindow (blur app content).
extern "C" void SetGlassViewBlendingMode(int viewId, int mode) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    // NSGlassEffectView inherits from NSView, not NSVisualEffectView
    SEL sel = sel_registerName("setBlendingMode:");
    if ([glass respondsToSelector:sel]) {
        ((void (*)(id, SEL, long long))objc_msgSend)(glass, sel, (long long)mode);
    }
  });
}

// Sets the private _scrimState property (overlay dimming layer).
extern "C" void SetGlassViewScrim(int viewId, int scrim) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    SEL sel = sel_registerName("set_scrimState:");
    if ([glass respondsToSelector:sel]) {
      ((void (*)(id, SEL, long long))objc_msgSend)(glass, sel, (long long)scrim);
    }
  });
}

// Sets the private _subduedState property (reduces vibrancy/saturation).
extern "C" void SetGlassViewSubdued(int viewId, int subdued) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    SEL sel = sel_registerName("set_subduedState:");
    if ([glass respondsToSelector:sel]) {
      ((void (*)(id, SEL, long long))objc_msgSend)(glass, sel, (long long)subdued);
    }
  });
}

// Sets the private _contentLensing property (refraction intensity).
extern "C" void SetGlassViewContentLensing(int viewId, int lensing) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    SEL sel = sel_registerName("set_contentLensing:");
    if ([glass respondsToSelector:sel]) {
      ((void (*)(id, SEL, long long))objc_msgSend)(glass, sel, (long long)lensing);
    }
  });
}

// Sets appearance: 0=Light, 1=Dark, 2=Auto. Uses public NSView.appearance API
// for reliable control, plus the private _adaptiveAppearance property.
extern "C" void SetGlassViewAdaptiveAppearance(int viewId, int appearance) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    // Use the public NSView.appearance API for reliable light/dark control
    switch (appearance) {
      case 0: // Light
        glass.appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
        break;
      case 1: // Dark
        glass.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
        break;
      default: // Auto — inherit from parent
        glass.appearance = nil;
        break;
    }

    // Also set the private property (may affect glass-specific rendering)
    int clamped = (appearance < 0) ? 0 : (appearance > 2) ? 2 : appearance;
    SEL sel = sel_registerName("set_adaptiveAppearance:");
    if ([glass respondsToSelector:sel]) {
      ((void (*)(id, SEL, long long))objc_msgSend)(glass, sel, (long long)clamped);
    }
  });
}

// Sets interaction state: 0=Normal, 1=Hovered. Clamped to 0-1 because
// values >= 2 trigger a Swift precondition crash in NSGlassEffectView.
extern "C" void SetGlassViewInteractionState(int viewId, int state) {
  RUN_ON_MAIN(^{
    auto it = g_registry.find(viewId);
    if (it == g_registry.end()) return;
    NSView* glass = it->second.glassView;

    // Clamp: values >= 2 crash (Swift precondition in NSGlassEffectView)
    int clamped = (state < 0) ? 0 : (state > 1) ? 1 : state;

    SEL sel = sel_registerName("set_interactionState:");
    if ([glass respondsToSelector:sel]) {
      ((void (*)(id, SEL, long long))objc_msgSend)(glass, sel, (long long)clamped);
    }
  });
}

#endif // PLATFORM_OSX
