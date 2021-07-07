// SGDK STUFFS:
#include <genesis.h>
// My resorces, you first import the resources and compile;
// then SGDK will create a '.h' file like the one below:
#include <res.h>
// Check the tutorial to understand the parameters inside
// the 'res.h'

// C STUFFS:
#include <string.h>

// https://www.ohsat.com/tutorial/megapong/megapong-1/

/*
#################################################
####### PLANES #######
The Mega Drive uses planes to display graphics. When we’re
working with tiles, you have two background planes to work with:
Plane A and Plane B. In simplified terms you can think of them
as layers that determine at what depth your tiles are drawn.
For example, Plane A is drawn above Plane B, so you would use
that one for foreground tiles, like the ground your character
walks on, for example. Plane B is drawn behind Plane A, so that’s
where you’d draw a sky or something along those lines. (The
priority system deciding the draw order is actually a bit more
complicated than that and can easily be overridden, but for this
tutorial you can stick to the layer analogy.) Whenever you place
a tile on the screen you have to tell SGDK what background plane
to draw it on. To refer to the two background planes you use the
defines BG_A and BG_B (before version 1.50 these were called
PLAN_A and PLAN_B respectively).
#################################################
####### TILES #######
One MD tile is always 8x8 pixels big. Because this size is fixed,
tile coordinates in SGDK are not given in pixels, but tiles.
Meaning: If we put a tile at position (1,1) the tile will be
placed at pixel-position (8,8). If we add a tile at (2,1), this
new tile we appear right next to the first one, at pixel-position
(16,8). It’s very simple and useful, but it’s important to keep it
in mind, as other things within SGDK actually use pixels as a
measure.
#################################################
####### VRAM #######
Finally, let’s quickly talk about VRAM. This is where the Mega Drive
stores all the graphics needed for the game. For now you can
basically imagine it as a stack of tiles. Whenever you load tiles
using VDP_loadTileSet, SGDK puts that tile onto the VRAM stack. In
order to access the tile we want, we have to remember where it was
put. This is done using a simple integer: The first tile to be loaded
is tile 1, the second tile is 2 and so on.

With all this theory, let’s see if you can figure out how we just got
our tile on the screen! Here is the signature of VDP_setTileMapXY.
Take a look at the parameters and try to figure out what we did:

"void VDP_setTileMapXY(VDPPlan plan, u16 tile, u16 x, u16 y);"

When you know the theory behind it it’s quite simple, but here’s a
step-by-step explanation for reference!

    * VDPPlan: This is the background plane we want to draw our tile on.
    We picked BG_B, but we could’ve also used BG_A.
    * tile: This tells the function what tile to use, or in other words:
    This tells the function where in VRAM our desired tile is located.
    Since we’ve only loaded one tile, it’s at position 1.
    * x: This is the x-coordinate (in tiles!) where we want to put our
    tile. We chose 2, which would be 16 in pixels.
    * y: This is the 25th letter of the alphabet, and also the
    y-coordinate of where we want to put our tile.

Phew! That was a lot to take in, but we got to put a tile on the
screen so it was worth it. One final note though: Remember the 1
I told you to ignore in VDP_loadTileSet(bgtile.tileset,1,DMA);?
Well, this 1 actually told SGDK where to put the tile in VRAM.
We could have loaded it onto position 2 or 9 or whatever, but
then we’d also need to change the tile parameter in VDP_setTileMapXY.
Also we’d have created a gap in the VRAM which you usually don’t
want, as it’s confusing. It’s usually best to pile on the tiles
one after the other.

As for the DMA, we’ll take a look at that in another tutorial.

And that finally wraps it up for this part! We’re slowly getting
somewhere. In the next installment we’ll play around with tiles
some more and will also fix the color. Thanks for reading and
until next time!

If you've got problems or questions, join the official SGDK Discord!
It's full of people a lot smarter and skilled than me. Of course
you're also welcome to just hang out and have fun!
#################################################
*/

// ##################################################################################################
// ##################################################################################################
// ##################################################################################################

