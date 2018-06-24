#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"
#include "cab202_sprites.h"
#include "player.h"

// The largest visible horizontal location.
int max_x;

// The vertical location of the turtle, relative to the terminal window.
int max_y;

// Game-in-progress status: true if and only if the game is finished.
bool game_over;

// Current level
int level = 1;
#define MAX_LEVEL 3

// Remaining lives left in game.
int lives = 3;

// Current speed
int speed = 100;
int desired_speed = 100;
bool speed_changing = false; // value indicates whether speed is changing or not
#define SLOW 25
#define NORMAL 100
#define FAST 400

// Start time, in seconds
double start_time;

// Elapsed minutes
int minutes = 0;

// Elapsed seconds;
int seconds = 0;

// Start time, in seconds
double start_time;

// Player sprite
player_id player;

// Platform sprite
#define NO_PLATFORMS 25
platform platforms[NO_PLATFORMS];

// boss sprite
boss_id boss;

//
#define LOOP_STEP 25
timer_id global_timer;

// ----------------------------------------------------------------
// Forward declarations of functions
// ----------------------------------------------------------------
void setup();
void event_loop();
void process_key( int key );
void draw_all();
void cleanup();
void wait_to_begin();
void pause_for_exit();

// Menu elements
void draw_score();
void draw_lives();
void lose_life();
void calculate_elapsed_time();
void draw_time();
void reset();
void restart();
void ask_to_restart();
void change_level();
void draw_level();
void change_desired_speed();
void change_speed();
void draw_speed();
void draw_border();

// ----------------------------------------------------------------
// main function
// ----------------------------------------------------------------

int main( void ) {
	srand( time( NULL ) ); 
	setup();
	event_loop();
	cleanup();
	return 0;
}

/*
 *	Set up the game. Sets the terminal to curses mode and places the player, platform, and boss.
 */

void setup() {
	setup_screen();
	max_x = screen_width() - 1;
	max_y = screen_height() - 1;
	setup_player( player );
	game_over = false;
	player.score = 0;
	setup_platform( platforms, NO_PLATFORMS, level );
	setup_boss();
	global_timer = create_timer( LOOP_STEP );
}

// ----------------------------------------------------------------
// Player Functions
// ----------------------------------------------------------------


/*
 * Sets ups player
 */
void setup_player(){
	static char * bitmap = 
	"0"
	"|"
	"M";
	
	if ( player.player_sprite != NULL ){ // destroys sprite if it already has been initialized
		sprite_destroy( player.player_sprite );
	}
	player.player_sprite = sprite_create( ( screen_width() - 1 ) / 2, screen_height() - 7, 1, 3, bitmap);
	player.on_platform = true;
	player.last_platform_hit = 0;
	player.score = 0;
	player.update_score = false;
}

/*
 * Sets up boss sprite. Clears previous boss sprites and bitmaps, if they exist.
 */
void setup_boss(){
	
	if ( boss.sprite_boss != NULL ){ // clears memory from sprite and bitmaps
		sprite_destroy( boss.sprite_boss );
		clear_bitmaps( &boss );
	}
	boss.radius = rand_between( 5, 15 );
	
	create_directional_bitmaps( &boss );
	
	boss.sprite_boss = sprite_create( -2 * boss.radius, screen_height(), boss.radius * 2, 
						boss.radius * 2, boss.bitmap_right );
	boss.sprite_boss->dx = 0.1;
	boss.sprite_boss->dy = ( -rand_between(1, 20) * 0.005 ); // boss moves in random diagonal direction
	
	boss.appear_delay = rand_between(130, 200);
	boss.appear_delay_count = 0;
	
	boss.turn_delay = boss.appear_delay + rand_between( 140, 300 );
	boss.turn_delay_count = 0;
	boss.turn_total = 0;
}

/*
 *	Processes keyboard timer events to progress game.
 */
void event_loop() {
	draw_all();
	wait_to_begin();

	while ( !game_over ) { // while game is not over, or lives is not at zero
		int key = get_char();
		bool player_changed = false;
		bool platform_changed = false;
		bool boss_changed = false;
		
		process_key( key );
		
		if ( timer_expired( global_timer ) ){
			player_changed = process_player( &player, key, level, platforms, NO_PLATFORMS, boss);
			platform_changed = process_platform( platforms, NO_PLATFORMS, level, speed );
			boss_changed = process_boss( &boss, level );
		}
		
		if ( player_changed || platform_changed || boss_changed ){
			draw_all();
		}
		
		lose_life( lives ); // check if you need to lose a life
		change_speed(); // changes speed if required
		
		timer_pause( 10 ); // gives cpu some time to catch it's breath
		 
	}
	
	pause_for_exit();
}

/*
 * Processes key for changing, or resetting level, quitting the game, or changing speed.
 */
void process_key( int key ){
	if ( key == 'q' ){
		game_over = true;
	} else if ( key == 'r' ){
		reset();
	} else if ( key == 'l' ){
		change_level();
	} else if ( ( key == '1' || key == '2' || key == '3' )
				&& !speed_changing ){ // only processes if speed is not changing
		change_desired_speed( key );
	}
}

