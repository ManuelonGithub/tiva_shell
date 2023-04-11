

#include <string.h>
#include <stdbool.h>

#include "shell_cfg.h"
#include "shell_history.h"

typedef struct {
    char buf[CMD_HIST_BUF];
    size_t wrPtr;

    size_t cmdIndexes[MAX_CMDS];
    size_t oldestIdx, nextIdx;
} shell_hist_ctx_t;

static shell_hist_ctx_t ctx;

static size_t hist_offset_to_index(size_t offset)
{
    // get the command index by subtracting nextIdx by offset+1
    // nextIdx tracks the index AFTER the newest command, thus the +1
    // & subtraction is bounded within MAX_CMDS
    int i = ctx.nextIdx-offset-1;
    if (i < 0) {
        i = MAX_CMDS+i;
    }

    // Return the history buffer index corresponding to the history offset
    return ctx.cmdIndexes[i];
}

static bool is_hist_empty()
{
    return (ctx.nextIdx == ctx.oldestIdx);
}

static size_t inc_cmdIdx(size_t idx)
{
    return (idx+1) % MAX_CMDS;
}

// Check if we increase wrPtr by +len would collide with an existing command
static int check_new_cmd_len_collides(size_t len)
{
    return  !is_hist_empty() &&
            (ctx.wrPtr <= ctx.cmdIndexes[ctx.oldestIdx]) &&
            (ctx.wrPtr+len >= ctx.cmdIndexes[ctx.oldestIdx]);
}


void hist_init()
{
    ctx.wrPtr = 0;
    ctx.oldestIdx = 0;
    ctx.nextIdx = 0;
}

unsigned int hist_count()
{
    if (ctx.oldestIdx > ctx.nextIdx) {
        return ctx.nextIdx + (MAX_CMDS-ctx.oldestIdx);
    }
    else {
        return ctx.nextIdx-ctx.oldestIdx;
    }
}

const char* hist_cmd_by_offset(unsigned int offset)
{
    // don't event try if the offset is out-of-bounds
    if (offset >= hist_count()) {
        return NULL;
    }

    return ctx.buf+hist_offset_to_index(offset);
}

void hist_add_entry(const char* cmd)
{
    bool same_as_prev_cmd = true;

    const char* prev_cmd = hist_cmd_by_offset(0);

    bool can_pre_write = true;
    size_t pre_write_cnt = 0;

    size_t len = 0;

    if (is_hist_empty()) {
        len = strlen(cmd);
        memcpy(ctx.buf+ctx.wrPtr, cmd, len);
        same_as_prev_cmd = false;
    }
    else {
        /*
         *  There are many parallel checks&operations that can be performed on a byte-by-byte basis:
         *      1. Checking if new cmd == newest stored cmd
         *      2. checking if the new cmd length will exceed the buffer limit
         *      3. checking if the new cmd length will collide with an existing stored cmd
         *      4. Writing the new command onto the history buffer
         *      5. Get the length of the command
         *  So we iterate through the cmd "manually" so all these checks&operations are performed in one go!
         */
        while (cmd[len] != '\0') {
            // keep track whether or not current command is the same as the newest cmd in history
            if (same_as_prev_cmd && (cmd[len] != prev_cmd[len])) {
                same_as_prev_cmd = false;
            }

            /*
             * 'pre-write' the current char into the buffer
             * It's 'pre-writing' because there's a chance we'll need to discard
             * these writes in case the new command is the same as the newest command
             */
            if (can_pre_write) {
                ctx.buf[ctx.wrPtr+len] = cmd[len];

                pre_write_cnt++;
                // Stop writing if we've hit the end of the hist buffer
                // Have to stop because we'll collide with an existing cmd which we can't overwrite yet
                if ((ctx.wrPtr+pre_write_cnt) >= CMD_HIST_BUF) {
                    can_pre_write = false;
                    pre_write_cnt = 0;
                }
                // Stop writing if we've collided with an existing cmd which we can't overwrite yet
                else if (check_new_cmd_len_collides(pre_write_cnt)) {
                    can_pre_write = false;
                }
            }

            len++;
        }
    }

    if (!same_as_prev_cmd) {
        if (!can_pre_write) {
            /*
             * Need to check if we've overflown the buffer again b/c
             * we might've stopped the pre-writing due to cmd collision,
             * but now with the cmd length known we may also reach the end of the hist buffer
             */
            if ((ctx.wrPtr+len) >= CMD_HIST_BUF) {
                /*
                 * Need to perform this check here in case there are valid commands
                 * in front of wrPtr prior to resetting it.
                 * Not doing this could cause other commands to be overwritten
                 * without altering the indexing system
                 */
                while (check_new_cmd_len_collides(len)) {
                    ctx.oldestIdx = inc_cmdIdx(ctx.oldestIdx);
                }
                ctx.wrPtr = 0;
            }

            // Check if the 'full-write' of the command will collide with existing commands
            // in which case they are discarded from the indexing system
            while (check_new_cmd_len_collides(len)) {
                ctx.oldestIdx = inc_cmdIdx(ctx.oldestIdx);
            }

            // write the remaining command onto the history buffer
            memcpy(ctx.buf+ctx.wrPtr+pre_write_cnt, cmd+pre_write_cnt, (len+1-pre_write_cnt));
        }
        else {
            // Gotta write that null-terminator!
            ctx.buf[ctx.wrPtr+len] = cmd[len];
        }

        ctx.cmdIndexes[ctx.nextIdx] = ctx.wrPtr;
        ctx.wrPtr += len+1;
        ctx.nextIdx = inc_cmdIdx(ctx.nextIdx);

        if (is_hist_empty()) {
            ctx.oldestIdx = inc_cmdIdx(ctx.oldestIdx);
        }
    }
}

