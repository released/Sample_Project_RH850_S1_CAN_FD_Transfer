
## Local source ownership

Each Boot/App/Transfer owns all sources, headers and build tools needed to compile.
No sibling checkout, absolute source path or parent-directory include is permitted.
Use hal/can, hal/device, hal/image, hal/flash and hal/isp by function; do not add
common or version-number directories. Root headers remain local editable contracts.
Project orchestration belongs in custom_func.c hardware_init()/loop(), not a
separate runtime entry source. Preserve role-specific ordering and SC bindings.
Keep local Boot/App contracts compatible; App-only builds cannot modify Boot.

## Diagnostic console

Use the existing project-local retarget.c/.h tiny_printf directly for foreground
text and full original format support. retarget.c retains C99/long long and
float support; other handwritten modules retain C90 style. Do not strip formats.
SendChar is its sole bounded UART0 output hook.
Keep protocol data separate; no printf/formatting in ISR or Flash P/E paths.
Do not reintroduce Device_Log/Device_LogHex/UartChar or manual numeric printers.
