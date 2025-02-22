# Raspberry Pi I2C Communication Flow - TMP117 Temperature Sensor

## Overview

This document explains the standard sequence and API usage for communicating with I2C devices (specifically the TMP117 temperature sensor) on a Raspberry Pi using C/C++ system calls.

## Sequence Steps

### 1. Open the I2C Bus Device

**Function:**
```c
int fd = open(const char *pathname, int flags);
```

**Purpose:**
Opens the I2C bus device file (usually `/dev/i2c-1`) for communication.

**Parameters:**
- `pathname`: Path to the I2C device (e.g., `/dev/i2c-1`).
- `flags`: Access mode. Use `O_RDWR` to allow both reading and writing.

**Returns:**
- Non-negative file descriptor (`fd`) on success.
- `-1` on failure; `errno` indicates the error.

**Example:**
```c
int fd = open("/dev/i2c-1", O_RDWR);
if (fd < 0) {
    // Handle error
}
```

---

### 2. Set the I2C Slave Address

**Function:**
```c
int ioctl(int fd, unsigned long request, int arg);
```

**Purpose:**
Configures the target I2C slave device address (e.g., TMP117's address `0x48`).

**Parameters:**
- `fd`: File descriptor returned by `open()`.
- `request`: `I2C_SLAVE` to set the slave address.
- `arg`: 7-bit slave address (e.g., `0x48`).

**Returns:**
- `0` on success.
- `-1` on failure; `errno` indicates the error.

**Example:**
```c
if (ioctl(fd, I2C_SLAVE, 0x48) < 0) {
    // Handle error
}
```

---

### 3. Write to the I2C Device

**Function:**
```c
ssize_t write(int fd, const void *buf, size_t count);
```

**Purpose:**
Sends data to the I2C slave device, typically specifying the register address.

**Parameters:**
- `fd`: File descriptor.
- `buf`: Pointer to data buffer (e.g., register address).
- `count`: Number of bytes to write.

**Returns:**
- Number of bytes written on success.
- `-1` on failure; `errno` indicates the error.

**Example:**
```c
unsigned char reg = 0x00; // Temperature register
if (write(fd, &reg, 1) != 1) {
    // Handle error
}
```

---

### 4. Read from the I2C Device

**Function:**
```c
ssize_t read(int fd, void *buf, size_t count);
```

**Purpose:**
Reads data from the I2C slave device.

**Parameters:**
- `fd`: File descriptor.
- `buf`: Buffer to store received data.
- `count`: Number of bytes to read (2 bytes for TMP117 temperature).

**Returns:**
- Number of bytes read on success.
- `-1` on failure; `errno` indicates the error.

**Example:**
```c
unsigned char buffer[2];
if (read(fd, buffer, 2) != 2) {
    // Handle error
}
```

---

### 5. Close the I2C Bus

**Function:**
```c
int close(int fd);
```

**Purpose:**
Closes the I2C device file descriptor to free system resources.

**Example:**
```c
close(fd);
```

---

## Headers Required

```c
#include <fcntl.h>           // For open()
#include <sys/ioctl.h>       // For ioctl()
#include <unistd.h>          // For read(), write(), close()
#include <linux/i2c-dev.h>   // For I2C_SLAVE
#include <errno.h>           // For errno
```

---

## References

- `man 2 open`
- `man 2 ioctl`
- `man 2 write`
- `man 2 read`
- `man 2 close`

