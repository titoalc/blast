#include "genesis.h"
#include "blast.h"

void blastmap_init(blastmap* bmap, tilemap* tmap, u16* tlookup, u16 tileoffset, VDPPlane plane) {
    bmap->tX = 64; 
    bmap->tY = 64;
    bmap->winX = bmap->tX;
    bmap->winY = bmap->tY;
    bmap->maph = 0;
    bmap->mapw = 0;    
    bmap->planwidth = VDP_getPlaneWidth();
    bmap->planheight = VDP_getPlaneHeight();
    bmap->tileoffset = tileoffset;
    bmap->plane = plane;
    bmap->tiles = tmap;
    bmap->tlookup = tlookup;

    // Set the global foreground map
    fore_map = bmap;
}

u8 check_up(blastmap* bmap, spritedef* sprite) {
    //char str[10];
    // Check if we are at a tile boundary, if not skip.
    //if(((sprite->vposy) % 8) != 0) {
    //    return 0;
    //}
    // Need to consider wrap-around 
    u16 tile_row = (sprite->vposy - 1) / 8;

    //if(tile_row 
    if(tile_row >= bmap->maph) {
        tile_row = 0;
    }

    return check_col(bmap, sprite, tile_row);
}
u8 check_down(blastmap* bmap, spritedef* sprite) {
    //char str[10];
    // Check if we are at a tile boundary, if not skip.
    //if(((sprite->vposy) % 8) != 0) {
    //    return 0;
    //}
    
    u16 tile_row = (sprite->vposy / 8) + sprite->tile_height;

    // Check wrap-around
    if(tile_row >= bmap->maph) {
        // If we go over a boundary, point to the correct tile pos.
        tile_row -= bmap->maph;
    }

    return check_col(bmap, sprite, tile_row);
}

u8 check_col(blastmap* bmap, spritedef* sprite, u8 tile_row) {
    u8 coll;
    u8 i;
    u16 tile;
    // How many cols to test?
    u8 tile_col = sprite->vposx / 8;
    u8 numcols = sprite->tile_width;
    
    if(((sprite->vposx) % 8) != 0) {
        // We are crossing rows
        numcols++;
    }

#ifdef DEBUG1
    char str[10];
    uintToStr(tile_col, str, 2);
    VDP_drawText(str, 24, 16);
    uintToStr(tile_row, str, 2);
    VDP_drawText(str, 24, 17);
#endif 

    coll = 0;
    for(i=0; i < numcols; i++, tile_col++) {

        if(tile_col >= bmap->mapw) {
            tile_col = 0;
        }

        tile = GET_COLL_XY(bmap, tile_col, tile_row);
        coll |= bmap->tiles->coll[(tile-(bmap->tileoffset))];
    }

    return coll;
}

u8 check_row(blastmap* bmap, spritedef* sprite, u8 tile_col) {
    u8 coll;
    u8 i;
    u16 tile;
    // How many rows to test?
    u8 tile_row = sprite->vposy / 8;
    u8 numrows = sprite->tile_height;

    if(((sprite->vposy) % 8) != 0) {
        // We are crossing rows
        numrows++;
    }

#ifdef DEBUG1
    char str[10];
    uintToStr(tile_col, str, 2);
    VDP_drawText(str, 24, 16);
    uintToStr(tile_row, str, 2);
    VDP_drawText(str, 24, 17);
#endif

    coll = 0;
    for(i=0; i < numrows; i++, tile_row++) {

        if(tile_row >= bmap->maph) {
            tile_row = 0;
        }

        tile = GET_COLL_XY(bmap, tile_col, tile_row);
        coll |= bmap->tiles->coll[(tile - (bmap->tileoffset))];
    }

    return coll;
}

u8 check_left(blastmap* bmap, spritedef* sprite) {
    //char str[10];
    // Check if we are at a tile boundary, if not skip.
    //if(((sprite->vposx) % 8) != 0) {
    //    return 0;
    //}
    // 
    u16 tile_col = (sprite->vposx - 1) / 8;
    if(tile_col >= bmap->mapw) {
        tile_col = 0;
    }
    return check_row(bmap, sprite, tile_col);
}

