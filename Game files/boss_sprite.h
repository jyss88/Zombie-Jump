typedef struct boss_id{
	sprite_id sprite_boss;
	int radius; // radius of boss sprite
	char* bitmap_up; // bitmap of boss sprite
	char* bitmap_down;
	char* bitmap_left;
	char* bitmap_right;
	int appear_delay; // delay for appearance
	int appear_delay_count;
	
	int turn_delay; // delay for turn
	int turn_delay_count; // count for turn delay
	int turn_total;
} boss_id;

// ----------------------------------------------------------------
// Forward declaration of functions
// ----------------------------------------------------------------
void setup_boss();
void draw_boss( boss_id boss );
char* create_bitmap( int radius, char character );
void create_directional_bitmaps( boss_id* boss );
void clear_bitmaps( boss_id* boss );
double calc_dist( double x_c, double y_c, double row, double column );
bool process_boss( boss_id* boss, int level );
void move_boss( boss_id* boss );
void fixed_sprite_turn( sprite_id sprite, double degrees );

#define M_PI 3.14159265359

// ----------------------------------------------------------------
// Boss functions
// ----------------------------------------------------------------

/*
 * Draws the boss
 */
void draw_boss( boss_id boss ){
	sprite_draw( boss.sprite_boss );
}

/*
 * Creates a circular bitmap with the specified character
 */
char* create_bitmap( int radius, char character ){
	int diameter = 2* radius;
	int area = diameter * diameter;
	char* bitmap = malloc( area * sizeof(char) ); // allocates memory for bitmap
	double center = radius;
	int row, column, distance;
	
	for ( int i = 0; i < area; i++ ){ // loops through each element
		row = i % diameter; // convert to separate row and index column
		column = i / diameter;
		
		distance = calc_dist( center, center, row, column ); // calculates array's distance from center
		
		if ( distance < round(radius) ){
			bitmap[i] = character; // fills place with character
		} else {
			bitmap[i] = ' '; // fills character with empty space, otherwise.
		}
	} 
	
	return bitmap;
}

/*
 * Creates directional bitmaps
 */
void create_directional_bitmaps( boss_id* boss ){
	boss->bitmap_up = create_bitmap( boss->radius, '^');
	boss->bitmap_down = create_bitmap( boss->radius, 'v');
	boss->bitmap_left = create_bitmap( boss->radius, '<');
	boss->bitmap_right = create_bitmap( boss->radius, '>');
}

/*
 * Frees memory assigned to bitmaps
 */
void clear_bitmaps( boss_id* boss ){
	free( boss->bitmap_up );
	free( boss->bitmap_down );
	free( boss->bitmap_left );
	free( boss->bitmap_right );
}

/*
 * Changes the bitmap based on direction
 */
void change_bitmap( boss_id* boss ){
	if ( abs(boss->sprite_boss->dx) > abs(boss->sprite_boss->dy) // if dx is bigger than dy
		&& boss->sprite_boss->dx >= 0 ){ // and is bigger than zero
		boss->sprite_boss->bitmap = boss->bitmap_right; // bitmap is right
	} else if (  abs(boss->sprite_boss->dx) > abs(boss->sprite_boss->dy) // if dx is bigger than dy
		&& boss->sprite_boss->dx < 0 ){ // and is less than zero
		boss->sprite_boss->bitmap = boss->bitmap_left; // bitmap is left
	} else if (  abs(boss->sprite_boss->dx) < abs(boss->sprite_boss->dy) // if dx is less than dy
		&& boss->sprite_boss->dx >= 0 ){ // and is greater than zero
		boss->sprite_boss->bitmap = boss->bitmap_up; // bitmap is up
	} else if (  abs(boss->sprite_boss->dx) < abs(boss->sprite_boss->dy) // if dx is less than dy
		&& boss->sprite_boss->dx < 0 ){ // and is less than zero
		boss->sprite_boss->bitmap = boss->bitmap_down; // bitmap is down
	} 
}

/*
 * Calculates the absolute distance between two points.
 */
double calc_dist( double x_c, double y_c, double row, double column ){
	int dx = x_c - column;
	int dy = y_c - row;
	double distance = sqrt( ( dx*dx) + (dy*dy) );
	return distance;
}
/*
 * Tries to move boss. Returns false otherwise.
 */
bool process_boss( boss_id* boss, int level ){
	bool boss_moved = false;
	
	if( boss->sprite_boss->is_visible && level == 3 ){ // don't move boss unless on level 3
		
		double x0 = round( boss->sprite_boss->x );
		double y0 = round( boss->sprite_boss->y );
		
		move_boss( boss );
		
		boss_moved = round( boss->sprite_boss->x ) != x0
					|| round ( boss->sprite_boss->y ) != y0;
	} 
	
	return boss_moved;
}

/*
 * Moves boss sprite. Turns boss in a full circle after a delay.
 */
void move_boss( boss_id* boss ){
	if ( boss->appear_delay_count >= boss->appear_delay ){
		if(( boss->turn_delay_count >= boss->turn_delay ) && ( boss->turn_total < 360 )){ // if delay has expired, and boss has not turned full 360 degrees
		fixed_sprite_turn( boss->sprite_boss, 2 ); // turns sprite
		boss->turn_total ++;
		} else{
			boss->turn_delay_count++; // increment turn delay count
		}
	
		sprite_step( boss->sprite_boss ); // moves boss sprite
	} else{
		boss->appear_delay_count++; // increment delay count
	}
	
	change_bitmap( boss );
	
}

/*
*	fixed_sprite_turn:
*
*	Sets the internally stored direction. That is, the step that is taken when
*	the sprite moves forward or backward.
*
*	The new direction is relative to the old one. If the old direction is 0,0 then
*	the new one will also be 0,0.
*
*	Input:
*		sprite: The ID of a sprite.
*		degrees: The angle to turn.
*/
void fixed_sprite_turn( sprite_id sprite, double degrees ) {
	double radians = degrees * M_PI / 180;
	double s = sin( radians );
	double c = cos( radians );
	double dx = c * sprite->dx + s * sprite->dy;
	double dy = -s * sprite->dx + c * sprite->dy;
	sprite->dx = dx;
	sprite->dy = dy;
}