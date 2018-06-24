#define BASE_DY_PLAYER 0.07
#define BASE_DX_PLAYER 0.2
#define BASE_JUMP_DY 0.15
#define ACCEL_PLAYER 2
#define TIMESTEP_PLAYER 0.001
#include "platforms.h"
#include "boss_sprite.h"
#include <ncurses.h>

typedef struct player_id{
	sprite_id player_sprite;
	bool on_platform;
	int last_platform_hit;
	int score;
	bool update_score;
} player_id;

// ----------------------------------------------------------------
// Forward declaration of functions
// ----------------------------------------------------------------
void setup_player();
void draw_player( player_id player );
bool process_player( player_id* player, int key, int level, platform* platforms, int no_plats, boss_id boss );
void process_key_player( player_id player, int key, int level, platform* plat, int no_plats );
void process_key_LVL1( player_id* player, int key, platform* plat, int no_plats );
void process_key_LVL2( player_id* player, int key, platform* plat, int no_plats );
void player_fall( player_id player, int level );
void player_fallNG( player_id* player );
void player_fallG( player_id* player);
int hit_top_platform( platform* plat, int no_plats, player_id* player );
bool hit_side_platform( platform* plat, int no_plats, player_id* player );
bool hit_boss( boss_id* boss, player_id* player );
double dist_from_boss( double xb, double yb, double rb, double x, double y );

// ----------------------------------------------------------------
// Player functions
// ----------------------------------------------------------------

/*
 *	Draws the player.
 */
void draw_player( player_id player ) {
	sprite_draw( player.player_sprite );
}

/*
 * Processes the player. Returns a boolean value indicating whether the player has moved or not.
 */
bool process_player( player_id* player, int key, int level, platform* platforms, int no_plats, boss_id boss ) {
	if ( player->player_sprite->is_visible ){
		bool player_moved = false;
		
		double x0 = round( player->player_sprite->x ); // remembers original position
		double y0 = round( player->player_sprite->y );
		
		process_key_player( *player, key, level , platforms, no_plats ); // moves the player based on key input
		
		player_fall( *player, level );

		if ( player->player_sprite->y >= screen_height() - 5 // if player reaches the bottom of the screen
			|| player->player_sprite->y < 2 ) { // if player hits the top of the screen
			player->player_sprite->is_visible = false;
		}
		
		int platform_hit = hit_top_platform( platforms, no_plats, player ); // detects which platform the player has hit (if any)
		
		if ( platform_hit >= 0 ){ // if player has hit a platform
			if ( platforms[ platform_hit ].safe ){
				player->player_sprite->y = platforms[platform_hit].y - 3; // match platform's behavior
				player->player_sprite->dy = 0;
				player->on_platform = true;
				
				if ( player->last_platform_hit != platform_hit ){
					player->score++; // increment score if player is not on last platform hit
					player->last_platform_hit = platform_hit;
				}
			} else{
				player->player_sprite->is_visible = false; // player dies
			}
		} else{
			player->on_platform = false; // otherwise player is not on platform
		}
		
		bool boss_hit = hit_boss( &boss, player );
		
		if ( boss_hit ){ // if player has hit boss
			player->player_sprite->is_visible = false; // player dies
		}
					
		player_moved = player_moved || round( player->player_sprite->x ) != x0 
									|| round( player->player_sprite->y ) != y0;
		// platform will have moved if the new rounded positions are not the same as the original positions.
		return player_moved;
	} else {
		return false;
	}
 }

 
/*
 * Processes key presses from player
 */ 
void process_key_player( player_id player, int key, int level, platform* plat, int no_plats ){
	if( level == 1 ){
		process_key_LVL1( &player, key, plat, no_plats );
	} else {
		process_key_LVL2( &player, key, plat, no_plats );
	}
}

/*
 * Processes keys for level 1
 */
void process_key_LVL1( player_id* player, int key, platform* plat, int no_plats ){
	if ( key == KEY_LEFT && player->on_platform && !hit_side_platform( plat, no_plats, player )){
		player->player_sprite->x--;
	} else if ( key == KEY_RIGHT && player->on_platform && !hit_side_platform( plat, no_plats, player )){
		player->player_sprite->x++;
	}
	
	// makes sure sprite is still in window
	while ( player->player_sprite->y < 0 ) player->player_sprite->y++;
	while( player->player_sprite->x < 0 ) player->player_sprite->x++;
	while( player->player_sprite->x > screen_width() - 1 ) player->player_sprite->x--;
}

