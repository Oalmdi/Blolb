#include "blobify.h"


blb_block_t *blb_block_create(uint32_t size){
    if(size == 0 || (size > BLB_BLOCK_MAX_SIZE)) return NULL;

    blb_block_t blk = (blb_block_t)malloc(sizeof(blb_block_t));

    if(blk){
        blk->base = (uint8_t*)malloc(size);
        if(!blk->base){
            free(blk);
            return NULL;
        }
        blk->size = size;
        blk->allocated = true;

        memset(blk->base, 0, blk->size);
        return blk;
    }

    return NULL;
}

void blb_block_delete(blb_block_t *block){
    if(block && block->allocated){
        if(block->base){
            free(block->base);
        }
        free(block);
    }
}

bool blb_block_put(blb_block_t *block, int32_t offset, uint8_t value){
    if((!block) || (!block->base) || (offset < 0) || (offset >= block->size)){
        return false;
    }
    block->base[offset] = value;
    return true;
}

bool blb_block_get(blb_block_t *block, int32_t offset, uint8_t *value){
     if((!block) || (!block->base) || (!value) || (offset < 0) || (offset >= block->size)){
        return false;
    }
    *value = block->base[offset];
    return true;
}

void blb_block_print(blb_block_t *block, FILE *output){
    if((!block) || (!block->base) || (!output)){
        return;
    }

    for(int32_t i = 0; i < block->size; i++){
        fprintf(output, "%d: %02X\n", i, block->base[i]);
    }
}

blb_range_t *blb_range_create(uint32_t start, uint32_t size, uint8_t step, bool fixed){
    blb_range_t range = (blb_range_t)malloc(sizeof(blb_range_t));
    if(!range) return NULL;

    range->start = start;
    range->size = size;
    range->step = step;
    range->fixed = fixed;

    return range;
}

void blb_range_delete(blb_range_t *range){
    if(range) free(range);
}

bool blb_range_slide(blb_range_t *range, int32_t steps){
    if((!range) || (steps == 0) || (range->fixed)) return false;

    int64_t location = range->start + steps;
    if((location < 0) /*|| location >= UINT32_MAX */){
        return false;
    }

    range->start = (uint32_t)location;
    return true;
}

bool blb_range_resize(blb_range_t *range, int32_t size){
    if((!range) || (size == 0) || (range->fixed)) return false;

    int64_t new_size = range->size + size;
    if((new_size < 0) /*|| new_size >= UINT32_MAX */){
        return false;
    }

    range->size = (uint32_t)new_size;
    return true;
}

bool blb_range_in(blb_range_t *range, int32_t value){
    if((range)){
        uint32_t end = range->start + range->size;
        return (value >= range->start) && (value < end);
    }
    return false;
}

blb_cursor_t *blb_cursor_create(int32_t offset, bool fixed){
    blb_cursor_t cursor = (blb_cursor_t)malloc(sizeof(blb_cursor_t));
    if(!cursor) return NULL;

    cursor->offset = offset;
    cursor->fixed = fixed;
    return cursor;
}

void blb_cursor_delete(blb_cursor_t *cursor){
    if(cursor) free(cursor);
}

bool blb_cursor_step(blb_cursor_t *cursor, int32_t step){
    if((!cursor) || (step == 0) || (cursor->fixed)) return false;

    cursor->offset += step;
    return true;
}

bool blb_cursor_jump(blb_cursor_t *cursor, uint32_t value){
    if((!cursor) || (cursor->fixed)) return false;

    cursor->offset = value;
    return true;
}

blb_blob_t *blb_blob_create(uint32_t size, uint8_t step){
    blb_blob_t blob = (blb_blob_t)malloc(sizeof(blb_blob_t));
    if(blob){
        blob->block = blb_block_create(size);
        if(blob->block){
            blob->range = blb_range_create(0, size, step, true);
            if(blob->range){
                blob->cursor = blb_cursor_create(-1, true);
                if(blob->cursor) {
                    return blob;
                }
                blb_range_delete(blob->range);
            }
            blb_block_delete(blob->block);
        }
        blb_blob_delete(blob);
    }
    return NULL;
}
void blb_blob_delete(blb_blob_t *blob){
    if(blob){
        if(blob->block) blb_block_delete(blob->block);
        if(blob->range) blb_range_delete(blob->range);
        if(blob->cursor) blb_cursor_delete(blob->cursor);
    }
}

bool blb_blob_step(blb_blob_t *blob, int32_t step){
    if(blob && blb_range_in(blob->range, (blob->range->start + step))){
        return blb_cursor_step(blob->cursor, step);
    }
    return false;
}