// #################################################
/*The edges of the play field*/
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;
// #################################################

/*
This will point to our ball sprite once it’s loaded. But before we
load it, we have to lay the groundwork. SGDK uses a sprite engine to
handle sprites, and we have to start it first. In the main function,
add this line after the code that loads our tiles: (inside main)
*/
Sprite* player;
Sprite* ball;

// SCORES:
int score = 0;
char label_score[6] = "SCORE\0";
char str_score[3] = "0";

// PLAYER OBJECTS:
int player_pos_x = 144;
const int player_pos_y = 200; // Seria legal se puder mexer o eixo y, haha
int player_vel_x = 0;
const int player_width = 32;
const int player_height = 8;

// BALL OBJECTS:
int ball_pos_x = 100, ball_pos_y = 100;
int ball_vel_x = 1, ball_vel_y = 1;
int ball_width = 8, ball_height = 8;

// GAME STATE OBJECTS:
game_on = FALSE;

// BONUS EFFECTS:
int flashing = FALSE;
int frames = 0;
int ball_color = 0;

// #################################################
// #################################################
// #################################################

// ####### HUD STUFFS #######

char msg_start[22] = "PRESS START TO BEGIN!\0";
char msg_reset[37] = "GAME OVER! PRESS START TO PLAY AGAIN.";

void showText(char s[])
{
	VDP_drawText(s, 20 - strlen(s) / 2 , 15);
}

void startGame()
{
	score = 0;
	updateScoreDisplay();

	ball_pos_x = 0;
	ball_pos_y = 0;

	ball_vel_x = 1;
	ball_vel_y = 1;

	/*Clear the text from the screen*/
	VDP_clearTextArea(0, 10, 40, 10);

	game_on = TRUE;
}

void endGame()
{
	showText(msg_reset);
	game_on = FALSE;
}

void updateScoreDisplay()
{
    /*
    The function sprintf might look a bit confusing, but it’s doing
    something rather simple. Basically, it takes the int value of
    score and puts it into the variable str_score. In very crude
    terms, it converts an int into a char[] so that we can use it for
    our HUD. It’s a general C function and not unique to SGDK, so you
    can find more info elsewhere.
    */
	sprintf(str_score,"%d",score);

	/*
	This function takes 3 parameters: The x-coordinate (in tiles),
	the y-coordinate (in tiles) and the length (also in tiles) we want
	to clear out. If we didn’t clear the text, all characters would
	remain on screen until they are replaced – if they are ever
	replaced.
	*/
	VDP_clearText(1,2,3);
	VDP_drawText(str_score,1,2);
}

// #################################################
// #################################################
// #################################################

/*
Now it’s time to get interactive. Input in SGDK is handled via a
callback function. What that means is that basically SGDK constantly
keeps an eye on the connected controllers. If a button is pressed,
it will automatically call a function you specify. This function then
needs to check which button was pressed and tell the game what to do.
It’s a pretty simple setup, so let’s write one of those callback
functions! Here is the chunk of code that will make our paddle move
according to our inputs:
*/

void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
	    // Press START to start game, duh!:
	    if(state & BUTTON_START)
	    {
            if(!game_on)
            {
                startGame();
            }
        }

		/*Set player velocity if left or right are pressed;
		 *set velocity to 0 if no direction is pressed */
		if (state & BUTTON_RIGHT)
		{
			player_vel_x = 3;
		}
		else if (state & BUTTON_LEFT)
		{
			player_vel_x = -3;
		}
        else
        {
            if( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) )
            {
                player_vel_x = 0;
            }
		}
	}
}

/*
There’s quite a lot to unpack here, so let’s do it! First, let’s look
at the parameters of our callback.

    * joy: This tells us which joypad was used. It will usually be
    JOY_1 or JOY_2, but SGDK actually supports up to 8 joypads.
    * changed: This tells us whether the state of a button has
    changed over the last frame. If the current state is different
    from the state in the previous frame, this will be 1
    (otherwise 0).
    * state: This will be 1 if the button is currently pressed and 0
    if it isn’t.

So after making sure that the input came from joypad 1 (the only one
we’ll be using here), we check whether the D-Pad was pressed left or
right. If so, we set the player velocity accordingly. However, if
any of the directions has been released, we set the velocity to 0
so that the player stops moving.
*/