/*
 * Processes keys for level 2
 */
void process_key_LVL2( player_id* player, int key, platform* plat, int no_plats ){
	if( key == KEY_UP && player->on_platform ){ // if player has not already jumped
		player->player_sprite->y -= 0.5; // moves the player slightly off the platform
		player->player_sprite->dy = -BASE_JUMP_DY; // gives the player a vertical velocity
	} else if ( key == KEY_DOWN && player->on_platform ){ // if player is already on platform
		player->player_sprite->dx = 0;
	} else if ( key == KEY_LEFT && player->on_platform ){
		player->player_sprite->dx = -BASE_DX_PLAYER; // gives the player horizontal velocity
	} else if ( key == KEY_RIGHT && player->on_platform ){
		player->player_sprite->dx = BASE_DX_PLAYER; 
	}
	
	if( hit_side_platform( plat, no_plats, player )){
		player->player_sprite->dx = 0; // stops sprite from moving if player hits side of platform
	}
	
	sprite_step( player->player_sprite ); // updates sprite position
	
	// makes sure sprite is still in window
	while ( player->player_sprite->y < 0 ){
		player->player_sprite->y++;
	} 
	while( player->player_sprite->x < 0 ){
		player->player_sprite->x++;
		player->player_sprite->dx = 0;
	} 
	while( player->player_sprite->x > screen_width() - 1 ){
		player->player_sprite->x--;
		player->player_sprite->dx = 0;
	} 
}

/*
 * Makes player fall
 */
void player_fall( player_id player, int level ){
	if ( level == 1 ){
		player_fallNG( &player );
	} else {
		player_fallG( &player );
	}
}
 
/*
 * Makes player fall with no gravity
 */
 void player_fallNG( player_id* player ){
	 player->player_sprite->dy = BASE_DY_PLAYER;
	 sprite_step( player->player_sprite );
 }

/*
 * Makes player fall under the influence of gravity
 * (don't drink and fall kids)
 */
 void player_fallG( player_id* player ){
	player->player_sprite->y += player->player_sprite->dy * TIMESTEP_PLAYER;
	player->player_sprite->dy += ACCEL_PLAYER * TIMESTEP_PLAYER;
 }
 
/*
 * Method for determining if the player hit a platform.
 * Returns the platform index of hit platform.
 * returns -1 otherwise.
 */ 
int hit_top_platform( platform* plat, int no_plats, player_id* player ){
	for ( int i = 0; i < no_plats; i++ ){
		if ( player->player_sprite->y > ( plat[i].y - 3 ) && player->player_sprite->y < ( plat[i].y + 1 ) 
		&& player->player_sprite->x <= ( plat[i].x + plat[i].width ) && player->player_sprite->x >= plat[i].x ){
				return i;
			}
	}
	
	return -1;
 }
 
/*
 * Returns true if player has hit the side of a platform
 */
bool hit_side_platform( platform* plat, int no_plats, player_id* player ){
	for ( int i = 0; i < no_plats; i++ ){
		if ( player->player_sprite->y > ( plat[i].y - 2 ) && player->player_sprite->y < ( plat[i].y + 1 ) 
			&& player->player_sprite->x <= ( plat[i].x + plat[i].width + 1 ) && player->player_sprite->x >= plat[i].x - 1 ){
				return true;
			}
	}
	
	return false;
}

/*
 * Returns true if player has hit the boss
 */
bool hit_boss( boss_id* boss, player_id* player ){	
	double distance = dist_from_boss( boss->sprite_boss->x, boss->sprite_boss->y, boss->radius,
									player->player_sprite->x, player->player_sprite->y );
	bool hit = distance <= boss->radius;
	
	return hit;
}

/*
 * Calculates distance between two sets of co-ordinates
 */
double dist_from_boss( double xb, double yb, double rb, double x, double y ){
	double dx = xb + rb - x;
	double dy = yb + rb - y;
	
	double dist = sqrt((dx*dx) + (dy*dy));
	return dist;
}