#ifndef SDLI_UTIL_H
#define SDLI_UTIL_H

#include <vuid.h>

//
// macros & constants
//

#define UNUSED_4(arg, ...) (void)arg
#define UNUSED_3(arg, ...) (void)arg, UNUSED_4(__VA_ARGS__, 0)
#define UNUSED_2(arg, ...) (void)arg, UNUSED_3(__VA_ARGS__, 0)
#define UNUSED_1(arg, ...) (void)arg, UNUSED_2(__VA_ARGS__, 0)
#define UNUSED(...) (UNUSED_1(__VA_ARGS__, 0))

#define Box v_box
#define Text v_txt
#define Image v_img

//
// types
//

// Navigator transition direction.
typedef enum NavigationDirection {
  NAV_FORWARD,
  NAV_BACKWARD,
} NavigationDirection;

typedef VNode* (*NavigableCreate)(const char*);
typedef void (*NavigableEnter)(VNode*, NavigationDirection);
typedef void (*NavigableLeave)(VNode*, NavigationDirection);

// Registration metadata for a Page or Screen node.
typedef struct Navigable {
  const char* id;
  NavigableCreate create;
  NavigableEnter enter;
  NavigableLeave leave;
} Navigable;

// A simple navigator that manages a stack of navigables (e.g. pages or screens)
// within a container node. The navigator supports transition directions,
// forward and backward, which can be used for the navigable to setup and
// cleanup and, in the future, support transition animations. Navigables must be
// registered with the navigator before they can be pushed to the stack.
typedef struct Navigator {
  const char* container_id;
  Navigable navigables[16];
  size_t navigables_size;
} Navigator;

//
// public functions
//

void Navigator_Init(Navigator* navigator, const char* container_id);
void Navigator_Drop(Navigator* navigator);
void Navigator_Register(Navigator* navigator,
                        const char* id,
                        NavigableCreate create,
                        NavigableEnter enter,
                        NavigableLeave leave);
// from is a guard preventing pushing the same navigable multiple times.
void Navigator_Push(Navigator* navigator, const char* from, const char* to);
// from is a guard preventing popping the same navigable multiple times.
void Navigator_Pop(Navigator* navigator, const char* from);

#endif  // SDLI_UTIL_H
