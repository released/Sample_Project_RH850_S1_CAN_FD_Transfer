# RH850 CAN FD Custom Transfer

Non-destructive R7F701684 test,500K nominal/2M data/BRS.
Each enabled bus has independent protocol state,statistics and duplicate cache.
The CAN IRQ adapter and build tools are contained in this project.

## Board and protocol

EVM connector CAN0 uses MCU CAN1;connector CAN1 uses MCU CAN4.
X1=16MHz,CPU120MHz,CAN communication clock40MHz.
Per-port RX FIFOs share EIINT23 and feed independent16-slot queues(15usable);OSTM0/EIINT84 ticks1ms.
No main-loop hardware RX polling fallback. UART0 TX P0_2/115200 is optional
bounded main-loop diagnostics;no UART work or protocol parsing in the ISR.

| Function | Wire format |
| --- | --- |
| Discovery/control | Standard710/711,eight-byte FD+BRS frames,Custom Transfer v1 |
| ECHO_FD64 | Command07,DLC15/64bytes,59echo bytes,CRC8-ATM over bytes0..62 |
| Target | 8501,short labelR7F70168;full partR7F701684 |
| Stats | 0requests,1TX success,2lost/drop events,3TX failures,4protocol/invalid,5CRC errors,6duplicates,7warning,8passive,9bus-off entries,10initialization/TX result,11ERFL low16 |

Counters saturate. CLEAR_STATS affects software statistics,not peripheral error
flags or CAN reset. Main-loop error-state observations may miss transitions
shorter than one loop. No Flash erase/write or App BIN is used.
The selected EVM transceiver and complete2Mbit/s physical link require
specification/termination/waveform verification.

## EVM ports and buffer allocation

The EVM target is R7F701684 (100 pins,1 MiB Code Flash),X1=16MHz.
Default enabled ports are MCU CAN1 and CAN4. Set each ENABLE_PORT_CAN0..5
flag to0 or1 in `can_port_config.h`;at least one must be1.
Each project owns its local `can_port_config.h`. Apply compatible port and
communication settings to the Boot and App that will be installed together.

| MCU port | TX / RX routing | EVM connector |
| --- | --- | --- |
| CAN1 | P10_7 Alt4 / P10_6 Alt4 | Board CAN0,CN6 |
| CAN4 | P0_10 Alt5 / P0_9 Alt5 | Board CAN1,CN8 |

The board connector labels are not MCU channel numbers.
BLDC V2:S7_6/5 ON;S7_4/2 ON and S7_3/1 OFF.
ASK V3:J20 1-2/3-4 for CAN1,5-6/7-8 for CAN4.
Use the board manual's S4 termination settings;leave S4_3/4 OFF for separate
buses. Connect only the bus under test to the host,then repeat on the other
bus. The Tool bridge-port selector selects a host port,not a device port.

Each enabled MCU channel owns RX FIFO[channel] and a16-slot RAM queue
(15usable),with independent counters and source-channel reply routing.
TX uses buffer0 local to that channel (status index channel*32).
There are no RX Message Buffers or common TX/RX FIFOs.
FD RX allocation is1216bytes per port including hardware message headers;
default two ports use2432bytes,all six7296bytes,within the21504-byte shared
RX/FIFO budget (hardware manual24.10.1.5). Classical uses320bytes per port.
CPU RAM queues are separate from CAN message RAM.
Disabled ports allocate neither queues nor active filters.
CAN0/2/3/5 require board-level routing and compatible external transceivers.

