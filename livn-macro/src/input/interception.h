// interception.h — Interception library header
// Source: https://github.com/oblitum/Interception
// Bu dosya Interception kütüphanesinin C API header'ıdır.

#ifndef _INTERCEPTION_H_
#define _INTERCEPTION_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void *InterceptionContext;
typedef int InterceptionDevice;
typedef int InterceptionPrecedence;
typedef unsigned short InterceptionFilter;

enum InterceptionKeyState {
    INTERCEPTION_KEY_DOWN             = 0x00,
    INTERCEPTION_KEY_UP               = 0x01,
    INTERCEPTION_KEY_E0               = 0x02,
    INTERCEPTION_KEY_E1               = 0x04
};

enum InterceptionMouseState {
    INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN   = 0x001,
    INTERCEPTION_MOUSE_LEFT_BUTTON_UP     = 0x002,
    INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN  = 0x004,
    INTERCEPTION_MOUSE_RIGHT_BUTTON_UP    = 0x008,
    INTERCEPTION_MOUSE_MIDDLE_BUTTON_DOWN = 0x010,
    INTERCEPTION_MOUSE_MIDDLE_BUTTON_UP   = 0x020,
    INTERCEPTION_MOUSE_MOVE              = 0x1000
};

enum InterceptionFilterKeyState {
    INTERCEPTION_FILTER_KEY_DOWN             = 0x01,
    INTERCEPTION_FILTER_KEY_UP               = 0x02,
    INTERCEPTION_FILTER_KEY_ALL              = 0x03
};

enum InterceptionFilterMouseState {
    INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_DOWN   = 0x001,
    INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_UP     = 0x002,
    INTERCEPTION_FILTER_MOUSE_ALL                = 0xFFF
};

typedef struct {
    unsigned short code;
    unsigned short state;
    unsigned int information;
} InterceptionKeyStroke;

typedef struct {
    unsigned short state;
    unsigned short flags;
    short rolling;
    int x;
    int y;
    unsigned int information;
} InterceptionMouseStroke;

typedef char InterceptionStroke[sizeof(InterceptionMouseStroke)];

InterceptionContext __stdcall interception_create_context(void);
void __stdcall interception_destroy_context(InterceptionContext context);
void __stdcall interception_set_filter(InterceptionContext context, int (*predicate)(InterceptionDevice), InterceptionFilter filter);
InterceptionDevice __stdcall interception_wait(InterceptionContext context);
InterceptionDevice __stdcall interception_wait_with_timeout(InterceptionContext context, unsigned long milliseconds);
int __stdcall interception_send(InterceptionContext context, InterceptionDevice device, const InterceptionStroke *stroke, unsigned int nstroke);
int __stdcall interception_receive(InterceptionContext context, InterceptionDevice device, InterceptionStroke *stroke, unsigned int nstroke);
int __stdcall interception_is_keyboard(InterceptionDevice device);
int __stdcall interception_is_mouse(InterceptionDevice device);

#ifdef __cplusplus
}
#endif

#endif