u8 check_right(blastmap* bmap, spritedef* sprite) {
    //char str[10];
    
    // Check if we are at a tile boundary, if not skip.
    //if(((sprite->vposx) % 8) != 0) {
    //    return 0;
    //}

    u16 tile_col = (sprite->vposx / 8) + sprite->tile_width;
    // Check wrap-around 
    if(tile_col >= bmap->mapw) {
        tile_col -= bmap->mapw;
    }
    return check_row(bmap, sprite, tile_col);
}

void screen_left(blastmap* bmap, u16* hScroll) {
    if(*hScroll % 8 == 0) {
        bmap->tX--;
        if(bmap->tX>=bmap->planwidth){
            bmap->tX = bmap->planwidth - 1;
        }
        bmap->winX--;
        if(bmap->winX>=bmap->mapw) {
            bmap->winX = bmap->mapw - 1;
        }
        load_map_col(bmap,0);
    }
    *hScroll+=1;
}

void screen_right(blastmap* bmap, u16* hScroll) {
    *hScroll-=1;
    if(*hScroll % 8 == 0) {
        bmap->tX++;
        if(bmap->tX>=bmap->planwidth){
            bmap->tX = 0;
        }
        bmap->winX++;
        if(bmap->winX>=bmap->mapw) {
            bmap->winX = 0;
        }
        load_map_col(bmap,winW+1);
    }
}

void screen_up(blastmap* bmap, u16* vScroll) {


    // Check only on tile boundaries 
    if(*vScroll % 8 == 0) {
        bmap->tY--;
        if(bmap->tY>=bmap->planheight){
            bmap->tY = bmap->planheight - 1;
        }
        bmap->winY--;
        if(bmap->winY>=bmap->maph){
            bmap->winY = bmap->maph - 1;
        }
        load_map_row(bmap,0);
    }
    *vScroll-=1;
    // vScroll - current vertical scroll position
}
void screen_down(blastmap* bmap, u16* vScroll) {
    *vScroll+=1;
    if(*vScroll % 8 == 0) {
        bmap->tY++;
        if(bmap->tY>=bmap->planheight){
            bmap->tY = 0;
        }
        bmap->winY++;
        if(bmap->winY>=bmap->maph){
            bmap->winY = 0;
        }
        //load_map_row(bmap,0);
        load_map_row(bmap,winH+1);
    }
}

void center_screen(blastmap* bmap, u16 sprite_idx, u16* hscroll, u16* vscroll) {
    spritedef *tsprite;
    tsprite = _sprite_all[sprite_idx];
   
    //(((a)>(b))?(a):(b))
    //u8 velmax = (abs(tsprite->velocity.x)>abs(tsprite->velocity.y))?abs(tsprite->velocity.x):abs(tsprite->velocity.y);

    //u8 scroll_factor = ((velmax/32)>0)?(velmax/32):1;
    u8 i;
    u8 scroll_factor;
    //u8 scroll_factor = 1;

    //128x104
    // Center the screen
    if(tsprite->circle.position.y > 124) {
        scroll_factor = (tsprite->circle.position.y - 104)>>3;
        //tsprite->circle.position.y -= ship->speed;
        vscroll_sprites(-scroll_factor);
        for(i=0;i<scroll_factor;i++){
            screen_down(bmap,vscroll);
        }
        //if(ship->speed == 2)
        //    screen_down(&backgroundmap,&vScroll);
    } else if (tsprite->circle.position.y < 84) {
        scroll_factor = (104 - tsprite->circle.position.y)>>3;
        //tsprite->circle.position.y += ship->speed;
        vscroll_sprites(scroll_factor);
        for(i=0;i<scroll_factor;i++){
            screen_up(bmap,vscroll);
        }
        //if(ship->speed == 2)
        //    screen_up(&backgroundmap,&vScroll);
    }

    if(tsprite->circle.position.x > 148){
        scroll_factor = (tsprite->circle.position.x - 128)>>4;

        //tsprite->circle.position.x -= ship->speed;
        hscroll_sprites(-scroll_factor);
        for(i=0;i<scroll_factor;i++){
            screen_right(bmap,hscroll);
        }
        //if(ship->speed == 2)
        //    screen_right(&backgroundmap,&hScroll);
    } else if(tsprite->circle.position.x < 108) {
        scroll_factor = (128 - tsprite->circle.position.x)>>4;
        //tsprite->circle.position.x += ship->speed;
        hscroll_sprites(scroll_factor);
        for(i=0;i<scroll_factor;i++){
            screen_left(bmap,hscroll);
        }
        //if(ship->speed == 2)
        //    screen_left(&backgroundmap,&hScroll);
    }
}


