# melonframe

A small, dependency-free C11 library for framing and de-framing byte streams
with a fixed, self-synchronizing packet format. It is intended for use over
byte-oriented transports (serial ports, sockets, pipes) where messages need to
be split out of a continuous stream of bytes, with corruption detection via a
CRC-16 checksum.

Builds as a shared library (`.dll` on Windows, `.so` on Linux) and exposes its
public API for use from other executables/processes, in both C and C++.

## Packet format

All multi-byte fields are big-endian (network byte order):

```
+--------+--------+--------+--------+------------------+--------+--------+
|  0xAA  |  0x55  | SIZE_H | SIZE_L | DATA (N bytes)   | CRC_H  | CRC_L  |
+--------+--------+--------+--------+------------------+--------+--------+
```

- **Header** (2 bytes): fixed magic `0xAA 0x55`.
- **Size** (2 bytes): length of `DATA`.
- **Data** (N bytes): the caller-supplied payload.
- **CRC** (2 bytes): CRC-16 computed over `[Header | Size | Data]`.

## API overview

Declared in [`melonframe.h`](melonframe.h):

- `melonframe_get_size_for_encoded` — compute the encoded size for a given
  payload size.
- `melonframe_encode` — encode a payload into a packet buffer.
- `melonframe_decoder_init` / `melonframe_decoder_free` / `melonframe_decoder_reset`
  — manage a streaming decoder instance.
- `melonframe_decoder_process_byte` — feed one byte at a time into the decoder;
  invokes a user-supplied callback (`melonframe_decoder_event_handler_t`) when
  a full packet is decoded, out of sync, or fails CRC.

All functions return a `melonframe_result_t` status code (`MELONFRAME_OK` on
success). The decoder is buffer-based: the caller owns and allocates the
`melonframe_buffer_t` backing storage; the library never allocates memory.

## Building

Requires CMake and a C11 compiler.

```sh
cmake -S . -B build
cmake --build build
```

This produces the `melonframe` shared library (`melonframe.dll` on Windows,
`libmelonframe.so` on Linux), which exports its public API automatically
(`__declspec(dllexport)` on Windows, default visibility on Linux/GCC/Clang) —
no additional build configuration is needed to call it from another program.

## Testing

Tests are registered with CTest:

```sh
ctest --test-dir build --output-on-failure
```

- `TestEncoding` — verifies `melonframe_encode` output against known bytes.
- `TestDecoding` — round-trips a large number of encoded packets through the
  streaming decoder and byte-for-byte compares the result.

## License

MIT — see [LICENSE](LICENSE).