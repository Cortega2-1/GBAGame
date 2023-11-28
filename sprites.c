/*
 * collide.c
 * program which demonstrates sprites colliding with tiles
 */
#include <stdio.h>
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#include "background1.h"
/* include the background image we are using */
#include "background.h"
/* include the sprite image we are using */
#include "koopa.h"
#include "map2.h"
#include "soccer.h"
//#include "soccer1.h"
/* include the tile map we are using */
#include "map.h"
/* the tile mode flags needed for display control register */
#define MODE0 0x00
#define BG0_ENABLE 0x100
#define BG1_ENABLE 0x200
/* flags to set sprite handling in display control register */
#define SPRITE_MAP_2D 0x0
#define SPRITE_MAP_1D 0x40
#define SPRITE_ENABLE 0x1000

/* the control registers for the four tile layers */
volatile unsigned short* bg0_control = (volatile unsigned short*) 0x4000008;
 volatile unsigned short* bg1_control = (volatile unsigned short*) 0x400000a;
/* palette is always 256 colors */
#define PALETTE_SIZE 256

/* there are 128 sprites on the GBA */
#define NUM_SPRITES 128

/* the display control pointer points to the gba graphics register */
volatile unsigned long* display_control = (volatile unsigned long*) 0x4000000;

/* the memory location which controls sprite attributes */
volatile unsigned short* sprite_attribute_memory = (volatile unsigned short*) 0x7000000;

/* the memory location which stores sprite image data */
volatile unsigned short* sprite_image_memory = (volatile unsigned short*) 0x6010000;

/* the address of the color palettes used for backgrounds and sprites */
volatile unsigned short* bg_palette = (volatile unsigned short*) 0x5000000;
volatile unsigned short* sprite_palette = (volatile unsigned short*) 0x5000200;
/* the button register holds the bits which indicate whether each button has
 * been pressed - this has got to be volatile as well
 */
volatile unsigned short* buttons = (volatile unsigned short*) 0x04000130;

/* scrolling registers for backgrounds */
volatile short* bg0_x_scroll = (unsigned short*) 0x4000010;
volatile short* bg0_y_scroll = (unsigned short*) 0x4000012;
volatile short* bg1_x_scroll = (unsigned short*) 0x4000014;
/* the bit positions indicate each button - the first bit is for A, second for
 * B, and so on, each constant below can be ANDED into the register to get the
 * status of any one button */
#define BUTTON_A (1 << 0)
#define BUTTON_B (1 << 1)
#define BUTTON_SELECT (1 << 2)
#define BUTTON_START (1 << 3)
#define BUTTON_RIGHT (1 << 4)
#define BUTTON_LEFT (1 << 5)
#define BUTTON_UP (1 << 6)
#define BUTTON_DOWN (1 << 7)
#define BUTTON_R (1 << 8)
#define BUTTON_L (1 << 9)

/* the scanline counter is a memory cell which is updated to indicate how
 * much of the screen has been drawn */
volatile unsigned short* scanline_counter = (volatile unsigned short*) 0x4000006;

/* wait for the screen to be fully drawn so we can do something during vblank */
void wait_vblank() {
    /* wait until all 160 lines have been updated */
    while (*scanline_counter < 160) { }
}

