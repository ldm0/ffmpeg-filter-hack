# FFmpeg filter hack

## Introduction

There are two filter example here:

1. `vf_slackmsg.c`: Send Slack message per 100 frames.
2. `vf_storage.c`: Check storage status per 100 frames, emit warnings when storage is limited.

## Usage

Fetch FFmpeg sources.

Add this to libavfilter/makefile

```makefile
OBJS-$(CONFIG_SLACKMSG_FILTER)               += vf_slackmsg.o
OBJS-$(CONFIG_STORAGE_FILTER)                += vf_storage.o
```

Add this to libavfilter/allfilters.c

```makefile
```c
extern AVFilter ff_vf_slackmsg;
extern AVFilter ff_vf_storage;
```

Then compile.