- [BLDC V2 manual](https://www.renesas.com/en/document/mat/rh850f1km-s1-bldc-starter-kit-v2-users-manual)
- [ASK V3 manual](https://www.renesas.com/en/document/mat/rh850f1km-s1-starter-kit-v3-user-manual-hardware)

## Build and operation

CS+ CC/CC-RH plus Windows PowerShell;no Python firmware runtime.
From this project directory:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .uild.ps1
```

After Prepare,CS+ Rebuild invokes the same pre/post guards.
Program `DefaultBuild/Sample_Project_RH850_S1_CAN_FD_Transfer.mot` using a
programmer. This standalone image starts at zero.
In Tool:Open + Probe Host → CAN FD/500K/2M/Custom Transfer → Discover Test
Target → Transfer Test. No update BIN is needed.

`can_transfer_protocol.c/.h` owns CRC/commands/replay and all64echo bytes.
`custom_func.c` owns trace/statistics/TX outside the ISR.
The local `hal/can/device_can.c` owns the IRQ queue adapter.
The independent full512-entry table is generated under `generated_projection`.

The paired Boot supplies `hal/can`,whose service/driver match standalone
MultiCHv4. EVM684/16MHz/CAN1+CAN4,exact protocol filters and FD+BRS checks remain
owned by this project family. Complete frames are read from IRQ-produced RAM
queues;the foreground sends replies and consumes matching TX completion events.
Queue admission is not TX success. Timeout requests one abort;an unconfirmed
result latches that bus's fault,without automatic restart or replay.
Explicit channel stop/recover and tag cancellation are C APIs,not extra Tool
commands. Settle the transaction/save diagnostics first. They do not change
other buses or the protocol's per-port state. Error flags remain retained.

## Hardware requirements and guides

- Linked-image guards check the generated MOT and vector layout.
- Physical CAN, IRQ and timing behavior depends on the fitted transceiver, termination and board configuration; software models do not establish electrical acceptance.

- [Custom Transfer operation](https://released.github.io/slide_simple_renesas_isp_tool/#rh850-classical-transfer)
- [RH850 engineering](https://released.github.io/slide_boot_loader_for_RL78/#rh850-can-device)
- [Hardware manual](https://www.renesas.com/en/document/mah/rh850f1km-rh850f1kh-users-manual-hardware)

## User code entry

`custom_func.c` provides `hardware_init()` for initialization and `loop()` for
foreground service. Add application initialization and foreground work through
these callbacks, retaining the existing communication service calls.
The selected main/startup adapter owns entry and handoff. Generated peripheral
files remain under `src/`; retain the project's MTPJ source selection when using
Code Generator or Smart Configurator, then rebuild to validate the image and vectors.

## Project layout and standalone build

This project includes its own source files, configuration headers and build tools.
A sibling Boot, App or Transfer checkout is not required to compile it.
Keep the project directory name aligned with its `.mtpj` filename; remove a ZIP download suffix when necessary.
Build with CS+ Rebuild, or run the local PowerShell entry:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

| Location | Responsibility |
| --- | --- |
| `custom_func.c` | Project initialization in `hardware_init()` and recurring work in `loop()`; runtime orchestration stays here |
| `hal/can` | CAN service, controller driver and CAN adapter, when used by this project |
| `hal/device` | Device, clock, peripheral and CPU handoff APIs |
| `hal/image` | Metadata parser/profile, image validation and software CRC |
| `hal/flash` | Flash access/geometry APIs, when required by the selected image |
| `hal/isp` | Update/Transfer protocol and transport processing APIs |
| Root configuration headers | Local MCU/Flash, handoff and communication contract |
| `tools` and root PowerShell helpers | Local metadata/IRQ generation and linked-image checks |
| `src` | Code Generator or Smart Configurator peripheral sources |

Only the sources registered in this project's CS+ configuration are linked.
Generated main/vector implementations must not become a second active entry.
Retain the `custom_func.c` callbacks and the selected startup adapter after code generation.

Boot and App each carry a local contract. Keep their MCU, Flash layout, image format,
IRQ ABI, handoff and communication settings compatible when installing a pair.
App releases are edited in `app_image_config.h` and `app_irq_handlers.h` in the App project.
Rebuild the selected App to regenerate its metadata, vector table, update BIN and timing BINs.
The App build does not read or rewrite a Boot checkout.

## Diagnostic text output

The project-local `retarget_config.h` selects the formatter at compile time:

```c
#define TINY_PRINTF_FULL 1 /* 1: Full C99-format mode; 0: Current mode. */
```

Rebuild after changing the mode. Full is the default and supports C99 integer,
string/character, pointer, `f/F/e/E/g/G/a/A`, `n`, `hh/h/l/ll/j/z/t/L`, flags,
numeric/dynamic width and precision. Current retains the existing format subset,
finite rounding and attempted-character counts. Both retain streamed precision
and the project's finite UART0 wait; ISP data uses its independent interface.

```c
#include "retarget.h"
/* Foreground, after UART0 initialization. Use types matching each format. */
tiny_printf("counter=%lu status=0x%08lX\r\n", counter, status);
tiny_printf("voltage=%*.*f size=%zu\r\n", 12, 3, voltage, size);
```

Full uses fixed C locale and nearest/ties-to-even rounding. Its exact decimal
conversion does not delegate to the vendor `printf`. Double/long-double precision
follows the selected CC-RH `-Xdbl_size` setting (both types must match); extra
printed decimals do not add floating-point precision. `%lc`/`%ls` accept C-locale
ASCII wide characters; unrepresentable characters return a negative result.
Ordinary `%s` copies encoded bytes. `%p` uses hexadecimal `0x...`, including
`0x0` for null; pointer notation is implementation-defined by the C standard.

Full returns the count accepted by UART, or a negative result on format/count/
encoding errors or UART timeout. This is not receiver/wire acknowledgement.
`SendChar()` returns the accepted unsigned byte or -1 on its existing wait bound.
Current mode ignores the hook error and retains attempted-output counts.
Format in foreground code, not in an ISR or Flash P/E path.

Format examples and the mode table are described in the
[RH850 Log guide](https://released.github.io/slide_boot_loader_for_RH850/app-development.html#rh850-runtime-log).

## UART0 local reset

- Diagnostic UART0 uses115200,8-N-1. Terminal TX connects to device UART0 RX with a common ground.
- Sending `z` or `Z` requests a normal MCU reset from the foreground loop.
- App restart follows the existing Boot/App validation flow. Tool Enter Boot remains the separate update-mode operation.