/* this function checks whether a particular button has been pressed */
unsigned char button_pressed(unsigned short button) {
    /* and the button register with the button constant we want */
    unsigned short pressed = *buttons & button;

    /* if this value is zero, then it's not pressed */
    if (pressed == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* return a pointer to one of the 4 character blocks (0-3) */
volatile unsigned short* char_block(unsigned long block) {
    /* they are each 16K big */
    return (volatile unsigned short*) (0x6000000 + (block * 0x4000));
}

/* return a pointer to one of the 32 screen blocks (0-31) */
volatile unsigned short* screen_block(unsigned long block) {
    /* they are each 2K big */
    return (volatile unsigned short*) (0x6000000 + (block * 0x800));
}

/* flag for turning on DMA */
#define DMA_ENABLE 0x80000000

/* flags for the sizes to transfer, 16 or 32 bits */
#define DMA_16 0x00000000
#define DMA_32 0x04000000
#define DMA_24 0x02000000
/* pointer to the DMA source location */
volatile unsigned int* dma_source = (volatile unsigned int*) 0x40000D4;

/* pointer to the DMA destination location */
volatile unsigned int* dma_destination = (volatile unsigned int*) 0x40000D8;

/* pointer to the DMA count/control */
volatile unsigned int* dma_count = (volatile unsigned int*) 0x40000DC;

/* copy data using DMA */
void memcpy16_dma(unsigned short* dest, unsigned short* source, int amount) {
    *dma_source = (unsigned int) source;
    *dma_destination = (unsigned int) dest;
    *dma_count = amount | DMA_16 | DMA_ENABLE;
}
/* function to setup background 0 for this program */
void setup_background() {

    /* load the palette from the image into palette memory*/
    memcpy16_dma((unsigned short*) bg_palette, (unsigned short*) background_palette, PALETTE_SIZE);

    /* load the image into char block 0 */
    memcpy16_dma((unsigned short*) char_block(0), (unsigned short*) background_data,
            (background_width * background_height) / 2);
 
    /* set all control the bits in this register */
    *bg0_control = 1 |    /* priority, 0 is highest, 3 is lowest */
        (0 << 2)  |       /* the char block the image data is stored in */
        (0 << 6)  |       /* the mosaic flag */
        (1 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
        (16 << 8) |       /* the screen block the tile data is stored in */
        (1 << 13) |       /* wrapping flag */
        (0 << 14);        /* bg size, 0 is 256x256 */

    /* load the tile data into screen block 16 */
    memcpy16_dma((unsigned short*) screen_block(16), (unsigned short*) map, map_width * map_height);
  /* set all control the bits in this register */
     *bg1_control = 0 |    /* priority, 0 is highest, 3 is lowest */
         (0 << 2)  |       /* the char block the image data is stored in */
         (0 << 6)  |       /* the mosaic flag */
         (1 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
         (24 << 8) |       /* the screen block the tile data is stored in */
         (1 << 13) |       /* wrapping flag */
         (0 << 14);        /* bg size, 0 is 256x256 */
  /* clear the tile map in screen block 16 to all black tile*/
      
     /* load the tile data into screen block 16 */
     memcpy16_dma((unsigned short*) screen_block(24), (unsigned short*) map2, map2_width * map2_height);
}
void setup_background1() {
      /* load the palette from the image into palette memory*/
      memcpy16_dma((unsigned short*) bg_palette, (unsigned short*) background1_palette, PALETTE_SIZE);
  
      /* load the image into char block 0 */
      memcpy16_dma((unsigned short*) char_block(0), (unsigned short*) background1_data,
              (background1_width * background1_height) / 2);
  
      /* bg0 is just all black so the pink does not show through! */
      *bg0_control = 3 |    /* priority, 0 is highest, 3 is lowest */
          (0 << 2)  |       /* the char block the image data is stored in */
          (0 << 6)  |       /* the mosaic flag */
          (1 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
          (16 << 8) |       /* the screen block the tile data is stored in */
         (1 << 13) |       /* wrapping flag */
          (0 << 14);        /* bg size, 0 is 256x256 */
  
  
      /* bg1 is our actual text background */
      *bg1_control = 0 |    /* priority, 0 is highest, 3 is lowest */
          (0 << 2)  |       /* the char block the image data is stored in */
         (0 << 6)  |       /* the mosaic flag */
         (1 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
        (24 << 8) |       /* the screen block the tile data is stored in */
         (1 << 13) |       /* wrapping flag */
         (0 << 14);        /* bg size, 0 is 256x256 */
 
     /* clear the tile map in screen block 16 to all black tile*/
     volatile unsigned short* ptr = screen_block(16);
     for (int i = 0; i < 32 * 32; i++) {
         ptr[i] = 95;
     }
 
     /* clear the text map to be all blanks */
     ptr = screen_block(24);
     for (int i = 0; i < 32 * 32; i++) {
         ptr[i] = 0;
     }
 }
/* just kill time */
void delay(unsigned int amount) {
    for (int i = 0; i < amount * 10; i++);
}

/* a sprite is a moveable image on the screen */
struct Sprite {
    unsigned short attribute0;
    unsigned short attribute1;
    unsigned short attribute2;
    unsigned short attribute3;
};

/* array of all the sprites available on the GBA */
struct Sprite sprites[NUM_SPRITES];
int next_sprite_index = 0;

/* the different sizes of sprites which are possible */
enum SpriteSize {
    SIZE_8_8,
    SIZE_16_16,
    SIZE_32_32,
    SIZE_64_64,
    SIZE_16_8,
    SIZE_32_8,
    SIZE_32_16,
    SIZE_64_32,
    SIZE_8_16,
    SIZE_8_32,
    SIZE_16_32,
    SIZE_32_64
};

/* function to initialize a sprite with its properties, and return a pointer */
struct Sprite* sprite_init(int x, int y, enum SpriteSize size,
        int horizontal_flip, int vertical_flip, int tile_index, int priority) {

    /* grab the next index */
    int index = next_sprite_index++;

    /* setup the bits used for each shape/size possible */
    int size_bits, shape_bits;
    switch (size) {
        case SIZE_8_8:   size_bits = 0; shape_bits = 0; break;
        case SIZE_16_16: size_bits = 1; shape_bits = 0; break;
        case SIZE_32_32: size_bits = 2; shape_bits = 0; break;
        case SIZE_64_64: size_bits = 3; shape_bits = 0; break;
        case SIZE_16_8:  size_bits = 0; shape_bits = 1; break;
        case SIZE_32_8:  size_bits = 1; shape_bits = 1; break;
        case SIZE_32_16: size_bits = 2; shape_bits = 1; break;
        case SIZE_64_32: size_bits = 3; shape_bits = 1; break;
        case SIZE_8_16:  size_bits = 0; shape_bits = 2; break;
        case SIZE_8_32:  size_bits = 1; shape_bits = 2; break;
        case SIZE_16_32: size_bits = 2; shape_bits = 2; break;
        case SIZE_32_64: size_bits = 3; shape_bits = 2; break;
    }

    int h = horizontal_flip ? 1 : 0;
    int v = vertical_flip ? 1 : 0;

    /* set up the first attribute */
    sprites[index].attribute0 = y |             /* y coordinate */
        (0 << 8) |          /* rendering mode */
        (0 << 10) |         /* gfx mode */
        (0 << 12) |         /* mosaic */
        (1 << 13) |         /* color mode, 0:16, 1:256 */
        (shape_bits << 14); /* shape */

    /* set up the second attribute */
    sprites[index].attribute1 = x |             /* x coordinate */
        (0 << 9) |          /* affine flag */
        (h << 12) |         /* horizontal flip flag */
        (v << 13) |         /* vertical flip flag */
        (size_bits << 14);  /* size */

    /* setup the second attribute */
    sprites[index].attribute2 = tile_index |   // tile index */
        (priority << 10) | // priority */
        (0 << 12);         // palette bank (only 16 color)*/

    /* return pointer to this sprite */
    return &sprites[index];
}
void sprite_update_all() {
    // copy them all over //
    memcpy16_dma((unsigned short*) sprite_attribute_memory, (unsigned short*) sprites, NUM_SPRITES * 4);
}

/* setup all sprites */
void sprite_clear() {
    /* clear the index counter */
    next_sprite_index = 0;

    /* move all sprites offscreen to hide them */
    for(int i = 0; i < NUM_SPRITES; i++) {
        sprites[i].attribute0 = SCREEN_HEIGHT;
        sprites[i].attribute1 = SCREEN_WIDTH;
    }
}

/* set a sprite postion */
void sprite_position(struct Sprite* sprite, int x, int y) {
    /* clear out the y coordinate */
    sprite->attribute0 &= 0xff00;

    /* set the new y coordinate */
    sprite->attribute0 |= (y & 0xff);

    /* clear out the x coordinate */
    sprite->attribute1 &= 0xfe00;

    /* set the new x coordinate */
    sprite->attribute1 |= (x & 0x1ff);
}

/* move a sprite in a direction */
void sprite_move(struct Sprite* sprite, int dx, int dy) {
    /* get the current y coordinate */
    int y = sprite->attribute0 & 0xff;

    /* get the current x coordinate */
    int x = sprite->attribute1 & 0x1ff;

    /* move to the new location */
    sprite_position(sprite, x + dx, y + dy);
}

/* change the vertical flip flag */
void sprite_set_vertical_flip(struct Sprite* sprite, int vertical_flip) {
    if (vertical_flip) {
        /* set the bit */
        sprite->attribute1 |= 0x2000;
    } else {
        /* clear the bit */
        sprite->attribute1 &= 0xdfff;
    }
}

/* change the vertical flip flag */
void sprite_set_horizontal_flip(struct Sprite* sprite, int horizontal_flip) {
    if (horizontal_flip) {
        /* set the bit */
        sprite->attribute1 |= 0x1000;
    } else {
        /* clear the bit */
        sprite->attribute1 &= 0xefff;
    }
}

/* change the tile offset of a sprite */
void sprite_set_offset(struct Sprite* sprite, int offset) {
    /* clear the old offset */
    sprite->attribute2 &= 0xfc00;

    /* apply the new one */
    sprite->attribute2 |= (offset & 0x03ff);
}
/*
// setup the sprite image and palette /
void setup_sprite_image() {
    // load the palette from the image into palette memory/
    memcpy16_dma((unsigned short*) sprite_palette, (unsigned short*) koopa_palette, PALETTE_SIZE);

    // load the image into sprite image memory /
    memcpy16_dma((unsigned short*) sprite_image_memory, (unsigned short*) koopa_data, (koopa_width * koopa_height) / 2);
 // load the palette from the image into palette memory/
memcpy16_dma((unsigned short*) sprite_palette + 256, (unsigned short*) soccer_palette, PALETTE_SIZE);
 
     // load the image into sprite image memory /
     memcpy16_dma((unsigned short*) sprite_image_memory + 256, (unsigned short*) soccer_data, (soccer_width * soccer_height) / 2);

}
*/
void setup_koopa_sprite_image() {
    /* load the palette from the image into palette memory */
    memcpy16_dma((unsigned short*) sprite_palette, (unsigned short*) koopa_palette, PALETTE_SIZE);

    /* load the image into sprite image memory */
    memcpy16_dma((unsigned short*) sprite_image_memory, (unsigned short*) koopa_data, (koopa_width * koopa_height) / 2);
}

/* setup the sprite image and palette for Square */
/* setup the sprite image and palette for Square */
void setup_square_sprite_image() {
    /* load the palette from the image into palette memory */
    memcpy16_dma((unsigned short*) sprite_palette, (unsigned short*) soccer_palette, PALETTE_SIZE);

    /* load the image into sprite image memory */
    memcpy16_dma((unsigned short*) sprite_image_memory, (unsigned short*) soccer_data, (soccer_width * soccer_height) / 2);
// Example for koopa with an offset of 16

}

// Initialize the Square
 struct Square {
     /* the actual sprite attribute info */
     struct Sprite* sprite;
 
     /* the x and y postion in pixels */
     int x, y;
 
     /* the koopa's y velocity in 1/256 pixels/second */
     int yvel;
 
     /* the koopa's y acceleration in 1/256 pixels/second^2 */
     int gravity;
 
     /* which frame of the animation he is on */
     int frame; 
     /* the number of frames to wait before flipping */
     int animation_delay;
 
     /* the animation counter counts how many frames until we flip */
     int counter;
 
     /* whether the koopa is moving right now or not */
     int move;
 
     /* the number of pixels away from the edge of the screen the koopa stays */
     int border;
 
     /* if the koopa is currently falling */
     int falling;
 };
void square_init(struct Square* square) {
square->x = 0;
    square->y = 100;
    square->yvel = 1;
    square->gravity = 50;
     square->border = 50;
     square->frame = 32;   
     square->move = 0;
     square->counter = 0;
     square->falling = 0;
     square->animation_delay = 8;
     square->sprite = sprite_init(square->x, square->y, SIZE_16_32, 0, 0, square->frame, 0);
 }
/* a struct for the koopa's logic and behavior */
struct Koopa {
    /* the actual sprite attribute info */
    struct Sprite* sprite;

    /* the x and y postion in pixels */
    int x, y;

    /* the koopa's y velocity in 1/256 pixels/second */
    int yvel;

    /* the koopa's y acceleration in 1/256 pixels/second^2 */
    int gravity; 

    /* which frame of the animation he is on */
    int frame;

    /* the number of frames to wait before flipping */
    int animation_delay;

    /* the animation counter counts how many frames until we flip */
    int counter;

    /* whether the koopa is moving right now or not */
    int move;

    /* the number of pixels away from the edge of the screen the koopa stays */
    int border;

    /* if the koopa is currently falling */
    int falling;
};

/* initialize the koopa */
void koopa_init(struct Koopa* koopa) {
    koopa->x = 0;
    koopa->y = 100;
    koopa->yvel = 1;
    koopa->gravity = 50;
    koopa->border = 40;
    koopa->frame = 0;
    koopa->move = 0;
    koopa->counter = 0;
    koopa->falling = 0;
    koopa->animation_delay = 8;
    koopa->sprite = sprite_init(koopa->x, koopa->y, SIZE_16_32, 0, 0, koopa->frame, 0);
}

/* move the koopa left or right returns if it is at edge of the screen */
int koopa_left(struct Koopa* koopa) {
    /* face left */
    sprite_set_horizontal_flip(koopa->sprite, 1);
    koopa->move = 1;

    /* if we are at the left end, just scroll the screen */
    if (koopa->x < koopa->border) {
        return 1;
    } else {
        /* else move left */
        koopa->x--;
        return 0;
    }
}
int koopa_right(struct Koopa* koopa) {
    /* face right */
    sprite_set_horizontal_flip(koopa->sprite, 0);
    koopa->move = 1;

    /* if we are at the right end, just scroll the screen */
    if (koopa->x > (SCREEN_WIDTH - 16 - koopa->border)) {
        return 1;
    } else {
        /* else move right */
        koopa->x++;
        return 0;
    }
}

/* stop the koopa from walking left/right */
void koopa_stop(struct Koopa* koopa) {
    koopa->move = 0;
    koopa->frame = 0;
    koopa->counter = 7;
    sprite_set_offset(koopa->sprite, koopa->frame);
}

/* start the koopa jumping, unless already fgalling */
void koopa_jump(struct Koopa* koopa) {
    if (!koopa->falling) {
        koopa->yvel = -1350;
        koopa->falling = 1;
    }
}

/* finds which tile a screen coordinate maps to, taking scroll into acco  unt */
unsigned short tile_lookup(int x, int y, int xscroll, int yscroll,
        const unsigned short* tilemap, int tilemap_w, int tilemap_h) {

    /* adjust for the scroll */
    x += xscroll;
    y += yscroll;

    /* convert from screen coordinates to tile coordinates */
    x >>= 3;
    y >>= 3;

    /* account for wraparound */
    while (x >= tilemap_w) {
        x -= tilemap_w;
    }
    while (y >= tilemap_h) {
        y -= tilemap_h;
    }
    while (x < 0) {
        x += tilemap_w;
    }
    while (y < 0) {
        y += tilemap_h;
    }

    /* the larger screen maps (bigger than 32x32) are made of multiple stitched
       together - the offset is used for finding which screen block we are in
       for these cases */
    int offset = 0;

    /* if the width is 64, add 0x400 offset to get to tile maps on right   */
    if (tilemap_w == 64 && x >= 32) {
        x -= 32;
        offset += 0x400;
    }

    /* if height is 64 and were down there */
    if (tilemap_h == 64 && y >= 32) {
        y -= 32;

        /* if width is also 64 add 0x800, else just 0x400 */
        if (tilemap_w == 64) {
            offset += 0x800;
        } else {
            offset += 0x400;
        }
    }

    /* find the index in this tile map */
    int index = y * 32 + x;

    /* return the tile */
    return tilemap[index + offset];
}

/* update the koopa */
void koopa_update(struct Koopa* koopa, int xscroll) {
    /* update y position and speed if falling */
    if (koopa->falling) {
        koopa->y += (koopa->yvel >> 8);
        koopa->yvel += koopa->gravity;
    }

    /* check which tile the koopa's feet are over */
    unsigned short tile = tile_lookup(koopa->x + 8, koopa->y + 32, xscroll, 0, map2,
            map2_width, map2_height);

    /* if it's block tile
     * these numbers refer to the tile indices of the blocks the koopa can walk on */
    if ((tile >= 1 && tile <= 6) || 
            (tile >= 12 && tile <= 17)) {
        /* stop the fall! */
        koopa->falling = 0;
        koopa->yvel = 0;

        /* make him line up with the top of a block works by clearing out the lower bits to 0 */
        koopa->y &= ~0x3;

        /* move him down one because there is a one pixel gap in the image */
        koopa->y++;

    } else {
        /* he is falling now */
        koopa->falling = 1;
    }


    /* update animation if moving */
    if (koopa->move) {
        koopa->counter++;
        if (koopa->counter >= koopa->animation_delay) {
            koopa->frame = koopa->frame + 16;
            if (koopa->frame > 16) {
                koopa->frame = 0;
            }
            sprite_set_offset(koopa->sprite, koopa->frame);
            koopa->counter = 0;
        }
    }

    /* set on screen position */
    sprite_position(koopa->sprite, koopa->x, koopa->y);
}
/* move the square left or right, returns if it is at the edge of the screen */
int square_left(struct Square* square) {
    /* face left */
    sprite_set_horizontal_flip(square->sprite, 1);
    square->move = 1;

    /* if we are at the left end, just scroll the screen */
    if (square->x < square->border) {
        return 1;
    } else {
        /* else move left */
        square->x--;
        return 0;
    }
}

int square_right(struct Square* square) {
    /* face right */
    sprite_set_horizontal_flip(square->sprite, 0);
    square->move = 10;

    /* if we are at the right end, just scroll the screen */
    if (square->x > (SCREEN_WIDTH - 16 - square->border)) {
        return 1;
    } else {
        /* else move right */
        square->x++;
        return 0;
    }
}

/* stop the square from walking left/right */
void square_stop(struct Square* square) {
    square->move = 0;
    square->frame = 32;
    square->counter = 7;
    sprite_set_offset(square->sprite, square->frame);
}

/* start the square jumping, unless already falling */
void square_jump(struct Square* square) {
    if (!square->falling) {
        square->yvel = -1350;
        square->falling = 1;
    }
}

/* update the square */
void square_update(struct Square* square, int xscroll) {
    /* update y position and speed if falling */
    if (square->falling) {
        square->y += (square->yvel >> 8);
        square->yvel += square->gravity;
    }

    /* check which tile the square's feet are over */
    unsigned short tile = tile_lookup(square->x + 8, square->y + 32, xscroll, 0, map2,
            map2_width, map2_height);

    /* if it's a block tile
     * these numbers refer to the tile indices of the blocks the square can walk on */
    if ((tile >= 1 && tile <= 6) || 
            (tile >= 12 && tile <= 17)) {
        /* stop the fall! */
        square->falling = 0;
        square->yvel = 0;

        /* make him line up with the top of a block works by clearing out the lower bits to 0 */
        square->y &= ~0x3;

        /* move him down one because there is a one-pixel gap in the image */
        square->y++;
    } else {
        /* he is falling now */
        square->falling = 1;
    }

    /* update animation if moving */
    if (square->move) {
        square->counter++;
        if (square->counter >= square->animation_delay) {
            square->frame = square->frame + 16;
            if (square->frame > 48) {
                square->frame = 32;
            }
            sprite_set_offset(square->sprite, square->frame);
            square->counter = 0;
        }
    }

    /* set on-screen position */
    sprite_position(square->sprite, square->x, square->y);
}

 /* function to set text on the screen at a given location */
 void set_text(char* str, int row, int col) {
     /* find the index in the texmap to draw to */
     int index = row * 32 + col;
 
     /* the first 32 characters are missing from the map (controls etc.) */
     int missing = 32;
 
     /* pointer to text map */
     volatile unsigned short* ptr = screen_block(16);
 
     /* for each character */
     while (*str) {
         /* place this character in the map */
         ptr[index] = *str - missing;
 
        /* move onto the next character */
         index++;
         str++;
     }
 }
#define TOTAL_COINS 10
void restart_game(struct Koopa* koopa, int xscroll, int* coin_count);
/* function to handle collisions with coins */
void count_coins(int* coin_count);
void handle_coin_collision(struct Square* square, struct Koopa* koopa, int xscroll, int* coin_count) {
unsigned short tile = tile_lookup(koopa->x + 8, koopa->y + 32, xscroll, 0, map2, map2_width, map2_height);
    /* If it's a coin tile, handle the collision */
    if (tile == 40 || tile == 41 || tile == 50 || tile == 51) {
        /* Increment the coin count */
            count_coins(coin_count);
        /* Check if all coins have been collected */
        if (*coin_count == TOTAL_COINS) {
            /* Switch to setup_background1 and show the total coin count message */
        koopa->x = 0;          
        koopa->y = 0;
delay(1500);
  setup_background1();
            char coin_msg[32];
            sprintf(coin_msg, "Total Score: %d", *coin_count);
            set_text(coin_msg, 0, 0);

            /* Wait for a moment to display the message */
            delay(1500); 
/* Ask if the player wants to play again or start over */
            char play_again_msg[100] = "Play again? Press 'B'";
            set_text(play_again_msg, 8, 0);

            while (!button_pressed(BUTTON_B)) {
    }
    restart_game(koopa, xscroll, coin_count);
            /* Wait for a moment to display the message */
    setup_background();
    koopa->x = 150;
    koopa->y = 0;            
    square->x = 50;
    square->y = 0;
    delay(1500);
        }
    }
}


int main() {
    /* we set the mode to mode 0 with bg0 on */
    *display_control = MODE0 | BG0_ENABLE |BG1_ENABLE | SPRITE_ENABLE | SPRITE_MAP_1D;
setup_background1();
while (!button_pressed(BUTTON_B)) {
char msg [100] = "Welcome! Press 'B' to continue.   How to Play: Collect Coins";
set_text(msg, 8, 0);
}   
    setup_background();
 
    /* setup the sprite image data */

    /* setup the sprite image data for Square */
  //setup_koopa_sprite_image();
  setup_square_sprite_image();
    //setup_koopa_sprite_image();
    /* clear all the sprites on screen now */
    sprite_clear();
    /* create the koopa */
    struct Koopa koopa;
    koopa_init(&koopa);
    struct Square square;
    square_init(&square); 
   int coin_count = 0;
    /* set initial scroll to 0 */
    int xscroll = 0;
    //struct Ball ball;
    /* loop forever */
    while (1) {
        /* update the koopa */
        square_update(&square, xscroll);
        koopa_update(&koopa, xscroll);
        /* now the arrow keys move the koopa */
        if (button_pressed(BUTTON_RIGHT)) {
            if (koopa_right(&koopa)) {
    square_right(&square);           
     xscroll++;
            }
        } else if (button_pressed(BUTTON_LEFT)) {
            if (koopa_left(&koopa)) {
                xscroll--;
            }
        } else {
            koopa_stop(&koopa);
        }

        /* check for jumping */
        if (button_pressed(BUTTON_A)) {
           koopa_jump(&koopa);
            square_jump(&square);
        }
        /* wait for vblank before scrolling and moving sprites */
        wait_vblank();
        *bg0_x_scroll = xscroll;
         *bg1_x_scroll = xscroll;
        sprite_update_all();
        handle_coin_collision(&square, &koopa, xscroll, &coin_count);
        /* delay some */
        delay(300);
}
}
