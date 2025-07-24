# XCEP

**A lightweight, header-only C99 exception library — thread-safe, cross-platform, no heap usage, fully nestable and customizable.**

## Features

- 🚀 **Header-only**: Single file inclusion
- 🧵 **Thread-safe**: Built-in thread-local storage support
- 🌐 **Cross-platform**: Works on Windows, Linux, macOS, and other platforms
- 💾 **Zero heap allocation**: Uses stack-based exception frames
- 🔄 **Fully nestable**: Support for nested try-catch blocks
- ⚙️ **Customizable**: Configurable features and exception handlers
- 📝 **C99 compliant**: Works with standard C99 compilers

## Quick Start

### Basic Usage

```c
#define XCEP_IMPLEMENTATION
#include "XCEP.h"
#include <stdio.h>

int main() {
    Try {
        printf("About to throw an exception\n");
        Throw(42, "Something went wrong");
        printf("This won't be printed\n");
    }
    Catch(42) {
        printf("Caught exception: %s (code: %d)\n", 
               CaughtException.message, CaughtException.code);
    }
    Finally {
        printf("Cleanup code here\n");
    }
    EndTry;
    
    return 0;
}
```


### Nested Exceptions

```c
Try {
    printf("Outer try block\n");
    
    Try {
        printf("Inner try block\n");
        Throw(100, "Inner exception");
    }
    Catch(100) {
        printf("Caught in inner: %s\n", CaughtException.message);
        Throw(200, "Re-throwing as different exception");
    }
    EndTry;
}
Catch(200) {
    printf("Caught in outer: %s\n", CaughtException.message);
}
EndTry;
```


### Exception Information

```c
Try {
    Throw(404, "Resource not found");
}
CatchAll {
    printf("Exception details:\n");
    printf("  Code: %d\n", CaughtException.code);
    printf("  Message: %s\n", CaughtException.message);
    printf("  Function: %s\n", CaughtException.function);
    printf("  File: %s\n", CaughtException.file);
    printf("  Line: %d\n", CaughtException.line);
}
EndTry;
```


## API Reference

### Core Macros

| Macro                  | Description                           |
|------------------------|---------------------------------------|
| `Try`                  | Begin a try block                     |
| `Catch(code)`          | Catch specific exception code         |
| `CatchAll`             | Catch any exception                   |
| `Finally`              | Execute code regardless of exceptions |
| `EndTry`               | End the try-catch block               |
| `Throw(code, message)` | Throws an exception. `message` must be a **C string literal**. |
| `Rethrow`              | Re-throw the current exception        |

### Exception Information

- `CaughtException.code` - Exception error code
- `CaughtException.message` - Exception message string
- `CaughtException.function` - Function where exception was thrown
- `CaughtException.file` - Source file where exception was thrown
- `CaughtException.line` - Line number where exception was thrown

### Exception Handlers

```c
// Global uncaught exception handler
SetUncaughtExceptionHandler(XCEPTEST_handler);

// Thread-specific uncaught exception handler (if thread-safe mode enabled)
SetThreadUncaughtExceptionHandler(XCEPTEST_thread_handler);
```


### Utility Functions

```c
// Print formatted exception information
PrintException("Error occurred", &exception);
```


## Configuration

Configure XCEP by editing macros in the header:

```c
// Enable/disable thread safety if not needed (default: 1)
#define XCEP_CONF_ENABLE_THREAD_SAFE 1

// Enable/disable short command names to avoid collision (default: 1)
#define XCEP_CONF_ENABLE_SHORT_COMMANDS 1
```


### Thread Safety

When `XCEP_CONF_ENABLE_THREAD_SAFE` is enabled:
- Exception stacks are thread-local
- Each thread maintains its own exception context
- Thread-specific uncaught exception handlers are available

### Short Commands

When `XCEP_CONF_ENABLE_SHORT_COMMANDS` is disabled, use prefixed versions:
- `XCEP_Try` instead of `Try`
- `XCEP_Catch(code)` instead of `Catch(code)`
- `XCEP_Throw(code, msg)` instead of `Throw(code, msg)`
- etc.

## Advanced Usage

### Custom Uncaught Exception Handler

```c
void XCEPTEST_exception_handler(const XCEP_t_Exception* ex) {
    fprintf(stderr, "FATAL: Unhandled exception %d: %s\n", 
            ex->code, ex->message);
    fprintf(stderr, "  at %s (%s:%d)\n", 
            ex->function, ex->file, ex->line);
    exit(ex->code);
}

int main() {
    SetUncaughtExceptionHandler(XCEPTEST_exception_handler);
    
    // This will trigger the custom handler
    Throw(1, "Unhandled exception");
    
    return 0;
}
```


### Exception Propagation

```c
void risky_function() {
    Try {
        Throw(500, "Database error");
    }
    Catch(500) {
        printf("Handling database error\n");
        Rethrow; // Propagate to caller
    }
    EndTry;
}

int main() {
    Try {
        risky_function();
    }
    Catch(500) {
        printf("Caught propagated exception: %s\n", CaughtException.message);
    }
    EndTry;
    
    return 0;
}
```


## Platform Support

XCEP automatically detects the compiler and platform to use appropriate thread-local storage:

- **MSVC**: `__declspec(thread)`
- **GCC/Clang with C11**: `_Thread_local`
- **GCC/Clang (older)**: `__thread`
- **C23**: `thread_local`

## Error Handling

- Calling `Rethrow` outside a catch block will trigger an assertion
- Uncaught exceptions will call the registered handler or exit with the exception code
- All exception frames are stack-allocated with no heap usage

## Integration

Simply copy `XCEP.h` to your project and:

```c
#define XCEP_IMPLEMENTATION
#include "XCEP.h"
```

The implementation should only be included once in your project.

## License

MIT license, See the included license in XCEP.h for more information.

---

**Note**: XCEP uses `setjmp`/`longjmp` internally. Be aware of the standard limitations regarding local variables and optimization when using this library. Always use `volatile` for variables that are modified in blocks and accessed after exceptions. `Try`
