# Sensor-Station

`Sensor-Station` is an embedded project based on `STM32L476`, built as a hands-on environment for improving firmware development skills, practicing work with MCU peripherals, and structuring code in a more maintainable, modular way.

This repository is primarily a learning and showcase project. Its main goal is not to ship a finished product, but to create a solid space for iterative development, experimentation, and deliberate skill-building in embedded systems.

## Goals

This project combines two roles: a learning playground and a public record of how I approach embedded development. It is focused on strengthening practical engineering skills such as:

- designing clear module boundaries and responsibilities,
- integrating sensors and hardware interfaces,
- building a simple application layer on top of HAL,
- working with interrupts, DMA, and asynchronous communication,
- keeping the repository and build setup clean and maintainable.

It is intentionally iterative: some parts are complete, some are experimental, and some are placeholders for the next steps.

## What The Project Does

At the moment, the firmware reads environmental data from a `BMP280`, exposes measurements through a UART console interface, and includes early support for an encoder-driven OLED user interface.

## Architecture

```text
APP
├── LOOP
├── BMP280
├── CONSOLE
├── DISPLAY
└── ENCODER
```

The firmware is organized around a small application layer that coordinates independent modules for scheduling, sensing, console output, display handling, and user input.

## Hardware Resources

The codebase currently uses or prepares support for:

- `STM32L476` microcontroller,
- `BMP280` sensor connected over `SPI3`,
- diagnostic console over `USART2` with `DMA` used for transmit,
- rotary encoder handled by `TIM4` in encoder mode,
- periodic application loop based on `TIM6`,
- prepared support for an `SSD1306` display.

## Tooling

The project is built around:

- `STM32CubeMX` for hardware configuration,
- `STM32 HAL`,
- `CMake` + `Ninja`,
- `gcc-arm-none-eabi` toolchain,
- `clangd` for editor integration.

## Third-Party Code

The project currently uses the third-party [`afiskon/stm32-ssd1306`](https://github.com/afiskon/stm32-ssd1306) OLED display library located in `libs/ssd1306/`.

This display layer is treated as an experimental UI component. One of the planned improvements is to adapt or replace the blocking display update path with a non-blocking implementation better suited for the timer-driven application loop.

## Build

The repository includes ready-to-use `CMake` presets:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

`Release` presets are also available.

To build the project, you will need at least:

- `CMake`,
- `Ninja`,
- `arm-none-eabi-gcc`.

## Design Direction

The project currently follows a simple timer-driven superloop architecture.
The long-term goal is to evolve selected components toward non-blocking and RTOS-friendly designs while keeping the codebase modular and easy to reason about.

## Current Features

- [x] Timer-driven main loop
- [x] UART console
- [x] BMP280 over SPI
- [x] Encoder support
- [ ] HC-SR04 support
- [ ] MPU6050 integration
- [ ] Non-blocking OLED refresh
- [ ] FreeRTOS-based branch

## License

This project is available under the terms described in [LICENSE](LICENSE).