void load_map(blastmap* bmap, int xoffset, int yoffset) {
    /*
     * load_map
     *
     * Loads an entire map
     *
     */

    int i;
    int j;

    //int start_index = 1;
    bmap->maph = bmap->tiles->height;
    bmap->mapw = bmap->tiles->width;

    int tilex;
    int tiley;
    
    for(i=0;i<bmap->maph;i++) {
        if(i+yoffset >= bmap->planheight) {
            tiley = (i + yoffset) - bmap->planheight;
        } else {
            tiley = i + yoffset;
        }

        for(j=0;j<bmap->mapw;j++) {
            if(j+xoffset >= bmap->planwidth) {
                tilex = (j + xoffset) - bmap->planwidth;

            } else {
                tilex = j + xoffset;

            }
            VDP_setTileMapXY(
                    bmap->plane, 
                    //(u16*)bmap->tiles->data[(i * (bmap->mapw)) + j] + bmap->tileoffset - 1,
                    (u16*)GET_TILE_XY(bmap, j, i), 
                    tilex,
                    tiley
            );
        }
    }
}

#define BITMAPTILE
void set_tile(blastmap* map, u16 mapx, u16 mapy, u16 screenx, u16 screeny) {
#ifdef BITMAPTILE
    VDP_setTileMapXY(
            map->plane, 
            TILE_ATTR_FULL(
                PAL0, 
                TRUE, 
                FALSE, 
                FALSE,
                (u16*)GET_TILE_XY(map, mapx, mapy)), 
            //(u16*)(bmap->tiles->data[(iW * (bmap->mapw)) + jW]) + bmap->tileoffset - 1,
            /*(u16*)( bmap->tiles->data[
                ( iW * (bmap->mapw)) + jW
            ] 
            + bmap->tileoffset - 1)),*/
            screenx,
            screeny
    );
#else

    char str[10];
    //u16 tile_in_map = (u16*)GET_TILE_XY(map, mapx, mapy);
    //u16 tile_in_map = (u16*)((map->tiles->data[(mapy * (map->mapw)) + mapx]) - 1);
    //u16 converted_tile = (map->tlookup[tile_in_map]) + map->tileoffset;

    u16 converted_tile = (u16*)(map->tlookup[((map->tiles->data[(mapy * (map->mapw)) + mapx]) - 1)] + map->tileoffset);

    uintToStr(converted_tile,str,3);
    VDP_drawText(str, 3, 3);
    VDP_setTileMapXY(
           map->plane,
           converted_tile,
           screenx,
           screeny
    ); 
#endif
}

void load_visible_map(blastmap* bmap, int xoffset, int yoffset) {
    /*
     * load_visible_map
     *
     * Load only the visible portion of a map
     *
     */

    u16 i;
    u16 j;
    u16 iW;
    u16 jW;
    u8 ycount;
    u8 xcount;

    bmap->mapw = bmap->tiles->width;
    bmap->maph = bmap->tiles->height;

    // Y pos in tilemap
    i = bmap->tY;
    // Y pos in window
    iW = bmap->winY;
    for(ycount=0;ycount<=winH;ycount++) {

        // wrap around
        if(i == bmap->planheight) {
            i -= bmap->planheight;
        } 
        if(iW >= bmap->maph) {
            iW -= bmap->maph;
        }

        // X pos in tilemap
        j = bmap->tX;
        // X pos in window
        jW = bmap->winX;
        for(xcount=0;xcount<=winW;xcount++){
            if(j == bmap->planwidth) {
                j -= bmap->planwidth;
            } 
            if(jW == bmap->mapw) {
                jW = 0;
            }
            set_tile(bmap, jW, iW, j, i);
            /*
            VDP_setTileMapXY(
                    bmap->plane, 
                    //(u16*)bmap->tiles->data[(i * (bmap->mapw)) + j] + bmap->tileoffset - 1,
                    (u16*)GET_TILE_XY(bmap, jW, iW), 
                    j,
                    i
            );
            */
            j++;
            jW++;
        }
        i++;
        iW++;
    }

}

