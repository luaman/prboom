#import "RMUDAnsiTextView.h"

id rgb(int r, int g, int b) {
    return [NSColor colorWithDeviceRed: r/255.0 green: g/255.0 blue: b/255.0 alpha: 1.0];
}

id to_num(int n) {
    return [NSNumber numberWithInt: n];
}

@implementation RMUDAnsiTextView
- (void)dealloc
{
  if (ansiColorTable) {
    [ansiColorTable release];
  }
  if (ansiSequenceRegex) {
    [ansiSequenceRegex release];
  }
  [super dealloc];
}
- (void)awakeFromNib
{
  ansiColorTable =
    [NSDictionary dictionaryWithObjectsAndKeys:
      rgb(0, 0, 0), to_num(30),
      rgb(190, 25, 25), to_num(31),
      rgb(60, 190, 60), to_num(32),
      rgb(200, 200, 75), to_num(33),
      rgb(0, 40, 185), to_num(34),
      rgb(170, 60, 190), to_num(35),
      rgb(50, 200, 200), to_num(36),
      rgb(255, 255, 255), to_num(37),
      rgb(255, 255, 255), to_num(39),
      rgb(0, 0, 0), to_num(40),
      rgb(190, 25, 25), to_num(41),
      rgb(60, 190, 60), to_num(42),
      rgb(200, 200, 75), to_num(43),
      rgb(0, 40, 185), to_num(44),
      rgb(170, 60, 190), to_num(45),
      rgb(50, 200, 200), to_num(46),
      rgb(255, 255, 255), to_num(47),
      rgb(255, 255, 255), to_num(49),
      nil];
  currentForegroundColorIndex = 30;
  currentBackgroundColorIndex = 47;
  
  ansiSequenceRegex =
    [AGRegex regexWithPattern:@"\\e\\[(\\d+;)?(\\d+;)?(\\d+;)?(\\d+;)?(\\d+)m"
                      options:0];

  [ansiSequenceRegex retain];
  [ansiColorTable retain];
  [self setFont:[NSFont fontWithName:@"Monaco" size:12.0]];
}
- (BOOL)isOpaque
{
  return YES;
}
- (NSColor *)currentForegroundColor
{
  if (currentForegroundColorIndex == -1) {
    return nil;
  } else {
    return [ansiColorTable objectForKey:to_num(currentForegroundColorIndex)];
  }
}
- (NSColor *)currentBackgroundColor
{
  if (currentBackgroundColorIndex == -1) {
    return nil;
  } else {
    return [ansiColorTable objectForKey:to_num(currentBackgroundColorIndex)];
  }
}
- (void)setAnsiString:(NSString *)ansiString
{
  [self setString:@""];
  [self appendAnsiString:ansiString];
}
- (void)appendAnsiString:(NSString *)ansiString
{
  NSString *remainingAnsiString = [ansiString copy];
  while ([remainingAnsiString length] != 0) {
    AGRegexMatch *match =
      [ansiSequenceRegex findInString:remainingAnsiString];
    if (match) {
      // ANSI sequence present, isolate it, change the current color, then
      // continue.
      NSString *ansiSequence = [[match group] copy];
      int ansiSequenceIndex = [match range].location;
      NSString *beforeSequence =
        [remainingAnsiString substringToIndex:ansiSequenceIndex];
      NSString *afterSequence =
        [remainingAnsiString substringFromIndex:
          (ansiSequenceIndex + [match range].length)];
      NSAttributedString *insertionString =
        [[NSAttributedString alloc]
          initWithString: [beforeSequence copy]
              attributes:
                [NSDictionary dictionaryWithObjectsAndKeys:
                  [self font], NSFontAttributeName,
                  [self currentForegroundColor], NSForegroundColorAttributeName,
                  [self currentBackgroundColor], NSBackgroundColorAttributeName,
                  nil]];
      [[self textStorage] appendAttributedString:insertionString];
      [self handleAnsiSequence:ansiSequence];
      remainingAnsiString = [afterSequence copy];
    } else {
      // No ANSI sequence present, just insert the string with the current
      // color.
      NSAttributedString *insertionString =
        [[NSAttributedString alloc]
          initWithString: [remainingAnsiString copy]
              attributes:
                [NSDictionary dictionaryWithObjectsAndKeys:
                  [self font], NSFontAttributeName,
                  [self currentForegroundColor], NSForegroundColorAttributeName,
                  [self currentBackgroundColor], NSBackgroundColorAttributeName,
                  nil]];
      [[self textStorage] appendAttributedString:insertionString];
      remainingAnsiString = @"";
    }
  }
  [[NSRunLoop currentRunLoop]
    performSelector: @selector(didChangeText)
             target: self
           argument: nil
              order: 0
              modes: [NSArray arrayWithObjects: NSDefaultRunLoopMode, nil]];
}
- (void)handleAnsiSequence:(NSString *)ansiSequence
{
  AGRegexMatch *match = [ansiSequenceRegex findInString:ansiSequence];
  NSMutableArray *options = [NSMutableArray array];
  int i;
  for (i = 1; i < [match count]; i++) {
    NSString *capture = [match groupAtIndex:i];
    if (capture) {
      [options addObject:to_num([capture intValue])];
    }
  }
  for (i = 0; i < [options count]; i++) {
    NSNumber *option = [options objectAtIndex:i];
    if ([[ansiColorTable allKeys] containsObject:option]) {
      int index = [option intValue];
      if (index == 0) {
        currentForegroundColorIndex = 30;
        currentBackgroundColorIndex = 47;
      }
      if ((index >= 30) && (index <= 39))
        currentForegroundColorIndex = index;
      if ((index >= 40) && (index <= 49))
        currentBackgroundColorIndex = index;
    }
  }
  [options release];
}
- (void)didChangeText
{
  [super didChangeText];
  NSClipView *clipView = (NSClipView *)[self superview];
  NSScrollView *scrollView = (NSScrollView *)[clipView superview];
  NSPoint proposedScrollPoint = NSMakePoint(0.0, [self frame].size.height);
  NSPoint scrollPoint = [clipView constrainScrollPoint:proposedScrollPoint];
  [clipView scrollToPoint:scrollPoint];
  [scrollView reflectScrolledClipView:clipView];
}
@end