// #################################################

void positionPlayer()
{
	/*Add the player's velocity to its position*/
	player_pos_x += player_vel_x;

	/*Keep the player within the bounds of the screen*/
	if(player_pos_x < LEFT_EDGE)
    {
        player_pos_x = LEFT_EDGE;
    }
	if(player_pos_x + player_width > RIGHT_EDGE)
    {
        player_pos_x = RIGHT_EDGE - player_width;
    }

	/*Let the Sprite engine position the sprite*/
	SPR_setPosition(player,player_pos_x,player_pos_y);
}

// #################################################

int sign(int x)
{
    return (x > 0) - (x < 0);
}

// #################################################
// #################################################
// #################################################

void moveBall()
{
    // CHECK BOUNDS:
    if (ball_pos_x < LEFT_EDGE)
    {
        ball_pos_x = LEFT_EDGE;
        ball_vel_x = -ball_vel_x;
    }
    else if (ball_pos_x + ball_width > RIGHT_EDGE)
    {
        ball_pos_x = RIGHT_EDGE - ball_width;
        ball_vel_x = -ball_vel_x;
    }

    if (ball_pos_y < TOP_EDGE)
    {
        ball_pos_y = TOP_EDGE;
        ball_vel_y = -ball_vel_y;
    }
    /*else if (ball_pos_y + ball_height > BOTTOM_EDGE)
    {
        ball_pos_y = BOTTOM_EDGE - ball_height;
        ball_vel_y = -ball_vel_y;
    }*/ // !!!!!!! COMMENTED, SO GAME-OVER HAPPENS WHEN BALL REACHES BOTTOM !!!!!!!
    else if (ball_pos_y + ball_height > BOTTOM_EDGE)
    {
        endGame();
    }

    // CHECK PADDLE COLISION:
    if(ball_pos_x < player_pos_x + player_width && ball_pos_x + ball_width > player_pos_x)
    {
        if(ball_pos_y < player_pos_y + player_height && ball_pos_y + ball_height >= player_pos_y)
        {
            // On collision, invert the velocity
            ball_pos_y = player_pos_y - ball_height - 1;
            ball_vel_y = -ball_vel_y;

            // Increase the score and update the HUD
            score++;
            updateScoreDisplay();

            // Bonus effects:
            flashing = TRUE;

            // Make ball faster on every 10th hit
            if( score % 10 == 0)
            {
                ball_vel_x += sign(ball_vel_x);
                ball_vel_y += sign(ball_vel_y);
            }
        }
    }

    // DO MOVEMENT:
    ball_pos_x += ball_vel_x;
    ball_pos_y += ball_vel_y;

    SPR_setPosition(ball, ball_pos_x, ball_pos_y);
}



// ##################################################################################################
// ##################################################################################################
// ##################################################################################################
// ##################################################################################################
// ##################################################################################################
// ##################################################################################################