/*
 * Resets the game. Returns player to starting position, and rescrambles platforms.
 * Lives and level remains the same.
 */
void reset(){
	clear_screen(); // clears screen
	setup(); // resets stuff
	draw_all(); // redraws stuff
	wait_to_begin();
}


/*
 * Asks player if they would like to restart. 
 * Waits for user input. If 'r', restarts game. If 'q', user quits.
 */
void ask_to_restart(){
	draw_formatted( 1, max_y / 2, "Game over! Press 'q' to quit, or any other key to restart." );
	
	int key = wait_char();
	
	if ( key == 'q' ){
		game_over = true;
	} else {
		lives = 3;
		level = 1;
		reset();
	}
}
 
 /*
 *	Redraws the screen
 */
void draw_all() {
	clear_screen();
	draw_boss( boss );
	draw_platforms( platforms, NO_PLATFORMS ); 
	draw_player( player ); 
	draw_score();
	draw_lives();
	draw_time();
	draw_level();
	draw_speed();
	draw_border();
	show_screen();
} 

/*
 *	Restore the terminal to normal mode.
 */
void cleanup() {
	cleanup_screen();
}

/*
 * Waits for player input before beginning
 */
void wait_to_begin(){
	draw_formatted(0, 0, "Please press any key to begin");
	wait_char();
	start_time = get_current_time();
}

/*
 *	Displays a messsage and waits for a keypress.
 */
void pause_for_exit() {
	draw_line( 0, max_y, max_x, max_y, ' ' );
	draw_string( 0, screen_height() - 1, "Press any key to exit..." );
	wait_char();
}

// ----------------------------------------------------------------
// Menu elements
// ----------------------------------------------------------------

/*
 *	Draws score in bottom left corner
 */
void draw_score() {
	draw_line( 0, max_y, max_x, max_y, ' ' );
	draw_formatted( 0, max_y, "Score: %d", player.score );
}

/*
 * Displays remaining lives.
 */
 void draw_lives() {
	 draw_line( 0, 0, max_x, 0, ' ' );
	 draw_formatted( 0, 0, "Remaining lives: %d", lives);
}

/*
 * Decrements lives, if player is not visible
 */
void lose_life(){
	if ( !player.player_sprite->is_visible && lives >= 0){
		lives --;
		draw_formatted(0, 0, "You have %d lives remaining! Press any key to reset.", lives );
		wait_char();
		reset();
	} else if ( lives < 1 ){
		ask_to_restart();
	}
 }

/*
 * Calculates and updats elapsed time in minutes and seconds
 */
void calculate_elapsed_time(){
	int elapsed_time = get_current_time() - start_time;
	minutes = round( elapsed_time ) / 60;
	seconds = elapsed_time % 60;
}

/*
 * updates and displays elapsed time 
 */
void draw_time(){
	calculate_elapsed_time();
	draw_formatted(58, 0, "Time: %d:%d", minutes, seconds);
}

/*
 * Changes level.
 */
 void change_level(){
	level = ( level % MAX_LEVEL ) + 1; // increments level (loops around when level gets to 3)
	reset();
 }
 
 
 /*
  * Draws level indicatior
  */
 void draw_level(){
	 draw_line( 0, max_y - 1, max_x, max_y - 1, ' ' );
	 draw_formatted(0, max_y - 1, "Level %d", level);
 }
 
/*
 * Changes desired speed
 */
void change_desired_speed( int key ){
	switch( key ) {
		case '1':
			desired_speed = SLOW;
			break;
			
		case '2':
			desired_speed = NORMAL;
			break;
			
		case '3':
			desired_speed = FAST;
			break;
	}
}

/*
 * Changes speed to match desired speed.
 * Does so in a smooth manner
 */
void change_speed(){	
	if ( speed != desired_speed ){
		speed_changing = true;
		if ( speed > desired_speed ){
			speed --; // decrement speed
		} else{
			speed ++; // increment speed
		}
	} else {
		speed_changing = false;
	}
}

/*
 * Draws speed in the bottom right corner
 */
void draw_speed(){
	if ( level == 3 ){ // only will activate if on level 3
		switch( speed) {
			case SLOW: 
				draw_string( max_x - 12, max_y - 1, "Speed: SLOW" );
				break;
			
			case NORMAL: 
				draw_string( max_x - 12, max_y - 1, "Speed: NORM" );
				break;
				
			case FAST: 
				draw_string( max_x - 12, max_y - 1, "Speed: FAST" );
				break;
			
			default:
				draw_string( max_x - 12, max_y - 1, "Changing..." );
				break;
		}
	}
}
 
 /*
  * Draws border at top and bottom of screen.
  */
void draw_border(){
	draw_line( 0, 1, max_x, 1, '-' );
	draw_line( 0, max_y - 2, max_x, max_y - 2, '-' );
}