int main3(void){
    blb_cursor_t *c = blb_cursor_create(7, false);
    if(c){
        printf("offset: %d fixed: %d\n",
            c->offset, c->fixed);
        
        blb_cursor_step(c, 1);
        blb_cursor_step(c, 1);
        blb_cursor_step(c, 4);
        blb_cursor_step(c, -2);
        
        printf("offset: %d fixed: %d\n",
            c->offset, c->fixed);

        blb_cursor_jump(c, 33);
        blb_cursor_step(c, -7);
        blb_cursor_step(c, 16);

        printf("offset: %d fixed: %d\n",
            c->offset, c->fixed);


        blb_cursor_delete(c);
    }
    return 0;
}

int main4(void){
    blb_range_t *r = blb_range_create(77, 100, 1, false);
    if(r){
        printf("start: %u size: %u step: %u fixed: %d\n",
            r->start, r->size, r->step, r->fixed);

        blb_range_slide(r, 8);
        blb_range_slide(r, INT32_MAX);
        // blb_range_slide(r, INT32_MAX);


        printf("start: %u size: %u step: %u fixed: %d\n",
            r->start, r->size, r->step, r->fixed);
        
        blb_range_resize(r, -16);
        blb_range_resize(r, 88);


        printf("start: %u size: %u step: %u fixed: %d\n",
            r->start, r->size, r->step, r->fixed);

        blb_range_delete(r);
    }
    return 0;
}

int main2(void){

    blb_block_t *blk = blb_block_create(4);
    struct blb_range_t rng;
    struct blb_cursor_t c;

    if(!blk){
        fprintf(stderr, "Unable to allocate new block\n");
        return 1;
    }

    blb_block_put(blk, 0, 0x24);
    blb_block_put(blk, 1, 0xfe);
    blb_block_put(blk, 2, 0xae);
    blb_block_put(blk, 3, 0xdb);

    uint8_t value = 0;
    if(blb_block_get(blk, 2, &value)){
        printf("value = %02x\n", value);
    }

    blb_block_print(blk, stdout);

    blb_block_delete(blk);
    return 0;
}



void blb_range_print(blb_range_t *range, FILE *output) {
    if (!range || !output) return;
    fprintf(output, "start: %u size: %u step: %u fixed: %d\n",
            range->start, range->size, range->step, range->fixed);
}

void blb_cursor_print(blb_cursor_t *cursor, FILE *output) {
    if (!cursor || !output) return;
    fprintf(output, "offset: %d fixed: %d\n",
            cursor->offset, cursor->fixed);
}

void blb_blob_print(blb_blob_t *blob, FILE *output) {
    if (!blob || !output) return;
    
    fprintf(output, "--- Blob Status ---\n");
    
    if (blob->range) {
        fprintf(output, "Range  -> ");
        blb_range_print(blob->range, output);
    }
    if (blob->cursor) {
        fprintf(output, "Cursor -> ");
        blb_cursor_print(blob->cursor, output);
    }
    if (blob->block) {
        fprintf(output, "Block Memory Map:\n");
        blb_block_print(blob->block, output);
    }
    
    fprintf(output, "-------------------\n");
}

bool blb_blob_jump(blb_blob_t *blob, uint32_t value){
    if (blob && blob->range && blob->cursor) {
        if (blb_range_in(blob->range, value)) {
            return blb_cursor_jump(blob->cursor, value);
        }
    }
    return false;
}

bool blb_blob_put(blb_blob_t *blob, uint8_t value){
    if (blob && blob->cursor && blob->block && blob->range) {
        if (blb_range_in(blob->range, blob->cursor->offset)) {
            return blb_block_put(blob->block, blob->cursor->offset, value);
        }
    }
    return false;
}

bool blb_blob_get(blb_blob_t *blob, uint8_t *value){
    if (blob && blob->cursor && blob->block && blob->range && value) {
        if (blb_range_in(blob->range, blob->cursor->offset)) {
            return blb_block_get(blob->block, blob->cursor->offset, value);
        }
    }
    return false;
}
int main(void){
    blb_blob_t *my_blob = blb_blob_create(10, 1);
    
    if (my_blob) {
        my_blob->cursor->fixed = false; 
        
        if (blb_blob_jump(my_blob, 4)) {
            blb_blob_put(my_blob, 0xAA);
        }
        
        if (blb_blob_step(my_blob, 1)) {
            blb_blob_put(my_blob, 0xBB);
        }
        
        blb_blob_print(my_blob, stdout);
        
        blb_blob_delete(my_blob);
    }

    return 0;
}