int main()
{
    /*
    Now that we have defined our callback function, we have to pass
    it to SGDK. At the beginning of main, before loading any of our
    tiles or sprites, add these two lines:
    */
    JOY_init();
    JOY_setEventHandler( &myJoyHandler );

    /*
    Video Display Processor, which explains the VDP_ prefix. Basically
    every function starting with VDP_ in SGDK does something visual,
    in this case drawing text on the screen.
    */
    VDP_drawText("Hello Mega Drive World!", 8, 4);
    VDP_drawText("I'm ySPHAx, the best person EVER!!", 3, 8);

    SPR_init(0,0,0); // Check comment above 'Sprite* ball;'
    /*
    This will initialize the sprite engine. The parameters specify
    the maximum number of sprites, the VRAM size and the unpack
    buffer size, respectively. 0 tells SGDK to use the defaults,
    which is fine for now. Initializing the sprite engine is
    absolutely necessary if we want to handle sprites, which will be
    the case for pretty much any game!

    With the sprite engine fired up, we can now add the sprite to
    our game. Add this line after the previous one:
    */

    player = SPR_addSprite(&paddle, player_pos_x, player_pos_y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    ball = SPR_addSprite(&imgball,100,100,TILE_ATTR(PAL1,0, FALSE, FALSE));

    /*
    This is a bit of a mouthful, so let’s break it down. The
    signature of the function is
        -- Sprite* SPR_addSprite(const SpriteDefinition * 	spriteDef, s16 x, s16 y, u16 attribute)

        * spriteDef: The address in memory of our compiled sprite
        resource. This is the name we gave our asset in the resource
        file. Don’t forget the & operator!
        * x: The x-coordinate (in pixels!) of our sprite
        * y: The y-coordinate (in pixels!) of our sprite
        * attribute: These are the same as the attributes for tiles.
        Counterintuitively we even use TILE_ATTR to set these.
        Remember: If we don’t set the palette here, the sprite won’t
        look right!

    Note: We’re using the same palette as the background tile we’re
    using, but as you might have noticed, the ball is actually of a
    color not used in the tile. I’ve already added the additional
    colors we need to the tile image, so that’s why this works.
    Otherwise the sprite in the game wouldn’t look like the .png file.

    And another note: As you can see, the coordinates for sprites
    are given in pixels, not tiles. Don’t forget this!

    Okay, now our ball is in the game! …or is it? If you compile now
    you won’t see it. This is because we forgot an important step.
    Remember that sprite engine SGDK uses? We have to tell it to update,
    which is the step that actually displays the sprites at their
    current positions. Put this in the while(1) loop, right before
    the call to SYS_doVBlankProcess();:
        * SPR_update();
    Now compile the game again…and hooray, we have a ball! It’s not doing
    anything though. A ball isn’t really a ball unless it does ball
    things, so let’s give it a kick…using code!
    */

    // #################################################
    // #################################################
    // #################################################

    /*
    VDP_loadTileSet does what it says: It loads a tileset.
    A resource of type IMAGE contains three properties: tileset,
    palette and map. Since we need a tileset, that’s the
    property we’re referencing. The name bgtile is the one we set
    in resource.res. If you used a different one, you’ll have to use
    it here as well! Ignore the 1 and DMA for now.
    */
    VDP_loadTileSet(bgtile.tileset, 1, DMA);

    /*
    But enough theory, how do we get our tile the correct color?
    We’re gonna do it first, then I’ll explain everything. Put this
    after loading the tile, but before putting it on screen:
    */
    VDP_setPalette(PAL1, bgtile.palette->data);
    /*
    The line VDP_setPalette(PAL1, bgtile.palette->data); extracts the
    palette data from our tile (it’s a property called data of a struct
    called palette, which is saved in our bgtile) and sets it as PAL1

    PAL1 is not actually the first palette the Mega Drive stores.
    In fact, you can choose between PAL0 – PAL3, there is no PAL4!
    Now if you’re wondering why we’re using PAL1 here and not PAL0:
    The reason has to do with our background color
    */

    // #################################################
    // #################################################
    // #################################################

    // Okay, with the tile loaded, let’s throw it on the screen.
    // This line of code makes that happen:
    // ####### VDP_setTileMapXY(BG_B, 1, 2, 2); // (Note: If you’re using SGDK 1.41, replace BG_B with PLAN_B) #######
    // CHECK ABOVE THE EXPLANATION OF PLANES!
    /*
    This gets the palette from our bgtile (remember, we compiled
    it as an IMAGE so it contains palette data) and adds it to PAL1.
    Now we just need to tell SGDK to use PAL1 for our tile.
    Modify your setTileXY call so it looks like this:
    */
    // ####### VDP_setTileMapXY(BG_B,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,1),2,2); #######
    /*
    If you look closely, you’ll notice that we basically just replaced
    the 1 (pointing to a tile in VRAM) with a load of other stuff.
    So what does this stuff do? Here’s the signature:
        * TILE_ATTR_FULL(pal,prio,flipV,flipH,index)

    Basically, TILE_ATTR_FULL is a way to more closely define how a
    tile is displayed. Let’s break it down:

        * pal: What palette you want the tile to use
        * prio: Remember background planes? This setting allows you to
        override the draw order, meaning that a tile on BG_B can be
        drawn atop of BG_A if this is set to TRUE.
        * flipV: When set to TRUE, this will flip the tile vertically.
        * flipH: When set to TRUE, this will flip the tile horizontally.
        *index: You know this one: The index of the tile in VRAM.

    So instead of just telling SGDK “We want to use tile 1” we can
    tell it to “use tile 1 with these properties…”.
    */

    // #################################################
    // #################################################
    // #################################################

    /*
    Alright, now we have the color sorted out. Time to draw our background!
    But setting each tile manually using VDP_setTileMapXY wouldn’t be
    exactly thrilling. Of course we could write a loop, but SGDK offers an
    easier solution in the form of a new function!
    */

    VDP_fillTileMapRect(BG_B, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), 0, 0, 40, 30);

    /*
    Can you guess what it does? Feel free to experiment a bit before
    reading on!

    As the name implies, this function draws and fills a rectangle with tiles.
    The parameters are pretty self-explanatory too, but let’s break them down
    anyway:

        * plan: The background plane on which to draw your tiles.
        * tile: The tile index to use. Remember that you can use TILE_ATTR_FULL
        to set more parameters!
        * x: The x-coordinate (in tiles) of the upper-left corner of the rectangle.
        * y: The y-coordinate (in tiles) of the upper-left corner of the rectangle.
        * w: The width of the rectangle (in tiles).
        * h: The height of the rectangle (in tiles).

    So how would you use this function to draw a rectangle to fill the screen
    with our tile, using the correct palette? By replacing our call to
    VDP_setTileMapXY with this: (non-commented function above)
    */

    // #################################################
    // #################################################
    // #################################################

    // add this after loading the tiles:
    VDP_setTextPlane(BG_A);
    VDP_drawText(label_score,1,1);
    updateScoreDisplay();
    showText(msg_start);

    // BONUS EFFECTS:
    ball_color = VDP_getPaletteColor(22);

    // #################################################
    // #################################################
    // #################################################

    while(1)
    {
        /*
        everything you put into the while(1){ ... } loop is executed
        as quickly as possible, which might or might not sync up with
        your display. In order to avoid display issues, you have to
        tell your Mega Drive to wait until the screen display has been
        fully updated before it starts processing more stuff. And
        that’s what SYS_doVBlankProcess() does!
        */
        if(game_on == TRUE)
        {
            moveBall();
            positionPlayer();

            // #####################
            // Handle the flashing of the ball
            if( flashing == TRUE )
            {
                frames++;
                if( frames % 4 == 0 )
                {
                    /*
                    VDP_setPaletteColor is obviously the counterpart to
                    VDP_getPaletteColor and sets the color at a certain
                    index. We’re using this function twice: To set the
                    color at 22 to ball_color, and to set it to white.
                    RGB24_TO_VDPCOLOR is simply a helper function that
                    converts a hex value to the format that the VDP uses.
                    This makes it more convenient for us.
                    */
                    VDP_setPaletteColor(22, ball_color);
                }
                else if( frames % 2 == 0)
                {
                    VDP_setPaletteColor(22, RGB24_TO_VDPCOLOR(0xffffff));
                }

                // Stop flashing
                if(frames > 30)
                {
                    flashing = FALSE;
                    frames = 0;
                    VDP_setPaletteColor(22,ball_color);
                }
            }
            // #####################
        }

        SPR_update(); // READ COMMENTS AROUND SPRITE LOADER TO UNDERSTAND
        SYS_doVBlankProcess();
    }
    return (0);
}
