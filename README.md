
# trdm

A simple [Device Mapper][dm] for educational purposes (*still WIP*).

This *device mapper* applies the same bitmask XOR to each byte.

[dm]: https://en.wikipedia.org/wiki/Device_mapper

## To Use

(Inside the VM.)

First create and run:

```bash
#!/bin/bash
N=15
BS=1024
I=/dev/zero
O=/tmp/dada

for i in $(seq 1 $N); do
  h=$(printf '\\%o' $(($i * 16 + $i)))
  dd if=$I bs=$BS count=1 status=none | tr '\0' $h >> $O
done
```

This script creates a file where each 1024-byte block is filled with a specific
value. Try `hexdump -C /tmp/dada`.

```txt
/tmp/dada
00000000: 11 11 11 11 11 11 11 11 | ........
00000400: 22 22 22 22 22 22 22 22 | """"""""
00000800: 33 33 33 33 33 33 33 33 | 33333333
00000c00: 44 44 44 44 44 44 44 44 | DDDDDDDD
00001000: 55 55 55 55 55 55 55 55 | UUUUUUUU
00001400: 66 66 66 66 66 66 66 66 | ffffffff
00001800: 77 77 77 77 77 77 77 77 | wwwwwwww
00001c00: 88 88 88 88 88 88 88 88 | ........
00002000: 99 99 99 99 99 99 99 99 | ........
00002400: aa aa aa aa aa aa aa aa | ........
00002800: bb bb bb bb bb bb bb bb | ........
00002c00: cc cc cc cc cc cc cc cc | ........
00003000: dd dd dd dd dd dd dd dd | ........
00003400: ee ee ee ee ee ee ee ee | ........
00003800: ff ff ff ff ff ff ff ff | ........
```

Now compile and add the *device mapper*:

```sh
cd shared0

make
make install

losetup --show --find /tmp/dada
echo "0 4 trdm /dev/loop0 4 17" | dmsetup create tr
yes A | tr -d \\n | head -c 3 | dd of=/dev/mapper/tr
```

These commands create a device mapper spanning **4** sectors (`0 4`), with an
offset of **4** sectors inside **/dev/loop0**, and apply a bitmask XOR of
**0x11** (`/dev/loop0 4 17`). It then writes 3 bytes **A**, each of which is
bitmasked with **0x11**, resulting in three **P** bytes (`0x41 ^ 0x11 = 0x50`).

```txt
/dev/mapper/tr
00000000: 41 41 41 22 22 22 22 22 | AAA"""""
00000400: 55 55 55 55 55 55 55 55 | UUUUUUUU
```

```txt
/tmp/dada (or /dev/loop0)
00000000: 11 11 11 11 11 11 11 11 | ........
00000400: 22 22 22 22 22 22 22 22 | """"""""
00000800: 50 50 50 33 33 33 33 33 | PPP33333
00000c00: 44 44 44 44 44 44 44 44 | DDDDDDDD
00001000: 55 55 55 55 55 55 55 55 | UUUUUUUU
00001400: 66 66 66 66 66 66 66 66 | ffffffff
00001800: 77 77 77 77 77 77 77 77 | wwwwwwww
00001c00: 88 88 88 88 88 88 88 88 | ........
00002000: 99 99 99 99 99 99 99 99 | ........
00002400: aa aa aa aa aa aa aa aa | ........
00002800: bb bb bb bb bb bb bb bb | ........
00002c00: cc cc cc cc cc cc cc cc | ........
00003000: dd dd dd dd dd dd dd dd | ........
00003400: ee ee ee ee ee ee ee ee | ........
00003800: ff ff ff ff ff ff ff ff | ........
```
