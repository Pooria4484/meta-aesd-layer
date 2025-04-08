#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif
#include <linux/slab.h> 
#include "aesd-circular-buffer.h"

/**
 * @brief Searches the circular buffer for a specific character offset and returns the buffer entry containing it.
 * 
 * @param buffer Pointer to the circular buffer.
 * @param char_offset Offset to locate, assuming all strings in the buffer are concatenated.
 * @param entry_offset_byte_rtn Output: offset within the found buffer entry where the character is located.
 * @return Pointer to the buffer entry that contains the char_offset, or NULL if not found.
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    size_t i;
    size_t cur_offset = 0;
    size_t index;

    // Total valid entries
    size_t entry_count = buffer->full ? AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED : buffer->in_offs;

    for (i = 0; i < entry_count; i++) {
        // Convert circular index to linear buffer index
        index = (buffer->out_offs + i) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;

        if (char_offset < (cur_offset + buffer->entry[index].size)) {
            *entry_offset_byte_rtn = char_offset - cur_offset;
            return &buffer->entry[index];
        }

        cur_offset += buffer->entry[index].size;
    }

    return NULL; // Offset out of bounds
}

/**
 * @brief Adds an entry to the circular buffer. If full, overwrites the oldest entry.
 * 
 * @param buffer Pointer to the circular buffer.
 * @param add_entry The entry to add to the buffer.
 */
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    // Overwrite the current in_offs index with the new entry
    if (buffer->full) {
        struct aesd_buffer_entry *old = &buffer->entry[buffer->out_offs];
        kfree(old->buffptr);
    }
    
    buffer->entry[buffer->in_offs] = *add_entry;

    // If buffer is full, advance out_offs to remove the oldest entry
    if (buffer->full) {
        buffer->out_offs = (buffer->out_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    }

    // Advance in_offs to the next position
    buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;

    // Mark buffer full if in_offs wraps around to out_offs
    if (buffer->in_offs == buffer->out_offs) {
        buffer->full = true;
    }
}

/**
 * @brief Initializes the circular buffer to an empty state.
 * 
 * @param buffer Pointer to the circular buffer to initialize.
 */
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer, 0, sizeof(struct aesd_circular_buffer));
}