void load_map_row(blastmap* bmap, u8 row) {
    /*
     * load_map_row
     *
     * Load a single row of the map
     *
     */

    // Track row/col on screen
    u8 i;
    u8 j;

    // Track position in tilemap data
    u16 iW;
    u16 jW;

    u8 xcount;


    // Set row position we want to load at on screen
    i = bmap->tY + row;
    // Set row posistion to load from tilemap data
    iW = bmap->winY + row;

    // wrap around
    if(i >= bmap->planheight) {
        // We have wrapped around in the plane
        i -= bmap->planheight;
    } 
    if(iW >= bmap->maph) {
        // We have wrapped around in the tilemap data
        iW -= bmap->maph;
    }

    j = bmap->tX;
    jW = bmap->winX;

    u16 numtiles = (winW+2);
    u16 tiles[numtiles];
    
    if(j == bmap->planwidth) {
        j = 0;
    } 
    for(xcount=0;xcount<=winW+1;xcount++){
        if(jW == bmap->mapw) {
            // Wrap around
            jW = 0;
        }
        
        tiles[xcount] = (u16*)GET_TILE_XY(bmap, jW, iW); 
        /*
        VDP_setTileMapXY(
                bmap->plane, 
                //(u16*)bmap->tiles->data[(iW * (bmap->mapw)) + jW] + bmap->tileoffset - 1,
                (u16*)GET_TILE_XY(bmap, jW, iW), 
                j,
                i
        );
        */
        //j++;
        jW++;
    }

    SYS_disableInts();
    if ( (j + numtiles) >= bmap->planwidth ) {
        // We will wrap around in the plane

        u16 numtiles_a = (bmap->planwidth - j);
        u16 numtiles_b = numtiles - numtiles_a;

        VDP_setTileMapDataRect(
                bmap->plane,
                tiles,
                j,
                i,
                numtiles_a,
                1,
                1,
                CPU
        );
        VDP_setTileMapDataRect(
                bmap->plane,
                (u16*)tiles + numtiles_a,
                0,
                i,
                numtiles_b,
                1,
                1,
                CPU
        );

    } else {

        // Does this handle plane wrap around? NOPE
        VDP_setTileMapDataRect(
                bmap->plane,
                tiles,
                j,
                i,
                numtiles,
                1,
                1,
                CPU
        );
  
    }
    SYS_enableInts();

    /*
    VDP_setTileMapData(
            bmap->plane.value,
            tiles,
            bmap->tX,
            numtiles,
            DMA
    );
    */
    //VDP_waitVSync();

}

void load_map_col(blastmap* bmap, u8 col) {
    /*
     * load_map_col
     *
     * Load a single column of the map
     *
     */

    u8 i;
    u8 j;
    u16 iW;
    u16 jW;
    u8 ycount;

    i = bmap->tX + col;
    iW = bmap->winX + col;

    // wrap around
    if(i >= bmap->planwidth) {
        i -= bmap->planwidth;
    } 
    if(iW >= bmap->mapw) {
        iW -= bmap->mapw;
    }

    j = bmap->tY;
    jW = bmap->winY;

    u16 numtiles = (winH+2);
    u16 tiles[numtiles];

    if(j == bmap->planheight) {
        j = 0;
    } 
    for(ycount=0;ycount<=winH+1;ycount++){
        if(jW == bmap->maph) {
            jW = 0;
        }
        tiles[ycount] = (u16*)GET_TILE_XY(bmap, iW, jW); 
       /* VDP_setTileMapXY(
                bmap->plane, 
                //(u16*)bmap->tiles->data[(jW * (bmap->mapw)) + iW] + bmap->tileoffset - 1,
                (u16*)GET_TILE_XY(bmap, iW, jW), 
                i,
                j
        );
        */
        jW++;
     //   j++;
    }

    SYS_disableInts();
    if ( (j + numtiles) >= bmap->planheight) {
        // We will wrap around in the plane

        u16 numtiles_a = (bmap->planheight - j);
        u16 numtiles_b = numtiles - numtiles_a;

        VDP_setTileMapDataRect(
                bmap->plane,
                tiles,
                i,
                j,
                1,
                numtiles_a,
                1,
                CPU
        );
        VDP_setTileMapDataRect(
                bmap->plane,
                (u16*)tiles + numtiles_a,
                i,
                0,
                1,
                numtiles_b,
                1,
                CPU
        );
    } else {
        // Does this handle plane wrap around?  NOPE
        VDP_setTileMapDataRect(
                bmap->plane,
                tiles,
                i,
                j,
                1,
                numtiles,
                1,
                CPU
        );
    }
    SYS_enableInts();
}
