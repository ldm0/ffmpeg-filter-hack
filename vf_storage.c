/*
 * Copyright (c) 2019-2020 Donough Liu
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Filter that check available storage while doing nothing to packets.
 */

#include "libavutil/imgutils.h"
#include "internal.h"
#include <sys/statvfs.h>

static int query_formats(AVFilterContext *ctx)
{
    AVFilterFormats *formats = NULL;
    int fmt;

    for (fmt = 0; av_pix_fmt_desc_get(fmt); fmt++) {
        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(fmt);
        int ret;
        if (desc->flags & AV_PIX_FMT_FLAG_HWACCEL)
            continue;
        if ((ret = ff_add_format(&formats, fmt)) < 0)
            return ret;
    }

    return ff_set_common_formats(ctx, formats);
}

static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
    AVFilterContext *ctx = inlink->dst;
    AVFilterLink *outlink = ctx->outputs[0];
    struct statvfs storage;
    static int counter = 99;
    float remaining_gigs;

    if (++counter >= 100) {
        // Check every 100 frames
        counter = 0;
        if (statvfs(".", &storage) == -1) {
            return AVERROR(ENFILE); 
        }
        remaining_gigs = (storage.f_bavail * storage.f_bsize) / (1024. * 1024. * 1024.);
        if (remaining_gigs < 0.01) {
            av_log(NULL, AV_LOG_WARNING, "\nRemaining space is less than 10M\n");
        } else {
            av_log(NULL, AV_LOG_INFO, "\nRemaining space: %.4fG\n", remaining_gigs);
        }
    }

    return ff_filter_frame(outlink, in);
}

static const AVFilterPad storage_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_VIDEO,
        .filter_frame = filter_frame,
    },
    { NULL }
};

static const AVFilterPad storage_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO,
    },
    { NULL }
};

AVFilter ff_vf_storage = {
    .name          = "storage",
    .description   = NULL_IF_CONFIG_SMALL("Check if we still have enough disk space to continue."),
    .query_formats = query_formats,
    .inputs        = storage_inputs,
    .outputs       = storage_outputs,
    .flags         = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
};
